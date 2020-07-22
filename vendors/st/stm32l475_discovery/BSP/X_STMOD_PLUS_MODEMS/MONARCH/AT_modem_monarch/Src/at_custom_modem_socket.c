/**
  ******************************************************************************
  * @file    at_custom_modem_specific.c
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
#include "at_custom_modem_socket.h"
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
#elif (USE_PRINTF == 1)
#define PRINT_INFO(format, args...)  (void) printf("MONARCH:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("MONARCH ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF == 0U */
#else
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
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

at_status_t fCmdBuild_SQNSCFG_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSCFG_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_modem_ctxt->socket_ctxt.socket_info != NULL)
    {
      /*
        * AT+SQNSCFG=<connId>,<cid>, <pktSz>, <maxTo>,<connTo>, <txTo>
        *
        * <cid> is the PDP context identifier
        * <txTo> : used for online data mode only (NOT SUPPORTED)
        */

      /* convert user cid (CS_PDN_conf_id_t) to PDP modem cid (value) */
      uint8_t pdp_modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist,
                                                          p_modem_ctxt->socket_ctxt.socket_info->conf_id);
      PRINT_DBG("user cid = %d, PDP modem cid = %d",
                (uint8_t)p_modem_ctxt->socket_ctxt.socket_info->conf_id, pdp_modem_cid)
      /* build the command */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld,%d,%d,%d,%d,0",
                     atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle),
                     pdp_modem_cid,
                     p_modem_ctxt->socket_ctxt.socket_info->ip_max_packet_size,
                     p_modem_ctxt->socket_ctxt.socket_info->trp_max_timeout,
                     p_modem_ctxt->socket_ctxt.socket_info->trp_conn_setup_timeout);
    }
    else
    {
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSCFGEXT_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSCFGEXT_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT+SQNSCFGEXT=<connId>,<srMode>, <recvDataMode>,<keepalive>,<listenAutorsp>,<sendDataMode>
      *
      * <keepalive> is currently not used
      *
      * AT+SQNSCFGEXT=<connId>,<srMode>,<recvDataMode>, <keepalive>,[<listenAutoRsp>],[<sendDataMode>]
      */
    uint8_t srMode = 1U;        /* SQNSRING format = 1 => SQNSRING: <connId>,<recData> */
    uint8_t recvDataMode = 0U;  /* 0: as text */
    uint8_t keepalive = 0U;     /* 0 (currently unused) */
    uint8_t listenAutorsp = 0U; /* 0 */
    uint8_t sendDataMode = 0U;  /* 0: as text */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld,%d,%d,%d,%d,%d",
                   atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle),
                   srMode,
                   recvDataMode,
                   keepalive,
                   listenAutorsp,
                   sendDataMode);
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSD_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSD_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_modem_ctxt->socket_ctxt.socket_info != NULL)
    {
      /*
        * AT+SQNSD=<connId>,<txProt>,<rPort>,<IPaddr>
        *          [,<closureType> [,<lPort> [,<connMode>]]]
        *
        */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld,%d,%d,\"%s\",0,%d,%d",
                     atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle),
                     ((p_modem_ctxt->socket_ctxt.socket_info->protocol == CS_TCP_PROTOCOL) ? 0 : 1),
                     p_modem_ctxt->socket_ctxt.socket_info->remote_port,
                     p_modem_ctxt->socket_ctxt.socket_info->ip_addr_value,
                     /*closureType fixed to 0*/
                     p_modem_ctxt->socket_ctxt.socket_info->local_port,
                     ((p_modem_ctxt->socket_ctxt.socket_info->trp_connect_mode == CS_CM_COMMAND_MODE) ? 1 : 0)
                    );
    }
    else
    {
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSH_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSH_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_modem_ctxt->socket_ctxt.socket_info != NULL)
    {
      /*
        * AT+SQNSH=<connId>
        */
      uint32_t connID = atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socket_info->socket_handle);
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld", connID);
    }
    else
    {
      retval = ATSTATUS_ERROR;
    }
  }
  return (retval);
}

