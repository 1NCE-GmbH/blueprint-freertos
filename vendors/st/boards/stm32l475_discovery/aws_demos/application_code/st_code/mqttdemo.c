/**
 ******************************************************************************
 * @file    mqttdemo.c
 * @author  MCD Application Team
 * @brief   Example of mqtt demo to interact with echo.u-blox.com
 *          using different socket protocols
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>

#include "mqttdemo.h"
#include "demo_config.h"
#include "dc_common.h"
#include "cellular_mngt.h"
#include "cellular_datacache.h"
#include "dc_time.h"
#include "cellular_service_task.h"
#include "cmsis_os_misrac2012.h"
#include "error_handler.h"
#include "plf_config.h"
#include "at_core.h"
/* Secure Sockets Include */
#include "iot_secure_sockets.h"

/* MQTT Demo Includes */
#include "iot_mqtt.h"
#include "aws_demo.h"
#include "iot_demo_runner.h"
#include "types/iot_mqtt_types.h"
#include "platform/iot_network_freertos.h"

/* Client credential includes. */
#include "aws_clientcredential.h"
#include "aws_clientcredential_keys.h"
#include "iot_default_root_certificates.h"

#include "nce_onboarding.h"
#include "nce_bg96_configuration.h"
#include "coap_main.h"
#include "at_core.h"
/* MQTT Demo Defines */
//#define <fi  clientcredentialIOT_THING_NAME
#define AWSIOT_NETWORK_TYPE_NONE     			 0x00000000
#define democonfigNETWORK_TYPES                  ( AWSIOT_NETWORK_TYPE_NONE )
#define democonfigDEMO_STACKSIZE                 ( configMINIMAL_STACK_SIZE * 8 )
#define democonfigDEMO_PRIORITY                  ( tskIDLE_PRIORITY + 5 )
#define CLIENT_IDENTIFIER_MAX_LENGTH             ( 24 )
#define KEEP_ALIVE_SECONDS                       ( 180 )
#define MQTT_TIMEOUT_MS                          ( 180000 )
#define PUBLISH_RETRY_LIMIT                      ( 10 )
#define PUBLISH_RETRY_MS                         ( 20000 )
#define ACKNOWLEDGEMENT_MESSAGE_FORMAT           "Client has received PUBLISH %.*s from server."
#define ACKNOWLEDGEMENT_MESSAGE_BUFFER_LENGTH    ( sizeof( ACKNOWLEDGEMENT_MESSAGE_FORMAT ) + 2 )
#define PUBLISH_PAYLOAD_BUFFER_LENGTH            ( sizeof( PUBLISH_PAYLOAD_FORMAT ) + 2 )
#define mqttdemo_ENABLED_SIZE_MAX  (uint8_t)(2U)
#define mqttdemo_NFM_ERROR_LIMIT_SHORT_MAX  5U
#define mqttdemo_SEND_PERIOD        20000U /* in ms. */
#define mqttdemo_MESSAGE_SIZE_MAX 1500U
char WILL_MESSAGE[] = "MQTT demo unexpectedly disconnected.";
uint16_t WILL_MESSAGE_LENGTH = sizeof(WILL_MESSAGE) - 1;
uint16_t ACKNOWLEDGEMENT_TOPIC_NAME_LENGTH = sizeof(ACKNOWLEDGEMENT_TOPIC_NAME)
		- 1;
static IotSemaphore_t demoNetworkSemaphore;

/* Private typedef -----------------------------------------------------------*/

/* Mqtt Demo socket state */
typedef enum {
	mqttdemo_SOCKET_INVALID = 0,
	mqttdemo_SOCKET_CREATED,
	mqttdemo_SOCKET_CONNECTED,
	// mqttdemo_SOCKET_SENDING,
	mqttdemo_SOCKET_WAITING_RSP,
	mqttdemo_SOCKET_CLOSING
} mqttdemo_socket_state_t;

/* Managed Protocol */
typedef enum {
	mqttdemo_SOCKET_PROTO_TCP = 0, /* create, connect, send,   recv     */
	mqttdemo_SOCKET_PROTO_UDP, /* create, connect, send,   recv     */
	mqttdemo_SOCKET_PROTO_UDP_SERVICE /* create,   NA   , sendto, recvfrom */
//  mqttdemo_SOCKET_PROTO_MAX_VALUE    /* Must be always at the end */
} mqttdemo_socket_protocol_t;

typedef struct {
	uint32_t count;
	uint32_t cnt_ok;
	uint32_t cnt_ko;
	uint32_t snd_ok;
	uint32_t snd_ko;
	uint32_t rcv_ok;
	uint32_t rcv_ko;
	uint32_t cls_ok;
	uint32_t cls_ko;
} mqttdemo_stat_t;
#include "iot_logging_setup.h"

/* Private defines -----------------------------------------------------------*/
//bool DEVICE_ONBOARDED ;
/*
 /* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static osMessageQId mqttdemo_queue;

static bool mqttdemo_socket_closing;
static mqttdemo_socket_state_t mqttdemo_socket_state;
static mqttdemo_socket_protocol_t mqttdemo_socket_protocol;
static mqttdemo_socket_protocol_t mqttdemo_socket_new_protocol;
static Socket_t mqttdemo_socket_id;

static uint8_t mqttdemo_buffer_snd[mqttdemo_MESSAGE_SIZE_MAX];
static uint16_t mqttdemo_snd_rcv_size;
static uint16_t mqttdemo_buffer_minimal_length;
static uint8_t *mqttdemo_welcome_msg;

/* NFM implementation */
/* limit nb of errors before to activate nfm */
static uint8_t mqttdemo_nfm_nb_error_limit_short;
/* current nb of errors */
static uint8_t mqttdemo_nfm_nb_error_short;
/* sleep timer index in the NFM array */
static uint8_t mqttdemo_nfm_sleep_timer_index;

/* State of MQTT_DEMO process */
static bool mqttdemo_process_flag; /* false: inactive,
 true:  active */
static bool mqttdemo_next_process_flag; /* false: inactive,
 true:  active */

