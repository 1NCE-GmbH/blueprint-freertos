/**
  ******************************************************************************
  * @file    at_modem_socket.h
  * @author  MCD Application Team
  * @brief   Header for at_modem_socket.c module
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
#ifndef AT_MODEM_SOCKET_H
#define AT_MODEM_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_common.h"
#include "cellular_service.h"
#include "cellular_service_int.h"
#include "ipc_common.h"

/* Exported constants --------------------------------------------------------*/
#define UNDEFINED_MODEM_SOCKET_ID  ((uint8_t) 255U)

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

at_status_t     atcm_socket_reserve_modem_cid(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);
at_status_t     atcm_socket_release_modem_cid(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);
uint32_t        atcm_socket_get_modem_cid(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);
at_status_t     atcm_socket_set_modem_cid(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle,
                                          uint32_t modemcid);
socket_handle_t atcm_socket_get_socket_handle(atcustom_modem_context_t *p_modem_ctxt, uint32_t modemCID);
at_status_t     atcm_socket_set_urc_data_pending(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);
at_status_t     atcm_socket_set_urc_closed_by_remote(atcustom_modem_context_t *p_modem_ctxt,
                                                     socket_handle_t sockHandle);
socket_handle_t atcm_socket_get_hdle_urc_data_pending(atcustom_modem_context_t *p_modem_ctxt);
socket_handle_t atcm_socket_get_hdlr_urc_closed_by_remote(atcustom_modem_context_t *p_modem_ctxt);
at_bool_t       atcm_socket_remaining_urc_data_pending(atcustom_modem_context_t *p_modem_ctxt);
at_bool_t       atcm_socket_remaining_urc_closed_by_remote(atcustom_modem_context_t *p_modem_ctxt);
at_bool_t       atcm_socket_is_connected(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);
at_status_t     atcm_socket_set_connected(atcustom_modem_context_t *p_modem_ctxt, socket_handle_t sockHandle);

#ifdef __cplusplus
}
#endif

#endif /* AT_MODEM_SOCKET_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
