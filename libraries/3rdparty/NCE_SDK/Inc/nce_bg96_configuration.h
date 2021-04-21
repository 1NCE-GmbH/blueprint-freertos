/*
 * nce_bg96_configuration.h
 *
 *  Created on: Sep 7, 2020
 *      Author: MohammedAbdelmaqsoud
 */
#include "iot_config.h"
#include "aws_cellular_config.h"
#include "cellular_config_defaults.h"
#include "cellular_types.h"
#include "cellular_common.h"

#include "iot_demo_logging.h"
/* Secure Sockets Include */
#include "iot_secure_sockets.h"
#include "platform/iot_network_freertos.h"
#include "nce_demo_config.h"
/*#include "nce_onboarding.h" */

/* Exported variable ------------------------------------------------------- */
/* The application needs to provide the cellular handle for the usage of AT Commands */
extern CellularHandle_t CellularHandle;
/* Exported functions ------------------------------------------------------- */

CellularPktStatus_t nce_send_modem_command( char * at_command );

CellularPktStatus_t nce_configure_onboarding_socket( void );

CellularPktStatus_t nce_reset_modem_fs( void );

CellularPktStatus_t nce_configure_ssl_socket( void );