/* State of Network connection */
static bool mqttdemo_network_is_on; /* false: network is down,
 true:  network is up,
 default value : see mqttdemo_init() */

/* Mqtt Demo statistic */
static mqttdemo_stat_t mqttdemo_stat;

/* Period of processing */
static uint32_t mqttdemo_period;

/* Global variables ----------------------------------------------------------*/

CS_Status_t cs_status;
static osThreadId mqttdemoTaskHandle;
UBaseType_t uxHighWaterMark;
char identifier[25];
char iotCoreEndpointUrl[50];

/* Private function prototypes -----------------------------------------------*/
/* Callback */

static void mqttdemo_notif_cb(dc_com_event_id_t dc_event_id,
		const void *private_gui_data);

static bool mqttdemo_is_nfm_sleep_requested(void);
static void mqttdemo_socket_thread(void const *argument);
/* Private functions ---------------------------------------------------------*/

//##########################################################################################################
static const struct IotNetworkServerInfo xMQTTBrokerInfo = { .pHostName =
		iotCoreEndpointUrl, .port =
clientcredentialMQTT_BROKER_PORT };

struct IotNetworkServerInfo AWS_IOT_NETWORK_SERVER_INFO_AFR_INITIALIZER = {
		.pHostName = iotCoreEndpointUrl, .port =
		clientcredentialMQTT_BROKER_PORT };

static struct IotNetworkCredentials xNetworkSecurityCredentials = {
/* Optional TLS extensions. For this demo, they are disabled. */
.pAlpnProtos = NULL, .maxFragmentLength = 0,

/* SNI is enabled by default. */
.disableSni = false,

/* Provide the certificate for validating the server. Only required for
 * demos using TLS. */
.pRootCa = tlsATS1_ROOT_CERTIFICATE_PEM, .rootCaSize =
		sizeof(tlsATS1_ROOT_CERTIFICATE_PEM),

/* Strong mutual authentication to authenticate both the broker and
 * the client. */
.pClientCert = keyCLIENT_CERTIFICATE_PEM, .clientCertSize =
		sizeof(keyCLIENT_CERTIFICATE_PEM), .pPrivateKey =
		keyCLIENT_PRIVATE_KEY_PEM, .privateKeySize =
		sizeof(keyCLIENT_PRIVATE_KEY_PEM) };

static IotMqttNetworkInfo_t xNetworkInfo = {
/* No connection to the MQTT broker has been established yet and we want to
 * establish a new connection. */
.createNetworkConnection = true, .u.setup.pNetworkServerInfo =
		&(xMQTTBrokerInfo),

/* Set the TLS credentials for the new MQTT connection. This member is NULL
 * for the plain text MQTT demo. */
.u.setup.pNetworkCredentialInfo = &xNetworkSecurityCredentials,

/* Use FreeRTOS+TCP network interface. */
.pNetworkInterface = IOT_NETWORK_INTERFACE_AFR,

/* Setup the callback which is called when the MQTT connection is
 * disconnected. The task handle is passed as the callback context which
 * is used by the callback to send a task notification to this task.*/
.disconnectCallback.function = NULL };

static const IotMqttConnectInfo_t xConnectInfo = {
/* Set this flag to true if connecting to the AWS IoT MQTT broker. */
.awsIotMqttMode = true,

/* Start with a clean session i.e. direct the MQTT broker to discard any
 * previous session data. Also, establishing a connection with clean session
 * will ensure that the broker does not store any data when this client
 * gets disconnected. */
.cleanSession = true,

/* Since we are starting with a clean session, there are no previous
 * subscriptions to be restored. */
.pPreviousSubscriptions = NULL, .previousSubscriptionCount = 0,

/* We do not want to publish Last Will and Testament (LWT) message if the
 * client gets disconnected. */
.pWillInfo = NULL,

/* Send an MQTT PING request every minute to keep the connection open if
 there is no other MQTT traffic. */
.keepAliveSeconds = (60),

/* The client identifier is used to uniquely identify this MQTT client to
 * the MQTT broker.  In a production device the identifier can be something
 * unique, such as a device serial number. */
.pClientIdentifier = identifier, .clientIdentifierLength =
		(uint16_t) sizeof(identifier) - 1,

/* This example does not authenticate the client and therefore username and
 * password fields are not used. */
.pUserName = NULL, .userNameLength = 0, .pPassword = NULL, .passwordLength = 0 };

/* MQTT Functions Definition ------------------------------------------------------*/

static int _initializeSystem(void) {
	int status = EXIT_SUCCESS;
	bool commonLibrariesInitialized = false;
	bool semaphoreCreated = false;

	/* Initialize common libraries required by network manager and demo. */
	if (IotSdk_Init() == true) {
		commonLibrariesInitialized = true;
	} else {
		PRINT_INFO("Failed to initialize the common library.");
		status = EXIT_FAILURE;
	}

	if (status == EXIT_SUCCESS) {
		/* Create semaphore to signal that a network is available for the demo. */
		if (IotSemaphore_Create(&demoNetworkSemaphore, 0, 1) != true) {
			PRINT_ERR(
					"Failed to create semaphore to wait for a network connection.");
			status = EXIT_FAILURE;
		} else {
			semaphoreCreated = true;
		}
	}

	if (status == EXIT_FAILURE) {
		if (semaphoreCreated == true) {
			IotSemaphore_Destroy(&demoNetworkSemaphore);
		}

		if (commonLibrariesInitialized == true) {
			IotSdk_Cleanup();
		}
	}

	return status;
}

/**
 * @brief Initialize the MQTT library.
 *
 * @return `EXIT_SUCCESS` if all libraries were successfully initialized;
 * `EXIT_FAILURE` otherwise.
 */
static int _initializeDemo(void) {
	int status = EXIT_SUCCESS;
	IotMqttError_t mqttInitStatus = IOT_MQTT_SUCCESS;

	mqttInitStatus = IotMqtt_Init();

	if (mqttInitStatus != IOT_MQTT_SUCCESS) {
		/* Failed to initialize MQTT library. */
		status = EXIT_FAILURE;
	}

	return status;
}

