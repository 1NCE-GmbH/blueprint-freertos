/**
  ******************************************************************************
  * @file    at_custom_modem_specific.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_specific.c module for BG96
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
#ifndef AT_CUSTOM_MODEM_BG96_H
#define AT_CUSTOM_MODEM_BG96_H

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
//////////////////////////////
#include "demo_config.h"
/* Exported constants --------------------------------------------------------*/
/* device specific parameters */
#define MODEM_MAX_SOCKET_TX_DATA_SIZE   CONFIG_MODEM_MAX_SOCKET_TX_DATA_SIZE /* cf AT+QISEND */
#define MODEM_MAX_SOCKET_RX_DATA_SIZE   CONFIG_MODEM_MAX_SOCKET_RX_DATA_SIZE /* cf AT+QIRD */
#define BG96_ACTIVATE_PING_REPORT       (1)

/* Exported types ------------------------------------------------------------*/

enum
{
  /* modem specific commands */
  CMD_AT_QPOWD = (CMD_AT_LAST_GENERIC + 1U), /* */
  CMD_AT_QCFG,                           /* Extended configuration settings */
  CMD_AT_QINDCFG,                        /* URC indication configuration settings */
  CMD_AT_QIND,                           /* URC indication */
  CMD_AT_QUSIM,                          /* URC indication SIM card typpe (SIM or USIM) */
  CMD_AT_QNWINFO,                        /* Query Network Information */
  CMD_AT_QENG,                           /* Switch on or off Engineering Mode */
  /* BG96 specific TCP/IP commands */
  CMD_AT_QICSGP,                         /* Configure parameters of a TCP/IP context */
  CMD_AT_QIACT,                          /* Activate a PDP context */
  CMD_AT_QIDEACT,                        /* Deactivate a PDP context */
  CMD_AT_QIOPEN,                         /* Open a socket service */
  CMD_AT_QICLOSE,                        /* Close a socket service */
  CMD_AT_QISEND,                         /* Send Data - waiting for prompt */
  CMD_AT_QISEND_WRITE_DATA,              /* Send Data - writing data */
  CMD_AT_QIRD,                           /* Retrieve the received TCP/IP Data */
  CMD_AT_QICFG,                          /* Configure optionnal parameters */
  CMD_AT_QISTATE,  						/* Query socket service status */
  CMD_AT_QSSLURC,  						/* TCP#IP secure so^^s */
  CMD_AT_QIURC,                          /* TCP/IP URC */
  CMD_AT_SOCKET_PROMPT,                  /* when sending socket data : prompt = "> " */
  CMD_AT_SEND_OK,                       /* socket send data OK */
  CMD_AT_SEND_FAIL,                      /* socket send data problem */
  CMD_AT_QIDNSCFG,                       /* configure address of DNS server */
  CMD_AT_QIDNSGIP,                       /* get IP address by Domain Name */
  CMD_AT_QPING,                          /* ping a remote server */
  CMD_AT_QCCID,                          /* show ICCID */
  CMD_AT_QINISTAT,                       /* query initialization status of SIM */
  CMD_AT_QCSQ,                           /* query and report signal strength */
  CMD_AT_QGMR,                           /* request Modem and Application Firmware Versions */

  /* modem specific events (URC, BOOT, ...) */
  CMD_AT_WAIT_EVENT,
  CMD_AT_BOOT_EVENT,
  CMD_AT_SIMREADY_EVENT,
  CMD_AT_RDY_EVENT,
  CMD_AT_POWERED_DOWN_EVENT,

};

/* device specific parameters */
typedef enum
{
  QIURC_UNKNOWN,
  QIURC_CLOSED,
  QIURC_RECV,
  QIURC_INCOMING_FULL,
  QIURC_INCOMING,
  QIURC_PDPDEACT,
  QIURC_DNSGIP,
} ATCustom_BG96_QIURC_function_t;

typedef enum
{
  QCFG_unknown,
  QCFG_gprsattach,
  QCFG_nwscanseq,
  QCFG_nwscanmode,
  QCFG_iotopmode,
  QCFG_roamservice,
  QCFG_band,
  QCFG_servicedomain,
  QCFG_sgsn,
  QCFG_msc,
  QCFG_PDP_DuplicateChk,
  QCFG_urc_ri_ring,
  QCFG_urc_ri_smsincoming,
  QCFG_urc_ri_other,
  QCFG_signaltype,
  QCFG_urc_delay,
} ATCustom_BG96_QCFG_function_t;

