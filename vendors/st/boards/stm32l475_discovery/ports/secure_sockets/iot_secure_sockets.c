/*
 * FreeRTOS Secure Sockets for STM32L4 Discovery kit IoT node V1.0.0 Beta 4
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/**
 * @file iot_secure_sockets.c
 * @brief WiFi and Secure Socket interface implementation for ST board.
 */

#include <stdbool.h>

/* Define _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE to prevent secure sockets functions
 * from redefining in iot_secure_sockets_wrapper_metrics.h */
//#define _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "iot_secure_sockets.h"

/* Modem includes */
#include "com_sockets_ip_modem.h"
#include "com_sockets_net_compat.h"
#include "com_sockets_err_compat.h"
#include "cellular_service.h"


/* WiFi driver includes.
#include "es_wifi.h"
#include "es_wifi_io.h" */

/* Socket and WiFi interface includes.
#include "iot_wifi.h"

/* WiFi configuration includes.
#include "aws_wifi_config.h" */



/* Credentials includes. */
#include "aws_clientcredential.h"
#include "iot_default_root_certificates.h"

#undef _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE

/**
 * @brief A Flag to indicate whether or not a socket is
 * secure i.e. it uses TLS or not.
 */
#define stsecuresocketsSOCKET_SECURE_FLAG          ( 1UL << 0 )

/**
 * @brief A flag to indicate whether or not a socket is closed
 * for receive.
 */
#define stsecuresocketsSOCKET_READ_CLOSED_FLAG     ( 1UL << 1 )

/**
 * @brief A flag to indicate whether or not a socket is closed
 * for send.
 */
#define stsecuresocketsSOCKET_WRITE_CLOSED_FLAG    ( 1UL << 2 )

/**
 * @brief A flag to indicate whether or not the socket is connected.
 */
#define stsecuresocketsSOCKET_IS_CONNECTED_FLAG    ( 1UL << 3 )

/**
 * @brief The maximum timeout accepted by the Inventek module.
 *
 * This value is dictated by the hardware and should not be
 * modified.
 */
#define stsecuresocketsMAX_TIMEOUT                 ( 60000 )

#define minMESSAGE_SIZE                                (5)

/*-----------------------------------------------------------*/

/**
 * @brief Represents the WiFi module.
 *
 * Since there is only one WiFi module on the ST board, only
 * one instance of this type is needed. All the operations on
 * the WiFi module must be serialized because a single operation
 * (like socket connect, send etc) consists of multiple AT Commands
 * sent over the same SPI bus. A semaphore is therefore used to
 * serialize all the operations.
 */


/**
 * @brief Represents a secure socket.
 */
typedef struct STSecureSocket
{
    int32_t ST_socket_handle;           /**< This is the socket handle created by the BG96 modem*/
    uint8_t ucInUse;                    /**< Tracks whether the socket is in use or not. */
    uint32_t ulFlags;                   /**< Various properties of the socket (secured etc.). */
    uint32_t ulSendTimeout;             /**< Send timeout. */
    uint32_t ulReceiveTimeout;          /**< Receive timeout. */
    char * pcDestination;               /**< Destination URL. Set using SOCKETS_SO_SERVER_NAME_INDICATION option in SOCKETS_SetSockOpt function. */
    void * pvTLSContext;                /**< The TLS Context. */
    char * pcServerCertificate;         /**< Server certificate. Set using SOCKETS_SO_TRUSTED_SERVER_CERTIFICATE option in SOCKETS_SetSockOpt function. */
    uint32_t ulServerCertificateLength; /**< Length of the server certificate. */
} STSecureSocket_t;
/*-----------------------------------------------------------*/

/**
 * @brief Secure socket objects.
 *
 * An index in this array is returned to the user from SOCKETS_Socket
 * function.
 */
static STSecureSocket_t xSockets[ CELLULAR_MAX_SOCKETS ];

/**
 * @brief WiFi module object.
 *
 * Since the ST board contains only one WiFi module, only one instance
 * is needed and there is no need to pass this to the user.
 */
//extern STWiFiModule_t xWiFiModule;

/**
 * @brief Maximum time to wait in ticks for obtaining the WiFi semaphore
 * before failing the operation.
 */
//static const TickType_t xSemaphoreWaitTicks = pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS );
/*-----------------------------------------------------------*/