/**
 * @brief Called by the MQTT library when an operation completes.
 *
 * The demo uses this callback to determine the result of PUBLISH operations.
 * @param[in] param1 The number of the PUBLISH that completed, passed as an intptr_t.
 * @param[in] pOperation Information about the completed operation passed by the
 * MQTT library.
 */
static void _operationCompleteCallback(void *param1,
		IotMqttCallbackParam_t *const pOperation) {
	intptr_t publishCount = (intptr_t) param1;

	/* Silence warnings about unused variables. publishCount will not be used if
	 * logging is disabled. */
	(void) publishCount;

	/* Print the status of the completed operation. A PUBLISH operation is
	 * successful when transmitted over the network. */
	if (pOperation->u.operation.result == IOT_MQTT_SUCCESS) {
		PRINT_INFO("MQTT %s %d successfully sent.",
				IotMqtt_OperationType(pOperation->u.operation.type),
				(int ) publishCount);
	} else {
		PRINT_ERR("MQTT %s %d could not be sent. Error %s.",
				IotMqtt_OperationType(pOperation->u.operation.type),
				(int ) publishCount,
				IotMqtt_strerror(pOperation->u.operation.result));
	}
}

/**
 * @brief Clean up the MQTT library.
 */
static void _cleanupDemo(void) {
	IotMqtt_Cleanup();
}

/**
 * @brief Called by the MQTT library when an incoming PUBLISH message is received.
 *
 * The demo uses this callback to handle incoming PUBLISH messages. This callback
 * prints the contents of an incoming message and publishes an acknowledgement
 * to the MQTT server.
 * @param[in] param1 Counts the total number of received PUBLISH messages. This
 * callback will increment this counter.
 * @param[in] pPublish Information about the incoming PUBLISH message passed by
 * the MQTT library.
 */
static void _mqttSubscriptionCallback(void *param1,
		IotMqttCallbackParam_t *const pPublish) {
	int acknowledgementLength = 0;
	size_t messageNumberIndex = 0, messageNumberLength = 1;
	IotSemaphore_t *pPublishesReceived = (IotSemaphore_t*) param1;
	const char *pPayload = pPublish->u.message.info.pPayload;
	char pAcknowledgementMessage[ACKNOWLEDGEMENT_MESSAGE_BUFFER_LENGTH] = { 0 };
	IotMqttPublishInfo_t acknowledgementInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;

	/* Print information about the incoming PUBLISH message. */
	PRINT_INFO(
			"Incoming PUBLISH received:\r\n" "Subscription topic filter: %.*s\r\n" "Publish topic name: %.*s\r\n" "Publish retain flag: %d\r\n" "Publish QoS: %d\r\n" "Publish payload: %.*s",
			pPublish->u.message.topicFilterLength,
			pPublish->u.message.pTopicFilter,
			pPublish->u.message.info.topicNameLength,
			pPublish->u.message.info.pTopicName,
			pPublish->u.message.info.retain, pPublish->u.message.info.qos,
			pPublish->u.message.info.payloadLength, pPayload);

	/* Find the message number inside of the PUBLISH message. */
	for (messageNumberIndex = 0;
			messageNumberIndex < pPublish->u.message.info.payloadLength;
			messageNumberIndex++) {
		/* The payload that was published contained ASCII characters, so find
		 * beginning of the message number by checking for ASCII digits. */
		if ((pPayload[messageNumberIndex] >= '0')
				&& (pPayload[messageNumberIndex] <= '9')) {
			break;
		}
	}

	/* Check that a message number was found within the PUBLISH payload. */
	if (messageNumberIndex < pPublish->u.message.info.payloadLength) {
		/* Compute the length of the message number. */
		while ((messageNumberIndex + messageNumberLength
				< pPublish->u.message.info.payloadLength)
				&& (*(pPayload + messageNumberIndex + messageNumberLength)
						>= '0')
				&& (*(pPayload + messageNumberIndex + messageNumberLength)
						<= '9')) {
			messageNumberLength++;
		}

		/* Generate an acknowledgement message. */
		acknowledgementLength = snprintf(pAcknowledgementMessage,
		ACKNOWLEDGEMENT_MESSAGE_BUFFER_LENGTH,
		ACKNOWLEDGEMENT_MESSAGE_FORMAT, (int) messageNumberLength,
				pPayload + messageNumberIndex);

		/* Check for errors from snprintf. */
		if (acknowledgementLength < 0) {
			PRINT_FORCE(
					"Failed to generate acknowledgement message for PUBLISH *.*s.",
					(int ) messageNumberLength, pPayload + messageNumberIndex);
		} else {
			/* Set the members of the publish info for the acknowledgement message. */
			acknowledgementInfo.qos = IOT_MQTT_QOS_1;
			acknowledgementInfo.pTopicName = ACKNOWLEDGEMENT_TOPIC_NAME;
			acknowledgementInfo.topicNameLength =
					ACKNOWLEDGEMENT_TOPIC_NAME_LENGTH;
			acknowledgementInfo.pPayload = pAcknowledgementMessage;
			acknowledgementInfo.payloadLength = (size_t) acknowledgementLength;
			acknowledgementInfo.retryMs = PUBLISH_RETRY_MS;
			acknowledgementInfo.retryLimit = PUBLISH_RETRY_LIMIT;

			/* Send the acknowledgement for the received message. This demo program
			 * will not be notified on the status of the acknowledgement because
			 * neither a callback nor IOT_MQTT_FLAG_WAITABLE is set. However,
			 * the MQTT library will still guarantee at-least-once delivery (subject
			 * to the retransmission strategy) because the acknowledgement message is
			 * sent at QoS 1. */
			if (IotMqtt_Publish(pPublish->mqttConnection, &acknowledgementInfo,
					0,
					NULL,
					NULL) == IOT_MQTT_STATUS_PENDING) {
				PRINT_INFO(
						"Acknowledgment message for PUBLISH %.*s will be sent.",
						(int ) messageNumberLength,
						pPayload + messageNumberIndex);
			} else {
				PRINT_FORCE(
						"Acknowledgment message for PUBLISH %.*s will NOT be sent.",
						(int ) messageNumberLength,
						pPayload + messageNumberIndex);
			}
		}
	}

	/* Increment the number of PUBLISH messages received. */
	IotSemaphore_Post(pPublishesReceived);
}

