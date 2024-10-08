/*
 *  lwm2m_demo.c
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

/**
 * @file Lwm2m_demo.c
 * @brief
 */
#include "lwm2mclient.h"
#include "nce_demo_config.h"
#if defined( CONFIG_LwM2M_DEMO_ENABLED )
/* The config header is always included first. */
    #include "iot_config.h"
    #include "cellular_app.h"
    #include "Lwm2m_demo.h"
    #include "liblwm2m.h"
/* COAP include. */
    #include "connection.h"
/*-----------------------------------------------------------*/

    #define MAX_PACKET_SIZE    2048

    static int g_quit = 0;
    bool DEVICE_BOOTSTRAPPED = false;

    #define OBJ_COUNT    4
    lwm2m_object_t * objArray[ OBJ_COUNT ];

/* only backup security and server objects */
    # define BACKUP_OBJECT_COUNT    2
/* lwm2m_object_t * backupObjectArray[BACKUP_OBJECT_COUNT]; */

    typedef struct
    {
        lwm2m_object_t * securityObjP;
        lwm2m_object_t * serverObject;
        Socket_t sock;
        connection_t * connList;
        int addressFamily;
    } client_data_t;


/*-----------------------------------------------------------*/
    static void update_battery_level( lwm2m_context_t * context )
    {
        static time_t next_change_time = 0;
        time_t tv_sec;

        tv_sec = lwm2m_gettime();

        if( tv_sec < 0 )
        {
            return;
        }

        if( next_change_time < tv_sec )
        {
            char value[ 15 ];
            int valueLength;
            lwm2m_uri_t uri;
            int level = rand() % 100;

            if( 0 > level )
            {
                level = -level;
            }

            if( lwm2m_stringToUri( "/3/0/9", 6, &uri ) )
            {
                valueLength = sprintf( value, "%d", level );
                IotLogInfo( "New Battery Level: %d\n", level );
                handle_value_changed( context, &uri, value, valueLength );
            }

            level = rand() % 20;

            if( 0 > level )
            {
                level = -level;
            }

            next_change_time = tv_sec + level + 10;
        }
    }

    void Wakaama_Recv( client_data_t data,
                       Socket_t socketHandle,
                       lwm2m_context_t * lwm2mH,
                       uint8_t * buffer,
                       void * fromSessionH )
    {
        /* Clear the buffer into which the echoed string will be
         * placed. */
        BaseType_t xReturned;

        /* Unused parameters. */
        ( void ) data;

        xReturned = SOCKETS_Recv( socketHandle, /* The socket being received from. */
                                  buffer,       /* The buffer into which the received data will be written. */
                                  1500,         /* The size of the buffer provided to receive the data. */
                                  0 );

        if( xReturned < 0 )
        {
            /* Error occurred.  Latch it so it can be detected
             * below. */
            IotLogError( "ERROR Receiving from CoAP server \r\n" );
            return;
        }
        else if( xReturned == 0 )
        {
            /* Timed out. */
            IotLogInfo( "Timed out receiving from CoAP server\r\n" );
            return;
        }

        lwm2m_handle_packet( lwm2mH, buffer, xReturned, fromSessionH );
    }

/*-----------------------------------------------------------*/



