/*
 * nce_onboarding.h
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */


#include "nce_bg96_configuration.h"
extern bool DEVICE_ONBOARDED;
/* Exported types ------------------------------------------------------------*/
typedef enum
{
    ONBOARDING_OK = 0,               /* No error */
    ONBOARDING_ERROR,                /* Generic error */
} Onboarding_Status_t;


/* Exported functions ------------------------------------------------------- */
Onboarding_Status_t nce_onboard_device( void );
CellularPktStatus_t nce_send_sms( char * smsTroubleshooting );
/* External variables --------------------------------------------------------*/
#if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED )
    #define TOPIC_FILTER_COUNT    ( 1 )
    extern char IOT_DEMO_MQTT_TOPIC_PREFIX[ 19 ];
    extern char MQTT_TOPIC[ 30 ];
    extern char WILL_TOPIC_NAME[ 35 ];
    extern char ACKNOWLEDGEMENT_TOPIC_NAME[ 40 ];
    extern uint16_t WILL_TOPIC_NAME_LENGTH;
    extern uint16_t TOPIC_FILTER_LENGTH;
    extern char sim_iccid[ 30 ];
    extern char identifier[ 25 ];
    extern char iotCoreEndpointUrl[ 50 ];
    extern const char * pTopics[ TOPIC_FILTER_COUNT ];
#endif /* if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED ) */
#if defined( ENABLE_DTLS )
    extern uint8_t PSK[ 500 ];
    extern uint8_t psk_identity[ 20 ];
#endif
