/**
  ******************************************************************************
  * @file    at_custom_modem_signalling.c
  * @author  MCD Application Team
  * @brief   This file provides all the 'signalling' code to the
  *          UG96 Quectel modem (3G)
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
#include <stdio.h>
#include <string.h>
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
#if (USE_TRACE_ATCUSTOM_SPECIFIC == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "UG96:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "UG96:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P2, "UG96 API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "UG96 ERROR:" format "\n\r", ## args)
#define PRINT_BUF(pbuf, size)       TRACE_PRINT_BUF_CHAR(DBG_CHAN_ATCMD, DBL_LVL_P1, (const CRC_CHAR_t *)pbuf, size);
#else
#define PRINT_INFO(format, args...)  (void) printf("UG96:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("UG96 ERROR:" format "\n\r", ## args);
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC */

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
at_status_t fCmdBuild_ATD_UG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATD_UG96()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "*99#");
  }
  return (retval);
}

at_status_t fCmdBuild_CGSN_UG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGSN_UG96()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* UG96 only supports EXECUTION form of CGSN */
    retval = ATSTATUS_ERROR;
  }
  return (retval);
}

at_status_t fCmdBuild_QPOWD_UG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_QPOWD_UG96()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* Normal Power Down */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1");
  }

  return (retval);
}

at_status_t fCmdBuild_QCFG_UG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_QCFG_UG96()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* only "stateurc/enable" is managed currently... could be extended */
    switch (ug96_shared.QCFG_command_param)
    {
      case QCFG_stateurc_check:
        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"stateurc/enable\"");
        break;

      case QCFG_stateurc_enable:
        PRINT_DBG("Request to enable urcstate")
        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"stateurc/enable\",1");
        break;

      case QCFG_stateurc_disable:
        PRINT_DBG("Request to disable urcstate")
        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"stateurc/enable\",0");
        break;

      default:
        PRINT_ERR("INVALID QCFG parameter")
        retval = ATSTATUS_ERROR;
        break;
    }
    /* disable State URC indication */
  }

  return (retval);
}

at_status_t fCmdBuild_QICSGP_UG96(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_QICSGP_UG96()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* cf TCP/IP AT commands manual v1.0
    *  AT+QICSGP: Configure parameters of a TCP/IP context
    *  AT+QICSGP=<cid>[,<context_type>,<APN>[,<username>,<password>)[,<authentication>]]]
    *  - cid: context id (rang 1 to 16)
    *  - context_type: 1 for IPV4, 2 for IPV6
    *  - APN: string for access point name
    *  - username: string
    *  - password: string
    *  - authentication: 0 for NONE, 1 for PAP, 2 for CHAP, 3 for PAP or CHAP
    *
    * example: AT+QICSGP=1,1,"UNINET","","",1
    */

    if (ug96_shared.QICGSP_config_command == AT_TRUE)
    {
      /* Write command is a config command */
      CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
      uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
      PRINT_INFO("user cid = %d, modem cid = %d", (uint8_t)current_conf_id, modem_cid)

      uint8_t context_type_value;
      uint8_t authentication_value;

      /* convert context type to numeric value */
      switch (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.pdp_type)
      {
        case CS_PDPTYPE_IP:
          context_type_value = 1U;
          break;
        case CS_PDPTYPE_IPV6:
        case CS_PDPTYPE_IPV4V6:
          context_type_value = 2U;
          break;

        default :
          context_type_value = 1U;
          break;
      }

      /*  authentication : 0,1,2 or 3 - cf modem AT cmd manuel - Use 0 for None */
      if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.username[0] == 0U)
      {
        /* no username => no authentication */
        authentication_value = 0U;
      }
      else
      {
        /* username => PAP or CHAP authentication type */
        authentication_value = 3U;
      }

      /* build command */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d,\"%s\",\"%s\",\"%s\",%d",
                     modem_cid,
                     context_type_value,
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].apn,
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.username,
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.password,
                     authentication_value
                    );
    }
    else
    {
      /* Write command is a query command */
      CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
      uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
      PRINT_INFO("user cid = %d, modem cid = %d", (uint8_t)current_conf_id, modem_cid)
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", modem_cid);
    }

  }

  return (retval);
}