at_status_t fCmdBuild_SQNSSENDEXT_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSSENDEXT_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * send data in command mode:
      * AT+SQNSSENDEXT=<connId>,<bytes to send><CR>
      * > ...DATA...
      *
      * DATA are sent using fCmdBuild_WRITE_SOCKET_DATA_MONARCH()
      */

    /* FIXED SIZE MODE
      * prepare message with connectionId, length and datas
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld,%ld",
                   atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->SID_ctxt.socketSendData_struct.socket_handle),
                   p_modem_ctxt->SID_ctxt.socketSendData_struct.buffer_size);
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSSEND_WRITE_DATA_MONARCH(atparser_context_t *p_atp_ctxt,
                                                  atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSSEND_WRITE_DATA_MONARCH()")

  /* after having send AT+SQNSSENDEXT and prompt received, now send DATA */

  /* only for raw command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD)
  {
    if (p_modem_ctxt->SID_ctxt.socketSendData_struct.p_buffer_addr_send != NULL)
    {
      uint32_t str_size = p_modem_ctxt->SID_ctxt.socketSendData_struct.buffer_size;
      (void) memcpy((void *)p_atp_ctxt->current_atcmd.params,
                    (const CRC_CHAR_t *)p_modem_ctxt->SID_ctxt.socketSendData_struct.p_buffer_addr_send,
                    (size_t) str_size);

      /* FIXED SIZE MODE: set raw command size */
      p_atp_ctxt->current_atcmd.raw_cmd_size = str_size;
    }
    else
    {
      PRINT_ERR("ERROR, send buffer is a NULL ptr !!!")
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSI_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSI_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT+SQNSI=<connId>
      */
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RECEIVE_DATA)
    {
      /* request socket infos to know amount of data available */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld",
                     atcm_socket_get_modem_cid(p_modem_ctxt,
                                               p_modem_ctxt->socket_ctxt.socketReceivedata.socket_handle));
    }
  }

  return (retval);
}

at_status_t fCmdBuild_SQNSRECV_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_SQNSRECV_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      * AT+SQNSRECV=<connId>,<maxByte>
      *
      * implementation: in <maxByte>, we put the value received in +SQNSI
      * The value socket_rx_expected_buf_size used to fill this field has been already checked.
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld,%ld",
                   atcm_socket_get_modem_cid(p_modem_ctxt, p_modem_ctxt->socket_ctxt.socketReceivedata.socket_handle),
                   p_modem_ctxt->socket_ctxt.socket_rx_expected_buf_size);

    /* ready to start receive socket buffer */
    p_modem_ctxt->socket_ctxt.socket_RxData_state = SocketRxDataState_waiting_header;
  }

  return (retval);

}

at_status_t fCmdBuild_PING_MONARCH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_PING_MONARCH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /*
      *  Ping a remote server:
      *  AT+PING=<IPaddr>,<count>[,<len>[,<interval>[,<timeout>[,<ttl>[,<cid>]]]]]]
      *    <IPaddr> (str): Address of the remote host.
      *             Any valid IP address in the format “xxx.xxx.xxx.xxx” or any host name solved with a DNS query.
      *    <count> (int)[1-64]: Number of Ping Echo Request to send (default: 4)
      *             Ping stop after sending <count> ECHO_REQUEST packets.
      *             With deadline option, ping waits for count ECHO_REPLY packets, until the timeout expires.
      *    <len> (int)[32-1400]: Length of Ping Echo Request message (default: 32).
      *    <interval> (int)[1-600]: Wait interval seconds between sending each Ping Echo Request (default: 1)
      *    <timeout> (int)[1-60]: Time to wait for a Echo Reply (in seconds)(default: 10).
      *                           The option affects only timeout in absence of any responses,
      *                           otherwise ping waits for two RTTs.
      *    <ttl> (int)[1-255]: IP time to live (default: 128)
      *    <cid> (int)[0-6]: PDP context identifier (default: Internet PDN)
      */

    /* validate ping request
     * As PING request are synchronous for this modem, always validate PING request before to send PING command.
     */
    atcm_validate_ping_request(p_modem_ctxt);
    /* prepare request */
    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
    uint8_t ttl = 128U;
    uint16_t interval = 1U;
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"%s\",%d,%d,%d,%d,%d,%d",
                   p_modem_ctxt->SID_ctxt.ping_infos.ping_params.host_addr,
                   p_modem_ctxt->SID_ctxt.ping_infos.ping_params.pingnum,
                   SEQUANS_PING_LENGTH,
                   interval,
                   p_modem_ctxt->SID_ctxt.ping_infos.ping_params.timeout,
                   ttl,
                   modem_cid);
  }

  return (retval);
}

