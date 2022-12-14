/*
 * /*
 *  Created on: Feb 10, 2021
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 *
 */

/**
 * @file iot_demo_udp.c
 * @brief Demonstrates usage UDP in FreeRTOS connecting with our endpoint.
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Set up logging for this demo. */
#include "iot_demo_logging.h"

/* Platform layer includes. */
#include "platform/iot_clock.h"
#include "platform/iot_threads.h"
#include "platform/iot_network.h"

/* COAP include. */
#include "coap_main.h"
#include "nce_demo_config.h"

#define udpLOOP_DELAY    ( ( TickType_t ) 150 / portTICK_PERIOD_MS )
static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS( 40000 );
static const TickType_t xSendTimeOut = pdMS_TO_TICKS( 40000 );

/*-----------------------------------------------------------*/

/* Declaration of demo function. */
int RunudpDemo( bool awsIotMode,
                const char * pIdentifier,
                void * pNetworkServerInfo,
                void * pNetworkCredentialInfo,
                const IotNetworkInterface_t * pNetworkInterface );

/*-----------------------------------------------------------*/

/**
 * @brief The function that runs the UDP demo, called by the demo runner.
 * @return `EXIT_SUCCESS` if the demo completes successfully; `EXIT_FAILURE` otherwise.
 */
int RunudpDemo( bool awsIotMode,
                const char * pIdentifier,
                void * pNetworkServerInfo,
                void * pNetworkCredentialInfo,
                const IotNetworkInterface_t * pNetworkInterface )
{
    char send_packet[ 100 ];
    SocketsSockaddr_t ServerAddress;
    NetPacket_t pPacket;
    BaseType_t xReturned;

    IotLogInfo( "Connecting to UDP server\r\n" );
    ServerAddress.usPort = SOCKETS_htons( UDP_PORT );
    ServerAddress.ulAddress = SOCKETS_GetHostByName( UDP_ENDPOINT );

    /* Check for errors from DNS lookup. */
    if( ServerAddress.ulAddress == ( uint32_t ) 0 )
    {
        IotLogError( "Failed to connect to server: DNS resolution failed: Server=%s.",
                     UDP_ENDPOINT );
        return EXIT_FAILURE;
    }
    else
    {
        /* Create UDP Socket */
        Socket_t udp = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM,
                                       SOCKETS_IPPROTO_UDP );
        configASSERT( udp != SOCKETS_INVALID_SOCKET );

        /* Set a time out so a missing reply does not cause the task to block
         * indefinitely. */
        SOCKETS_SetSockOpt( udp, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
        SOCKETS_SetSockOpt( udp, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

        /* Connect to the udp end point. */
        IotLogInfo( "Connecting to udp server\r\n" );

        if( SOCKETS_Connect( udp, &ServerAddress, sizeof( ServerAddress ) ) == 0 )
        {
            IotLogInfo( "Connected to udp server \r\n" );
            sprintf( send_packet, &PUBLISH_PAYLOAD_FORMAT, UDP_ENDPOINT );
            int32_t SendVal = SOCKETS_Send( udp, &send_packet,
                                            strlen( send_packet ), NULL );
            IotLogInfo( "Sending %s of length %d to udp server\r\n", send_packet, strlen( send_packet ) );

            if( SendVal < 0 )
            {
                /* Error? */
                IotLogError( "Failed to send to udp server\r\n" );
                return EXIT_FAILURE;
            }
            else  /* Close this socket before looping back to create another. */
            {
                xReturned = SOCKETS_Close( udp );
                configASSERT( xReturned == SOCKETS_ERROR_NONE );

                /* Pause for a short while to ensure the network is not too
                 * congested. */
                vTaskDelay( udpLOOP_DELAY );
            }
        }
        else
        {
            IotLogError( " failed to connect to udp server %s.\r\n",
                         UDP_ENDPOINT );
            return EXIT_FAILURE;
        }
    }

    /* Delete self. */
    vTaskDelete( NULL );
    return EXIT_SUCCESS;
}

/*-----------------------------------------------------------*/
