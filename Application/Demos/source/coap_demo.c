/*
 *  coap_demo.c
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "nce_demo_config.h"
#if defined( CONFIG_COAP_DEMO_ENABLED )
    #include "coap_demo.h"
    #include "er-coap-13.h"
    #include "cellular_app.h"
    #include "cellular_types.h"
    #include "queue.h"
    #include "task.h"
    #include "FreeRTOS.h"
    #include "event_groups.h"
    #include "nce_iot_c_sdk.h"


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
 * @brief Maximum size for an empty CoAP acknowledgment (ACK) message.
 *
 * This constant defines the maximum size of an empty CoAP ACK message in bytes.
 * Typically, this message only contains headers for confirming the receipt of
 * a CoAP message and does not include any payload data.
 */
    #define MAX_SIZE_EMPTY_ACK    12U

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
     * @brief Callback function to handle incoming CoAP data on a UDP socket.
     *
     * This function is triggered when data is received on the specified socket. It processes
     * the incoming CoAP message and sends an acknowledgment (ACK) in response. The steps
     * include:
     * - Configuring socket timeouts to avoid blocking.
     * - Receiving data from the UDP socket into a buffer.
     * - Parsing the received data as a CoAP message.
     * - Constructing and serializing an acknowledgment (ACK) message.
     * - Sending the ACK back to the sender.
     * - Handling errors related to message serialization and memory allocation.
     *
     * @param[in] pCallbackContext Context passed to the callback (socket details).
     */

    void CoAPDataReadyCallback( void * pCallbackContext )
    {
        /* Set receive and send timeouts for the socket to avoid indefinite blocking. */
        SOCKETS_SetSockOpt( pCallbackContext, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
        SOCKETS_SetSockOpt( pCallbackContext, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );
        /* Buffer to store received data. Size defined by NCE_RECEIVE_BUFFER_SIZE_BYTES macro. */
        char receive_buffer[ NCE_RECEIVE_BUFFER_SIZE_BYTES ];
        /* CoAP message packet buffer to parse incoming messages. */
        static coap_packet_t message[ 50 ];
        /* Pointers for message acknowledgment handling. */
        void * msgAck;
        uint16_t msgAckBuffer_len;
        uint8_t * msgAckBuffer;
        /* Variable to store the number of received bytes. */
        int32_t ReceivedBytes;
        /* Variable to store status of acknowledgment send operation. */
        int32_t status_ack;

        IotLogInfo( "***************** DATA RECEIVED ***************** \r\n" );
        /* Receive data from the socket into the receive buffer. */
        ReceivedBytes = SOCKETS_Recv( pCallbackContext, receive_buffer, sizeof( receive_buffer ) - 1, 0 );

        /* If data has been received, proceed with message processing. */
        if( ReceivedBytes != 0 )
        {
            /* Parse the received CoAP message. */
            coap_parse_message( message, receive_buffer, ReceivedBytes );
            /* Allocate memory for the CoAP acknowledgment message. */
            msgAck = pvPortMalloc( sizeof( coap_packet_t ) );
            /* Initialize the acknowledgment message (ACK) with a status of "Changed 2.04". */
            coap_init_message( msgAck, COAP_TYPE_ACK, CHANGED_2_04, message->mid );
            /* Set the token of the acknowledgment message based on the received message. */
            coap_set_header_token( msgAck, message->token, message->token_len );
            /* Get the size of the serialized acknowledgment message. */
            msgAckBuffer_len = coap_serialize_get_size( msgAck );

            /* Check if the buffer size calculation failed. */
            if( msgAckBuffer_len == 0 )
            {
                IotLogError( "Buffer could not be serialized" );
            }

            /* Allocate memory for the serialized acknowledgment message buffer. */
            msgAckBuffer = ( uint8_t * ) pvPortMalloc( msgAckBuffer_len );

            /* Check if memory allocation for the message buffer failed. */
            if( msgAckBuffer == NULL )
            {
                IotLogError( "Buffer could not allocate memory" );
            }

            /* Serialize the acknowledgment message into the buffer. */
            msgAckBuffer_len = coap_serialize_message( msgAck, msgAckBuffer );

            /* Check if serialization of the acknowledgment message failed. */
            if( msgAckBuffer_len == 0 )
            {
                /* Free the allocated buffer and reset the pointer if serialization failed. */
                vPortFree( msgAckBuffer );
                msgAckBuffer = NULL;
                IotLogError( "Failed to serialize acknowledgment message into the buffer. Buffer has been freed." );
            }

            IotLogInfo( "Send Acknowledgement \r\n" );
            /* Send the acknowledgment message back to the sender via the socket. */
            status_ack = SOCKETS_Send( pCallbackContext, msgAckBuffer, MAX_SIZE_EMPTY_ACK, NULL );
        }
    }

    /**
     * @brief Task function to send CoAP messages to a server over UDP.
     *
     * This function establishes a UDP connection to a CoAP server, constructs and sends
     * CoAP messages, and handles potential DTLS encryption if enabled. It runs
     * indefinitely, sending messages at regular intervals defined by
     * CONFIG_COAP_DATA_UPLOAD_FREQUENCY_SECONDS.
     *
     * Function Flow:
     * - Performs DNS lookup for the CoAP server address.
     * - Creates a UDP socket and configures the necessary options, such as timeouts
     *   for sending and receiving.
     * - Establishes a connection to the server using `SOCKETS_Connect()`.
     * - Constructs a CoAP message with the POST method and sets the URI query path
     *   and payload.
     * - Serializes the CoAP message into a buffer and sends it via the UDP socket.
     * - Logs the message for debugging purposes.
     * - The task pauses between sending messages based on a configured frequency.
     *
     * The function supports DTLS (if enabled) by configuring the socket with
     * the appropriate security settings.
     */
    void SendCoAPData()
    {
        /* Initialize a CoAP packet structure */
        coap_packet_t request_packet;
        SocketsSockaddr_t ServerAddress; /* Structure to hold server's address and port */
        Socket_t sockfd;                 /* Socket handle for sending data */

        /* Clear the CoAP packet structure */

        memset( &request_packet, 0, sizeof( coap_packet_t ) );

        IotLogInfo( "Connecting to the CoAP server\r\n" );
        /* Set server address and port (convert port to network byte order) */

        ServerAddress.usPort = SOCKETS_htons( CONFIG_COAP_SERVER_PORT );
        ServerAddress.ulAddress = SOCKETS_GetHostByName( CONFIG_COAP_SERVER_ADDRESS );

        /* Check for errors from DNS lookup. */
        if( ServerAddress.ulAddress == ( uint32_t ) 0 )
        {
            IotLogError( "Failed to connect to server: DNS resolution failed: Server=%s.",
                         CONFIG_COAP_SERVER_ADDRESS );
            vTaskDelete( NULL ); /* Delete task if DNS lookup fails */
            return;
        }
        else
        {
            /* Create UDP Socket */
            sockfd = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );
            /* If DTLS is enabled, set the socket option to require TLS */

            #ifdef ENABLE_DTLS
                SOCKETS_SetSockOpt( sockfd, 0, SOCKETS_SO_REQUIRE_TLS, NULL, ( size_t ) 0 );
                IotLogDebug( "DTLS Socket." );
            #endif
            /* Ensure the socket was created successfully */

            configASSERT( sockfd != SOCKETS_INVALID_SOCKET );

            /* Set socket options for receive and send timeouts to prevent indefinite blocking */

            SOCKETS_SetSockOpt( sockfd, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
            SOCKETS_SetSockOpt( sockfd, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

            /* Connect to the CoAP server using the resolved server address */
            IotLogInfo( "Connecting to CoAP server %s:%u\r\n", IP_TO_STRING( ServerAddress.ulAddress ), SOCKETS_ntohs( ServerAddress.usPort ) );
            int result = SOCKETS_Connect( sockfd, &ServerAddress, sizeof( ServerAddress ) );
            /* Check if connection to the server was successful */

            if( result != 0 )
            {
                IotLogError( "Failed to connect to CoAP server %s.\r\n",
                             CONFIG_COAP_SERVER_ADDRESS );
                vTaskDelete( NULL ); /* Delete task if connection fails */
                return;
            }
        }

        /* Infinite loop to send CoAP messages periodically */

        while( 1 )
        {
            IotLogInfo( "Connected to CoAP server %s:%u\r\n", IP_TO_STRING( ServerAddress.ulAddress ), SOCKETS_ntohs( ServerAddress.usPort ) );

            /* Initialize the CoAP message as confirmable (CON) with a POST method */
            memset( &request_packet, 0, sizeof( coap_packet_t ) );
            coap_init_message( &request_packet, COAP_TYPE_CON, COAP_POST, coap_get_mid() );

            /* Set the URI Query path for the message (e.g., "t=test") */
            coap_set_header_uri_query( &request_packet, CONFIG_COAP_URI_QUERY );
            #if defined( CONFIG_NCE_ENERGY_SAVER )
                /* Buffer to store the transmitted bytes, initialized with a size of 50 bytes. */

                char pcTransmittedString[ 50 ];
                /* Initialize the buffer to all null ('\0') characters. This ensures the buffer is empty and ready for use. */
                ( void ) memset( &pcTransmittedString, ( uint8_t ) '\0', sizeof( pcTransmittedString ) );
                uint8_t selector = 1;

                /* Initialize data elements to be transmitted:
                 * - battery_level: Integer value representing battery percentage (99%).
                 * - signal_strength: Integer value representing signal strength (84 dBm).
                 * - software_version: String representing the software version ("2.2.1").
                 * Each element is packed into an Element2byte_gen_t structure, which includes its type, value, and template length.
                 */
                Element2byte_gen_t battery_level = { .type = E_INTEGER, .value.i = 99, .template_length = 1 };
                Element2byte_gen_t signal_strength = { .type = E_INTEGER, .value.i = 84, .template_length = 1 };
                Element2byte_gen_t software_version = { .type = E_STRING, .value.s = "2.2.1", .template_length = 5 };
                os_energy_save( pcTransmittedString, selector, 3, battery_level, signal_strength, software_version );
                coap_set_payload( &request_packet, pcTransmittedString, strlen( pcTransmittedString ) );
            #else /* if defined( CONFIG_NCE_ENERGY_SAVER ) */
                /* Set the payload for the CoAP message */
                coap_set_payload( &request_packet, PUBLISH_PAYLOAD_FORMAT, strlen( PUBLISH_PAYLOAD_FORMAT ) );
            #endif /* if defined( CONFIG_NCE_ENERGY_SAVER ) */
            /* Buffer to hold the serialized CoAP message */
            uint8_t buffer[ 256 ];
            /* Serialize the CoAP message into the buffer */

            size_t message_size = coap_serialize_message( &request_packet, buffer );
            /* Send the serialized CoAP message via the UDP socket */
            int32_t SendVal = SOCKETS_Send( sockfd, buffer, message_size, 0 );
            /* Check if the message was sent successfully */

            if( SendVal < 0 )
            {
                IotLogError( "Failed to send to CoAP server\r\n" );
            }
            else
            {
                IotLogInfo( "Sent CoAP message to CoAP server\r\n" );
            }

            /* Log the serialized message for debugging purposes */
            IotLogDebug( "Serialized CoAP message:\n" );

            for(size_t i = 0; i < message_size; ++i)
            {
                IotLogDebug( "%02x ", buffer[ i ] );
            }

            /* Delay the task for a specified interval (in seconds) before sending the next message */

            vTaskDelay( pdMS_TO_TICKS( CONFIG_COAP_DATA_UPLOAD_FREQUENCY_SECONDS * 1000 ) );
        }
    }

    /**
     * @brief Task function to listen for and process incoming CoAP messages over UDP.
     *
     * This function sets up a UDP socket to listen for incoming CoAP messages on a
     * specified port. It continuously waits for data to be received on the socket,
     * processes the received CoAP messages, and handles any necessary acknowledgments.
     *
     * Function Flow:
     * - Initializes a UDP socket using the IPv4 protocol.
     * - Binds the socket to the specified local address and port.
     * - Waits for an event that signals data has been received using
     *   `xEventGroupWaitBits()`.
     * - Calls a data-ready callback function to process the received CoAP messages.
     * - Introduces a delay between consecutive data reception loops to prevent
     *   excessive CPU usage.
     */
    void ListenCoAPData()
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
            CoAPDataReadyCallback( listenSocket );

            /* Introduce a delay before the next iteration to prevent excessive CPU usage. */
            vTaskDelay( DEFAULT_TASK_DELAY_MS );
            IotLogDebug( "Waiting for incoming messages" );
        }
    }

    /**
     * @brief Demo function to initialize and manage CoAP communication tasks.
     *
     * This function sets up a FreeRTOS queue and two tasks for CoAP communication:
     * one for sending CoAP messages (`SendCoAPData`) and another for listening
     * and receiving CoAP messages (`ListenCoAPData`). The function creates the
     * necessary queue and tasks, handles error checks, and logs status updates.
     * Once tasks are created successfully, it enters an infinite loop to keep
     * the tasks running.
     */
    void CoAPDemo()
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

        /* Create the task for sending CoAP messages. */
        xTaskCreateStatus = xTaskCreate(
            SendCoAPData,             /* Task function. */
            "SendCoAPData",           /* Task name. */
            THREAD_STACK_SIZE,        /* Stack size in bytes. */
            NULL,                     /* Task parameter (none in this case). */
            configMAX_PRIORITIES - 1, /* Task priority: highest priority. */
            NULL                      /* Task handle (not used). */
            );

        /* Check if the task was created successfully. */
        if( xTaskCreateStatus != pdPASS )
        {
            IotLogError( "Failed to create the SendCoAPData task.\r\n" );
            return; /* Exit the function if task creation fails. */
        }

        /* Log message indicating successful task creation. */
        IotLogDebug( "SendCoAPData task created successfully.\r\n" );

        /* Create the task for listening and receiving CoAP messages. */
        xTaskCreateStatus = xTaskCreate(
            ListenCoAPData,           /* Task function. */
            "ListenCoAPData",         /* Task name. */
            THREAD_STACK_SIZE,        /* Stack size in bytes. */
            NULL,                     /* Task parameter (none in this case). */
            configMAX_PRIORITIES - 1, /* Task priority: highest priority. */
            NULL                      /* Task handle (not used). */
            );

        /* Check if the task was created successfully. */
        if( xTaskCreateStatus != pdPASS )
        {
            IotLogError( "Failed to create the ListenCoAPData task.\r\n" );
            return; /* Exit the function if task creation fails. */
        }

        /* Log message indicating successful task creation. */
        IotLogDebug( "ListenCoAPData task created successfully.\r\n" );

        /* Infinite loop to keep the demo running and tasks active. */
        while( 1 )
        {
            /* The function does not exit, keeping the tasks running. */
        }
    }
#endif /* ifdef CONFIG_COAP_DEMO_ENABLED */