typedef enum
{
  QINDCFG_unknown,
  QINDCFG_all,
  QINDCFG_csq,
  QINDCFG_smsfull,
  QINDCFG_ring,
  QINDCFG_smsincoming,
} ATCustom_BG96_QINDCFG_function_t;

typedef enum
{
  QCSQ_unknown,
  QCSQ_noService,
  QCSQ_gsm,
  QCSQ_catM1,
  QCSQ_catNB1,
} ATCustom_BG96_QCSQ_sysmode_t;

/* QCINITSTAT defines */
#define QCINITSTAT_INITIALSTATE    ((uint32_t) 0x0)
#define QCINITSTAT_CPINREADY       ((uint32_t) 0x1)
#define QCINITSTAT_SMSINITCOMPLETE ((uint32_t) 0x2)
#define QCINITSTAT_PBINITCOMPLETE  ((uint32_t) 0x4)

typedef uint32_t ATCustom_BG96_QCFGbandGSM_t;
#define QCFGBANDGSM_NOCHANGE ((ATCustom_BG96_QCFGbandGSM_t) 0x0)
#define QCFGBANDGSM_900      ((ATCustom_BG96_QCFGbandGSM_t) 0x1)
#define QCFGBANDGSM_1800     ((ATCustom_BG96_QCFGbandGSM_t) 0x2)
#define QCFGBANDGSM_850      ((ATCustom_BG96_QCFGbandGSM_t) 0x4)
#define QCFGBANDGSM_1900     ((ATCustom_BG96_QCFGbandGSM_t) 0x8)
#define QCFGBANDGSM_ANY      ((ATCustom_BG96_QCFGbandGSM_t) 0xF)

typedef uint32_t ATCustom_BG96_QCFGbandCatM1_t;
/* CatM1 band bitmap is 64 bits long. We consider it as two 32 bits values.
*  0xMMMMMMMM.LLLLLLLL:
*     MMMMMMMM is the MSB part of the bitmap
*     LLLLLLLL is the LSB part of the bitmap
*/
/* LSB bitmap part */
#define QCFGBANDCATM1_B1_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B1_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x1)
#define QCFGBANDCATM1_B2_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B2_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x2)
#define QCFGBANDCATM1_B3_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B3_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x4)
#define QCFGBANDCATM1_B4_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B4_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x8)
#define QCFGBANDCATM1_B5_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B5_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x10)
#define QCFGBANDCATM1_B8_MSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B8_LSB       ((ATCustom_BG96_QCFGbandCatM1_t) 0x80)
#define QCFGBANDCATM1_B12_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B12_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x800)
#define QCFGBANDCATM1_B13_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B13_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x1000)
#define QCFGBANDCATM1_B18_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B18_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x20000)
#define QCFGBANDCATM1_B19_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B19_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x40000)
#define QCFGBANDCATM1_B20_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B20_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x80000)
#define QCFGBANDCATM1_B26_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B26_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x2000000)
#define QCFGBANDCATM1_B28_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_B28_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x8000000)
#define QCFGBANDCATM1_NOCHANGE_MSB ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
#define QCFGBANDCATM1_NOCHANGE_LSB ((ATCustom_BG96_QCFGbandCatM1_t) 0x40000000)
/* MSB bitmap part */
#define QCFGBANDCATM1_B39_MSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x40)
#define QCFGBANDCATM1_B39_LSB      ((ATCustom_BG96_QCFGbandCatM1_t) 0x0)
/* ALL M1 bands bitmaps */
#define QCFGBANDCATM1_ANY_MSB  ((ATCustom_BG96_QCFGbandCatM1_t) 0x40)
#define QCFGBANDCATM1_ANY_LSB  ((ATCustom_BG96_QCFGbandCatM1_t) 0x0a0e189f)

