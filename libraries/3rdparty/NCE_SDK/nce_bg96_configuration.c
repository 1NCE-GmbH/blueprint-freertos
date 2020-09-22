/*
 * nce_bg96_configuration.c
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */


#include "nce_bg96_configuration.h"

/* Trace Define */
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_MQTTDEMO, DBL_LVL_P0, "1NCE_Demo: " format "\n\r", ## args)

/* Modem Defines */
CS_CHAR_t at_command[MAX_DIREXT_CMD_SIZE] ;
CS_Status_t result;

static void CST_cellular_direct_cmd_callback(CS_direct_cmd_rx_t direct_cmd_rx) {
	UNUSED(direct_cmd_rx);
}

CS_Status_t nce_send_modem_command(CS_CHAR_t* at_command) {
	CS_direct_cmd_tx_t modem_command = { .cmd_timeout =10000 };
    modem_command.cmd_size = strlen(at_command);
    memcpy(modem_command.cmd_str, at_command, modem_command.cmd_size);
    printf("%s --- %d",modem_command.cmd_str,modem_command.cmd_size);
    result = osCDS_direct_cmd(&modem_command,
			CST_cellular_direct_cmd_callback);
	return (result);
}

CS_Status_t nce_configure_onboarding_socket(void) {


	result = nce_send_modem_command("AT+QSSLCFG=\"seclevel\",0,0");

	result = nce_send_modem_command("AT+QSSLCFG=\"sslversion\",0,4");

	result = nce_send_modem_command("AT+QSSLCFG=\"ciphersuite\",0,0xFFFF");

	result = nce_send_modem_command("AT+QSSLCFG=\"ignorelocaltime\",0,1");

	result = nce_send_modem_command("AT+QSSLCLOSE=1");

	return (result);
}

CS_Status_t nce_reset_modem_fs(void) {
	/*Cleaning Memory*/
	result = nce_send_modem_command("AT+QFDEL=\"*\"");

	return (result);
}



CS_Status_t nce_configure_ssl_socket(void) {


	result = nce_send_modem_command("AT+QSSLCFG=\"cacert\",1,\"root.pem\"");
	HAL_Delay(2000);


	result = nce_send_modem_command("AT+QSSLCFG=\"clientcert\",1,\"clientcert.pem\"");
	HAL_Delay(2000);


	result = nce_send_modem_command("AT+QSSLCFG=\"clientkey\",1,\"clientkey.pem\"");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCFG=\"seclevel\",1,2");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCFG=\"sslversion\",1,4");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCFG=\"ignorelocaltime\",1,1");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCLOSE=1,10");
	HAL_Delay(2000);

	result = nce_send_modem_command("AT+QSSLCLOSE=2,10");
	HAL_Delay(2000);

	return (result);

}