/* Analyze command functions ------------------------------------------------------- */

at_action_rsp_t fRspAnalyze_SQNSRING_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                             const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSRING()")

  /*
    * analyze parameters for +SQNSRING
    *  SQNSRING: <connId>
    *  or SQNSRING: <connId>,<recData>
    *  or SQNSRING: <connId>,<recData>,<data>
    *
    * where recData = size of received data
    */

  /* this is an URC */
  START_PARAM_LOOP();
  if (element_infos->param_rank == 2U)
  {
    uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
    socket_handle_t sockHandle = atcm_socket_get_socket_handle(p_modem_ctxt, connId);
    (void) atcm_socket_set_urc_data_pending(p_modem_ctxt, sockHandle);
    PRINT_INFO("<SEQMONARCH custom> +SQNSRING URC for connId=%ld (socket handle=%ld)",
               connId,
               sockHandle)
  }
  if (element_infos->param_rank == 3U)
  {
    /* <recData> - parameter not used */
    PRINT_DBG("<SEQMONARCH custom> +SQNSRING URC recData=%ld bytes",
              ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
  }
  if (element_infos->param_rank == 4U)
  {
    /* data in command not supported actually (and probably never) */
    PRINT_ERR("error, receiving data in SQNSRING not supported !!! ")
  }
  END_PARAM_LOOP();

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNSI_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                          const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSI()")

  /*
    * +SQNSI:<connId>,<sent>,<received>,<buff_in>,<ack_waiting>
    */
  START_PARAM_LOOP();
  if (element_infos->param_rank == 2U)
  {
    /* <connId> */
    uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
    p_modem_ctxt->socket_ctxt.socket_current_connId = connId;
    PRINT_DBG("<SEQMONARCH custom> +SQNSI: connId=%ld", connId)
  }
  if (element_infos->param_rank == 3U)
  {
    /* <sent> - parameter not used */
    PRINT_DBG("<SEQMONARCH custom> +SQNSI: sent=%ld",
              ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
  }
  if (element_infos->param_rank == 4U)
  {
    /* <received> - parameter not used */
    PRINT_DBG("<SEQMONARCH custom> +SQNSI: received=%ld",
              ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
  }
  if (element_infos->param_rank == 5U)
  {
    /* <buff_in> */
    uint32_t buff_in = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                 element_infos->str_size);
    PRINT_DBG("<SEQMONARCH custom> +SQNSI: buff_in=%ld", buff_in)

    /* check that the size to read does not exceed maximum size ( = client buffer size) */
    if (buff_in > p_modem_ctxt->socket_ctxt.socketReceivedata.max_buffer_size)
    {
      /* The size to read exceed client buffer size.
       * Limit the value to max client buffer size.
       */
      p_modem_ctxt->socket_ctxt.socket_rx_expected_buf_size =
        p_modem_ctxt->socket_ctxt.socketReceivedata.max_buffer_size;
      PRINT_INFO("Limit request to maximum buffer size (%ld) whereas more data are available (%ld)",
                 p_modem_ctxt->socket_ctxt.socketReceivedata.max_buffer_size,
                 buff_in)
    }
    else
    {
      /* Update size to read */
      p_modem_ctxt->socket_ctxt.socket_rx_expected_buf_size = buff_in;
    }
  }
  if (element_infos->param_rank == 6U)
  {
    /* <ack_waiting> - parameter not used */
    PRINT_DBG("<SEQMONARCH custom> +SQNSI: ack_waiting=%ld",
              ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
  }
  END_PARAM_LOOP();

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNSS_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                          const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSS()")
  at_bool_t monarch_snqss_for_requested_socket = AT_FALSE;

  /*
    * +SQNSS: <connId1>,<state1>,<locIP1>,<locPort1>,<remIP1>,<remPort1><CR><LF>
    * ...
    * +SQNSS: <connId6>,<state6>,<locIP6>,<locPort6>,<remIP6>,<remPort6><CR><LF>
    *
    *  if a channel is close (state=0), <locIP>,<locPort>,<remIP>,<remPort> are omitted
    */

  START_PARAM_LOOP();
  if (element_infos->param_rank == 2U)
  {
    /* <connId> */
    uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
    socket_handle_t sockHandle = atcm_socket_get_socket_handle(p_modem_ctxt, connId);
    /* if this connection ID corresponds to requested socket handle, we will report the following infos */
    if (sockHandle == p_modem_ctxt->socket_ctxt.socket_cnx_infos->socket_handle)
    {
      monarch_snqss_for_requested_socket = AT_TRUE;
    }
    else
    {
      monarch_snqss_for_requested_socket = AT_FALSE;
    }
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: connId=%ld (requested=%d)",
              connId, ((monarch_snqss_for_requested_socket == AT_TRUE) ? 1 : 0))
  }
  else if (element_infos->param_rank == 3U)
  {
    /* <state> - parameter not used */
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: state=%ld",
              ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
  }
  else if (element_infos->param_rank == 4U)
  {
    /* <locIP> */
    atcm_extract_IP_address((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx],
                            (uint16_t) element_infos->str_size,
                            (uint8_t *) p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->loc_ip_addr_value);
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: locIP=%s",
              p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->loc_ip_addr_value)
  }
  else if (element_infos->param_rank == 5U)
  {
    /* <locPort> */
    uint32_t locPort = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                 element_infos->str_size);
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: locPort=%ld", locPort)

    if (monarch_snqss_for_requested_socket == AT_TRUE)
    {
      p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->loc_port = (uint16_t) locPort;
    }
  }
  else if (element_infos->param_rank == 6U)
  {
    /* <remIP> */
    atcm_extract_IP_address((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx],
                            (uint16_t) element_infos->str_size,
                            (uint8_t *) p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->rem_ip_addr_value);
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: remIP=%s",
              p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->rem_ip_addr_value)
  }
  else if (element_infos->param_rank == 7U)
  {
    /* <remPort> */
    uint32_t remPort = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                 element_infos->str_size);
    PRINT_DBG("<SEQMONARCH custom> +SQNSS: remPort=%ld", remPort)

    if (monarch_snqss_for_requested_socket == AT_TRUE)
    {
      p_modem_ctxt->socket_ctxt.socket_cnx_infos->infos->rem_port = (uint16_t) remPort;
    }
  }
  END_PARAM_LOOP();

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNSH_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                          const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSH_MONARCH()")

  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t)CMD_AT_SQNSH)
  {
    /*
      * +SQNSH:<connId>
      */
    START_PARAM_LOOP();
    if (element_infos->param_rank == 2U)
    {
      /* <connId> */
      uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
      p_modem_ctxt->socket_ctxt.socket_current_connId = connId;
      PRINT_INFO("<SEQMONARCH custom> +SQNSH: connId=%ld", connId)
    }
    END_PARAM_LOOP();
  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP();
    if (element_infos->param_rank == 2U)
    {
      /* <connId> */
      uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
      socket_handle_t sockHandle = atcm_socket_get_socket_handle(p_modem_ctxt, connId);
      (void) atcm_socket_set_urc_closed_by_remote(p_modem_ctxt, sockHandle);
      PRINT_INFO("<SEQMONARCH custom> URC +SQNSH: connId=%ld (socket handle=%ld)",
                 connId,
                 sockHandle)
    }
    END_PARAM_LOOP();
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNSRECV_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                             const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSRECV()")

  /*
    * +SQNSRECV:<connId>,<bytes_received>
    */
  START_PARAM_LOOP();
  if (element_infos->param_rank == 2U)
  {
    /* <connId> */
    uint32_t connId = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
    PRINT_DBG("<SEQMONARCH custom> +SQNSRECV: connId=%ld", connId)

    /* check if connId received is the one we asked */
    if (connId != p_modem_ctxt->socket_ctxt.socket_current_connId)
    {
      PRINT_ERR("<SEQMONARCH custom> error, data received not on the expected socket (%ld instead of %ld)", connId,
                p_modem_ctxt->socket_ctxt.socket_current_connId)
      retval = ATACTION_RSP_ERROR;
    }
  }
  if (element_infos->param_rank == 3U)
  {
    /* <bytes_received> */
    /* NOTE !!! the size is purely informative in current implementation
      *  indeed, due to real time constraints, the socket data header is analyzed directly
      * in ATCustom_MONARCH_checkEndOfMsgCallback()
      */
    PRINT_INFO("<SEQMONARCH custom> +SQNSRECV: bytes_received=%ld",
               ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))


    if (p_modem_ctxt->socket_ctxt.socket_receive_state == SocketRcvState_RequestData_Header)
    {
      /* data header analyzed, ready to analyze data payload */
      p_modem_ctxt->socket_ctxt.socket_receive_state = SocketRcvState_RequestData_Payload;
    }
  }
  END_PARAM_LOOP();

  return (retval);
}