/**
 * @brief Add or remove subscriptions by either subscribing or unsubscribing.
 *
 * @param[in] mqttConnection The MQTT connection to use for subscriptions.
 * @param[in] operation Either #IOT_MQTT_SUBSCRIBE or #IOT_MQTT_UNSUBSCRIBE.
 * @param[in] pTopicFilters Array of topic filters for subscriptions.
 * @param[in] pCallbackParameter The parameter to pass to the subscription
 * callback.
 *
 * @return `EXIT_SUCCESS` if the subscription operation succeeded; `EXIT_FAILURE`
 * otherwise.
 */
static int _modifySubscriptions(IotMqttConnection_t mqttConnection,
		IotMqttOperationType_t operation, const char **pTopicFilters,
		void *pCallbackParameter) {
	int status = EXIT_SUCCESS;
	int32_t i = 0;
	IotMqttError_t subscriptionStatus = IOT_MQTT_STATUS_PENDING;
	IotMqttSubscription_t pSubscriptions[TOPIC_FILTER_COUNT] = {
	IOT_MQTT_SUBSCRIPTION_INITIALIZER };

	/* Set the members of the subscription list. */
	for (i = 0; i < TOPIC_FILTER_COUNT; i++) {
		pSubscriptions[i].qos = IOT_MQTT_QOS_1;
		pSubscriptions[i].pTopicFilter = pTopicFilters[i];
		pSubscriptions[i].topicFilterLength = TOPIC_FILTER_LENGTH;
		pSubscriptions[i].callback.pCallbackContext = pCallbackParameter;
		pSubscriptions[i].callback.function = _mqttSubscriptionCallback;
	}

	/* Modify subscriptions by either subscribing or unsubscribing. */
	if (operation == IOT_MQTT_SUBSCRIBE) {
		subscriptionStatus = IotMqtt_TimedSubscribe(mqttConnection,
				pSubscriptions,
				TOPIC_FILTER_COUNT, 0,
				MQTT_TIMEOUT_MS);

		/* Check the status of SUBSCRIBE. */
		switch (subscriptionStatus) {
		case IOT_MQTT_SUCCESS:
			PRINT_INFO("All demo topic filter subscriptions accepted.")
			;
			break;

		case IOT_MQTT_SERVER_REFUSED:

			/* Check which subscriptions were rejected before exiting the demo. */
			for (i = 0; i < TOPIC_FILTER_COUNT; i++) {
				if (IotMqtt_IsSubscribed(mqttConnection,
						pSubscriptions[i].pTopicFilter,
						pSubscriptions[i].topicFilterLength,
						NULL) == true) {
					PRINT_INFO("Topic filter %.*s was accepted.",
							pSubscriptions[i].topicFilterLength,
							pSubscriptions[i].pTopicFilter);
				} else {
					PRINT_ERR("Topic filter %.*s was rejected.",
							pSubscriptions[i].topicFilterLength,
							pSubscriptions[i].pTopicFilter);
				}
			}

			status = EXIT_FAILURE;
			break;

		default:

			status = EXIT_FAILURE;
			break;
		}
	} else if (operation == IOT_MQTT_UNSUBSCRIBE) {
		subscriptionStatus = IotMqtt_TimedUnsubscribe(mqttConnection,
				pSubscriptions,
				TOPIC_FILTER_COUNT, 0,
				MQTT_TIMEOUT_MS);

		/* Check the status of UNSUBSCRIBE. */
		if (subscriptionStatus != IOT_MQTT_SUCCESS) {
			status = EXIT_FAILURE;
		}
	} else {
		/* Only SUBSCRIBE and UNSUBSCRIBE are valid for modifying subscriptions. */
		PRINT_ERR("MQTT operation %s is not valid for modifying subscriptions.",
				IotMqtt_OperationType(operation));

		status = EXIT_FAILURE;
	}

	return status;
}

/**
 * @brief Transmit all messages and wait for them to be received on topic filters.
 *
 * @param[in] mqttConnection The MQTT connection to use for publishing.
 * @param[in] pTopicNames Array of topic names for publishing. These were previously
 * subscribed to as topic filters.
 * @param[in] pPublishReceivedCounter Counts the number of messages received on
 * topic filters.
 *
 * @return `EXIT_SUCCESS` if all messages are published and received; `EXIT_FAILURE`
 * otherwise.
 */
