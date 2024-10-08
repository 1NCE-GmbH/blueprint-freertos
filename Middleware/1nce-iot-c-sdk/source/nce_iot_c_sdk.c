/*
 * MIT License
 *
 * Copyright (c) 2022 1NCE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file nce_iot_c_sdk.c
 * @brief Implements the user-facing functions in nce_iot_c_sdk.h.
 * @author  Hatim Jamali & Mohamed Abdelmaksoud
 * @date 01 Mar 2022
 */

#include "nce_iot_c_sdk.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "log_interface.h"

#ifdef __ZEPHYR__
LOG_MODULE_REGISTER( NCE_SDK, CONFIG_NCE_SDK_LOG_LEVEL );
#endif /* ifdef  __ZEPHYR__ */

#ifdef NCE_DEVICE_AUTHENTICATOR

static uint16_t message_id = 1000;

/**
 * @brief Create Incremental Message ID for CoAP onboarding
 *
 */
static uint16_t _getNextMessageID()
{
    message_id++;
    return message_id;
}

/**
 * @brief Process the DTLS credential to match the requirement
 *
 * @param[in] packet: the target packet to be processed.
 * @param[in] nceKey: the new DTLS credential required.
 *
 */
static int _get_psk( char * packet,
                     DtlsKey_t * nceKey )
{
    int status = NCE_SDK_PARSING_ERROR;
    char * resp = strstr( packet, "89" );

    if( resp == NULL )
    {
        NceOSLogError( "ERROR: Packet can't be NULL\n" );
        return status;
    }
    else
    {
        char * p = strtok( resp, "," );

        if( p == NULL )
        {
            NceOSLogError( "ERROR: Parsing Error\n" );
            return status;
        }
        else
        {
            strcpy( nceKey->PskIdentity, p );
        }

        p = strtok( NULL, "," );

        if( p == NULL )
        {
            NceOSLogError( "ERROR: Parsing Error\n" );
            return status;
        }
        else
        {
            strcpy( nceKey->Psk, p );
        }

        NceOSLogInfo( "DTLS Credentials Recieved.\n" );
        return NCE_SDK_SUCCESS;
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Connect to 1NCE server with reconnection retries.
 *
 * If connection fails, retry is attempted after a timeout.
 * 1NCE endpoint require DTLS Connection
 *
 * @param[in] osNetwork: udp interface object.
 *
 * @return The status of the final connection attempt.
 */
static int _os_udp_connect( os_network_ops_t * osNetwork )
{
    int status = NCE_SDK_CONNECT_ERROR;
    int attempts = 1;

    if( osNetwork == NULL )
    {
        NceOSLogError( "ERROR\n" );
        return status;
    }
    else
    {
        if( osNetwork->os_socket == NULL )
        {
            NceOSLogError( "socket can't be NULL\n" );
            return status;
        }
        else
        {
            NceOSLogInfo( "Connecting to 1NCE onboarding Service\n" );

            do
            {
                NceOSLogInfo( "connect to osNetwork" );
                status = osNetwork->nce_os_udp_connect( osNetwork->os_socket, NceOnboard );
                attempts++;
            } while( status != 0 && attempts < NCE_SDK_ATTEMPTS );
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

/**
 * @brief Send CoAP GET request to 1NCE.
 *
 *
 * @param[in] osNetwork: udp interface object.
 * @param[in] pBuffer: Buffer to be used by the interface.
 * @param[in] bufferSize: allocated size for the interface buffer.
 *
 * @return The amount of bytes received.
 */
static int _os_coap_onboard( os_network_ops_t * osNetwork,
                             void * pBuffer,
                             size_t bufferSize )
{
    int status = NCE_SDK_SEND_ERROR;

    /*  0x50 : 01 -> CoAP Version
     *          01 -> Non-Confirmable CoAP message
     *          0000 -> 0 Token length
     *  0x01 : GET REQUEST
     */
    static const unsigned char coap_header[] = { 0x50, 0x01 };

    /*  0x3d : 0011 -> Opt. delta (3)
     *          1101 -> Opt. Length (13)
     *  0x03 : Opt. Length Extended (3) for a total length of 16 "coap.os.1nce.com"
     */
    static const unsigned char uri_host_option[] = { 0x3d, 0x03 };

    /*0x89 : 1000 -> Opt. delta (8)
     *          1001 -> Opt. Length (9) "bootstrap"
     */
    static const unsigned char uri_path_option[] = { 0x89 };

    /* Create Message ID */
    uint16_t message_id = _getNextMessageID();
    char message_id_str[ 2 ];

    memcpy( message_id_str, &message_id, 2 );


    NceOSLogInfo( "Start 1NCE device onboarding.\n" );
    memset( pBuffer, '\0', bufferSize * sizeof( char ) );
    sprintf( pBuffer, "%.2s%.2s%.2s%s%.1sbootstrap", coap_header, message_id_str, uri_host_option, NceOnboard.host, uri_path_option );
    NceOSLogInfo( "Send Device Authenticator request.\n" );
    status = osNetwork->nce_os_udp_send( osNetwork->os_socket, pBuffer, strlen( pBuffer ) );

    if( status < 0 )
    {
        NceOSLogError( "Failed to send Device Authenticator request.\n" );
        status = NCE_SDK_SEND_ERROR;
    }
    else
    {
        memset( pBuffer, '\0', bufferSize * sizeof( char ) );
        status = osNetwork->nce_os_udp_recv( osNetwork->os_socket, pBuffer, bufferSize );

        if( status < 0 )
        {
            NceOSLogError( "Failed to receive Device credential.\n" );
            status = NCE_SDK_RECEIVE_ERROR;
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

int os_auth( os_network_ops_t * osNetwork,
             DtlsKey_t * nceKey )
{
    int status = NCE_SDK_CONNECT_ERROR;
    int attempts = 1;
    char packet[ 150 ];

    status = _os_udp_connect( osNetwork );

    if( status < 0 )
    {
        NceOSLogError( "Failed to Connect to 1NCE Endpoint\n" );
        return status;
    }
    else
    {
        do
        {
            status = _os_coap_onboard( osNetwork, packet, sizeof( packet ) );
            attempts++;
        } while( status <= 0 && attempts < NCE_SDK_ATTEMPTS );
    }

    if( status > 0 )
    {
        status = _get_psk( packet, nceKey );

        if( status < 0 )
        {
            NceOSLogError( "Failed to parse response.\n" );
            return status;
        }
    }

    status = osNetwork->nce_os_udp_disconnect( osNetwork->os_socket );

    if( status < 0 )
    {
        NceOSLogError( "Failed to close socket.\n" );
        return status;
    }

    return status;
}

#endif /* ifdef NCE_DEVICE_AUTHENTICATOR */

/*-----------------------------------------------------------*/

#ifdef NCE_ENERGY_SAVER

int os_energy_save( char * packet,
                    int selector,
                    int num_args,
                    ... )
{
    int location = 1;
    va_list ap;
    Element2byte_gen_t e;
    int i;

    memcpy( packet, &selector, 1 );
    va_start( ap, num_args );

    for( i = 0; i < num_args; i++ )
    {
        e = va_arg( ap, Element2byte_gen_t );

        if( ( e.type == E_INTEGER ) && ( e.value.i != *( e.value.bytes ) ) )
        {
            NceOSLogError( "Conversion Error, Check template length.\n" );
            return NCE_SDK_BINARY_PAYLOAD_ERROR;
        }

        memcpy( packet + location, &e.value.bytes, e.template_length );
        location += e.template_length;
        NceOSLogDebug( "location %d.\n", location );
    }

    va_end( ap );

    if( location == 1 )
    {
        NceOSLogError( "Conversion Error.\n" );
        return NCE_SDK_BINARY_PAYLOAD_ERROR;
    }

    return location;
}

#endif /* ifdef NCE_ENERGY_SAVER */
