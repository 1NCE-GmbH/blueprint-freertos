/*
 * nce_onboarding.h
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */


#include "nce_bg96_configuration.h"
#include "cellular_service_os.h"


 /* Exported types ------------------------------------------------------------*/
 typedef enum
 {
   ONBOARDING_OK = 0,                /* No error */
   ONBOARDING_ERROR,                 /* Generic error */
 } Onboarding_Status_t;


/* Exported functions ------------------------------------------------------- */

static void nce_prepare_and_upload_certificates(uint8_t *all);
static void nce_dtls_psk(uint8_t *complete_response);
Onboarding_Status_t nce_onboard_device(void);
void waitforprocessing(int n);

/* External variables --------------------------------------------------------*/
 #define TOPIC_FILTER_COUNT                       ( 1 )
 extern char IOT_DEMO_MQTT_TOPIC_PREFIX[19];
 extern char WILL_TOPIC_NAME[35];
 extern char ACKNOWLEDGEMENT_TOPIC_NAME[40];
 extern uint16_t WILL_TOPIC_NAME_LENGTH ;
 extern uint16_t TOPIC_FILTER_LENGTH ;
 extern char sim_iccid[30]  ;
 extern char identifier[25] ;
extern char	iotCoreEndpointUrl[50] ;
extern uint8_t PSK[500];
extern uint8_t psk_identity[20];
// extern char iccid[30], identifier[25], iotCoreEndpointUrl[50];
 extern const char *pTopics[TOPIC_FILTER_COUNT];