/**
 * @brief LwM2MDemo function initializes and starts an LwM2M client.
 *
 * This function sets up an LwM2M client by initializing required security, server, device,
 * and firmware objects. It handles communications with an LwM2M server, including registering,
 * bootstrapping, and sending periodic updates.
 *
 * The client runs in a loop, processing incoming requests and managing its state.
 *
 * @note Depending on configuration, the client supports both DTLS-enabled and non-DTLS communications.
 */
    void LwM2MDemo()
    {
        /* Client data structure and return value. */
        client_data_t data;
        int result;
        lwm2m_context_t * lwm2mH = NULL;
        const char * localPort = "56830";
        const char * server = LWM2M_ENDPOINT;
        char * name = CONFIG_NCE_ICCID;
        int lifetime = 180000;
        int batterylevelchanging = 0;
        time_t reboot_time = 0;
        lwm2m_client_state_t previousState = STATE_INITIAL;

        #if defined( ENABLE_DTLS )
            const char * serverPort = LWM2M_DTLS_PORT_STR;
            char * pskId = CONFIG_NCE_ICCID;
            char * pskBuffer = CONFIG_LWM2M_BOOTSTRAP_PSK;
        #else
            const char * serverPort = LWM2M_BSSERVER_PORT_STR;
            char * pskId = NULL;
            char * pskBuffer = NULL;
        #endif
        uint16_t pskLen = strlen( pskBuffer );
        /* Initialize client data structure to zero. */
        memset( &data, 0, sizeof( client_data_t ) );
        data.addressFamily = SOCKETS_AF_INET;

        /* Bind LwM2M client to the specified port. */
        IotLogInfo( "Trying to bind LWM2M Client to port %s\r\n", localPort );
        data.sock = create_socket( localPort, data.addressFamily );
        configASSERT( data.sock != SOCKETS_INVALID_SOCKET );
        /* Construct the server URI. */
        char serverUri[ 50 ];
        sprintf( serverUri, "coap://%s:%s", server, serverPort );
        /* Initialize security object depending on DTLS configuration. */
        int serverId = 0;
        #if defined( ENABLE_DTLS )
            objArray[ 0 ] = get_security_object( serverId, serverUri, pskId, pskBuffer, pskLen, true );
        #else
            objArray[ 0 ] = get_security_object( serverId, serverUri, NULL, NULL, 0, true );
        #endif

        if( NULL == objArray[ 0 ] )
        {
            IotLogError( "Failed to create security object\r\n" );
        }

        data.securityObjP = objArray[ 0 ];
        /* Initialize server, device, and firmware objects. */

        objArray[ 1 ] = get_server_object( serverId, "U", lifetime, false );

        if( NULL == objArray[ 1 ] )
        {
            IotLogError( "Failed to create server object\r\n" );
        }

        objArray[ 2 ] = get_object_device();

        if( NULL == objArray[ 2 ] )
        {
            IotLogError( "Failed to create Device object\r\n" );
        }

        objArray[ 3 ] = get_object_firmware();

        if( NULL == objArray[ 3 ] )
        {
            IotLogError( "Failed to create Firmware object\r\n" );
        }
        /*
         * The liblwm2m library is now initialized with the functions that will be in
         * charge of communication
         */
        lwm2mH = lwm2m_init( &data );

        if( NULL == lwm2mH )
        {
            IotLogError( "lwm2m_init() failed\r\n" );
        }

        /* Configure the LwM2M client with its unique name and object array. */

        result = lwm2m_configure( lwm2mH, name, NULL, NULL, OBJ_COUNT, objArray );

        if( result != 0 )
        {
            IotLogError( "lwm2m_configure() failed: 0x%X\r\n", result );
        }

        /* Initialize value change callback. */
        init_value_change( lwm2mH );
        IotLogInfo( "LWM2M Client \"%s\" started on port %s\r\n", name, localPort );

        /* Main loop to handle client-server communication. */

        while( 0 == g_quit )
        {
            struct timeval tv;
            /* Handle client rebooting. */

            if( g_reboot )
            {
                time_t tv_sec;

                tv_sec = lwm2m_gettime();

                if( 0 == reboot_time )
                {
                    reboot_time = tv_sec + 5;
                }

                if( reboot_time < tv_sec )
                {
                    /*
                     * Message should normally be lost with reboot ...
                     */
                    IotLogInfo( "reboot time expired, rebooting ..." );
                    system_reboot();
                }
                else
                {
                    tv.tv_sec = reboot_time - tv_sec;
                }
            }
            /* Update battery level if necessary. */

            else if( batterylevelchanging )
            {
                update_battery_level( lwm2mH );
                tv.tv_sec = 5;
            }
            /* Default timeout value for lwm2m_step. */
            else
            {
                tv.tv_sec = 60;
            }

            tv.tv_usec = 0;


            /*
             * This function does two things:
             *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
             *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
             *    (eg. retransmission) and the time between the next operation
             */
            result = lwm2m_step( lwm2mH, &( tv.tv_sec ) );
            IotLogInfo( " -> State: " );
            /* Log client state. */

            switch( lwm2mH->state )
            {
                case STATE_INITIAL:
                    IotLogInfo( "STATE_INITIAL\r\n" );
                    break;

                case STATE_BOOTSTRAP_REQUIRED:
                    IotLogInfo( "STATE_BOOTSTRAP_REQUIRED\r\n" );
                    break;

                case STATE_BOOTSTRAPPING:
                    IotLogInfo( "STATE_BOOTSTRAPPING\r\n" );
                    break;

                case STATE_REGISTER_REQUIRED:
                    IotLogInfo( "STATE_REGISTER_REQUIRED\r\n" );
                    break;

                case STATE_REGISTERING:
                    IotLogInfo( "STATE_REGISTERING\r\n" );
                    break;

                case STATE_READY:
                    IotLogInfo( "STATE_READY\r\n" );
                    break;

                default:
                    IotLogInfo( "Unknown...\r\n" );
                    break;
            }

            /* Handle errors in lwm2m_step. */

            if( result != 0 )
            {
                IotLogInfo( "lwm2m_step() failed: 0x%X\r\n", result );

                if( previousState == STATE_BOOTSTRAPPING )
                {
                    #ifdef LWM2M_WITH_LOGS
                        IotLogInfo( "[BOOTSTRAP] restore security and server objects\r\n" );
                    #endif
                    lwm2mH->state = STATE_INITIAL;
                }
                else
                {
                    IotLogError( "[BOOTSTRAP] Error restore security and server objects\r\n" );
                }
            }

            #ifdef LWM2M_BOOTSTRAP
                update_bootstrap_info( &previousState, lwm2mH );
            #endif
            /* Receive and process incoming packets. */
            uint8_t buffer[ MAX_PACKET_SIZE ];
            memset( buffer, '\0', MAX_PACKET_SIZE );

            if( lwm2mH->serverList != NULL )
            {
                /* Receive from the primary server. */

                connection_t * connP = ( connection_t * ) lwm2mH->serverList->sessionH;

                if( ( ( lwm2mH->state == STATE_REGISTERING ) || ( lwm2mH->state == STATE_READY ) ) && ( connP != NULL ) )
                {
                    Wakaama_Recv( data, connP->sock, lwm2mH, buffer, connP );

                    /* Send device and connectivity monitoring objects if ready. */

                    #if defined( LWM2M_OBJECT_SEND )
                        if( ( lwm2mH->state == STATE_READY ) )
                        {
                            IotLogInfo( "************** U P D A T I N G  ************** \r\n" );
                            /* Define the object, instance, and resource to be sent */
                            lwm2m_uri_t uri;
                            lwm2m_stringToUri( LWM2M_OBJECT_SEND, strlen( LWM2M_OBJECT_SEND ), &uri );

                            /* Sending the device and connectivity monitoring objects to the LwM2M server */
                            int result = lwm2m_send( lwm2mH, LWM2M_OBJECT_SEND, sizeof(LWM2M_OBJECT_SEND));

                            if( result == 0 )
                            {
                                if( LWM2M_URI_IS_SET_INSTANCE( &uri ) )
                                {
                                    if( LWM2M_URI_IS_SET_RESOURCE( &uri ) )
                                    {
                                        IotLogInfo( "Sent LwM2M Object: /%d/%d/%d", uri.objectId, uri.instanceId, uri.resourceId );
                                    }
                                    else
                                    {
                                        IotLogInfo( "Sent LwM2M Object: /%d/%d", uri.objectId, uri.instanceId );
                                    }
                                }
                                else
                                {
                                    IotLogInfo( "Sent LwM2M Object: /%d", uri.objectId );
                                }
                            }
                            else
                            {
                                IotLogError( "Failed to send LwM2M object %s\n", LWM2M_OBJECT_SEND );
                            }

                            IotLogInfo( "************** U P D A T E D  ************** \r\n" );
                            /* Delay to avoid spamming server with updates. */
                            vTaskDelay( pdMS_TO_TICKS( CONFIG_LWM2M_SEND_FREQUENCY_SECONDS * 1000 ) );
                        }
                    #endif /* if defined( LWM2M_OBJECT_SEND ) */
                }

                /* Receive data from the LwM2M Server. */
                connection_t * connP1 = ( connection_t * ) lwm2mH->serverList->next->sessionH;

                if( ( ( lwm2mH->state == STATE_REGISTERING ) || ( lwm2mH->state == STATE_READY ) ) && ( connP1 != NULL ) )
                {
                    Wakaama_Recv( data, connP1->sock, lwm2mH, buffer, connP1 );
                }
            }
            else
            {
                /* Handle bootstrap server communication. */
                connection_t * connP = ( connection_t * ) lwm2mH->bootstrapServerList->sessionH;

                if( ( lwm2mH->bootstrapServerList->status != STATE_BS_FINISHED ) && ( connP != NULL ) )
                {
                    Wakaama_Recv( data, connP->sock, lwm2mH, buffer, connP );
                }
            }
        }
    }

/*-----------------------------------------------------------*/
#endif /* if defined( CONFIG_LwM2M_DEMO_ENABLED ) */
