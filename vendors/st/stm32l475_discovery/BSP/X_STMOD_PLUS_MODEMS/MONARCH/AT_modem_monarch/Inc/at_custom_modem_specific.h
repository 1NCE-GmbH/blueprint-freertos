/**
  ******************************************************************************
  * @file    at_custom_modem_specific.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_specific.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) YYYY STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef AT_CUSTOM_MODEM_MONARCH_H
#define AT_CUSTOM_MODEM_MONARCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_api.h"
#include "at_modem_signalling.h"
#include "cellular_service.h"
#include "cellular_service_int.h"
#include "ipc_common.h"

/* Exported constants --------------------------------------------------------*/
/* device specific parameters */
#define MODEM_MAX_SOCKET_TX_DATA_SIZE  CONFIG_MODEM_MAX_SOCKET_TX_DATA_SIZE
#define MODEM_MAX_SOCKET_RX_DATA_SIZE  CONFIG_MODEM_MAX_SOCKET_RX_DATA_SIZE
#define SEQUANS_PING_LENGTH            ((uint16_t)32U)
#define GM01Q_ACTIVATE_PING_REPORT     (1)  /* temporary solution, ping is synchrone, report not supported by com */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* modem specific commands */
  CMD_AT_SQNSRING  = (CMD_AT_LAST_GENERIC + 1U),       /* socket ring */
  CMD_AT_SQNSCFG,     /* socket configuration */
  CMD_AT_SQNSCFGEXT,  /* socket extended configuration */
  CMD_AT_SQNSD,       /* socket dial */
  CMD_AT_SQNSH,       /* socket shutdown */
  CMD_AT_SQNSI,       /* socket information  */
  CMD_AT_SQNSS,       /* socket status */
  CMD_AT_SQNSRECV,    /* socket, receive data in command mode */
  CMD_AT_SQNSSENDEXT, /* socket, extended send data in command mode (waiting for prompt)  */
  CMD_AT_SQNSSEND_WRITE_DATA, /* socket, send data in command mode (send data) */
  CMD_AT_RESET,       /* hardware reset */
  CMD_AT_SQNCTM,      /* Conformance Test mode (not described in Monarch ref. manual but in Calliope ref. manual */
  CMD_AT_AUTOATT,     /* Automatic Attach */
  CMD_AT_CGDCONT_REPROGRAM, /* special case, reprogram CGDCONT */
  CMD_AT_CLEAR_SCAN_CFG,    /* clear bands */
  CMD_AT_ADD_SCAN_BAND,     /* add a band */
  CMD_AT_ICCID,       /* ICCID request */
  CMD_AT_SQNDNSLKUP,  /* DNS request */
  CMD_AT_PING,        /* Ping request */
  CMD_AT_SMST,        /* SIM test */
  CMD_AT_CESQ,        /* Extended signal quality */
  CMD_AT_SQNSSHDN,    /* Power Down Modem */

  /* modem specific events (URC, BOOT, ...) */
  CMD_AT_WAIT_EVENT,
  CMD_AT_BOOT_EVENT,
  CMD_AT_SIMREADY_EVENT,
  CMD_AT_SYSSTART_TYPE1,
  CMD_AT_SYSSTART_TYPE2,
  CMD_AT_SYSSHDN,
  CMD_AT_SOCKET_PROMPT,
  CMD_AT_SHUTDOWN,

} ATCustom_MONARCH_cmdlist_t;

typedef struct
{
  AT_CHAR_t hostIPaddr[MAX_SIZE_IPADDR]; /* = host_name parameter from CS_DnsReq_t */
} ATCustom_MONARCH_dns_t;

typedef struct
{
  ATCustom_MONARCH_dns_t  SQNDNSLKUP_dns_info;   /* memorize infos received for DNS in +SQNDNSLKUP */
  uint8_t                 SMST_sim_error_status; /* memorize infos received for DNS in +SMST */
  bool                    waiting_for_ring_irq;
} monarch_shared_variables_t;


/* External variables --------------------------------------------------------*/
extern monarch_shared_variables_t monarch_shared;

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void        ATCustom_MONARCH_init(atparser_context_t *p_atp_ctxt);
uint8_t     ATCustom_MONARCH_checkEndOfMsgCallback(uint8_t rxChar);
at_status_t ATCustom_MONARCH_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout);
at_endmsg_t ATCustom_MONARCH_extractElement(atparser_context_t *p_atp_ctxt,
                                            const IPC_RxMessage_t *p_msg_in,
                                            at_element_info_t *element_infos);
at_action_rsp_t ATCustom_MONARCH_analyzeCmd(at_context_t *p_at_ctxt,
                                            const IPC_RxMessage_t *p_msg_in,
                                            at_element_info_t *element_infos);
at_action_rsp_t ATCustom_MONARCH_analyzeParam(at_context_t *p_at_ctxt,
                                              const IPC_RxMessage_t *p_msg_in,
                                              at_element_info_t *element_infos);
at_action_rsp_t ATCustom_MONARCH_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos);

at_status_t ATCustom_MONARCH_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_MONARCH_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_MONARCH_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_MONARCH_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_MODEM_MONARCH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

