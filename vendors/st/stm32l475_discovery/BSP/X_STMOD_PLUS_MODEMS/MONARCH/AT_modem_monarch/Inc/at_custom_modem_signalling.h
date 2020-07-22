/**
  ******************************************************************************
  * @file    at_custom_modem_signalling.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_signalling.c module
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
#ifndef AT_CUSTOM_SIGNALLING_MONARCH_H
#define AT_CUSTOM_SIGNALLING_MONARCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_api.h"
#include "at_modem_signalling.h"
#include "at_custom_modem_specific.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/* MONARCH specific build commands */
at_status_t fCmdBuild_ATD_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_SQNCTM_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_AUTOATT_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CGDCONT_REPROGRAM_MONARCH(atparser_context_t *p_atp_ctxt,
                                                atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_CLEAR_SCAN_CFG_MONARCH(atparser_context_t *p_atp_ctxt,
                                             atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_ADD_SCAN_BAND_MONARCH(atparser_context_t *p_atp_ctxt,
                                            atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_SQNDNSLKUP_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);
at_status_t fCmdBuild_SMST_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt);

/* MONARCH specific analyze commands */
at_action_rsp_t fRspAnalyze_Error_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                          const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_SQNCCID_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                            const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_SQNDNSLKUP_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                               const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_SMST_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);
at_action_rsp_t fRspAnalyze_CESQ_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_SIGNALLING_MONARCH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

