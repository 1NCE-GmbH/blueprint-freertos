/**
  ******************************************************************************
  * @file    at_custom_modem_signalling.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code to the
  *          Sequans Monarch modem
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

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "at_modem_signalling.h"
#include "at_modem_socket.h"
#include "at_custom_modem_signalling.h"
#include "at_custom_modem_specific.h"
#include "at_datapack.h"
#include "at_util.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"
#include "plf_modem_config.h"
#include "error_handler.h"

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCUSTOM_SPECIFIC == 1)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0,"MONARCH:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "MONARCH:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P2, "MONARCH API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "MONARCH ERROR:" format "\n\r", ## args)
#define PRINT_BUF(pbuf, size)       TRACE_PRINT_BUF_CHAR(DBG_CHAN_ATCMD, DBL_LVL_P1, (const CRC_CHAR_t *)pbuf, size);
#elif (USE_PRINTF == 1)
#define PRINT_INFO(format, args...)  (void) printf("MONARCH:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("MONARCH ERROR:" format "\n\r", ## args);
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_PRINTF == 0U */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC == 1 */

/* START_PARAM_LOOP and END_PARAM_LOOP macros are used to loop on all fields
*  received in a message.
*  Only non-null length fields are analysed.
*  End the analyze when the end of the message or an error has been detected.
*/
#define START_PARAM_LOOP()  uint8_t exitcode = 0U;\
  do {\
    if (atcc_extractElement(p_at_ctxt, p_msg_in, element_infos) != ATENDMSG_NO) {exitcode = 1U;}\
    if (element_infos->str_size != 0U)\
    {\

#define END_PARAM_LOOP()  }\
  if (retval == ATACTION_RSP_ERROR) {exitcode = 1U;}\
  } while (exitcode == 0U);

/* Private defines -----------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/* Build command functions ------------------------------------------------------- */
at_status_t fCmdBuild_ATD_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATD_MONARCH()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
    PRINT_INFO("Activate PDN (user cid = %d, modem cid = %d)", (uint8_t)current_conf_id, modem_cid)

    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "*99***%d#", modem_cid);
  }
  return (retval);
}

at_status_t fCmdBuild_SQNCTM_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNCTM_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT+SQNCTM=<ctm>
      *
      * implementation: if MONARCH_CONFORMANCE_TEST_MODE is defined, use it for <ctm> value.
      *                 Otherwise, use "standard" by default
      */
#if defined(MONARCH_CONFORMANCE_TEST_MODE)
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"%s\"", MONARCH_CONFORMANCE_TEST_MODE);
#else
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"standard\"");
#endif /* MONARCH_CONFORMANCE_TEST_MODE */
  }

  return (retval);

}

at_status_t fCmdBuild_AUTOATT_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_AUTOATT_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT^AUTOATT=<auto_att>
      *
      * implementation: if MONARCH_AUTOATTACH is defined, use it for <auto_att> value.
      *                 Otherwise, use 1 by default
      */
#if defined(MONARCH_AUTOATTACH)
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", MONARCH_AUTOATTACH);
#else
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1");
#endif /* MONARCH_AUTOATTACH */
  }

  return (retval);

}