static int _publishAllMessages(IotMqttConnection_t mqttConnection,
		const char **pTopicNames, IotSemaphore_t *pPublishReceivedCounter) {
	int status = EXIT_SUCCESS;
	intptr_t publishCount = 0, i = 0;
	IotMqttError_t publishStatus = IOT_MQTT_STATUS_PENDING;
	IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
	IotMqttCallbackInfo_t publishComplete = IOT_MQTT_CALLBACK_INFO_INITIALIZER;
	char pPublishPayload[PUBLISH_PAYLOAD_BUFFER_LENGTH] = { 0 };

	/* The MQTT library should invoke this callback when a PUBLISH message
	 * is successfully transmitted. */
	publishComplete.function = _operationCompleteCallback;

	/* Set the common members of the publish info. */
	publishInfo.qos = IOT_MQTT_QOS_1;
	publishInfo.topicNameLength = TOPIC_FILTER_LENGTH;
	publishInfo.pPayload = pPublishPayload;
	publishInfo.retryMs = PUBLISH_RETRY_MS;
	publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

	/* Loop to PUBLISH all messages of this demo. */
	for (publishCount = 0;
			publishCount
					< IOT_DEMO_MQTT_PUBLISH_BURST_SIZE
							* IOT_DEMO_MQTT_PUBLISH_BURST_COUNT;
			publishCount++) {
		/* Announce which burst of messages is being published. */
		if (publishCount % IOT_DEMO_MQTT_PUBLISH_BURST_SIZE == 0) {
			PRINT_INFO("Publishing messages %d to %d.", publishCount,
					publishCount + IOT_DEMO_MQTT_PUBLISH_BURST_SIZE - 1);
		}

		/* Pass the PUBLISH number to the operation complete callback. */
		publishComplete.pCallbackContext = (void*) publishCount;

		/* Choose a topic name (round-robin through the array of topic names). */
		publishInfo.pTopicName = pTopicNames[publishCount % TOPIC_FILTER_COUNT];
		/* Generate the payload for the PUBLISH. */
		status = snprintf(pPublishPayload,
		PUBLISH_PAYLOAD_BUFFER_LENGTH,
		PUBLISH_PAYLOAD_FORMAT, (int) publishCount);

		/* Check for errors from snprintf. */
		if (status < 0) {
			PRINT_ERR("Failed to generate MQTT PUBLISH payload for PUBLISH %d.",
					(int ) publishCount);
			status = EXIT_FAILURE;

			break;
		} else {
			publishInfo.payloadLength = (size_t) status;
			status = EXIT_SUCCESS;
		}

		/* PUBLISH a message. This is an asynchronous function that notifies of
		 * completion through a callback. */
		publishStatus = IotMqtt_Publish(mqttConnection, &publishInfo, 0,
				&publishComplete,
				NULL);

		if (publishStatus != IOT_MQTT_STATUS_PENDING) {
			PRINT_ERR("MQTT PUBLISH %d returned error %s.", (int ) publishCount,
					IotMqtt_strerror(publishStatus));
			status = EXIT_FAILURE;

			break;
		}

		/* If a complete burst of messages has been published, wait for an equal
		 * number of messages to be received. Note that messages may be received
		 * out-of-order, especially if a message was lost and had to be retried. */
		if ((publishCount % IOT_DEMO_MQTT_PUBLISH_BURST_SIZE)
				== ( IOT_DEMO_MQTT_PUBLISH_BURST_SIZE - 1)) {
			PRINT_INFO("Waiting for %d publishes to be received.",
					IOT_DEMO_MQTT_PUBLISH_BURST_SIZE);

			for (i = 0; i < IOT_DEMO_MQTT_PUBLISH_BURST_SIZE; i++) {
				if (IotSemaphore_TimedWait(pPublishReceivedCounter,
				MQTT_TIMEOUT_MS) == false) {
					PRINT_ERR(
							"Timed out waiting for incoming PUBLISH messages.");
					status = EXIT_FAILURE;
					break;
				}
			}

			PRINT_INFO("%d publishes received.", i);
		}

		/* Stop publishing if there was an error. */
		if (status == EXIT_FAILURE) {
			break;
		}
	}

	return status;
}

/**
 * @brief Establish a new connection to the MQTT server.
 *
 * @param[in] awsIotMqttMode Specify if this demo is running with the AWS IoT
 * MQTT server. Set this to `false` if using another MQTT server.
 * @param[in] pIdentifier NULL-terminated MQTT client identifier.
 * @param[in] pNetworkServerInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkCredentialInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkInterface The network interface to use for this demo.
 * @param[out] pMqttConnection Set to the handle to the new MQTT connection.
 *
 * @return `EXIT_SUCCESS` if the connection is successfully established; `EXIT_FAILURE`
 * otherwise.
 */
static int _establishMqttConnection( bool awsIotMqttMode,
		const char *pIdentifier, void *pNetworkServerInfo,
		void *pNetworkCredentialInfo,
		const IotNetworkInterface_t *pNetworkInterface,
		IotMqttConnection_t *pMqttConnection) {
	int status = EXIT_SUCCESS;
	IotMqttError_t connectStatus = IOT_MQTT_STATUS_PENDING;
	IotMqttNetworkInfo_t networkInfo = IOT_MQTT_NETWORK_INFO_INITIALIZER;
	IotMqttConnectInfo_t connectInfo = IOT_MQTT_CONNECT_INFO_INITIALIZER;
	IotMqttPublishInfo_t willInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
	char pClientIdentifierBuffer[CLIENT_IDENTIFIER_MAX_LENGTH] = { 0 };

	/* Set the members of the network info not set by the initializer. This
	 * struct provided information on the transport layer to the MQTT connection. */
	networkInfo.createNetworkConnection = true;
	networkInfo.u.setup.pNetworkServerInfo = pNetworkServerInfo;
	networkInfo.u.setup.pNetworkCredentialInfo = pNetworkCredentialInfo;
	networkInfo.pNetworkInterface = pNetworkInterface;

#if ( IOT_MQTT_ENABLE_SERIALIZER_OVERRIDES == 1 ) && defined( IOT_DEMO_MQTT_SERIALIZER )
	networkInfo.pMqttSerializer = IOT_DEMO_MQTT_SERIALIZER;
#endif

	/* Set the members of the connection info not set by the initializer. */
	connectInfo.awsIotMqttMode = awsIotMqttMode;
	connectInfo.cleanSession = true;
	connectInfo.keepAliveSeconds = KEEP_ALIVE_SECONDS;
	connectInfo.pWillInfo = NULL;
	//    connectInfo.pWillInfo = &willInfo;

	/* Set the members of the Last Will and Testament (LWT) message info. The
	 * MQTT server will publish the LWT message if this client disconnects
	 * unexpectedly. */
	willInfo.pTopicName = WILL_TOPIC_NAME;
	willInfo.topicNameLength = WILL_TOPIC_NAME_LENGTH;
	willInfo.pPayload = WILL_MESSAGE;
	willInfo.payloadLength = WILL_MESSAGE_LENGTH;

	connectInfo.pClientIdentifier = identifier;
	connectInfo.clientIdentifierLength = (uint16_t) strlen(identifier);

	/* Establish the MQTT connection. */
	if (status == EXIT_SUCCESS) {
		PRINT_INFO("MQTT demo client identifier is %.*s (length %hu).",
				connectInfo.clientIdentifierLength,
				connectInfo.pClientIdentifier,
				connectInfo.clientIdentifierLength);

		connectStatus = IotMqtt_Connect(&networkInfo, &connectInfo,
		MQTT_TIMEOUT_MS, pMqttConnection);

		if (connectStatus != IOT_MQTT_SUCCESS) {
			PRINT_ERR("MQTT CONNECT returned error %s.",
					IotMqtt_strerror(connectStatus));

			status = EXIT_FAILURE;
		}
	}

	return status;
}

