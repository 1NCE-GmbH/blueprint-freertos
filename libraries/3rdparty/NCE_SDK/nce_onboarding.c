/*
 * nce_onboarding.c
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include <stdbool.h>
#include "demo_config.h"
#include "nce_onboarding.h"
#include "trace_interface.h"

/* Trace Define */
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_MQTTDEMO, DBL_LVL_P0, "1NCE_Demo: " format "\n\r", ## args)
#define PRINT_ERR(format, args...)  \
  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "1NCE_Demo ERROR: " format "\n\r", ## args)

/* Onboarding Defines */
uint8_t PSK[500];
uint8_t psk_identity[20];
uint8_t PART[1800];
char find[] = "\\n";
char replace_with[] = "\n";

int offset = &PART[0];
char end_key[] = "-----END RSA PRIVATE KEY-----";
int end_key_len = sizeof(end_key);
char end_cert[] = "-----END CERTIFICATE-----";
int end_cert_len = sizeof(end_cert);
char end_identifier[] = "\"";
char *location;
int client_cert_size = 0;
int client_cert_cmd_size = 0;
int client_key_size = 0;
int client_key_cmd_size = 0;
int root_size = 0;
int root_cmd_size = 0;
char amazonRootCaUrl[80];
char sim_iccid[30];
/* MQTT Defines */
char IOT_DEMO_MQTT_TOPIC_PREFIX[19];
char WILL_TOPIC_NAME[35];
char ACKNOWLEDGEMENT_TOPIC_NAME[40];
const char *pTopics[TOPIC_FILTER_COUNT];
uint16_t WILL_TOPIC_NAME_LENGTH = 0;
uint16_t TOPIC_FILTER_LENGTH = 0;
/* Modem Defines */
CS_Status_t cs_status;

at_status_t retval = ATSTATUS_OK;

static void CST_cellular_direct_cmd_callback(CS_direct_cmd_rx_t direct_cmd_rx) {
	UNUSED(direct_cmd_rx);
}

Onboarding_Status_t nce_onboard_device(void) {

	Onboarding_Status_t onboarding_status = ONBOARDING_ERROR;
	cs_status = nce_reset_modem_fs();

	if (cs_status != CELLULAR_OK) {
		PRINT_ERR("Onboarding Error : Modem Reset ");
		return (onboarding_status);
	} else {
		PRINT_INFO(" ************** Modem Reset Done ************** \n");
	}

	#if defined(USE_UDP) && defined(DTLS_DEMO)
	nce_enable_qssl();
	#endif

	cs_status = nce_configure_onboarding_socket();

	if (cs_status != CELLULAR_OK) {
		PRINT_ERR("Onboarding Error : Onboarding Socket Configuration ");
		return (onboarding_status);
	} else {
		PRINT_INFO(
				" ************** Onboarding Socket Configuration Done ************** \n");
	}
	/* Connect to onboarding endpoint */

	Socket_t cert_socket = SOCKETS_Socket( SOCKETS_AF_INET,
	SOCKETS_SOCK_STREAM, SOCKETS_IPPROTO_TCP);
	uint32_t ROOT_IP = SOCKETS_GetHostByName(&ONBOARDING_ENDPOINT);
	SocketsSockaddr_t root_address = { .ucLength = sizeof(SocketsSockaddr_t),
			.ucSocketDomain = SOCKETS_AF_INET, .usPort = SOCKETS_htons(443),
			.ulAddress = ROOT_IP };
	uint32_t timeout = 30000;

	int32_t set_timeout = SOCKETS_SetSockOpt(cert_socket,
	COM_SOL_SOCKET, SOCKETS_SO_RCVTIMEO, &timeout, (int32_t) sizeof(timeout));
	int32_t lRetVal = SOCKETS_Connect(cert_socket, &root_address,
			sizeof(SocketsSockaddr_t));
	char send_packet[100];
#ifdef DTLS_DEMO
	sprintf(send_packet, "GET /device-api/onboarding/coap HTTP/1.1\r\n"
				"Host: %s\r\n"
				"Accept: text/csv\r\n\r\n", ONBOARDING_ENDPOINT);
#else
	sprintf(send_packet, "GET /device-api/onboarding HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Accept: text/csv\r\n\r\n", ONBOARDING_ENDPOINT);

#endif
	int32_t SendVal = SOCKETS_Send(cert_socket, &send_packet,
			strlen(send_packet), NULL);
	HAL_Delay(1000);

	expected_bytes = sizeof(PART) - 500;

	/* receive the onboarding data */

	uint8_t *complete_response = (uint8_t*) pvPortMalloc(
			5000 * sizeof(uint8_t));

	(void) memset(&PART, (int8_t) '\0', sizeof(PART));

//	waitforprocessing(5);

	int32_t rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
			(int32_t) sizeof(PART),
			COM_MSG_WAIT);
	HAL_Delay(1000);

	PRINT_INFO(" ************** Raw Response (1) **************   %d bytes \n",
			strlen(PART))