/**
 * @brief Get a free socket from the free socket pool.
 *
 * Iterates over the xSockets array to see if it can find
 * a free socket. A free or unused socket is indicated by
 * the zero value of the ucInUse member of STSecureSocket_t.
 *
 * @return Index of the socket in the xSockets array, if it is
 * able to find a free socket, SOCKETS_INVALID_SOCKET otherwise.
 */
static uint32_t prvGetFreeSocket( void );

/**
 * @brief Returns the socket back to the free socket pool.
 *
 * Marks the socket as free by setting ucInUse member of the
 * STSecureSocket_t structure as zero.
 */
static void prvReturnSocket( uint32_t ulSocketNumber );

/**
 * @brief Checks whether or not the provided socket number is valid.
 *
 * Ensures that the provided number is less than wificonfigMAX_SOCKETS
 * and the socket is "in-use" i.e. ucInUse is set to non-zero in the
 * socket structure.
 *
 * @param[in] ulSocketNumber The provided socket number to check.
 *
 * @return pdTRUE if the socket is valid, pdFALSE otherwise.
 */
static BaseType_t prvIsValidSocket( uint32_t ulSocketNumber );

/**
 * @brief Sends the provided data over WiFi.
 *
 * @param[in] pvContext The caller context. Socket number in our case.
 * @param[in] pucData The data to send.
 * @param[in] xDataLength Length of the data.
 *
 * @return Number of bytes actually sent if successful, SOCKETS_SOCKET_ERROR
 * otherwise.
 */
static BaseType_t prvNetworkSend( void * pvContext,
                                  const unsigned char * pucData,
                                  size_t xDataLength );

/**
 * @brief Receives the data over WiFi.
 *
 * @param[in] pvContext The caller context. Socket number in our case.
 * @param[out] pucReceiveBuffer The buffer to receive the data in.
 * @param[in] xReceiveBufferLength The length of the provided buffer.
 *
 * @return The number of bytes actually received if successful, SOCKETS_SOCKET_ERROR
 * otherwise.
 */
static BaseType_t prvNetworkRecv( void * pvContext,
                                  unsigned char * pucReceiveBuffer,
                                  size_t xReceiveBufferLength );
/*-----------------------------------------------------------*/

static uint32_t prvGetFreeSocket( void ) /* OK */
{
    uint32_t ulIndex;

    /* Iterate over xSockets array to see if any free socket
     * is available. */
    for( ulIndex = 0; ulIndex < ( uint32_t ) CELLULAR_MAX_SOCKETS; ulIndex++ )
    {
        /* Since multiple tasks can be accessing this simultaneously,
         * this has to be in critical section. */
        taskENTER_CRITICAL();

        if( xSockets[ ulIndex ].ucInUse == 0U )
        {
            /* Mark the socket as "in-use". */
            xSockets[ ulIndex ].ucInUse = 1;
            taskEXIT_CRITICAL();

            /* We have found a free socket, so stop. */
            break;
        }
        else
        {
            taskEXIT_CRITICAL();
        }
    }

    /* Did we find a free socket? */
    if( ulIndex == ( uint32_t ) CELLULAR_MAX_SOCKETS )
    {
        /* Return SOCKETS_INVALID_SOCKET if we fail to
         * find a free socket. */
        ulIndex = ( uint32_t ) SOCKETS_INVALID_SOCKET;
    }

    return ulIndex;
}
/*-----------------------------------------------------------*/