at_action_rsp_t fRspAnalyze_SQNSRECV_data_MONARCH(at_context_t *p_at_ctxt,
                                                  atcustom_modem_context_t *p_modem_ctxt,
                                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_SQNSRECV_data()")

  PRINT_DBG("DATA received: size=%ld vs %d", p_modem_ctxt->socket_ctxt.socket_rx_expected_buf_size,
            element_infos->str_size)

  /* Recopy data to client buffer if:
  *   - pointer on data buffer exists
  *   - and size of data <= maximum size
  */
  if ((p_modem_ctxt->socket_ctxt.socketReceivedata.p_buffer_addr_rcv != NULL) &&
      (element_infos->str_size <= p_modem_ctxt->socket_ctxt.socketReceivedata.max_buffer_size))
  {
    /* recopy data to client buffer */
    (void)memcpy((void *)p_modem_ctxt->socket_ctxt.socketReceivedata.p_buffer_addr_rcv,
                 (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                 (size_t) element_infos->str_size);
    p_modem_ctxt->socket_ctxt.socketReceivedata.buffer_size = element_infos->str_size;
  }
  else
  {
    PRINT_ERR("ERROR (receive buffer is a NULL ptr or data exceed buffer size)")
    retval = ATACTION_RSP_ERROR;
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_PING_MONARCH(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                         const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  at_action_rsp_t retval;
#if (GM01Q_ACTIVATE_PING_REPORT == 1)
  retval = ATACTION_RSP_URC_FORWARDED; /* received a valid intermediate answer */
#else
  retval = ATACTION_RSP_INTERMEDIATE; /* because report is not managed actually */
#endif /* (GM01Q_ACTIVATE_PING_REPORT == 1) */

  PRINT_API("enter fRspAnalyze_PING_MONARCH()")

  START_PARAM_LOOP()

  /*  answer to AT+PING
    *  Once the single Echo Reply message is receive +PING notification will be displayed on AT channel:
    *  +PING: <replyId>,<IPaddr>,<time>,<ttl>
    *  With :
    *    <replyId>: Echo Reply number
    *    <IPaddr>: Remote host IP address
    *    <time>: time required to receive the response (in ms)
    *    <ttl>: time to live of the Echo Reply message
    * Note:
    *   When the Echo Request timeout expires (no reply received on time) the response
    *   will contain <time>=-1 and <ttl>=-1
    */

  if (element_infos->param_rank == 2U)
  {
    /* new ping response: clear ping response structure */
    clear_ping_resp_struct(p_modem_ctxt);
    p_modem_ctxt->persist.ping_resp_urc.ping_status = CELLULAR_ERROR; /* will be updated if all params are correct */

    /* <replyId> */
    uint32_t replyid = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                 element_infos->str_size);
    PRINT_DBG("+PING <replyid> = %ld", replyid)

#if (GM01Q_ACTIVATE_PING_REPORT == 1)
    p_modem_ctxt->persist.urc_avail_ping_rsp = AT_TRUE;
#endif /* (GM01Q_ACTIVATE_PING_REPORT == 1) */
    p_modem_ctxt->persist.ping_resp_urc.index = (uint8_t) replyid;
    PRINT_DBG("intermediate ping report")
    p_modem_ctxt->persist.ping_resp_urc.is_final_report = CELLULAR_FALSE;
    /* Sequans modem does not provide length, it's an input parameter */
    p_modem_ctxt->persist.ping_resp_urc.ping_size = SEQUANS_PING_LENGTH;
  }
  else if (element_infos->param_rank == 3U)
  {
    /* <IPaddr> */
    atcm_extract_IP_address((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx],
                            (uint16_t) element_infos->str_size,
                            (uint8_t *) p_modem_ctxt->persist.ping_resp_urc.ping_addr);
    PRINT_DBG("+PING <IPaddr>=%s", p_modem_ctxt->persist.ping_resp_urc.ping_addr)
  }
  else if (element_infos->param_rank == 4U)
  {
    /* <time> */
    uint8_t neg = ATutil_isNegative(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
    if (neg == 1U)
    {
      /* +PING <time> negative value detected (invalid) */
      p_modem_ctxt->persist.ping_resp_urc.time = 0xFFFFU;
    }
    else
    {
      /* get <time> */
      uint32_t time = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      PRINT_DBG("+PING <time> = %ld", time)
      p_modem_ctxt->persist.ping_resp_urc.time = (uint32_t)time;
    }
  }
  else if (element_infos->param_rank == 5U)
  {
    /* <ttl> */
    uint8_t neg = ATutil_isNegative(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
    if (neg == 1U)
    {
      /* +PING <ttl> negative value detected (invalid) */
      p_modem_ctxt->persist.ping_resp_urc.ttl = 0xFFU;
    }
    else
    {
      /* get <ttl> */
      uint32_t ttl = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                               element_infos->str_size);
      PRINT_DBG("+PING <ttl> = %ld", ttl)
      p_modem_ctxt->persist.ping_resp_urc.ttl = (uint8_t) ttl;

      /* finally, after having received all parameters, set the final status for this ping */
      if (p_modem_ctxt->persist.ping_resp_urc.time != 0xFFFFU)
      {
        p_modem_ctxt->persist.ping_resp_urc.ping_status = CELLULAR_OK;
      }
    }
  }

  END_PARAM_LOOP()

  return (retval);
}

/*----------------------------------------------------------------------------*/
void clear_ping_resp_struct(atcustom_modem_context_t *p_modem_ctxt)
{
  /* clear all parameters except the index:
    * save the index, reset the structure and recopy saved index
    */
  uint8_t saved_idx = p_modem_ctxt->persist.ping_resp_urc.index;
  (void) memset((void *)&p_modem_ctxt->persist.ping_resp_urc, 0, sizeof(CS_Ping_response_t));
  p_modem_ctxt->persist.ping_resp_urc.index = saved_idx;
}

/* Private function Definition -----------------------------------------------*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