#ifdef DTLS_DEMO
	strcat(complete_response,
					strstr(PART, "Express\r\n\r\n") + strlen("Express\r\n\r\n"));
	nce_dtls_psk(complete_response);
#else
	strcat(complete_response,
			strstr(PART, "Express\r\n\r\n\"") + strlen("Express\r\n\r\n\""));


	(void) memset(&PART, (int8_t) '\0', sizeof(PART));

	rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
			(int32_t) sizeof(PART),
			COM_MSG_WAIT);
	PRINT_INFO(" ************** Raw Response (2) **************   %d bytes \n",
			strlen(PART))

	strncat(complete_response, PART, strlen(PART));
	(void) memset(&PART, (int8_t) '\0', sizeof(PART));

	rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
			(int32_t) sizeof(PART),
			COM_MSG_WAIT);


	PRINT_INFO(" ************** Raw Response (3) **************   %d bytes \n",
			strlen(PART))

	strncat(complete_response, PART, strlen(PART));
	(void) memset(&PART, (int8_t) '\0', sizeof(PART));

	rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
			(int32_t) sizeof(PART),
			COM_MSG_WAIT);

	PRINT_INFO(" ************** Raw Response (4) **************   %d bytes \n",
			strlen(PART))

	strncat(complete_response, PART, strlen(PART));
	(void) memset(&PART, (int8_t) '\0', sizeof(PART));
	nce_prepare_and_upload_certificates(complete_response);
#endif

	PRINT_INFO(" ************** Response Received ************** \n");


	onboarding_status = ONBOARDING_OK;

	return (onboarding_status);
}

char* str_replace(char *orig, char *rep, char *with) {

	char *result;
	char *ins;
	char *tmp;
	int len_rep;
	int len_with;
	int len_front;
	int count;
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL;
	if (!with)
		with = "";
	len_with = strlen(with);
	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}
	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
	if (!result)
		return NULL;
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep;
	}
	strcpy(tmp, orig);
	return result;
}