static void prvReturnSocket( uint32_t ulSocketNumber ) /* OK */
{
    /* Since multiple tasks can be accessing this simultaneously,
     * this has to be in critical section. */
    taskENTER_CRITICAL();
    {
        /* Mark the socket as free. */
        xSockets[ ulSocketNumber ].ucInUse = 0;
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

static BaseType_t prvIsValidSocket( uint32_t ulSocketNumber ) /* OK */
{
    BaseType_t xValid = pdFALSE;

    /* Check that the provided socket number is within the valid
     * index range. */
    if( ulSocketNumber < ( uint32_t ) CELLULAR_MAX_SOCKETS )
    {
        /* Since multiple tasks can be accessing this simultaneously,
         * this has to be in critical section. */
        taskENTER_CRITICAL();
        {
            /* Check that this socket is in use. */
            if( xSockets[ ulSocketNumber ].ucInUse == 1U )
            {
                /* This is a valid socket number. */
                xValid = pdTRUE;
            }
        }
        taskEXIT_CRITICAL();
    }

    return xValid;
}
/*-----------------------------------------------------------*/

static BaseType_t prvNetworkSend( void * pvContext, /* OK */
                                  const unsigned char * pucData,
                                  size_t xDataLength )
{
    uint32_t ulSocketNumber = ( uint32_t ) pvContext; /*lint !e923 cast is necessary for port. */
    BaseType_t xRetVal = SOCKETS_SOCKET_ERROR;

    /* Sends the data. Note that this is a blocking function and the send timeout must be set properly
     * using SOCKETS_SetSockOpt. If the timeout expires when sending data, the function will return
     * an ERROR, so please be wise in setting the value. Also note that you should not use the option
     * COM_MSG_DONTWAIT as it does not handle sending buffers greater than the MSS (1460).
     */
    xRetVal = ( BaseType_t )com_send_ip_modem ((uint32_t)xSockets[ulSocketNumber].ST_socket_handle,
                (const com_char_t *) &pucData[0], (int32_t) xDataLength, 0);

    /* If the data was successfully sent, return the actual
     * number of bytes sent. Otherwise return SOCKETS_SOCKET_ERROR. */
    if ( xRetVal <= 0 )
    {
        xRetVal = SOCKETS_SOCKET_ERROR;
    }


    /* To allow other tasks of equal priority that are using this API to run as
     * a switch to an equal priority task that is waiting for the mutex will
     * only otherwise occur in the tick interrupt - at which point the mutex
     * might have been taken again by the currently running task.
     */

    taskYIELD();

 return xRetVal;
}
/*-----------------------------------------------------------*/

static BaseType_t prvNetworkRecv( void * pvContext,
                                  unsigned char * pucReceiveBuffer,
                                  size_t xReceiveBufferLength )

{
    uint32_t ulSocketNumber = ( uint32_t ) pvContext; /*lint !e923 cast is needed for portability. */
    BaseType_t xRetVal = 0;
    int32_t xReceiveValue,xTotalBytesReceived = 0;
    unsigned char * tmpReceiveBuffer = pucReceiveBuffer;
    int32_t tmpReceiveBufferLength = xReceiveBufferLength;
    int32_t errorCount = 0;

    for (;;)
    {
        /* Receive the data. Note that this is a blocking function unless COM_MSG_DONTWAIT is specified.
            * The receive timeout must be properly set using SOCKETS_SetSockOpt. If the timeout expires
            * when receiving data, the function will return an ERROR, so please be wise in setting the value.
            * Also note that you should not use the option COM_MSG_DONTWAIT as it does not handle receiving
            * buffers greater than the MSS (1460). If the timeout expires and there is data in the modem
            * receive buffer the data cache may try to flush it and this may result in data loss
            */

        xReceiveValue = (BaseType_t) com_recv_ip_modem ((uint32_t)xSockets[ulSocketNumber].ST_socket_handle,
                tmpReceiveBuffer, (int32_t) tmpReceiveBufferLength, 0);

        if( xReceiveValue < 0 )
        {
            /* check if the receive timeout expired and return 0 as this is a valid case */
            if (xReceiveValue == COM_SOCKETS_ERR_TIMEOUT)
            {
                /* The socket read has timed out too. Returning SOCKETS_EWOULDBLOCK
                    * will cause mBedTLS to fail and so we must return zero. */
                xTotalBytesReceived = 0;
                break;
            }
            else
            {
                if (errorCount++ > 5) {
                    /* We had a communication error status of some sort */
                    xRetVal = SOCKETS_SOCKET_ERROR;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        else if (( xReceiveValue >= minMESSAGE_SIZE ) || (xReceiveValue == xReceiveBufferLength))
        {
            /* We received enough data and need to pass it back */
            xTotalBytesReceived +=  xReceiveValue;
            break;
        }
        else
        {
            /* we received less than minMESSAGE_SIZE bytes, so wait a little longer to see if something is coming back.
                * This also allows other tasks of equal priority to take the hand.
                */
            tmpReceiveBuffer = tmpReceiveBuffer + xReceiveValue;
            tmpReceiveBufferLength = tmpReceiveBufferLength - xReceiveValue;
            xTotalBytesReceived += xReceiveValue;
            vTaskDelay( pdMS_TO_TICKS(5) );
        }
    }

    if (xRetVal != SOCKETS_SOCKET_ERROR )
    {
        xRetVal = (BaseType_t) xTotalBytesReceived;
    }

    return xRetVal;
}

/*-----------------------------------------------------------*/

Socket_t SOCKETS_Socket( int32_t lDomain, /* OK */
                         int32_t lType,
                         int32_t lProtocol )
{
    uint32_t ulSocketNumber;
    int32_t  ulModemSocketNumber;


    /* Ensure that only supported values are supplied. */
    configASSERT( lDomain == SOCKETS_AF_INET );
    configASSERT( ( lType == SOCKETS_SOCK_STREAM && lProtocol == SOCKETS_IPPROTO_TCP ) );

    /* Try to get a free socket. */
    ulSocketNumber = prvGetFreeSocket();


    /* Get a free socket from the modem pool */

    ulModemSocketNumber = com_socket_ip_modem(lDomain, lType, lProtocol);


    /* If we get a free socket, set its attributes. */
    if( ulSocketNumber != ( uint32_t ) SOCKETS_INVALID_SOCKET && (ulModemSocketNumber < CELLULAR_MAX_SOCKETS))
    {
        /* Initialize all the members to sane values. */
        xSockets[ ulSocketNumber ].ulFlags = 0;
        xSockets[ ulSocketNumber ].ulSendTimeout = socketsconfigDEFAULT_SEND_TIMEOUT;
        xSockets[ ulSocketNumber ].ulReceiveTimeout = socketsconfigDEFAULT_RECV_TIMEOUT;
        xSockets[ ulSocketNumber ].pcDestination = NULL;
        xSockets[ ulSocketNumber ].pvTLSContext = NULL;
        xSockets[ ulSocketNumber ].pcServerCertificate = NULL;
        xSockets[ ulSocketNumber ].ulServerCertificateLength = 0;
        xSockets[ ulSocketNumber ].ST_socket_handle = ulModemSocketNumber; /*+*/


    }
    else {
            ulSocketNumber = ( uint32_t ) SOCKETS_INVALID_SOCKET;
        }

    /* If we fail to get a free socket, we return SOCKETS_INVALID_SOCKET. */
    return ( Socket_t ) ulSocketNumber; /*lint !e923 cast required for portability. */
}

/* IS_ONLINE */

/*-----------------------------------------------------------*/

int32_t SOCKETS_Connect( Socket_t xSocket,
                         SocketsSockaddr_t * pxAddress,
                         Socklen_t xAddressLength )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    //TLSParams_t xTLSParams = { 0 };
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;
    com_sockaddr_in_t  destination_address;
    uint32_t lRecvTimeout,lSendTimeout;

    /* Ensure that a valid socket was passed. */
    if( prvIsValidSocket( ulSocketNumber ) == pdTRUE )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        /* Start the client connection. */

        destination_address.sin_family      = COM_AF_INET;
        destination_address.sin_port        = pxAddress->usPort;
        destination_address.sin_addr.s_addr =  pxAddress->ulAddress;

        int32_t res = com_connect_ip_modem((uint32_t)xSockets[ulSocketNumber].ST_socket_handle ,
                (com_sockaddr_t *) &destination_address, sizeof(com_sockaddr_in_t));
        if(res == CELLULAR_OK)
        {
            /* Successful connection is established. */
            lRetVal = SOCKETS_ERROR_NONE;
        }
    }

    return lRetVal;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Recv( Socket_t xSocket,
                      void * pvBuffer,
                      size_t xBufferLength,
                      uint32_t ulFlags )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    int32_t lReceivedBytes = SOCKETS_SOCKET_ERROR;

    /* Remove warning about unused parameters. */
    ( void ) ulFlags;

    /* Ensure that a valid socket was passed and the
     * passed buffer is not NULL. */
    if( ( prvIsValidSocket( ulSocketNumber ) == pdTRUE ) &&
        ( pvBuffer != NULL ) )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        /* Check that receive is allowed on the socket. */
        if( ( pxSecureSocket->ulFlags & stsecuresocketsSOCKET_READ_CLOSED_FLAG ) == 0UL )
        {
                lReceivedBytes = prvNetworkRecv( xSocket, pvBuffer, xBufferLength );
        }
        else
        {
            /* The socket has been closed for read. */
            lReceivedBytes = SOCKETS_ECLOSED;
        }
    }

    return lReceivedBytes;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Send( Socket_t xSocket,
                      const void * pvBuffer,
                      size_t xDataLength,
                      uint32_t ulFlags )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    int32_t lSentBytes = SOCKETS_SOCKET_ERROR;

    /* Remove warning about unused parameters. */
    ( void ) ulFlags;

    /* Ensure that a valid socket was passed and the passed buffer
     * is not NULL. */
    if( ( prvIsValidSocket( ulSocketNumber ) == pdTRUE ) &&
        ( pvBuffer != NULL ) )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        /* Check that send is allowed on the socket. */
        if( ( pxSecureSocket->ulFlags & stsecuresocketsSOCKET_WRITE_CLOSED_FLAG ) == 0UL )
        {
                lSentBytes = prvNetworkSend( xSocket, pvBuffer, xDataLength );
        }
        else
        {
            /* The socket has been closed for write. */
            lSentBytes = SOCKETS_ECLOSED;
        }
    }

    return lSentBytes;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Shutdown( Socket_t xSocket,
                          uint32_t ulHow )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;

    /* Ensure that a valid socket was passed. */
    if( prvIsValidSocket( ulSocketNumber ) == pdTRUE )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        switch( ulHow )
        {
            case SOCKETS_SHUT_RD:
                /* Further receive calls on this socket should return error. */
                pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_READ_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            case SOCKETS_SHUT_WR:
                /* Further send calls on this socket should return error. */
                pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_WRITE_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            case SOCKETS_SHUT_RDWR:
                /* Further send or receive calls on this socket should return error. */
                pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_READ_CLOSED_FLAG;
                pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_WRITE_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            default:
                /* An invalid value was passed for ulHow. */
                lRetVal = SOCKETS_EINVAL;
                break;
        }
    }
    else
    {
        /* Invalid socket was passed. */
        lRetVal = SOCKETS_EINVAL;
    }

    return lRetVal;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Close( Socket_t xSocket )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;

    /* Ensure that a valid socket was passed. */
    if( prvIsValidSocket( ulSocketNumber ) == pdTRUE )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        /* Mark the socket as closed. */
        pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_READ_CLOSED_FLAG;
        pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_WRITE_CLOSED_FLAG;

        /* Free the space allocated for pcDestination. */
        if( pxSecureSocket->pcDestination != NULL )
        {
            vPortFree( pxSecureSocket->pcDestination );
        }

        /* Free the space allocated for pcServerCertificate. */
        if( pxSecureSocket->pcServerCertificate != NULL )
        {
            vPortFree( pxSecureSocket->pcServerCertificate );
        }

            /* Stop the client connection. */
            if(com_closesocket_ip_modem((uint32_t)xSockets[ulSocketNumber].ST_socket_handle) == CELLULAR_OK )
            {
                /* Connection close successful. */
                lRetVal = SOCKETS_ERROR_NONE;
            }



        /* Return the socket back to the free socket pool. */
        prvReturnSocket( ulSocketNumber );
    }
    else
    {
        /* Bad argument. */
        lRetVal = SOCKETS_EINVAL;
    }

    return lRetVal;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_SetSockOpt( Socket_t xSocket,
                            int32_t lLevel,
                            int32_t lOptionName,
                            const void * pvOptionValue,
                            size_t xOptionLength )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    STSecureSocket_t * pxSecureSocket;
    int32_t lRetVal = SOCKETS_ERROR_NONE;
    uint32_t lTimeout,lRecvTimeout,lSendTimeout;

    /* Ensure that a valid socket was passed. */
    if( prvIsValidSocket( ulSocketNumber ) == pdTRUE )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        switch( lOptionName )
        {
            case SOCKETS_SO_SERVER_NAME_INDICATION:

                if( ( pxSecureSocket->ulFlags & stsecuresocketsSOCKET_IS_CONNECTED_FLAG ) == 0 )
                {
                    /* Non-NULL destination string indicates that SNI extension should
                     * be used during TLS negotiation. */
                    pxSecureSocket->pcDestination = ( char * ) pvPortMalloc( 1U + xOptionLength );

                    if( pxSecureSocket->pcDestination == NULL )
                    {
                        lRetVal = SOCKETS_ENOMEM;
                    }
                    else
                    {
                        memcpy( pxSecureSocket->pcDestination, pvOptionValue, xOptionLength );
                        pxSecureSocket->pcDestination[ xOptionLength ] = '\0';
                    }
                }
                else
                {
                    /* SNI must be set before connection is established. */
                    lRetVal = SOCKETS_SOCKET_ERROR;
                }

                break;

            case SOCKETS_SO_TRUSTED_SERVER_CERTIFICATE:

                if( ( pxSecureSocket->ulFlags & stsecuresocketsSOCKET_IS_CONNECTED_FLAG ) == 0 )
                {
                    /* Non-NULL server certificate field indicates that the default trust
                     * list should not be used. */
                    pxSecureSocket->pcServerCertificate = ( char * ) pvPortMalloc( xOptionLength );

                    if( pxSecureSocket->pcServerCertificate == NULL )
                    {
                        lRetVal = SOCKETS_ENOMEM;
                    }
                    else
                    {
                        memcpy( pxSecureSocket->pcServerCertificate, pvOptionValue, xOptionLength );
                        pxSecureSocket->ulServerCertificateLength = xOptionLength;
                    }
                }
                else
                {
                    /* Trusted server certificate must be set before the connection is established. */
                    lRetVal = SOCKETS_SOCKET_ERROR;
                }

                break;

            case SOCKETS_SO_REQUIRE_TLS:

              if( ( pxSecureSocket->ulFlags & stsecuresocketsSOCKET_IS_CONNECTED_FLAG ) == 0 )
                {
                    /* Mark that it is a secure socket. */
                    pxSecureSocket->ulFlags |= stsecuresocketsSOCKET_SECURE_FLAG;
                }
                else
                {
                    /* Require TLS must be set before the connection is established. */
                    lRetVal = SOCKETS_SOCKET_ERROR;
                }

                break;

            case SOCKETS_SO_SNDTIMEO:

                lTimeout = *( ( const uint32_t * ) pvOptionValue ); /*lint !e9087 pvOptionValue is passed in as an opaque value, and must be casted for setsockopt. */

                /* Valid timeouts are 0 (no timeout) or 1-30000ms. */
                if( lTimeout < stsecuresocketsMAX_TIMEOUT )
                {
                    /* Store send timeout. */
                    pxSecureSocket->ulSendTimeout = lTimeout;
                }
                else
                {
                    lRetVal = SOCKETS_EINVAL;
                }

                break;

            case SOCKETS_SO_RCVTIMEO:

                lTimeout = *( ( const uint32_t * ) pvOptionValue ); /*lint !e9087 pvOptionValue is passed in as an opaque value, and must be casted for setsockopt. */

                /* Valid timeouts are 0 (no timeout) or 1-30000ms. */
                if( lTimeout < stsecuresocketsMAX_TIMEOUT )
                {
                    /* Store receive timeout. */
                    pxSecureSocket->ulReceiveTimeout = lTimeout;
                }
                else
                {
                    lRetVal = SOCKETS_EINVAL;
                }

                break;

            default:

                lRetVal = SOCKETS_ENOPROTOOPT;
                break;
        }
    }
    else
    {
        lRetVal = SOCKETS_SOCKET_ERROR;
    }

    return lRetVal;
}
/*-----------------------------------------------------------*/

uint32_t SOCKETS_GetHostByName( const char * pcHostName )
{
    uint32_t ulIPAddres = -1;
    com_sockaddr_t distantaddr;

    /* Do a DNS Lookup. */

    com_gethostbyname_ip_modem ((const com_char_t *) pcHostName, &distantaddr);
    ulIPAddres = ((com_sockaddr_in_t *)&distantaddr)->sin_addr.s_addr;

    return ulIPAddres;
}
/*-----------------------------------------------------------*/

BaseType_t SOCKETS_Init( void )
{
    uint32_t ulIndex;

    /* Mark all the sockets as free and closed. */
    for( ulIndex = 0; ulIndex < ( uint32_t ) CELLULAR_MAX_SOCKETS; ulIndex++ )
    {
        xSockets[ ulIndex ].ucInUse = 0;
        xSockets[ ulIndex ].ulFlags = 0;

        xSockets[ ulIndex ].ulFlags |= stsecuresocketsSOCKET_READ_CLOSED_FLAG;
        xSockets[ ulIndex ].ulFlags |= stsecuresocketsSOCKET_WRITE_CLOSED_FLAG;
    }

    /* Empty initialization for ST board. */
    return pdPASS;
}
/*-----------------------------------------------------------*/