/**
 * @brief The function that runs the MQTT demo, called by the demo runner.
 *
 * @param[in] awsIotMqttMode Specify if this demo is running with the AWS IoT
 * MQTT server. Set this to `false` if using another MQTT server.
 * @param[in] pIdentifier NULL-terminated MQTT client identifier.
 * @param[in] pNetworkServerInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkCredentialInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkInterface The network interface to use for this demo.
 *
 * @return `EXIT_SUCCESS` if the demo completes successfully; `EXIT_FAILURE` otherwise.
 */
int RunMqttDemo( bool awsIotMqttMode, const char *pIdentifier,
		void *pNetworkServerInfo, void *pNetworkCredentialInfo,
		const IotNetworkInterface_t *pNetworkInterface) {
	/* Return value of this function and the exit status of this program. */
	int status = EXIT_SUCCESS;

	/* Handle of the MQTT connection used in this demo. */
	IotMqttConnection_t mqttConnection = IOT_MQTT_CONNECTION_INITIALIZER;

	/* Counts the number of incoming PUBLISHES received (and allows the demo
	 * application to wait on incoming PUBLISH messages). */
	IotSemaphore_t publishesReceived;

	/* Flags for tracking which cleanup functions must be called. */
	bool librariesInitialized = false, connectionEstablished = false;

	/* Initialize the libraries required for this demo. */
	status = _initializeDemo();

	if (status == EXIT_SUCCESS) {
		/* Mark the libraries as initialized. */
		librariesInitialized = true;
		HAL_Delay(5000);
		/* Establish a new MQTT connection. */
		status = _establishMqttConnection(awsIotMqttMode, pIdentifier,
				pNetworkServerInfo, pNetworkCredentialInfo, pNetworkInterface,
				&mqttConnection);

	}

	if (status == EXIT_SUCCESS) {
		/* Mark the MQTT connection as established. */
		connectionEstablished = true;
		PRINT_INFO("MQTT CONNECTED")

		/* Add the topic filter subscriptions used in this demo. */
		status = _modifySubscriptions(mqttConnection, IOT_MQTT_SUBSCRIBE,
				pTopics, &publishesReceived);
	}

	if (status == EXIT_SUCCESS) {
		/* Create the semaphore to count incoming PUBLISH messages. */
		if (IotSemaphore_Create(&publishesReceived, 0,
		IOT_DEMO_MQTT_PUBLISH_BURST_SIZE) == true) {
			/* PUBLISH (and wait) for all messages. */
			status = _publishAllMessages(mqttConnection, pTopics,
					&publishesReceived);

			/* Destroy the incoming PUBLISH counter. */
			IotSemaphore_Destroy(&publishesReceived);
		} else {
			/* Failed to create incoming PUBLISH counter. */
			status = EXIT_FAILURE;
		}
	}

	if (status == EXIT_SUCCESS) {
		/* Remove the topic subscription filters used in this demo. */
		status = _modifySubscriptions(mqttConnection, IOT_MQTT_UNSUBSCRIBE,
				pTopics,
				NULL);
	}

	/* Disconnect the MQTT connection if it was established. */
	if (connectionEstablished == true) {
		IotMqtt_Disconnect(mqttConnection, 0);
	}

	/* Clean up libraries if they were initialized. */
	if (librariesInitialized == true) {
		_cleanupDemo();
	}

	return status;
}

static void prvMQTTConnect(void) {
	IotMqttError_t xResult;
	IotMqttConnection_t xMQTTConnection = IOT_MQTT_CONNECTION_INITIALIZER;

	/* Set the context to pass into the disconnect callback function. */
	xNetworkInfo.disconnectCallback.pCallbackContext =
			(void*) xTaskGetCurrentTaskHandle();

	/* Establish the connection to the MQTT broker - It is a blocking call and
	 * will return only when connection is complete or a timeout occurs.  The
	 * network and connection structures are declared and initialised at the top
	 * of this file. */

	xResult = RunMqttDemo(xConnectInfo.awsIotMqttMode,
			xConnectInfo.pClientIdentifier,
			xNetworkInfo.u.setup.pNetworkServerInfo,
			xNetworkInfo.u.setup.pNetworkCredentialInfo,
			xNetworkInfo.pNetworkInterface);

	if (xResult != IOT_MQTT_SUCCESS) {
		PRINT_INFO("MQTT CONNECT returned error %s.", IotMqtt_strerror(xResult));

	} else {
		PRINT_INFO("D O N E");
	}

	configASSERT(xResult == IOT_MQTT_SUCCESS);
}

/* Generic Functions Definition ------------------------------------------------------*/

/**
 * @brief  Callback called when a value in datacache changed
 * @note   Managed datacache value changed
 * @param  dc_event_id - value changed
 * @note   -
 * @param  private_gui_data - value provided at service subscription
 * @note   Unused parameter
 * @retval -
 */
static void mqttdemo_notif_cb(dc_com_event_id_t dc_event_id,
		const void *private_gui_data) {
	// UNUSED(private_gui_data);

	if (dc_event_id == DC_CELLULAR_NIFMAN_INFO) {
		dc_nifman_info_t dc_nifman_info;
		(void) dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO,
				(void*) &dc_nifman_info, sizeof(dc_nifman_info));
		if (dc_nifman_info.rt_state == DC_SERVICE_ON) {
			PRINT_FORCE("Echo: Network is up\n\r")
			mqttdemo_network_is_on = true;
			(void) osMessagePut(mqttdemo_queue, (uint32_t) dc_event_id, 0U);
		} else {
			PRINT_FORCE("Echo: Network is down\n\r")
			mqttdemo_network_is_on = false;
		}
	}
}

