/*
 * MQTT Broker endpoint.
 */
//static const char clientcredentialMQTT_BROKER_ENDPOINT[] = "a1to42vigz595n-ats.iot.eu-central-1.amazonaws.com";
#define ONBOARDING_ENDPOINT  "device.dev.connectivity-suite.cloud"
#define PUBLISH_PAYLOAD_FORMAT                   "Hello world !"
#define SUB_TOPIC   "/1nce_test"


/* Use of a "define" and not a "static const" here to be able to
* use pre-compile concatenation on the string. */
//#define clientcredentialIOT_THING_NAME "freertos_nbiot"

/*
 * Port number the MQTT broker is using.
 */
#define clientcredentialMQTT_BROKER_PORT 8883
