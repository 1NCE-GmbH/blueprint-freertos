/*
 *  udp_demo.c
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "nce_demo_config.h"
#ifdef CONFIG_UDP_DEMO_ENABLED
    #include "udp_demo.h"
    #include "nce_iot_c_sdk.h"
    #include "cellular_app.h"
    #include "queue.h"
    #include "task.h"
    #include "FreeRTOS.h"
    #include "event_groups.h"
    #include "cellular_types.h"

    typedef struct xSOCKET
    {
        CellularSocketHandle_t cellularSocketHandle;
        uint32_t ulFlags;
        uint32_t xSendFlags;
        uint32_t xRecvFlags;

        TickType_t receiveTimeout;
        TickType_t sendTimeout;

        char * pcDestination;
        void * pvTLSContext;
        char * pcServerCertificate;
        uint32_t ulServerCertificateLength;

        EventGroupHandle_t socketEventGroupHandle;
    } _cellularSecureSocket_t;

    /**
     * @brief Socket receive operation timeout in ticks.
     *
     * This constant sets the maximum amount of time that the system
     * will wait for data to be received on the socket. After this period, the receive
     * operation will timeout if no data is received.
     */
    static const TickType_t xReceiveTimeOut = DEFAULT_SOCKET_TIMEOUT_MS;

    /**
     * @brief Socket send operation timeout in ticks.
     *
     * This constant defines the maximum amount of time allowed for data
     * to be sent through the socket. If the data is not sent within this timeframe,
     * the send operation will timeout.
     */
    static const TickType_t xSendTimeOut = DEFAULT_SOCKET_TIMEOUT_MS;

    /**
     * @brief Callback function to handle incoming UDP data.
     *
     * This function is triggered when data is received on the specified socket. It processes
     * the incoming UDP message. The steps include:
     *
     * - Configuring socket timeouts to avoid blocking.
     * - Receiving data from the UDP socket into a buffer.
     * - Parsing the received data.
     * - Handling errors related to memory allocation.
     *
     * @param[in] pCallbackContext Context passed to the callback (socket details).
     */
    void UdpServiceDataReadyCallback( void * pCallbackContext )
    {
        /* Set receive and send timeouts for the socket to avoid indefinite blocking. */
        SOCKETS_SetSockOpt( pCallbackContext, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
        SOCKETS_SetSockOpt( pCallbackContext, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );
        /* Buffer to store received data. Size defined by NCE_RECEIVE_BUFFER_SIZE_BYTES macro. */
        char receive_buffer[ NCE_RECEIVE_BUFFER_SIZE_BYTES ];
        /* Variable to store the number of received bytes. */
        int32_t ReceivedBytes;

        /* Receive data from the socket into the receive buffer. */
        ReceivedBytes = SOCKETS_Recv( pCallbackContext, receive_buffer, sizeof( receive_buffer ) - 1, 0 );

        /* If data has been received, proceed with message processing. */
        if( ReceivedBytes > 0 )
        {
            /* Null-terminate the received data to ensure it can be treated as a string */
            receive_buffer[ ReceivedBytes ] = '\0';

            /* Log the number of bytes received and the actual data */
            IotLogInfo( "Received %d bytes:\n %s\r\n", ReceivedBytes, receive_buffer );
        }
        else if( ReceivedBytes == 0 )
        {
            /* No data received, continue waiting for incoming data */
            IotLogInfo( "No data received.\r\n" );
        }
        else
        {
            /* An error occurred while receiving data */
            IotLogError( "Error receiving data from UDP socket.\r\n" );
        }
    }

/**
 * @brief Task function to send data over UDP.
 *
 * This function establishes a UDP connection to a server, constructs a message, and sends
 * it at regular intervals. It optionally includes energy-saving information in the message
 * if `CONFIG_NCE_ENERGY_SAVER` is defined.
 *
 * Function Flow:
 * - Performs DNS lookup for the UDP server address.
 * - Creates a UDP socket and configures timeouts for sending and receiving.
 * - Constructs and sends the message.
 * - Logs the message for debugging purposes.
 * - The task pauses between sending messages based on a configured frequency.
 */
    void SendUDPData()
    {
        /* Buffer to store the packet to be sent */
        char send_packet[ 100 ];

        /* Structure to hold the server address and port */
        SocketsSockaddr_t ServerAddress;

        /* Main loop to send data periodically */
        while( 1 )
        {
            /* Log a message indicating the connection attempt */
            IotLogInfo( "Connecting to UDP server\r\n" );

            /* Set server port and resolve server address through DNS */
            ServerAddress.usPort = SOCKETS_htons( CONFIG_UDP_SERVER_PORT );
            ServerAddress.ulAddress = SOCKETS_GetHostByName( CONFIG_UDP_SERVER_ADDRESS );

            /* Check for DNS resolution errors */
            if( ServerAddress.ulAddress == ( uint32_t ) 0 )
            {
                IotLogError( "Failed to connect to server: DNS resolution failed: Server=%s.",
                             CONFIG_UDP_SERVER_ADDRESS );
                vTaskDelete( NULL ); /* Exit task if DNS resolution fails */
                return;
            }

            /* Create a UDP socket */
            Socket_t udp = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );
            configASSERT( udp != SOCKETS_INVALID_SOCKET );

            /* Set timeouts for the socket */
            SOCKETS_SetSockOpt( udp, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
            SOCKETS_SetSockOpt( udp, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

            /* Attempt to connect to the UDP server */
            int result = SOCKETS_Connect( udp, &ServerAddress, sizeof( ServerAddress ) );

            /* Check if connection was successful */
            if( result == 0 )
            {
                IotLogInfo( "Connected to UDP server %s:%u\r\n", IP_TO_STRING( ServerAddress.ulAddress ), SOCKETS_ntohs( ServerAddress.usPort ) );

                #if defined( CONFIG_NCE_ENERGY_SAVER )
                    /* Buffer to store the transmitted bytes, initialized with a size of 50 bytes. */
                    char pcTransmittedString[ 50 ];

                    /* Initialize the buffer to all null ('\0') characters. */
                    ( void ) memset( &pcTransmittedString, ( uint8_t ) '\0', sizeof( pcTransmittedString ) );
                    uint8_t selector = 1;

                    /* Initialize data elements to be transmitted:
                     * - battery_level: Integer value representing battery percentage (99%).
                     * - signal_strength: Integer value representing signal strength (84 dBm).
                     * - software_version: String representing the software version ("2.2.1").
                     */
                    Element2byte_gen_t battery_level = { .type = E_INTEGER, .value.i = 99, .template_length = 1 };
                    Element2byte_gen_t signal_strength = { .type = E_INTEGER, .value.i = 84, .template_length = 1 };
                    Element2byte_gen_t software_version = { .type = E_STRING, .value.s = "2.2.1", .template_length = 5 };

                    /* Generate the energy-saving data and add it to the transmitted string */
                    os_energy_save( pcTransmittedString, selector, 3, battery_level, signal_strength, software_version );
                    sprintf( send_packet, "%s", pcTransmittedString ); /* Copy the payload into send_packet buffer */
                #else /* if defined( CONFIG_NCE_ENERGY_SAVER ) */
                    /* Prepare the payload to be sent */
                    char pcTransmittedString[] = PUBLISH_PAYLOAD_FORMAT;
                    sprintf( send_packet, "%s", pcTransmittedString ); /* Copy the payload into send_packet buffer */
                #endif /* if defined( CONFIG_NCE_ENERGY_SAVER ) */

                /* Send the packet to the server */
                int32_t SendVal = SOCKETS_Send( udp, send_packet, strlen( send_packet ), 0 );
                IotLogInfo( "Sending '%s' of length %d to UDP server\r\n", send_packet, strlen( send_packet ) );

                /* Check if sending was successful */
                if( SendVal < 0 )
                {
                    IotLogError( "Failed to send to UDP server\r\n" );
                }
                else
                {
                    IotLogInfo( "Data successfully sent to the server\r\n" );

                    /* Close the socket after sending */
                    result = SOCKETS_Close( udp );
                    configASSERT( result == SOCKETS_ERROR_NONE );

                    /* Pause to avoid network congestion (frequency of sending) */
                    vTaskDelay( pdMS_TO_TICKS( CONFIG_UDP_DATA_UPLOAD_FREQUENCY_SECONDS * 1000 ) );
                }
            }
            else
            {
                IotLogError( "Failed to connect to UDP server %s.\r\n", CONFIG_UDP_SERVER_ADDRESS );
                vTaskDelay( DEFAULT_TASK_DELAY_MS ); /* Wait 5 seconds before retrying */
            }
        }
    }

    /**
     * @brief Task function to listen for incoming UDP data.
     *
     * This function creates a UDP socket bound to a specific port and listens for
     * incoming UDP messages. Upon receiving data, it processes the message and logs it.
     * The task runs indefinitely, receiving messages at regular intervals.
     *
     * Function Flow:
     * - Binds the UDP socket to the listening port.
     * - Waits for incoming data using `SOCKETS_Recv()`.
     * - Logs the received message for debugging.
     * - The task continues to listen for messages in an infinite loop.
     */
    void ListenUDPData()
    {
        /* Socket address structure to define the local address and port for the UDP service. */
        SocketsSockaddr_t my_addr =
        {
            .ucSocketDomain = SOCKETS_AF_INET,               /* Set the address family to IPv4 */
            .usPort         = SOCKETS_htons( NCE_RECV_PORT ) /* Set the port for receiving data */
        };
        Socket_t listenSocket;                               /* Socket handle for UDP communications */

        listenSocket = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );

        /* Ensure the socket was created successfully. */
        configASSERT( listenSocket != SOCKETS_INVALID_SOCKET );

        /* Bind the UDP socket to the local address and port specified in my_addr. */
        SOCKETS_SetSockOpt( listenSocket, 0, SOCKETS_UDP_SERVICE, 1U, sizeof( uint8_t ) );

        if( SOCKETS_Connect( listenSocket, &my_addr, sizeof( my_addr ) ) != 0 )
        {
            IotLogError( "Failed to bind UDP socket to port %d.\r\n", NCE_RECV_PORT );
            vTaskDelay( DEFAULT_TASK_DELAY_MS ); /* Wait for 5 seconds before retrying */
        }

        IotLogInfo( "UDP socket bound to port %d, waiting for incoming messages.\r\n", NCE_RECV_PORT );

        /* Main task loop for receiving and processing data. */
        while( 1 )
        {
            /* Wait for the socket event indicating data is ready to be received. */
            xEventGroupWaitBits( listenSocket->socketEventGroupHandle,
                                 SOCKET_DATA_RECEIVED_CALLBACK_BIT,
                                 ( BaseType_t ) pdTRUE,  /* Clear the bit when it is set */
                                 ( BaseType_t ) pdFALSE, /* Do not wait for all bits */
                                 portMAX_DELAY );        /* Wait indefinitely */
            /* Call the UDP service callback to handle the received data. */
            UdpServiceDataReadyCallback( listenSocket );

            /* Introduce a delay before the next iteration to prevent excessive CPU usage. */
            vTaskDelay( DEFAULT_TASK_DELAY_MS );
            IotLogInfo( "Waiting for incoming messages" );
        }
    }

    /**
     * @brief Demo function to initialize and manage UDP communication tasks.
     *
     * This function sets up a FreeRTOS queue and two tasks for UDP communication:
     * one for sending UDP messages (`SendUDPData`) and another for listening
     * and receiving UDP messages (`ListenUDPData`). The function creates the
     * necessary queue and tasks, handles error checks, and logs status updates.
     * Once tasks are created successfully, it enters an infinite loop to keep
     * the tasks running.
     */
    void UdpDemo()
    {
        QueueHandle_t xQueue; /* Queue to hold received data */

        /* Create the queue to hold received data. */
        xQueue = xQueueCreate( 2, THREAD_STACK_SIZE );

        /* Check if queue creation was successful. */
        if( xQueue == NULL )
        {
            IotLogError( "Queue creation failed: insufficient memory or invalid parameters.\r\n" );
            return; /* Exit the function if queue creation fails. */
        }

        /* Log message indicating successful queue creation. */
        IotLogInfo( "Queue created successfully.\r\n" );

        /* Variable to hold task creation status. */
        BaseType_t xTaskCreateStatus;

        /* Create the task for sending UDP messages. */
        xTaskCreateStatus = xTaskCreate(
            SendUDPData,              /* Task function. */
            "SendUDPData",            /* Task name. */
            THREAD_STACK_SIZE,        /* Stack size in bytes. */
            NULL,                     /* Task parameter (none in this case). */
            configMAX_PRIORITIES - 1, /* Task priority: highest priority. */
            NULL                      /* Task handle (not used). */
            );

        /* Check if the task was created successfully. */
        if( xTaskCreateStatus != pdPASS )
        {
            IotLogError( "Failed to create the SendUDPData task.\r\n" );
            return; /* Exit the function if task creation fails. */
        }

        /* Log message indicating successful task creation. */
        IotLogDebug( "SendUDPData task created successfully.\r\n" );

        /* Create the task for listening and receiving UDP messages. */
        xTaskCreateStatus = xTaskCreate(
            ListenUDPData,            /* Task function. */
            "ListenUDPData",          /* Task name. */
            THREAD_STACK_SIZE,        /* Stack size in bytes. */
            NULL,                     /* Task parameter (none in this case). */
            configMAX_PRIORITIES - 1, /* Task priority: highest priority. */
            NULL                      /* Task handle (not used). */
            );

        /* Check if the task was created successfully. */
        if( xTaskCreateStatus != pdPASS )
        {
            IotLogError( "Failed to create the ListenUDPData task.\r\n" );
            return; /* Exit the function if task creation fails. */
        }

        /* Log message indicating successful task creation. */
        IotLogDebug( "ListenUDPData task created successfully.\r\n" );

        /* Infinite loop to keep the demo running and tasks active. */
        while( 1 )
        {
            /* The function does not exit, keeping the tasks running. */
        }
    }
#endif /* ifdef CONFIG_UDP_DEMO_ENABLED */
