/*
 * nce_bg96_configuration.h
 *
 *  Created on: Sep 7, 2020
 *      Author: MohammedAbdelmaqsoud
 */
#include "demo_config.h"
#include "cellular_runtime_custom.h"
#include "cellular_service.h"
#include "trace_interface.h"
#include "com_sockets_net_compat.h"
#include "com_sockets_addr_compat.h"
/* Secure Sockets Include */
#include "iot_secure_sockets.h"
#include "at_core.h"
#include "platform/iot_network_freertos.h"
//#include "cellular_service.h"
//#include "nce_onboarding.h"


/* Exported functions ------------------------------------------------------- */

CS_Status_t nce_send_modem_command(CS_CHAR_t* at_command);

CS_Status_t nce_configure_onboarding_socket(void);

CS_Status_t nce_reset_modem_fs(void);

CS_Status_t nce_configure_ssl_socket(void);


