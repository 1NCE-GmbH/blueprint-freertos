/*
 * /*
 *  Created on: June 10, 2021
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 *
 */

/**
 * @file iot_demo_coap.c
 * @brief Demonstrates usage of lobaro COAP library.
 */
#include "nce_demo_config.h"
#if defined(CONFIG_LwM2M_DEMO_ENABLED)
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
#include "lwm2mclient.h"
//#include "nce_onboarding.h"

#include "connection.h"
/*-----------------------------------------------------------*/

#define MAX_PACKET_SIZE 2048
#define DEFAULT_SERVER_IPV6 "[::1]"
#define DEFAULT_SERVER_IPV4 "35.158.0.92"

int g_reboot = 0;
static int g_quit = 0;
bool DEVICE_BOOTSTRAPPED = false;

#define OBJ_COUNT 9
lwm2m_object_t * objArray[OBJ_COUNT];

// only backup security and server objects
# define BACKUP_OBJECT_COUNT 2
lwm2m_object_t * backupObjectArray[BACKUP_OBJECT_COUNT];

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    Socket_t sock;
    connection_t * connList;
    int addressFamily;
} client_data_t;


/* Declaration of demo function. */
int Runlwm2mDemo( bool awsIotMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface );

/*-----------------------------------------------------------*/
static void update_battery_level(lwm2m_context_t * context)
{
    static time_t next_change_time = 0;
    time_t tv_sec;

    tv_sec = lwm2m_gettime();
    if (tv_sec < 0) return;

    if (next_change_time < tv_sec)
    {
        char value[15];
        int valueLength;
        lwm2m_uri_t uri;
        int level = rand() % 100;

        if (0 > level) level = -level;
        if (lwm2m_stringToUri("/3/0/9", 6, &uri))
        {
            valueLength = sprintf(value, "%d", level);
            IotLogInfo("New Battery Level: %d\n", level);
            handle_value_changed(context, &uri, value, valueLength);
        }
        level = rand() % 20;
        if (0 > level) level = -level;
        next_change_time = tv_sec + level + 10;
    }
}

void Wakaama_Recv(client_data_t data,Socket_t socketHandle, lwm2m_context_t *lwm2mH,uint8_t * buffer,void * fromSessionH){
	/* Clear the buffer into which the echoed string will be
			 * placed. */
			BaseType_t xReturned;
			xReturned = SOCKETS_Recv(socketHandle, /* The socket being received from. */
			buffer, /* The buffer into which the received data will be written. */
			1500, /* The size of the buffer provided to receive the data. */
			0);
			if (xReturned < 0) {
				/* Error occurred.  Latch it so it can be detected
				 * below. */
				IotLogError( "ERROR Receiving from CoAP server \r\n" );
				return;
			} else if (xReturned == 0) {
				/* Timed out. */
				IotLogInfo( "Timed out receiving from CoAP server\r\n" );
				return;
			}
			lwm2m_handle_packet(lwm2mH, buffer, xReturned, fromSessionH);
}

/*-----------------------------------------------------------*/



/**
 * @brief The function that runs the COAP demo, called by the demo runner.
 * @return `EXIT_SUCCESS` if the demo completes successfully; `EXIT_FAILURE` otherwise.
 */