void waitforprocessing(int n) {
	CS_Status_t result;
	for (int i = 0; i < n; i++) {
		result = nce_send_modem_command("AT");
	}
	HAL_Delay(500);

}
#ifdef DTLS_DEMO
static void nce_dtls_psk(uint8_t *complete_response){
	/* get the first token */
	char *token = strtok(complete_response, ",");
	memcpy(PSK, token, strlen(token));
	PRINT_INFO(" *****PSK********* %s ************** \n",PSK)
	token = strtok(NULL, ",");
	memcpy(psk_identity, token, strlen(token));
	PRINT_INFO(" ******ID******** %s ************** \n",psk_identity)

}
#else
static void nce_prepare_and_upload_certificates(uint8_t *complete_response) {

	/* get the first token */
	char *token = strtok(complete_response, ",");
	memcpy(sim_iccid, token, strlen(token));
	int i = 0;
	offset = &sim_iccid[0];
	location = strstr(sim_iccid, end_identifier);
	int identifier_size = location - offset;
	memcpy(identifier, sim_iccid, identifier_size);
	memcpy(IOT_DEMO_MQTT_TOPIC_PREFIX, sim_iccid, identifier_size);

	/* walk through other tokens */
	while (token != NULL) {

		token = strtok(NULL, ",");
		if (i == 0) {
			(void) memset(&iotCoreEndpointUrl, (int8_t) '\0',
					sizeof(iotCoreEndpointUrl));
			memcpy(iotCoreEndpointUrl, token + 1, strlen(token) - 2);
		}
		if (i == 1) {
			memcpy(amazonRootCaUrl, token, strlen(token));
		}
		if (i == 2) {
			/*Process root.pem*/
			memcpy(PART, token + 1, strlen(token) - 1);
			memcpy(PART, str_replace(PART, find, replace_with), strlen(PART));
			offset = &PART[0];
			location = strstr(PART, end_cert);
			root_size = location + end_cert_len - offset;
			PART[root_size] = '\n';
			char root_cmd[30] = "AT+QFUPL=\"root.pem\",";
			sprintf(root_cmd, "%s%d,60", root_cmd, root_size);
			root_cmd_size = strlen(root_cmd);
			/*Upload root.pem*/
			CS_direct_cmd_tx_t crt_command_root = { .cmd_str = root_cmd,
					.cmd_size = root_cmd_size, .cmd_timeout = 10 };
			memcpy(crt_command_root.cmd_str, root_cmd, root_cmd_size);
			cs_status = osCDS_direct_cmd(&crt_command_root,
					CST_cellular_direct_cmd_callback);

			(void) osMutexWait(CellularServiceMutexHandle, RTOS_WAIT_FOREVER);
			retval = sendToIPC(0, (uint8_t*) &PART[0], root_size);
			(void) osMutexRelease(CellularServiceMutexHandle);

			(void) memset(&PART, (int8_t) '\0', sizeof(PART));
			waitforprocessing(10);
		}
		if (i == 3) {
			/* Process client_cert.pem */
			memcpy(PART, token + 1, strlen(token) - 1);
			memcpy(PART, str_replace(PART, find, replace_with), strlen(PART));
			offset = &PART[0];
			location = strstr(PART, end_cert);
			client_cert_size = location + end_cert_len - offset;
			PART[client_cert_size] = '\n';
			char client_cert_cmd[40] = "AT+QFUPL=\"clientcert.pem\",";
			sprintf(client_cert_cmd, "%s%d,60", client_cert_cmd,
					client_cert_size);
			client_cert_cmd_size = strlen(client_cert_cmd);
			/* Upload client_cert.pem */
			CS_direct_cmd_tx_t crt_command_client_cert = { .cmd_str =
					client_cert_cmd, .cmd_size = client_cert_cmd_size,
					.cmd_timeout = 30 };
			memcpy(crt_command_client_cert.cmd_str, client_cert_cmd,
					client_cert_cmd_size);
			cs_status = osCDS_direct_cmd(&crt_command_client_cert,
					CST_cellular_direct_cmd_callback);

			(void) osMutexWait(CellularServiceMutexHandle, RTOS_WAIT_FOREVER);
			retval = sendToIPC(0, (uint8_t*) &PART[0], client_cert_size);
			(void) osMutexRelease(CellularServiceMutexHandle);
			(void) memset(&PART, (int8_t) '\0', sizeof(PART));
		}
		if (i == 4) {
			/* Process client_key.pem */
			memcpy(PART, token + 1, strlen(token) - 1);
			memcpy(PART, str_replace(PART, find, replace_with), strlen(PART));
			offset = &PART[0];
			location = strstr(PART, end_key);
			client_key_size = location + end_key_len - offset;
			PART[client_key_size] = '\n';
			char client_key_cmd[40] = "AT+QFUPL=\"clientkey.pem\",";
			sprintf(client_key_cmd, "%s%d,60", client_key_cmd, client_key_size);
			waitforprocessing(10);
			client_key_cmd_size = strlen(client_key_cmd);
			/* Upload client_key.pem */
			CS_direct_cmd_tx_t crt_command_client_key = { .cmd_str =
					client_key_cmd, .cmd_size = client_key_cmd_size,
					.cmd_timeout = 30 };
			memcpy(crt_command_client_key.cmd_str, client_key_cmd,
					client_key_cmd_size);
			cs_status = osCDS_direct_cmd(&crt_command_client_key,
					CST_cellular_direct_cmd_callback);

			(void) osMutexWait(CellularServiceMutexHandle, RTOS_WAIT_FOREVER);
			retval = sendToIPC(0, (uint8_t*) &PART[0], client_key_size);
			retval = sendToIPC(0, (uint8_t*) &PART[0], client_key_size);
			(void) osMutexRelease(CellularServiceMutexHandle);
			HAL_Delay(10000);
			(void) memset(&PART, (int8_t) '\0', sizeof(PART));
			vPortFree(complete_response);
		}
		i++;
	}
	waitforprocessing(10);
	PRINT_INFO(
			" ************** Certificates Uploaded Successfully  ************** \n");
}

#endif
