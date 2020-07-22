/**
  ******************************************************************************
  * @file    at_modem_signalling.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_signalling.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef AT_MODEM_SIGNALLING_H
#define AT_MODEM_SIGNALLING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "cellular_service.h"
#include "cellular_service_int.h"
#include "ipc_common.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
enum
{
  /* standard commands */
  CMD_AT = 0, /* empty command or empty answer */
  CMD_AT_OK,
  CMD_AT_CONNECT,
  CMD_AT_RING,
  CMD_AT_NO_CARRIER,
  CMD_AT_ERROR,
  CMD_AT_NO_DIALTONE,
  CMD_AT_BUSY,
  CMD_AT_NO_ANSWER,
  CMD_AT_CME_ERROR,
  CMD_AT_CMS_ERROR,

  /* 3GPP TS 27.007 and GSM 07.07 commands */
  CMD_AT_CGMI, /* Request manufacturer identification */
  CMD_AT_CGMM, /* Model identification */
  CMD_AT_CGMR, /* Revision identification */
  CMD_AT_CGSN, /* Product serial number identification */
  CMD_AT_CIMI, /* IMSI */
  CMD_AT_CEER, /* Extended error report */
  CMD_AT_CMEE, /* Report mobile equipment error */
  CMD_AT_CPIN, /* Enter PIN */
  CMD_AT_CFUN, /* Set phone functionality */
  CMD_AT_COPS, /* Operator selection */
  CMD_AT_CNUM, /* Subscriber number */
  CMD_AT_CGATT,   /* PS attach or detach */
  CMD_AT_CREG,    /* Network registration: enable or disable +CREG urc */
  CMD_AT_CGREG,   /* GPRS network registation status: enable or disable +CGREG urc */
  CMD_AT_CEREG,   /* EPS network registration status: enable or disable +CEREG urc */
  CMD_AT_CGEREP,  /* Packet domain event reporting: enable or disable +CGEV urc */
  CMD_AT_CGEV,    /* EPS bearer indication status */
  CMD_AT_CSQ,     /* Signal quality */
  CMD_AT_CGDCONT, /* Define PDP context */
  CMD_AT_CGACT,   /* PDP context activate or deactivate */
  CMD_AT_CGDATA,  /* Enter Data state */
  CMD_AT_CGAUTH,  /* Define PDP context authentication parameters */
  CMD_AT_CGPADDR, /* Show PDP Address */
  CMD_AT_CPSMS,    /* Power Saving Mode setting */
  CMD_AT_CEDRXS,   /* eDRX settings */
  CMD_AT_CEDRXP,   /* eDRX URC */
  CMD_AT_CEDRXRDP, /* eDRX Read Dynamic Parameters */

  /* V.25TER commands */
  CMD_ATD,       /* Dial */
  CMD_ATE,       /* Command Echo */
  CMD_ATH,       /* Hook control (disconnect existing connection) */
  CMD_ATO,       /* Return to online data state (switch from COMMAND to DATA mode) */
  CMD_ATV,       /* DCE response format */
  CMD_AT_AND_W,  /* Store current Parameters to User defined profile */
  CMD_AT_AND_D,  /* Set DTR function mode */
  CMD_ATX,       /* CONNECT Result code and monitor call progress */
  CMD_ATZ,       /* Set all current parameters to user defined profile */
  CMD_AT_GSN,    /* Request product serial number identification */
  CMD_AT_IPR,    /* Fixed DTE rate */
  CMD_AT_IFC,    /* set DTE-DCE local flow control */

  /* other */
  CMD_AT_ESC_CMD,     /* escape command for switching from DATA to COMMAND mode */
  CMD_AT_DIRECT_CMD,  /* allow user to send command directly to the modem */
  CMD_AT_LAST_GENERIC, /* keep it at last position */

};

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */


/* ==========================  Build 3GPP TS 27.007 commands ========================== */
at_status_t fCmdBuild_NoParams(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGSN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CMEE(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CPIN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CFUN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_COPS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGATT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CEREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGEREP(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGDCONT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGACT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGAUTH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGDATA(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGPADDR(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CPSMS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CEDRXS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CEDRXP(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);

/* ==========================  Build V.25ter commands ========================== */
at_status_t fCmdBuild_ATD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ATE(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ATV(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ATX(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ATZ(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_IPR(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_IFC(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ESCAPE_CMD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_AT_AND_D(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);

/* ==========================  Build special commands ========================== */
at_status_t fCmdBuild_DIRECT_CMD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);

/* ==========================  Analyze 3GPP TS 27.007 commands ========================== */
at_action_rsp_t fRspAnalyze_None(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_Error(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CmeErr(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CmsErr(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGMI(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGMM(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGMR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGSN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CIMI(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CEER(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CPIN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CFUN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_COPS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CNUM(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGATT(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CEREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGEV(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CSQ(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CGPADDR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                    const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CPSMS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CEDRXS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CEDRXP(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CEDRXRDP(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                     const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);

/* ==========================  Analyze V.25ter commands ========================== */
at_action_rsp_t fRspAnalyze_GSN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_IPR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);

/* ==========================  Analyze special commands ========================== */
at_action_rsp_t fRspAnalyze_DIRECT_CMD(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);

#ifdef __cplusplus
}
#endif

#endif /* AT_MODEM_SIGNALLING_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
