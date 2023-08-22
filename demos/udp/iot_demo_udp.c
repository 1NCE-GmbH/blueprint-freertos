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
#include "nce_demo_config.h"
#if defined(CONFIG_UDP_DEMO_ENABLED)
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
#include "task.h"
#include "queue.h"
#include "nce_iot_c_sdk.h"
/* COAP include. */
#include "coap_main.h"
#include "udp_impl.h"

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
/* Function prototypes */
void SendDataTask(void *pvParameters);
void ListeningTask(void *pvParameters);
/*-----------------------------------------------------------*/
// Global variables
QueueHandle_t xQueue; // Queue to hold received data
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
	/* Create the queue to hold received data. */
	    xQueue = xQueueCreate(10, NCE_RECEIVE_BUFFER_SIZE);
	    if (xQueue == NULL)
	    {
	        IotLogError("Failed to create the queue.\r\n");
	        return EXIT_FAILURE;
	    }

	    /* Create the tasks. */
	    BaseType_t xTaskCreateStatus;
	    xTaskCreateStatus = xTaskCreate(SendDataTask, "SendDataTask", configMINIMAL_STACK_SIZE*4, NULL, tskIDLE_PRIORITY + 4, NULL);
	    if (xTaskCreateStatus != pdPASS)
	    {
	        IotLogError("Failed to create the SendDataTask.\r\n");
	        return EXIT_FAILURE;
	    }

	    xTaskCreateStatus = xTaskCreate(ListeningTask, "ListeningTask", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 4, NULL);
	    if (xTaskCreateStatus != pdPASS)
	    {
	        IotLogError("Failed to create the ListeningTask.\r\n");
	        return EXIT_FAILURE;
	    }
    return EXIT_SUCCESS;
}

/* SendDataTask - Task to connect to the 1NCE OS endpoint and send data */
void SendDataTask(void *pvParameters)
{
    (void)pvParameters;

    char send_packet[100];
    SocketsSockaddr_t ServerAddress;
    BaseType_t xReturned;

    IotLogInfo("Connecting to UDP server\r\n");
    ServerAddress.usPort = SOCKETS_htons(UDP_PORT);
    ServerAddress.ulAddress = SOCKETS_GetHostByName(UDP_ENDPOINT);

    /* Check for errors from DNS lookup. */
    if (ServerAddress.ulAddress == (uint32_t)0)
    {
        IotLogError("Failed to connect to server: DNS resolution failed: Server=%s.",
                    UDP_ENDPOINT);
        vTaskDelete(NULL);
        return;
    }
    else
    {
        /* Create UDP Socket */
        Socket_t udp = SOCKETS_Socket(SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP);
        configASSERT(udp != SOCKETS_INVALID_SOCKET);

        /* Set a time out so a missing reply does not cause the task to block
         * indefinitely. */
        SOCKETS_SetSockOpt(udp, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof(xReceiveTimeOut));
        SOCKETS_SetSockOpt(udp, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof(xSendTimeOut));

        /* Connect to the udp endpoint. */
        IotLogInfo("Connecting to udp server\r\n");

        if (SOCKETS_Connect(udp, &ServerAddress, sizeof(ServerAddress)) == 0)
        {
            IotLogInfo("Connected to udp server \r\n");
#ifndef CONFIG_NCE_ENERGY_SAVER
            /* Add payload */
            char pcTransmittedString[] = PUBLISH_PAYLOAD_FORMAT;
#else
            /* Add payload */
            char pcTransmittedString[500];
            (void)memset(&pcTransmittedString, (uint8_t)'\0', sizeof(pcTransmittedString));
            uint8_t selector = 4;
            Element2byte_gen_t battery_level = {.type = E_INTEGER, .value.i = 99, .template_length = 1};
            Element2byte_gen_t signal_strength = {.type = E_INTEGER, .value.i = 84, .template_length = 1};
            Element2byte_gen_t software_version = {.type = E_STRING, .value.s = "2.2.1", .template_length = 5};
            os_energy_save(pcTransmittedString, selector, 3, battery_level, signal_strength, software_version);
#endif
            sprintf(send_packet, &pcTransmittedString, UDP_ENDPOINT);
            int32_t SendVal = SOCKETS_Send(udp, &send_packet, strlen(send_packet), NULL);
            IotLogInfo("Sending %s of length %d to udp server\r\n", send_packet, strlen(send_packet));

            if (SendVal < 0)
            {
                /* Error? */
                IotLogError("Failed to send to udp server\r\n");
            }
            else /* Close this socket before looping back to create another. */
            {
                xReturned = SOCKETS_Close(udp);
                configASSERT(xReturned == SOCKETS_ERROR_NONE);

                /* Pause for a short while to ensure the network is not too
                 * congested. */
                vTaskDelay(udpLOOP_DELAY);
            }
        }
        else
        {
            IotLogError(" failed to connect to udp server %s.\r\n",
                        UDP_ENDPOINT);
        }
    }

    /* Delete self. */
    vTaskDelete(NULL);
}
/* Listening task that continuously receives data from the UDP server */
void ListeningTask(void *pvParameters)
{
    (void)pvParameters;

    char receive_buffer[NCE_RECEIVE_BUFFER_SIZE];
    SocketsSockaddr_t my_addr = {
    .ucSocketDomain = SOCKETS_AF_INET,
    .usPort = SOCKETS_htons(NCE_RECV_PORT)
	};

    while (1)
    {
        Socket_t udp = SOCKETS_Socket(SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP);
        configASSERT(udp != SOCKETS_INVALID_SOCKET);

        BIND=true;
        if (SOCKETS_Connect( udp,&my_addr,sizeof( my_addr ) ) != 0)
        {
            IotLogError("Failed to bind UDP socket to port %d.\r\n", NCE_RECV_PORT);
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retrying
            continue;
        }
        BIND=false;
        IotLogInfo("Listening on UDP port %d...\r\n", NCE_RECV_PORT);

        // Continuously receive and process data
        while (1)
        {
            int32_t ReceivedBytes = SOCKETS_Recv(udp, receive_buffer, sizeof(receive_buffer) - 1, 0);
            if (ReceivedBytes > 0)
            {
                receive_buffer[ReceivedBytes] = '\0'; // Null-terminate the received data
                // Print the received data
                IotLogInfo("Received %d bytes:\n %s\r\n",
                           ReceivedBytes, receive_buffer);

            }
        }

        // Close the socket and wait a bit before retrying
        SOCKETS_Close(udp);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*-----------------------------------------------------------*/
#endif