typedef uint32_t ATCustom_BG96_QCFGbandCatNB1_t;
/* CatNB1 band bitmap is 64 bits long. We consider it as two 32 bits values.
*  0xMMMMMMMM.LLLLLLLL:
*     MMMMMMMM is the MSB part of the bitmap
*     LLLLLLLL is the LSB part of the bitmap
*/
/* LSB bitmap part */
#define QCFGBANDCATNB1_B1_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B1_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x1)
#define QCFGBANDCATNB1_B2_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B2_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x2)
#define QCFGBANDCATNB1_B3_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B3_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x4)
#define QCFGBANDCATNB1_B4_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B4_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x8)
#define QCFGBANDCATNB1_B5_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B5_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x10)
#define QCFGBANDCATNB1_B8_MSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B8_LSB   ((ATCustom_BG96_QCFGbandCatNB1_t) 0x80)
#define QCFGBANDCATNB1_B12_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B12_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x800)
#define QCFGBANDCATNB1_B13_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B13_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x1000)
#define QCFGBANDCATNB1_B18_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B18_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x20000)
#define QCFGBANDCATNB1_B19_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B19_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x40000)
#define QCFGBANDCATNB1_B20_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B20_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x80000)
#define QCFGBANDCATNB1_B26_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B26_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x2000000)
#define QCFGBANDCATNB1_B28_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_B28_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x8000000)
#define QCFGBANDCATNB1_NOCHANGE_MSB ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_NOCHANGE_LSB ((ATCustom_BG96_QCFGbandCatNB1_t) 0x40000000)
/* MSB bitmap part - not used for the moment */
/* ALL NB1 bands bitmaps */
#define QCFGBANDCATNB1_ANY_MSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0x0)
#define QCFGBANDCATNB1_ANY_LSB  ((ATCustom_BG96_QCFGbandCatNB1_t) 0xA0E189F)

typedef uint32_t ATCustom_BG96_QCFGiotopmode_t;
#define QCFGIOTOPMODE_CATM1       ((ATCustom_BG96_QCFGiotopmode_t) 0x0)
#define QCFGIOTOPMODE_CATNB1      ((ATCustom_BG96_QCFGiotopmode_t) 0x1)
#define QCFGIOTOPMODE_CATM1CATNB1 ((ATCustom_BG96_QCFGiotopmode_t) 0x2)

typedef uint32_t ATCustom_BG96_QCFGscanmode_t;
#define QCFGSCANMODE_AUTO     ((ATCustom_BG96_QCFGscanmode_t) 0x0)
#define QCFGSCANMODE_GSMONLY  ((ATCustom_BG96_QCFGscanmode_t) 0x1)
#define QCFGSCANMODE_LTEONLY  ((ATCustom_BG96_QCFGscanmode_t) 0x3)

typedef uint32_t ATCustom_BG96_QCFGscanseq_t;
/* individual values */
#define  QCFGSCANSEQ_AUTO        ((ATCustom_BG96_QCFGscanseq_t) 0x0)
#define  QCFGSCANSEQ_GSM         ((ATCustom_BG96_QCFGscanseq_t) 0x1)
#define  QCFGSCANSEQ_LTECATM1    ((ATCustom_BG96_QCFGscanseq_t) 0x2)
#define  QCFGSCANSEQ_LTECATNB1   ((ATCustom_BG96_QCFGscanseq_t) 0x3)
/* combined values */
#define  QCFGSCANSEQ_GSM_M1_NB1 ((ATCustom_BG96_QCFGscanseq_t) 0x010203)
#define  QCFGSCANSEQ_GSM_NB1_M1 ((ATCustom_BG96_QCFGscanseq_t) 0x010302)
#define  QCFGSCANSEQ_M1_GSM_NB1 ((ATCustom_BG96_QCFGscanseq_t) 0x020103)
#define  QCFGSCANSEQ_M1_NB1_GSM ((ATCustom_BG96_QCFGscanseq_t) 0x020301)
#define  QCFGSCANSEQ_NB1_GSM_M1 ((ATCustom_BG96_QCFGscanseq_t) 0x030102)
#define  QCFGSCANSEQ_NB1_M1_GSM ((ATCustom_BG96_QCFGscanseq_t) 0x030201)