int Runlwm2mDemo( bool awsIotMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface )
{
    /* Return value of this function and the exit status of this program. */
    int status = EXIT_FAILURE;
    client_data_t data;
    int result;
    lwm2m_context_t * lwm2mH = NULL;
    const char * localPort = "56830";
    const char * server = LWM2M_ENDPOINT;
    char * name = democonfigCLIENT_ICCID;
    int lifetime = 180000;
    int batterylevelchanging = 0;
    time_t reboot_time = 0;
    int opt;
    bool bootstrapRequested = false;
    bool serverPortChanged = false;
    lwm2m_client_state_t previousState = STATE_INITIAL;

	#ifdef LWM2M_BOOTSTRAP
		const char * serverPort = LWM2M_BSSERVER_PORT_STR;
		char * pskId = NULL;
		char * pskBuffer=  NULL;
	#else
		const char * serverPort = LWM2M_DTLS_PORT_STR;
	    char * pskId = "1nce_freertos_test";
	    char * pskBuffer =  "***PSK***";
	#endif
    uint16_t pskLen = strlen(pskBuffer);
    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = SOCKETS_AF_INET;
    if (!server)
        {
            server = (SOCKETS_AF_INET == data.addressFamily ? DEFAULT_SERVER_IPV4 : DEFAULT_SERVER_IPV6);
        }
    IotLogInfo("Trying to bind LWM2M Client to port %s\r\n", localPort );
    data.sock = create_socket(localPort, data.addressFamily);
    configASSERT( data.sock != SOCKETS_INVALID_SOCKET );
    char serverUri[50];
    sprintf (serverUri, "coap://%s:%s", server, serverPort);
	#ifdef LWM2M_BOOTSTRAP
		int serverId = 0;
		objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, true);
	#else
		int serverId = 123;
		objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
	#endif
    if (NULL == objArray[0])
    {
    	IotLogError( "Failed to create security object\r\n");
    	status = EXIT_FAILURE;
    }

    data.securityObjP = objArray[0];

    objArray[1] = get_server_object(serverId, "U", lifetime, false);

    if (NULL == objArray[1])
        {
    		IotLogError("Failed to create server object\r\n");
    		status = EXIT_FAILURE;
        }

        objArray[2] = get_object_device();
        if (NULL == objArray[2])
        {
        	IotLogError("Failed to create Device object\r\n");
        	status = EXIT_FAILURE;
        }

        objArray[3] = get_object_firmware();
        if (NULL == objArray[3])
        {
        	IotLogError( "Failed to create Firmware object\r\n");
        	status = EXIT_FAILURE;
        }

        objArray[4] = get_object_location();
        if (NULL == objArray[4])
        {
        	IotLogError( "Failed to create location object\r\n");
        	status = EXIT_FAILURE;
        }

        objArray[5] = get_test_object();
        if (NULL == objArray[5])
        {
        	IotLogError( "Failed to create test object\r\n");
        	status = EXIT_FAILURE;
        }

        objArray[6] = get_object_conn_m();
        if (NULL == objArray[6])
        {
        	IotLogError("Failed to create connectivity monitoring object\r\n");
        	status = EXIT_FAILURE;
        }

        objArray[7] = get_object_conn_s();
        if (NULL == objArray[7])
        {
        	IotLogError("Failed to create connectivity statistics object\r\n");
        	status = EXIT_FAILURE;
        }

        int instId = 0;
        objArray[8] = acc_ctrl_create_object();
        if (NULL == objArray[8])
        {
        	IotLogError("Failed to create Access Control object\r\n");
        	status = EXIT_FAILURE;
        }
        else if (acc_ctrl_obj_add_inst(objArray[8], instId, 3, 0, serverId)==false)
        {
        	IotLogError("Failed to create Access Control object instance\r\n");
        	status = EXIT_FAILURE;
        }
        else if (acc_ctrl_oi_add_ac_val(objArray[8], instId, 0, 0xF /* == 0b000000000001111 */)==false)
        {
        	IotLogError("Failed to create Access Control ACL default resource\r\n");
        	status = EXIT_FAILURE;
        }
        else if (acc_ctrl_oi_add_ac_val(objArray[8], instId, 999, 0x1 /* == 0b000000000000001 */)==false)
        {
        	IotLogError("Failed to create Access Control ACL resource for serverId: 999\r\n");
        	status = EXIT_FAILURE;
        }
        /*
         * The liblwm2m library is now initialized with the functions that will be in
         * charge of communication
         */
        lwm2mH = lwm2m_init(&data);
        if (NULL == lwm2mH)
        {
        	IotLogError("lwm2m_init() failed\r\n");
        	status = EXIT_FAILURE;
        }

        /*
         * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
         * the number of objects we will be passing through and the objects array
         */
        result = lwm2m_configure(lwm2mH, name, NULL, NULL, OBJ_COUNT, objArray);
        if (result != 0)
        {
        	IotLogError("lwm2m_configure() failed: 0x%X\r\n", result);
        	status = EXIT_FAILURE;
        }
        /**
         * Initialize value changed callback.
         */
        init_value_change(lwm2mH);
        IotLogInfo("LWM2M Client \"%s\" started on port %s\r\n", name, localPort);
        IotLogInfo("> ");


        /*
         * We now enter in a while loop that will handle the communications from the server
         */
        while (0 == g_quit)
        {
            struct timeval tv;

            if (g_reboot)
            {
                time_t tv_sec;

                tv_sec = lwm2m_gettime();

                if (0 == reboot_time)
                {
                    reboot_time = tv_sec + 5;
                }
                if (reboot_time < tv_sec)
                {
                    /*
                     * Message should normally be lost with reboot ...
                     */
                	IotLogInfo("reboot time expired, rebooting ...");
                    system_reboot();
                }
                else
                {
                    tv.tv_sec = reboot_time - tv_sec;
                }
            }
            else if (batterylevelchanging)
                    {
                        update_battery_level(lwm2mH);
                        tv.tv_sec = 5;
                    }
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
                    result = lwm2m_step(lwm2mH, &(tv.tv_sec));
                    IotLogInfo(" -> State: ");
                    switch (lwm2mH->state)
                    {
                         case STATE_INITIAL:
                            IotLogInfo( "STATE_INITIAL\r\n");break;
                         case STATE_BOOTSTRAP_REQUIRED:
                           	IotLogInfo("STATE_BOOTSTRAP_REQUIRED\r\n");break;
                         case STATE_BOOTSTRAPPING:
                           	IotLogInfo("STATE_BOOTSTRAPPING\r\n");break;
                         case STATE_REGISTER_REQUIRED:
                           	IotLogInfo("STATE_REGISTER_REQUIRED\r\n");break;
                         case STATE_REGISTERING:
                           	IotLogInfo( "STATE_REGISTERING\r\n");break;
                         case STATE_READY:
                           	IotLogInfo("STATE_READY\r\n");
                           	break;
                         default:
                          	IotLogInfo("Unknown...\r\n");break;
                    }

                    if (result != 0)
                    {
                    	IotLogInfo("lwm2m_step() failed: 0x%X\r\n", result);
                        if(previousState == STATE_BOOTSTRAPPING)
                        {
#ifdef LWM2M_WITH_LOGS
                        	IotLogInfo( "[BOOTSTRAP] restore security and server objects\r\n");
#endif
                            lwm2mH->state = STATE_INITIAL;
                        }
                        else return EXIT_FAILURE;
                    }
#ifdef LWM2M_BOOTSTRAP
        update_bootstrap_info(&previousState, lwm2mH);
#endif

        uint8_t buffer[MAX_PACKET_SIZE];
        memset(buffer, '\0', MAX_PACKET_SIZE);

        if (lwm2mH->serverList != NULL)
        {
            connection_t * connP = (connection_t*) lwm2mH->serverList->sessionH;
            if((lwm2mH->state==STATE_REGISTERING || lwm2mH->state==STATE_READY) && connP != NULL ){
            	Wakaama_Recv(data,connP->sock, lwm2mH, buffer, connP);


#if defined(LWM2M_OBJECT_SEND)
	            if((lwm2mH->state==STATE_READY)){

	            	IotLogInfo("************** U P D A T I N G  ************** \r\n");

	                /* Sending the device and connectivity monitoring objects to the LwM2M server */
	            	lwm2m_send(lwm2mH,LWM2M_OBJECT_SEND ,4);

	                return EXIT_SUCCESS;

	            }
#endif
            }
            connection_t * connP1 = (connection_t*) lwm2mH->serverList->next->sessionH;
            if((lwm2mH->state==STATE_REGISTERING || lwm2mH->state==STATE_READY) && connP1 != NULL ){
            	Wakaama_Recv(data,connP1->sock, lwm2mH, buffer, connP1);
            }


        }
        else
        {
            connection_t * connP = (connection_t*) lwm2mH->bootstrapServerList->sessionH;
        	if (lwm2mH->bootstrapServerList->status != STATE_BS_FINISHED && connP != NULL){
        	Wakaama_Recv(data,connP->sock, lwm2mH, buffer, connP);
        	}
        }

        }

    return status;
}

/*-----------------------------------------------------------*/
#endif