at_status_t fCmdBuild_CGDCONT_REPROGRAM_MONARCH(atparser_context_t *p_atp_ctxt,
                                                atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGDCONT_REPROGRAM_MONARCH()")

  /* only for raw command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD)
  {
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "AT+CGDCONT=%d,\"%s\",\"%s\"\r",
                   PDP_CONTEXT_DEFAULT_MODEM_CID,
                   PDP_CONTEXT_DEFAULT_TYPE,
                   PDP_CONTEXT_DEFAULT_APN);
    p_atp_ctxt->current_atcmd.raw_cmd_size = strlen((const CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params);
  }

  return (retval);
}

at_status_t fCmdBuild_SQNDNSLKUP_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNDNSLKUP_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT+QIDNSGIP=<hostName>[,<ipType>]
      *
      * Write command will return in ERROR if data APN is not yet activated (see AT+CGDCONT).
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"%s\"",
                   p_modem_ctxt->SID_ctxt.dns_request_infos->dns_req.host_name);
  }

  return (retval);
}

at_status_t fCmdBuild_SMST_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SMST_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*  +SMST=<interface>
      * interface
      * Optional parameter to select the SIM interface.
      *   > 0: selects external SIM (default interface)
      *   > 1: selects internal SIM
      *   > 0: selects SCI0 interface (default interface)
      *   > 1: selects SCI1 interface
      */
    uint8_t interface;
    if (p_modem_ctxt->persist.sim_selected == CS_MODEM_SIM_SOCKET_0)
    {
      interface = 0U;
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", interface);
    }
    else if (p_modem_ctxt->persist.sim_selected == CS_MODEM_SIM_ESIM_1)
    {
      interface = 1U;
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", interface);
    }
    else
    {
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

/* Analyze command functions ------------------------------------------------------- */
at_action_rsp_t fRspAnalyze_Error_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                          const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval;
  PRINT_API("enter fRspAnalyze_Error_MONARCH()")

  switch (p_atp_ctxt->current_SID)
  {
    case SID_CS_DIAL_COMMAND:
      /* in case of error during socket connection,
        * release the modem CID for this socket_handle
        */
      (void) atcm_socket_release_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle);
      break;

    default:
      break;
  }

  /* analyze Error for Sequans modems */
  switch (p_atp_ctxt->current_atcmd.id)
  {
    case CMD_AT_CLEAR_SCAN_CFG:
    case CMD_AT_ADD_SCAN_BAND:
      /* error is ignored */
      retval = ATACTION_RSP_FRC_END;
      break;

    default:
      retval = fRspAnalyze_Error(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);
      break;
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNCCID_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                            const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_INTERMEDIATE; /* received a valid intermediate answer */
  PRINT_API("enter fRspAnalyze_SQNCCID_MONARCH()")

  /* analyze parameters for +QCCID */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    PRINT_DBG("ICCID:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.iccid),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t) element_infos->str_size);
  }
  else
  {
    /* other parameters ignored */
    __NOP(); /* to avoid warning */
  }
  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNDNSLKUP_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                               const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_INTERMEDIATE; /* received a valid intermediate answer */
  PRINT_API("enter fRspAnalyze_SQNDNSLKUP_MONARCH()")

  /* analyze parameters for +SQNDNSLKUP:
    *   +SQNDNSLKUP: <hostName>,<ipAddress>
    */
  START_PARAM_LOOP()

  if (element_infos->param_rank == 2U)
  {
    /* ignore parameter */
  }
  else if (element_infos->param_rank == 3U)
  {
    (void) memcpy((void *)monarch_shared.SQNDNSLKUP_dns_info.hostIPaddr,
                  (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                  (size_t) element_infos->str_size);
  }
  else
  { /* nothing to do */ }

  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_SMST_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_INTERMEDIATE; /* received a valid intermediate answer */
  PRINT_API("enter fRspAnalyze_SMST_MONARCH()")

  /* analyze parameters for +SMST:
    *   +SMST=status
    *
    *  status = OK (test completed with a positive status)
    *  status = NO SIM (no sim card was detected)
    *  status = NOK (test completed and detected a problem)
    */
  START_PARAM_LOOP()

  if (element_infos->param_rank == 2U)
  {
    CRC_CHAR_t status[32];
    (void) memset((void *)status, 0, 32);

    /* retrieve SMST value */
    (void) memcpy((void *)status,
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t) element_infos->str_size);
    PRINT_DBG("<SEQMONARCH custom> +SMST: status=%s", status)

    /* extract value and compare it to expected value */
    if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&status[0], "NOK") != NULL)
    {
      /* keep it before OK test !! */
      PRINT_ERR("decoded value = NOK (ERROR !)")
      monarch_shared.SMST_sim_error_status = 1U;
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&status[0], "NO SIM") != NULL)
    {
      PRINT_ERR("decoded value = NO SIM (ERROR !)")
      monarch_shared.SMST_sim_error_status = 1U;
    }
    else  if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&status[0], "OK") != NULL)
    {
      PRINT_DBG("decoded value = OK")
      monarch_shared.SMST_sim_error_status = 0U;
    }
    else
    {
      /* value not expected */
      PRINT_ERR("+SMST unexpected decoded value (ERROR!)")
      monarch_shared.SMST_sim_error_status = 1U;
    }
  }

  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_CESQ_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CESQ_MONARCH()")

  /* analyze parameters for CESQ */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /*
      *  format: +QCSQ: <sysmode>,sysmode>,[,<value1>[,<value2>[,<value3>[,<value4>]]]]
      *
      *  <sysmode> "NOSERVICE", "GSM", "CAT-M1" or "CAT-NB1"
      *
      * if <sysmode> = "NOSERVICE"
      *    no values
      * if <sysmode> = "GSM"
      *    <value1> = <gsm_rssi>
      * if <sysmode> = "CAT-M1" or "CAT-NB1"
      *    <value1> = <lte_rssi> / <value2> = <lte_rssp> / <value3> = <lte_sinr> / <value4> = <lte_rsrq>
      */

    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      /* <rxlev> */
      PRINT_DBG("<rxlev> = %ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size));
    }
    else if (element_infos->param_rank == 3U)
    {
      /* <ber> */
      PRINT_DBG("<ber> = %ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size));
    }
    else if (element_infos->param_rank == 4U)
    {
      /* <rscp> */
      PRINT_DBG("<rscp> = %ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size));
    }
    else if (element_infos->param_rank == 5U)
    {
      /* <ecno> */
      PRINT_DBG("<ecno> = %ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size));
    }
    else if (element_infos->param_rank == 6U)
    {
      /* <rsrq> */
      PRINT_DBG("<rsrq> = %ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size));
    }
    else if (element_infos->param_rank == 7U)
    {
      /* <rsrp> */
      /* rsrp range is -44 dBm to -140 dBm */
      PRINT_INFO("<rsrp> = %ld (about -%ld dBm)",
                 ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                           element_infos->str_size),
                 (ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                            element_infos->str_size) - 141U));
    }
    else
    { /* nothing to do */ }

    END_PARAM_LOOP()
  }

  return (retval);
}

/* Private function Definition -----------------------------------------------*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