/**
 * @brief  Check if NFM sleep has to be done
 * @note   -
 * @param  -
 * @retval bool - false/true NFM sleep hasn't to be done/has to be done
 */
static bool mqttdemo_is_nfm_sleep_requested(void) {
	bool result;

	if (mqttdemo_nfm_nb_error_short >= mqttdemo_nfm_nb_error_limit_short) {
		result = true;
	} else {
		result = false;
	}

	return result;
}

/* Demo Code ------------------------------------------------------*/
/**
 * @brief  Initialization
 * @note   mqttdemo initialization
 * @param  -
 * @retval -
 */
void mqttdemo_init(void) {
	/* Socket initialization */
	mqttdemo_socket_id = COM_SOCKET_INVALID_ID;
	mqttdemo_socket_state = mqttdemo_SOCKET_INVALID;
	mqttdemo_socket_closing = false;
	mqttdemo_period = mqttdemo_SEND_PERIOD;
	/* Configuration by menu or default value */
	mqttdemo_socket_new_protocol = mqttdemo_socket_protocol;
	/* NFM initialization */
	mqttdemo_nfm_nb_error_limit_short = mqttdemo_NFM_ERROR_LIMIT_SHORT_MAX;
	mqttdemo_nfm_nb_error_short = 0U;
	mqttdemo_nfm_sleep_timer_index = 0U;

	/* Network state */
	mqttdemo_network_is_on = false;

	/* buffer */
	mqttdemo_buffer_snd[0] = (uint8_t) '\0';

	/* Statistic initialization */
	(void) memset((void*) &mqttdemo_stat, 0, sizeof(mqttdemo_stat));

	osMessageQDef(mqttdemo_queue, 1, uint32_t);
	mqttdemo_queue = osMessageCreate(osMessageQ(mqttdemo_queue), NULL);

	if (mqttdemo_queue == NULL) {
		ERROR_Handler(DBG_CHAN_MQTTDEMO, 1, ERROR_FATAL);
	}
}
#ifdef COAP

CoAP_RespHandler_fn_t CoAP_Resp_handler(CoAP_Message_t *pRespMsg,
		NetEp_t *Sender) {
	configPRINTF(
			( "MESSAGE Payload : %s \r\n" , pRespMsg->Payload ));
	PrintEndpoint(Sender);
}
#endif
#ifndef USE_UDP
/**
 * @brief  Socket thread
 * @note   Infinite loop Mqtt Demo body
 * @param  argument - parameter osThread
 * @note   UNUSED
 * @retval -
 */