typedef struct
{
  ATCustom_BG96_QCFGscanseq_t    nw_scanseq;
  ATCustom_BG96_QCFGscanmode_t   nw_scanmode;
  ATCustom_BG96_QCFGiotopmode_t  iot_op_mode;
  ATCustom_BG96_QCFGbandGSM_t    gsm_bands;
  ATCustom_BG96_QCFGbandCatM1_t  CatM1_bands_MsbPart;
  ATCustom_BG96_QCFGbandCatM1_t  CatM1_bands_LsbPart;
  ATCustom_BG96_QCFGbandCatNB1_t CatNB1_bands_MsbPart;
  ATCustom_BG96_QCFGbandCatNB1_t CatNB1_bands_LsbPart;
} ATCustom_BG96_mode_band_config_t;

/* Engineering Mode - Cell Type information */
#define QENG_CELLTYPE_SERVINGCELL    (uint8_t)(0x0U) /* get 2G or 3G serving cell information */
#define QENG_CELLTYPE_NEIGHBOURCELL  (uint8_t)(0x1U) /* get 2G or 3G neighbour cell information */
#define QENG_CELLTYPE_PSINFO         (uint8_t)(0x2U) /* get 2G or 3G cell information during packet switch connected */

/* QIOPEN service type parameter */
#define QIOPENSERVICETYPE_TCP_CLIENT  (uint8_t)(0x0U)
#define QIOPENSERVICETYPE_UDP_CLIENT  (uint8_t)(0x1U)
#define QIOPENSERVICETYPE_TCP_SERVER  (uint8_t)(0x2U)
#define QIOPENSERVICETYPE_UDP_SERVICE (uint8_t)(0x3U)

typedef struct
{
  at_bool_t finished;
  at_bool_t wait_header; /* indicate we are waiting for +QIURC: "dnsgip",<err>,<IP_count>,<DNS_ttl> */
  uint32_t  error;
  uint32_t  ip_count;
  uint32_t  ttl;
  AT_CHAR_t hostIPaddr[MAX_SIZE_IPADDR]; /* = host_name parameter from CS_DnsReq_t */
} bg96_qiurc_dnsgip_t;

typedef struct
{
  ATCustom_BG96_mode_band_config_t  mode_and_bands_config;  /* memorize current BG96 mode and bands configuration */
  ATCustom_BG96_QCFG_function_t     QCFG_command_param;
  at_bool_t                         QCFG_command_write;
  ATCustom_BG96_QINDCFG_function_t  QINDCFG_command_param;
  at_bool_t                         QIOPEN_waiting;         /* memorize if waiting for QIOPEN */
  uint8_t                           QIOPEN_current_socket_connected;
  at_bool_t                  QICGSP_config_command;  /* memorize if QICSGP write command is a config or a query cmd */
  bg96_qiurc_dnsgip_t        QIURC_dnsgip_param;     /* memorize infos received in the URC +QIURC:"dnsgip" */
  at_bool_t                  QINISTAT_error;         /* memorize AT+QINISTAT has reported an error  */
  uint8_t                    bg96_sim_status_retries; /* memorize number of attempts to access SIM (with QINISTAT) */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
  at_bool_t                  pdn_already_active; /* check if request PDN to activate is already active (QIACT) */
#endif /* USE_SOCKETS_TYPE */
} bg96_shared_variables_t;

/* External variables --------------------------------------------------------*/
extern bg96_shared_variables_t bg96_shared;
extern atcustom_LUT_t ATCMD_BG96_LUT[];
extern atcustom_modem_context_t BG96_ctxt;


/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/* Exported functions ------------------------------------------------------- */
void        ATCustom_BG96_init(atparser_context_t *p_atp_ctxt);
uint8_t     ATCustom_BG96_checkEndOfMsgCallback(uint8_t rxChar);
at_status_t ATCustom_BG96_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout);
at_endmsg_t ATCustom_BG96_extractElement(atparser_context_t *p_atp_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_BG96_analyzeCmd(at_context_t *p_at_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos);
at_action_rsp_t ATCustom_BG96_analyzeParam(at_context_t *p_at_ctxt,
                                           const IPC_RxMessage_t *p_msg_in,
                                           at_element_info_t *element_infos);
at_action_rsp_t ATCustom_BG96_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos);

at_status_t ATCustom_BG96_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_BG96_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_BG96_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf);
at_status_t ATCustom_BG96_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_MODEM_BG96_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
