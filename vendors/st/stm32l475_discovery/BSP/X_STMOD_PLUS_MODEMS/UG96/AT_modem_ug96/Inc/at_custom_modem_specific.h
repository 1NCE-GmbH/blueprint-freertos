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
#ifndef AT_CUSTOM_MODEM_UG96_H
#define AT_CUSTOM_MODEM_UG96_H

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
#define MODEM_MAX_SOCKET_TX_DATA_SIZE   CONFIG_MODEM_MAX_SOCKET_TX_DATA_SIZE /* cf AT+QISEND */
#define MODEM_MAX_SOCKET_RX_DATA_SIZE   CONFIG_MODEM_MAX_SOCKET_RX_DATA_SIZE /* cf AT+QIRD */
#define UG96_ACTIVATE_PING_REPORT       (1)

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* modem specific commands */
  CMD_AT_QPOWD = (CMD_AT_LAST_GENERIC + 1U), /* */
  CMD_AT_QCFG,                           /* Extended configuration settings */
  CMD_AT_QUSIM,                          /* URC indication SIM card typpe (SIM or USIM) */
  CMD_AT_QIND,                           /* URC indication */

  /* UG96 specific TCP/IP commands */
  CMD_AT_QICSGP,                         /* Configure parameters of a TCP/IP context */
  CMD_AT_QIACT,                          /* Activate a PDP context */
  CMD_AT_QIDEACT,                        /* Deactivate a PDP context */
  CMD_AT_QIOPEN,                         /* Open a socket service */
  CMD_AT_QICLOSE,                        /* Close a socket service */
  CMD_AT_QISEND,                         /* Send Data - waiting for prompt */
  CMD_AT_QISEND_WRITE_DATA,              /* Send Data - writing data */
  CMD_AT_QIRD,                           /* Retrieve the received TCP/IP Data */
  CMD_AT_QISTATE,                        /* Query socket service status */
  CMD_AT_QIURC,                          /* TCP/IP URC */
  CMD_AT_SOCKET_PROMPT,                  /* when sending socket data : prompt = "> " */
  CMD_AT_SEND_OK,                        /* socket send data OK */
  CMD_AT_SEND_FAIL,                      /* socket send data problem */
  CMD_AT_QIDNSCFG,                       /* configure address of DNS server */
  CMD_AT_QIDNSGIP,                       /* get IP address by Domain Name */
  CMD_AT_QPING,                          /* ping a remote server */
  CMD_AT_QCCID,                          /* show ICCID */

  /* modem specific events (URC, BOOT, ...) */
  CMD_AT_WAIT_EVENT,
  CMD_AT_BOOT_EVENT,
  CMD_AT_SIMREADY_EVENT,
  CMD_AT_RDY_EVENT,
  CMD_AT_POWERED_DOWN_EVENT,

} ATCustom_UG96_cmdlist_t;

/* device specific parameters */
typedef enum
{
  QCFG_unknown,
  QCFG_stateurc_check,
  QCFG_stateurc_enable,
  QCFG_stateurc_disable,
} ATCustom_UG96_QCFG_function_t;

typedef enum
{
  QIURC_UNKNOWN,
  QIURC_CLOSED,
  QIURC_RECV,
  QIURC_INCOMING_FULL,
  QIURC_INCOMING,
  QIURC_PDPDEACT,
  QIURC_DNSGIP,
} ATCustom_UG96_QIURC_function_t;

/* QIOPEN service type parameter */
typedef uint8_t ATCustom_UG96_QIOPENservicetype_t;
#define QIOPENSERVICETYPE_TCP_CLIENT  (ATCustom_UG96_QIOPENservicetype_t)(0x0U)
#define QIOPENSERVICETYPE_UDP_CLIENT  (ATCustom_UG96_QIOPENservicetype_t)(0x1U)
#define QIOPENSERVICETYPE_TCP_SERVER  (ATCustom_UG96_QIOPENservicetype_t)(0x2U)
#define QIOPENSERVICETYPE_UDP_SERVICE (ATCustom_UG96_QIOPENservicetype_t)(0x3U)

typedef struct
{
  at_bool_t finished;
  at_bool_t wait_header; /* indicate we are waiting for +QIURC: "dnsgip",<err>,<IP_count>,<DNS_ttl> */
  uint32_t  error;
  uint32_t  ip_count;
  uint32_t  ttl;
  AT_CHAR_t hostIPaddr[MAX_SIZE_IPADDR]; /* = host_name parameter from CS_DnsReq_t */
} ug96_qiurc_dnsgip_t;

typedef struct
{
  at_bool_t                     waiting_sim_card_ready;
  at_bool_t                     sim_card_ready;
  at_bool_t                     change_ipr_baud_rate;
  ATCustom_UG96_QCFG_function_t QCFG_command_param;
  ATCustom_UG96_QCFG_function_t QCFG_received_param_name;
  ATCustom_UG96_QCFG_function_t QCFG_received_param_value;
  at_bool_t                     QIOPEN_waiting;            /* memorize if waiting for QIOPEN */
  uint8_t                       QIOPEN_current_socket_connected;
  at_bool_t                     QICGSP_config_command; /* memorize if QICSGP write command is a config or a query cmd */
  ug96_qiurc_dnsgip_t           QIURC_dnsgip_param;    /* memorize infos received in the URC +QIURC:"dnsgip" */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
  at_bool_t                     pdn_already_active; /* check if request PDN to activate is already active (QIACT) */
#endif /* USE_SOCKETS_TYPE */
} ug96_shared_variables_t;

/* External variables --------------------------------------------------------*/
extern ug96_shared_variables_t ug96_shared;

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void        ATCustom_UG96_init(atparser_context_t *p_atp_ctxt);
uint8_t     ATCustom_UG96_checkEndOfMsgCallback(uint8_t rxChar);
at_status_t ATCustom_UG96_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout);
at_endmsg_t ATCustom_UG96_extractElement(atparser_context_t *p_atp_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_analyzeCmd(at_context_t *p_at_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_analyzeParam(at_context_t *p_at_ctxt,
                                           const IPC_RxMessage_t *p_msg_in,
                                           at_element_info_t *element_infos);
at_action_rsp_t ATCustom_UG96_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos);

at_status_t ATCustom_UG96_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_UG96_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_UG96_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_UG96_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_MODEM_UG96_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