#endif
static void mqttdemo_socket_thread(void const *argument) {
#ifndef USE_UDP
	uint32_t nfmc_tempo;
	int status;
	(void) memset(&IOT_DEMO_MQTT_TOPIC_PREFIX, (int8_t) '\0',
			sizeof(IOT_DEMO_MQTT_TOPIC_PREFIX));

#endif
	PRINT_FORCE("\n\r<<<  connecting  >>>\n\r")
	(void) osMessageGet(mqttdemo_queue, RTOS_WAIT_FOREVER);
	(void) osDelay(1000U);
	PRINT_INFO("mqttdemo_network_is_on) %d", mqttdemo_network_is_on)

	if (SYSTEM_Init() == pdPASS) {
		PRINT_INFO("SYSTEM_Init Done")

	}

	if (mqttdemo_network_is_on == true) {
		PRINT_INFO("---------STARTING DEMO---------\n")
#ifndef USE_UDP
		PRINT_FORCE("\n\r<<< MQTT STARTED >>>\n\r")
		mqttdemo_process_flag = true;
		Onboarding_Status_t onboarding_status ;
		CS_Status_t cs_status ;


		if (DEVICE_ONBOARDED == false) {


			onboarding_status = nce_onboard_device();

			if (onboarding_status != ONBOARDING_OK) {
				PRINT_ERR("Onboarding Error");
			} else {
				PRINT_INFO(" ************** Onboarding Done ************** \n");
			    char topic[29];
			    sprintf(topic,"%s/1nce_test",IOT_DEMO_MQTT_TOPIC_PREFIX);
				pTopics[0] = topic;
				sprintf(ACKNOWLEDGEMENT_TOPIC_NAME,"%s/acknowledgements",IOT_DEMO_MQTT_TOPIC_PREFIX);
				sprintf(WILL_TOPIC_NAME,"%s/will",IOT_DEMO_MQTT_TOPIC_PREFIX);
				WILL_TOPIC_NAME_LENGTH   = sizeof( WILL_TOPIC_NAME ) - 1 ;
				TOPIC_FILTER_LENGTH = sizeof( IOT_DEMO_MQTT_TOPIC_PREFIX  ) + sizeof( SUB_TOPIC  ) - 1;
				DEVICE_ONBOARDED = true;
			}

		}


		/* MODEM SSL CONFIGURATION */

		HAL_Delay(6000);
		cs_status = nce_configure_ssl_socket();

		if (cs_status != CELLULAR_OK) {
			PRINT_ERR("Onboarding Error : Modem SSL Configuration ");
			return (onboarding_status);
		} else {
			PRINT_INFO(" ************** SSL Configuration Done ************** \n");
		}

		PRINT_INFO("Starting MQTT Demo ")
		status = _initializeSystem();
		prvMQTTConnect();


#else
#ifndef COAP
		PRINT_FORCE("\n\r<<< UDP STARTED >>>\n\r")
		Socket_t cert_socket = SOCKETS_Socket( SOCKETS_AF_INET,
		SOCKETS_SOCK_DGRAM, COM_IPPROTO_UDP);
		UBaseType_t uxHighWaterMark;
		uxHighWaterMark = uxTaskGetStackHighWaterMark(mqttdemoTaskHandle);
		PRINT_FORCE("\n\r<<< UDP %d >>>\n\r", uxHighWaterMark)

		uint32_t UDP_IP = SOCKETS_GetHostByName(&UDP_ENDPOINT);
		SocketsSockaddr_t udp_address = { .ucLength = sizeof(SocketsSockaddr_t),
				.ucSocketDomain =
				SOCKETS_AF_INET, .usPort = SOCKETS_htons(UDP_PORT), .ulAddress =
						UDP_IP };

		uint32_t timeout = 30000;

		int32_t set_timeout = SOCKETS_SetSockOpt(cert_socket,
		COM_SOL_SOCKET, SOCKETS_SO_RCVTIMEO, &timeout,
				(int32_t) sizeof(timeout));
		int32_t lRetVal = SOCKETS_Connect(cert_socket, &udp_address,
				sizeof(SocketsSockaddr_t));
		PRINT_FORCE("\n\r<<< %d >>>\n\r", lRetVal)
		char send_packet[100];
		sprintf(send_packet, &PUBLISH_PAYLOAD_FORMAT, UDP_ENDPOINT);
		int32_t SendVal = SOCKETS_Send(cert_socket, &send_packet,
				strlen(send_packet), NULL);
		uxHighWaterMark = uxTaskGetStackHighWaterMark(mqttdemoTaskHandle);
		PRINT_FORCE("\n\r<<< UDP %d >>>\n\r", uxHighWaterMark)
		SOCKETS_Close(cert_socket);
		PRINT_FORCE("\n\r<<< UDP FINISH >>>\n\r")
#else
		SocketsSockaddr_t ServerAddress;
		NetPacket_t pPacket;
		int status;
		uint32_t timeout = 30000;
//		DNS Resolution
		uint32_t COAP_IP = SOCKETS_GetHostByName(&COAP_ENDPOINT);
		uint8_t configCOAP_SERVER_ADDR3 = (uint8_t) (COAP_IP >> 24);
		uint8_t configCOAP_SERVER_ADDR2 = (uint8_t) (COAP_IP >> 16);
		uint8_t configCOAP_SERVER_ADDR1 = (uint8_t) (COAP_IP >> 8);
		uint8_t configCOAP_SERVER_ADDR0 = (uint8_t) (COAP_IP >> 0);
		const NetEp_t ServerEp = { .NetType = IPV4, .NetPort =
		configCOAP_PORT, .NetAddr = { .IPv4 = { .u8 = { configCOAP_SERVER_ADDR0,
				configCOAP_SERVER_ADDR1, configCOAP_SERVER_ADDR2,
				configCOAP_SERVER_ADDR3 } } } };
//		Create Socket and Allocate memorie
		Socket_t udp = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM,
		COM_IPPROTO_UDP);
		CoAP_Socket_t *socket = AllocSocket();

		SocketsSockaddr_t coap_address = {
				.ucLength = sizeof(SocketsSockaddr_t), .ucSocketDomain =
				SOCKETS_AF_INET, .usPort = SOCKETS_htons(configCOAP_PORT),
				.ulAddress = COAP_IP };

		int32_t set_timeout = SOCKETS_SetSockOpt(udp,
		COM_SOL_SOCKET, SOCKETS_SO_RCVTIMEO, &timeout,
				(int32_t) sizeof(timeout));
		if (SOCKETS_Connect(udp, &coap_address, sizeof(coap_address)) == 0) {
			PRINT_INFO("Connected to CoAP client\r\n")
			/* Add socket and Network Interface configuration */
			socket->Handle = udp;
			socket->Tx = CoAP_Send_Wifi;
			/* Create confirmable CoAP POST packet with URI Path option */
			CoAP_Message_t *pReqMsg = CoAP_CreateMessage(CON, REQ_POST,
					CoAP_GetNextMid(), CoAP_GenerateToken());

			CoAP_addNewPayloadToMessage(pReqMsg, PUBLISH_PAYLOAD_FORMAT,
					strlen(PUBLISH_PAYLOAD_FORMAT));
//			add QUery support as option
			CoAP_AddOption(pReqMsg, OPT_NUM_URI_QUERY, configCOAP_URI_PATH,
					strlen(configCOAP_URI_PATH));

			/* Create CoAP Client Interaction to send a confirmable POST Request  */
			CoAP_StartNewClientInteraction(pReqMsg, socket->Handle, &ServerEp,
					CoAP_Resp_handler);
			CoAP_Interaction_t *pIA = CoAP_GetLongestPendingInteraction();

			/* Execute the Interaction list  */
			while (pIA != NULL) {

				CoAP_doWork();

				if (pIA->State == COAP_STATE_WAITING_RESPONSE) {

					CoAP_Recv_Wifi(socket->Handle, &pPacket, ServerEp);
				}

				pIA = CoAP_GetLongestPendingInteraction();

			}

		}
		/* DO NOT EDIT - This message is used in the test framework to
		 * determine whether or not the demo was successful. */
		PRINT_INFO("Demo completed successfully.");

		_cleanup();
	} else {
		/* DO NOT EDIT - This demo end marker is used in the test framework to
		 * determine the end of a demo. */

		PRINT_ERR("Failed to initialize the demo. exiting...");
#endif
#endif

	}
	PRINT_INFO("-------DEMO FINISHED-------\n");
}

/**
 * @brief  Start
 * @note   mqttdemo start
 * @param  -
 * @retval -
 */

void mqttdemo_start(void) {
	(void) dc_com_register_gen_event_cb(&dc_com_db, mqttdemo_notif_cb,
			(void*) NULL);
	/* Create Mqtt Demo thread  */
	osThreadDef(mqttdemoTask, mqttdemo_socket_thread, mqttdemo_THREAD_PRIO, 0,
			USED_mqttdemo_THREAD_STACK_SIZE);
	mqttdemoTaskHandle = osThreadCreate(osThread(mqttdemoTask), NULL);
	if (mqttdemoTaskHandle == NULL) {
		ERROR_Handler(DBG_CHAN_MQTTDEMO, 2, ERROR_FATAL);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