/* Analyze command functions ------------------------------------------------------- */

at_action_rsp_t fRspAnalyze_Error_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_ERROR;
  PRINT_API("enter fRspAnalyze_Error_UG96()")

  switch (p_atp_ctxt->current_SID)
  {
    case SID_CS_DIAL_COMMAND:
      /* in case of error during socket connection,
      * release the modem CID for this socket_handle
      */
      (void) atcm_socket_release_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle);
      break;

    default:
      /* nothing to do */
      break;
  }

  /* analyze Error for UG96 */
  switch (p_atp_ctxt->current_atcmd.id)
  {
    case CMD_AT_CREG:
    case CMD_AT_CGREG:
    case CMD_AT_CEREG:
      /* error is ignored */
      retval = ATACTION_RSP_FRC_END;
      break;

    case CMD_AT_CGDCONT:
      if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_INIT_MODEM)
      {
        /* error is ignored in this case because this cmd is only informative */
        retval = ATACTION_RSP_FRC_END;
      }
      break;

    default:
      retval = fRspAnalyze_Error(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);
      break;
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CPIN_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                      const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CPIN_UG96()")

  /*  Quectel WCDMA UGxx AT Commands Manual V1.7
  *   analyze parameters for +CPIN
  *
  *   if +CPIN is not received after AT+CPIN request, it's an URC
  */

  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CPIN)
  {
    retval = fRspAnalyze_CPIN(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);
  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      PRINT_DBG("URC +CPIN received")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CFUN_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                      const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CFUN_UG96()")

  /*  Quectel WCDMA UGxx AT Commands Manual V1.7
  *   analyze parameters for +CFUN
  *
  *   if +CFUN is received, it's an URC
  */

  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CFUN)
  {
    PRINT_DBG("response to +CFUN received")
    retval = fRspAnalyze_CFUN(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);
  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      PRINT_DBG("URC +CFUN received")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_QIND_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                      const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_QIND_UG96()")

  /*  Quectel WCDMA UGxx AT Commands Manual V1.7
  *   analyze parameters for +QIND
  *
  *   it's an URC
  */

  /* this is an URC */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    PRINT_DBG("URC +QIND received")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    AT_CHAR_t line[32] = {0};
    /* copy element to line for parsing */
    if (element_infos->str_size <= 32U)
    {
      (void) memcpy((void *)&line[0],
                    (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                    (size_t) element_infos->str_size);

      /* extract value and compare it to expected value */
      if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SMS DONE") != NULL)
      {
        PRINT_DBG("received SMS DONE")
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "PB DONE") != NULL)
      {
        PRINT_DBG("received PB DONE")
        ug96_shared.sim_card_ready = AT_TRUE;

        if (ug96_shared.waiting_sim_card_ready == AT_TRUE)
        {
          /* if we were waiting for this event, we can continue the sequence */
          PRINT_INFO("PB DONE received, unlock sequence")
          ug96_shared.waiting_sim_card_ready = AT_FALSE;
          /* UNLOCK the WAIT EVENT */
          retval = ATACTION_RSP_FRC_END;
        }
      }
      else
      {
        /* ignored */
        __NOP(); /* to avoid warning */
      }
    }
    else
    {
      PRINT_ERR("param ignored (exceed maximum size)")
      retval = ATACTION_RSP_IGNORED;
    }


  }
  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_QCFG_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                      const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_QCFG_UG96()")

  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_QCFG)
  {
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      PRINT_DBG("+QUSIM received (param %d)", element_infos->param_rank)
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

      if (ug96_shared.QCFG_command_param == QCFG_stateurc_check)
      {
        PRINT_DBG("+QCFG staturc settings received")
        ug96_shared.QCFG_received_param_name = QCFG_stateurc_check;
      }
    }
    if (element_infos->param_rank == 3U)
    {
      PRINT_DBG("+QUSIM received (param %d)", element_infos->param_rank)
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
      uint32_t param = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                 element_infos->str_size);
      /* */
      switch (ug96_shared.QCFG_received_param_name)
      {
        case QCFG_stateurc_check:
        {
          if (param == 0U)
          {
            PRINT_DBG("+QCFG staturc is disabled")
            ug96_shared.QCFG_received_param_value = QCFG_stateurc_disable;
          }
          else if (param == 1U)
          {
            PRINT_DBG("+QCFG staturc is enabled")
            ug96_shared.QCFG_received_param_value = QCFG_stateurc_enable;
          }
          else
          {
            /* param ignored */
            __NOP(); /* to avoid warning */
          }
          break;
        }

        default:
        {
          PRINT_DBG("+QCFG field type not managed")
          break;
        }
      }
    }
    END_PARAM_LOOP()
  }
  return (retval);
}

