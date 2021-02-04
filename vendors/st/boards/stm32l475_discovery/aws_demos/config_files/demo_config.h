/*
 * demo_config.h
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

/* MQTT Configuration */ 
#define ONBOARDING_ENDPOINT  "device.connectivity-suite.cloud"
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define SUB_TOPIC   "/1nce_test"
#define clientcredentialMQTT_BROKER_PORT 8883

/* UDP Configuration */ 
#define USE_UDP
#define UDP_ENDPOINT "udp.connectivity-suite.cloud"
#define UDP_PORT 4445

/* COAP Configuration with/without DTLS */
//#define DTLS_DEMO
#define COAP
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5683
#define configCOAP_URI_PATH    "t=test"

#if  !defined(USE_UDP) && !defined(DTLS_DEMO)
#define USE_OFFLOAD_SSL
#endif
