/*
 * nce_demo_config.h
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "iot_demo_runner.h"

#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define democonfigCLIENT_ICCID "<ICCID>"

/* C2D Parameters */
#define NCE_RECV_PORT 3000
#define NCE_RECEIVE_BUFFER_SIZE 200

/* UDP Configuration */
#if defined(CONFIG_UDP_DEMO_ENABLED)
#define UDP_ENDPOINT "udp.os.1nce.com"
#define UDP_PORT 4445
#define CONFIG_NCE_ENERGY_SAVER

#endif
/* COAP Configuration with/without DTLS */
#if defined(CONFIG_COAP_DEMO_ENABLED)
#define COAP_ENDPOINT           "coap.os.1nce.com"
#define configCOAP_PORT         5683
#define democonfigCLIENT_IDENTIFIER    "t=test"
/*#define CONFIG_NCE_ENERGY_SAVER*/
#if ( configCOAP_PORT == 5684 )
	#define ENABLE_DTLS
#endif
/* Enable send the Information to 1NCE's client support */
/*#define TROUBLESHOOTING*/
#if  defined( TROUBLESHOOTING ) && ( configCOAP_PORT == 5684 )
    #ifndef ENABLE_DTLS
        #define ENABLE_DTLS
    #endif
#endif
#endif
/* LWM2M Configuration */
#if defined(CONFIG_LwM2M_DEMO_ENABLED)
#define LWM2M_ENDPOINT    "lwm2m.os.1nce.com"
#define ENABLE_DTLS
#define LWM2M_CLIENT_MODE
#define LWM2M_BOOTSTRAP
#ifdef ENABLE_DTLS
char lwm2m_psk[30];
char lwm2m_psk_id[30];
#endif
#define LWM2M_SUPPORT_SENML_JSON
#define LWM2M_LITTLE_ENDIAN
#define LWM2M_SUPPORT_TLV
#define LWM2M_COAP_DEFAULT_BLOCK_SIZE 1024
#define LWM2M_VERSION_1_1
#define LWM2M_SINGLE_SERVER_REGISTERATION
//#define LWM2M_PASSIVE_REPORTING
#if defined(LWM2M_PASSIVE_REPORTING)
#define LWM2M_1NCE_LIFETIME   30000
#else
	#define LWM2M_OBJECT_SEND "/4/0"
#endif


#endif