at_action_rsp_t fRspAnalyze_QIURC_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_QIURC_UG96()")
  ATCustom_UG96_QIURC_function_t ug96_current_qiurc_ind = QIURC_UNKNOWN;

  /*IP AT Commands manual - LTE Module Series - V1.0
    * URC
    * +QIURC:"closed",<connectID> : URC of connection closed
    * +QIURC:"recv",<connectID> : URC of incoming data
    * +QIURC:"incoming full" : URC of incoming connection full
    * +QIURC:"incoming",<connectID> ,<serverID>,<remoteIP>,<remote_port> : URC of incoming connection
    * +QIURC:"pdpdeact",<contextID> : URC of PDP deactivation
    *
    * for DNS request:
    * header: +QIURC: "dnsgip",<err>,<IP_count>,<DNS_ttl>
    * infos:  +QIURC: "dnsgip",<hostIPaddr>]
    */

  /* this is an URC */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    AT_CHAR_t line[32] = {0};

    /* init param received info */
    ug96_current_qiurc_ind = QIURC_UNKNOWN;

    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    /* copy element to line for parsing */
    if (element_infos->str_size <= 32U)
    {
      (void) memcpy((void *)&line[0],
                    (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                    (size_t) element_infos->str_size);

      /* extract value and compare it to expected value */
      if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "closed") != NULL)
      {
        PRINT_DBG("+QIURC closed info received")
        ug96_current_qiurc_ind = QIURC_CLOSED;
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "recv") != NULL)
      {
        PRINT_DBG("+QIURC recv info received")
        ug96_current_qiurc_ind = QIURC_RECV;
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "incoming full") != NULL)
      {
        PRINT_DBG("+QIURC incoming full info received")
        ug96_current_qiurc_ind = QIURC_INCOMING_FULL;
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "incoming") != NULL)
      {
        PRINT_DBG("+QIURC incoming info received")
        ug96_current_qiurc_ind = QIURC_INCOMING;
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "pdpdeact") != NULL)
      {
        PRINT_DBG("+QIURC pdpdeact info received")
        ug96_current_qiurc_ind = QIURC_PDPDEACT;
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "dnsgip") != NULL)
      {
        PRINT_DBG("+QIURC dnsgip info received")
        ug96_current_qiurc_ind = QIURC_DNSGIP;
        if (p_atp_ctxt->current_SID != (at_msg_t) SID_CS_DNS_REQ)
        {
          /* URC not expected */
          retval = ATACTION_RSP_URC_IGNORED;
        }
      }
      else
      {
        PRINT_ERR("+QIURC field not managed")
      }
    }
    else
    {
      PRINT_ERR("param ignored (exceed maximum size)")
      retval = ATACTION_RSP_IGNORED;
    }
  }
  else if (element_infos->param_rank == 3U)
  {
    uint32_t connectID;
    uint32_t contextID;
    socket_handle_t sockHandle;

    switch (ug96_current_qiurc_ind)
    {
      case QIURC_RECV:
        /* <connectID> */
        connectID = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
        sockHandle = atcm_socket_get_socket_handle(p_modem_ctxt, connectID);
        (void) atcm_socket_set_urc_data_pending(p_modem_ctxt, sockHandle);
        PRINT_DBG("+QIURC received data for connId=%ld (socket handle=%ld)", connectID, sockHandle)
        /* last param */
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case QIURC_CLOSED:
        /* <connectID> */
        connectID = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
        sockHandle = atcm_socket_get_socket_handle(p_modem_ctxt, connectID);
        (void) atcm_socket_set_urc_closed_by_remote(p_modem_ctxt, sockHandle);
        PRINT_DBG("+QIURC closed for connId=%ld (socket handle=%ld)", connectID, sockHandle)
        /* last param */
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case QIURC_INCOMING:
        /* <connectID> */
        PRINT_DBG("+QIURC incoming for connId=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        break;

      case QIURC_PDPDEACT:
        /* <contextID> */
        contextID = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
        PRINT_DBG("+QIURC pdpdeact for contextID=%ld", contextID)
        /* Need to inform  upper layer if pdn event URC has been subscribed
         * apply same treatment than CGEV NW PDN DEACT
        */
        p_modem_ctxt->persist.pdn_event.conf_id = atcm_get_configID_for_modem_cid(&p_modem_ctxt->persist,
                                                                                  (uint8_t)contextID);
        /* Indicate that an equivalent to +CGEV URC has been received */
        p_modem_ctxt->persist.pdn_event.event_origine = CGEV_EVENT_ORIGINE_NW;
        p_modem_ctxt->persist.pdn_event.event_scope = CGEV_EVENT_SCOPE_PDN;
        p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_DEACTIVATION;
        p_modem_ctxt->persist.urc_avail_pdn_event = AT_TRUE;
        /* last param */
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case QIURC_DNSGIP:
        /* <err> or <hostIPaddr>]> */
        if (ug96_shared.QIURC_dnsgip_param.wait_header == AT_TRUE)
        {
          /* <err> expected */
          ug96_shared.QIURC_dnsgip_param.error =
            ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
          PRINT_DBG("+QIURC dnsgip with error=%ld", ug96_shared.QIURC_dnsgip_param.error)
          if (ug96_shared.QIURC_dnsgip_param.error != 0U)
          {
            /* Error when trying to get host address */
            ug96_shared.QIURC_dnsgip_param.finished = AT_TRUE;
            retval = ATACTION_RSP_ERROR;
          }
        }
        else
        {
          /* <hostIPaddr> expected
          *  with the current implementation, in case of many possible host IP address, we use
          *  the last one received
          */
          (void) memcpy((void *)ug96_shared.QIURC_dnsgip_param.hostIPaddr,
                        (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                        (size_t) element_infos->str_size);
          PRINT_DBG("+QIURC dnsgip Host address #%ld =%s", ug96_shared.QIURC_dnsgip_param.ip_count,
                    ug96_shared.QIURC_dnsgip_param.hostIPaddr)
          ug96_shared.QIURC_dnsgip_param.ip_count--;
          if (ug96_shared.QIURC_dnsgip_param.ip_count == 0U)
          {
            /* all expected URC have been reecived */
            ug96_shared.QIURC_dnsgip_param.finished = AT_TRUE;
            /* last param */
            retval = ATACTION_RSP_FRC_END;
          }
          else
          {
            retval = ATACTION_RSP_IGNORED;
          }
        }
        break;

      case QIURC_INCOMING_FULL:
      default:
        /* no parameter expected */
        PRINT_ERR("parameter not expected for this URC message")
        break;
    }
  }
  else if (element_infos->param_rank == 4U)
  {
    switch (ug96_current_qiurc_ind)
    {
      case QIURC_INCOMING:
        /* <serverID> */
        PRINT_DBG("+QIURC incoming for serverID=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        break;

      case QIURC_DNSGIP:
        /* <IP_count> */
        if (ug96_shared.QIURC_dnsgip_param.wait_header == AT_TRUE)
        {
          /* <QIURC_dnsgip_param.ip_count> expected */
          ug96_shared.QIURC_dnsgip_param.ip_count =
            ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
          PRINT_DBG("+QIURC dnsgip IP count=%ld", ug96_shared.QIURC_dnsgip_param.ip_count)
          if (ug96_shared.QIURC_dnsgip_param.ip_count == 0U)
          {
            /* No host address available */
            ug96_shared.QIURC_dnsgip_param.finished = AT_TRUE;
            retval = ATACTION_RSP_ERROR;
          }
        }
        break;

      case QIURC_RECV:
      case QIURC_CLOSED:
      case QIURC_PDPDEACT:
      case QIURC_INCOMING_FULL:
      default:
        /* no parameter expected */
        PRINT_ERR("parameter not expected for this URC message")
        break;
    }
  }
  else if (element_infos->param_rank == 5U)
  {
    AT_CHAR_t remoteIP[32] = {0};

    switch (ug96_current_qiurc_ind)
    {
      case QIURC_INCOMING:
        /* <remoteIP> */
        (void) memcpy((void *)&remoteIP[0],
                      (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                      (size_t) element_infos->str_size);
        PRINT_DBG("+QIURC remoteIP for remoteIP=%s", remoteIP)
        break;

      case QIURC_DNSGIP:
        /* <DNS_ttl> */
        if (ug96_shared.QIURC_dnsgip_param.wait_header == AT_TRUE)
        {
          /* <qiurc_dnsgip.ttl> expected */
          ug96_shared.QIURC_dnsgip_param.ttl =
            ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
          PRINT_DBG("+QIURC dnsgip time-to-live=%ld", ug96_shared.QIURC_dnsgip_param.ttl)
          /* no error, now waiting for URC with IP address */
          ug96_shared.QIURC_dnsgip_param.wait_header = AT_FALSE;
        }
        break;

      case QIURC_RECV:
      case QIURC_CLOSED:
      case QIURC_PDPDEACT:
      case QIURC_INCOMING_FULL:
      default:
        /* no parameter expected */
        PRINT_ERR("parameter not expected for this URC message")
        break;
    }
  }
  else if (element_infos->param_rank == 6U)
  {
    switch (ug96_current_qiurc_ind)
    {
      case QIURC_INCOMING:
        /* <remote_port> */
        PRINT_DBG("+QIURC incoming for remote_port=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        /* last param */
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case QIURC_RECV:
      case QIURC_CLOSED:
      case QIURC_PDPDEACT:
      case QIURC_INCOMING_FULL:
      case QIURC_DNSGIP:
      default:
        /* no parameter expected */
        PRINT_ERR("parameter not expected for this URC message")
        break;
    }
  }
  else
  {
    PRINT_ERR("parameter not expected for this URC message")
  }
  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_QCCID_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_INTERMEDIATE; /* received a valid intermediate answer */
  PRINT_API("enter fRspAnalyze_QCCID_UG96()")

  /* analyze parameters for +QCCID */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    PRINT_DBG("ICCID:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    /* UG96 specific treatment:
     *  ICCID reported by the modem includes a blank character (space, code=0x20) at the beginning
     *  remove it if this is the case
     */
    uint16_t src_idx = element_infos->str_start_idx;
    size_t ccid_size = element_infos->str_size;
    if ((p_msg_in->buffer[src_idx] == 0x20U) &&
        (ccid_size >= 2U))
    {
      ccid_size -= 1U;
      src_idx += 1U;
    }

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.iccid),
                  (const void *)&p_msg_in->buffer[src_idx],
                  (size_t)ccid_size);
  }
  else
  {
    /* other parameters ignored */
    __NOP(); /* to avoid warning */
  }
  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_QUSIM_UG96(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_QUSIM_UG96()")

  /*  Quectel WCDMA UGxx AT Commands Manual V1.7
  *   analyze parameters for +QUSIM
  *
  *   it's an URC
  */

  /* this is an URC */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    PRINT_DBG("URC +QUSIM received")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
  }
  END_PARAM_LOOP()

  return (retval);
}

/* Private function Definition ---------------------------------------------- */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

