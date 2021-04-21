/*
 * nce_demo_config.h
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "iot_demo_runner.h"

/* MQTT Configuration */ 
#define ONBOARDING_ENDPOINT  "device.connectivity-suite.cloud"
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define SUB_TOPIC "/1nce_test"
#define clientcredentialMQTT_BROKER_PORT 8883
#define democonfigRANGE_SIZE 500
/* UDP Configuration */ 
#define UDP_ENDPOINT "udp.connectivity-suite.cloud"
#define UDP_PORT 4445

/* COAP Configuration with/without DTLS */
//#define ENABLE_DTLS
#define COAP_ENDPOINT           "coap.connectivity-suite.cloud"
#define configCOAP_PORT         5683
#define configCOAP_URI_QUERY    "t=test"

/* Enable send the Information to 1NCE's client support */
#define TROUBLESHOOTING

#if  defined( TROUBLESHOOTING ) && ( configCOAP_PORT == 5684 )
    #ifndef ENABLE_DTLS
        #define ENABLE_DTLS
    #endif
#endif

#if  defined(CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED)
#define USE_OFFLOAD_SSL
#endif
