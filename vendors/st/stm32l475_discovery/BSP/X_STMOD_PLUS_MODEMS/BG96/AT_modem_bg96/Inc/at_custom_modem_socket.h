/**
  ******************************************************************************
  * @file    at_custom_modem_socket.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_socket.c module for BG96
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
#ifndef AT_CUSTOM_SOCKET_BG96_H
#define AT_CUSTOM_SOCKET_BG96_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "at_custom_modem_specific.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern uint8_t WAIT_RESP_FRM_QSSLOPEN ;

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/* BG96 build commands overriding common function  or specific */
at_status_t fCmdBuild_QIACT_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QIOPEN_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QICLOSE_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QISEND_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QISEND_WRITE_DATA_BG96(atparser_context_t *p_atp_ctxt,
                                             atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QIRD_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QISTATE_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QIDNSCFG_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QIDNSGIP_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_QPING_BG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);

/* BG96 specific analyze commands */
at_action_rsp_t fRspAnalyze_QIACT_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QIOPEN_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                        const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QIRD_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                      const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QIRD_data_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                           const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QISTATE_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QINDCFG_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_QPING_BG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_SOCKET_BG96_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
