/**
  ******************************************************************************
  * @file    at_parser.c
  * @author  MCD Application Team
  * @brief   This file provides code for AT Parser
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "at_core.h"
#include "at_parser.h"
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "at_modem_signalling.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define FILTER_SOCKET_TRACES   (1U)   /* filter display of big frame in trace output */
#define FILTER_DEFAULT_LENGH   (300U)  /* default lengh to activate the filter */

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATPARSER == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "ATParser:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "ATParser:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "ATParser ERROR:" format "\n\r", ## args)
#define PRINT_BUF(pbuf, size)       TRACE_PRINT_BUF_CHAR(DBG_CHAN_ATCMD, DBL_LVL_P0, (const CRC_CHAR_t *)pbuf, size);
#define PRINT_BUF_HEXA(pbuf, size)  TRACE_PRINT_BUF_HEX(DBG_CHAN_ATCMD, DBL_LVL_P0, (const CRC_CHAR_t *)pbuf, size);
#define PRINT_INDENT()              TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "    ")
#else
uint8_t print_bufffer[3000];
#define PRINT_INFO(format, args...)  (void) printf("ATParser:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("ATParser ERROR:" format "\n\r", ## args);
#define PRINT_BUF(...)        __NOP(); /* Nothing to do */
#define PRINT_BUF_HEXA(...)   __NOP(); /* Nothing to do */
#define PRINT_INDENT()               (void) printf("    ");
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)    __NOP(); /* Nothing to do */
#define PRINT_ERR(...)    __NOP(); /* Nothing to do */
#define PRINT_BUF(...)    __NOP(); /* Nothing to do */
#define PRINT_BUF_HEXA(...) __NOP(); /* Nothing to do */
#define PRINT_INDENT(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATPARSER */

/* Private variables ---------------------------------------------------------*/
#define MAX_CMD_FORMAT_SIZE (8U)
typedef struct
{
  AT_CHAR_t    cmd_prefix[MAX_CMD_FORMAT_SIZE];
  uint16_t     cmd_prefix_size;
  AT_CHAR_t    cmd_separator[MAX_CMD_FORMAT_SIZE];
  uint16_t     cmd_separator_size;
} cmd_format_LUT_t;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void reset_parser_context(atparser_context_t *p_atp_ctxt);
static void reset_current_command(atparser_context_t *p_atp_ctxt);
static void display_buffer(const at_context_t *p_at_ctxt, const uint8_t *p_buf, uint16_t buf_size, uint8_t is_TX_buf);
static uint16_t build_command(at_context_t *p_at_ctxt, uint8_t *p_ATcmdBuf, uint16_t ATcmdBuf_maxSize);
static bool write_data2buffer(uint8_t *p_ATcmdBuf, const AT_CHAR_t *p_str, uint16_t str_size,
                              uint16_t *cmd_total_length, uint16_t *remaining_size);

/* Functions Definition ------------------------------------------------------*/
at_status_t ATParser_initParsers(sysctrl_device_type_t device_type)
{
  return (atcc_initParsers(device_type));
}

void ATParser_init(at_context_t *p_at_ctxt, IPC_CheckEndOfMsgCallbackTypeDef *p_checkEndOfMsgCallback)
{
  /* reset request context */
  reset_parser_context(&p_at_ctxt->parser);

  /* get callback pointer */
  *p_checkEndOfMsgCallback = atcc_checkEndOfMsgCallback(p_at_ctxt);

  /* default termination string for AT command: <CR>
   * this value can be changed in ATCustom init if needed
   */
  (void) memset((AT_CHAR_t *)p_at_ctxt->parser.endstr, 0, AT_CMD_MAX_END_STR_SIZE);
  (void) sprintf((CRC_CHAR_t *)p_at_ctxt->parser.endstr, "\r");

  /* call custom init */
  atcc_init(p_at_ctxt);
}

void  ATParser_process_request(at_context_t *p_at_ctxt,
                               at_msg_t msg_id, at_buf_t *p_cmd_buf)
{
  /* reset the context */
  reset_parser_context(&p_at_ctxt->parser);

  /* save ptr on input cmd buffer */
  p_at_ctxt->parser.p_cmd_input = p_cmd_buf;

  /* set the SID */
  p_at_ctxt->parser.current_SID = msg_id;
}

/**
  * @brief  Write to the buffer
  * @param  p_ATcmdBuf Pointer to the command buffer to build.
  * @param  p_str Pointer to the string to write.
  * @param  str_size Size of the string to write.
  * @param  cmd_total_length Pointer to total buffer size.
  * @param  remaining_size Pointer to remaining buffer size.
  * @retval returns true if succeed to write to buffer
  */
static bool write_data2buffer(uint8_t *p_ATcmdBuf, const AT_CHAR_t *p_str, uint16_t str_size,
                              uint16_t *p_cmd_total_length, uint16_t *p_remaining_size)
{
  bool retval;

  if ((str_size > 0U) && (str_size < *p_remaining_size))
  {
    (void) memcpy((void *) &p_ATcmdBuf[*p_cmd_total_length],
                  (const AT_CHAR_t *)p_str,
                  str_size);
    *p_cmd_total_length += str_size;
    *p_remaining_size -= str_size;
    retval = true;
  }
  else
  {
    /* invalid size */
    retval = false;
  }

  return (retval);
}

/**
  * @brief  The function build the command buffer.
  * @param  p_at_ctxt Pointer to AT context structure.
  * @param  p_ATcmdBuf Pointer to the command buffer to build.
  * @retval return the size of the command buffer.
  */
static uint16_t build_command(at_context_t *p_at_ctxt, uint8_t *p_ATcmdBuf, uint16_t ATcmdBuf_maxSize)
{
  /*
    * AT commands format:
    * AT+<X>=?    : TEST COMMAND
    * AT+<X>?     : READ COMMAND
    * AT+<X>=...  : WRITE COMMAND
    * AT+<X>      : EXECUTION COMMAND
    *
    * A command can be decomposed as follow:
    * <cmd_prefix><cmd_name><cmd_sep><cmd_params><cmd_endstr>
    * where:
    *  <cmd_prefix> is the command prefix (exple: "AT")
    *  <cmd_name> is the command name (exple: "+CFUN")
    *  <cmd_sep> is the separator between name and params for this type of command (exple: for a write cmd we have "=")
    *  <cmd_params> are the command parameters (exple: "1,0")
    *  <cmd_endstr> is the command termination string (exple: carriage return)
    *  For the example above, the command generated is "AT+CFUN=1,0<CR>"
  */

  static const cmd_format_LUT_t CMD_FORMAT[ATTYPE_MAX_VAL] =
  {
    /* <cmd_prefix> <cmd_prefix_size> <cmd_separator> <cmd_separator_size> */
    {  "", 0U,   "", 0U},  /* ATTYPE_UNKNOWN_CMD                 */
    {"AT", 2U, "=?", 2U},  /* ATTYPE_TEST_CMD:      AT+<x>=?     */
    {"AT", 2U,  "?", 1U},  /* ATTYPE_READ_CMD:      AT+<x>?      */
    {"AT", 2U,  "=", 1U},  /* ATTYPE_WRITE_CMD:     AT+<x>=<...> */
    {"AT", 2U,   "", 0U},  /* ATTYPE_EXECUTION_CMD: AT+<x>       */
    {  "", 0U,   "", 0U},  /* ATTYPE_NO_CMD                      */
    {  "", 0U,   "", 0U}   /* ATTYPE_RAW_CMD                     */
  };

  uint16_t cmd_total_length;
  at_type_t cmd_type = p_at_ctxt->parser.current_atcmd.type;

  if ((cmd_type == ATTYPE_TEST_CMD) ||
      (cmd_type == ATTYPE_READ_CMD) ||
      (cmd_type == ATTYPE_WRITE_CMD) ||
      (cmd_type == ATTYPE_EXECUTION_CMD))
  {
    /* start to build the command with the format:
    * <cmd_prefix><cmd_name><cmd_sep><cmd_params><cmd_endstr>
    */
    cmd_total_length = 0;
    const AT_CHAR_t *p_str;
    uint16_t str_size;
    uint16_t remaining_size = ATcmdBuf_maxSize;

    /* build <cmd_prefix> part */
    p_str = CMD_FORMAT[cmd_type].cmd_prefix;
    str_size = CMD_FORMAT[cmd_type].cmd_prefix_size;
    (void) write_data2buffer(p_ATcmdBuf, p_str, str_size, &cmd_total_length, &remaining_size);

    /* build <cmd_name> part */
    p_str = p_at_ctxt->parser.current_atcmd.name;
    str_size = (uint16_t) strlen((CRC_CHAR_t *) &p_at_ctxt->parser.current_atcmd.name);
    (void) write_data2buffer(p_ATcmdBuf, p_str, str_size, &cmd_total_length, &remaining_size);

    /* build <cmd_sep> part */
    p_str = &CMD_FORMAT[cmd_type].cmd_separator[0];
    str_size = CMD_FORMAT[cmd_type].cmd_separator_size;
    (void) write_data2buffer(p_ATcmdBuf, p_str, str_size, &cmd_total_length, &remaining_size);

    /* build <cmd_params> part */
    p_str = p_at_ctxt->parser.current_atcmd.params;
    str_size = (uint16_t) strlen((CRC_CHAR_t *) &p_at_ctxt->parser.current_atcmd.params);
    (void) write_data2buffer(p_ATcmdBuf, p_str, str_size, &cmd_total_length, &remaining_size);

    /* build <cmd_endstr> part */
    p_str = p_at_ctxt->parser.endstr;
    str_size = (uint16_t) strlen((CRC_CHAR_t *) &p_at_ctxt->parser.endstr);
    (void) write_data2buffer(p_ATcmdBuf, p_str, str_size, &cmd_total_length, &remaining_size);
  }
  else if (cmd_type == ATTYPE_RAW_CMD)
  {
    /* RAW command: command with NON-AT format
    * send it as provided without header and without end string
    * raw cmd content has been copied into parser.current_atcmd.params
    * its size is in parser.current_atcmd.raw_cmd_size
    *
    */
    if ((p_at_ctxt->parser.current_atcmd.raw_cmd_size != 0U)
        && (p_at_ctxt->parser.current_atcmd.raw_cmd_size <= ATcmdBuf_maxSize))
    {
      (void) memcpy((void *)p_ATcmdBuf,
                    (void *)p_at_ctxt->parser.current_atcmd.params,
                    p_at_ctxt->parser.current_atcmd.raw_cmd_size);
      cmd_total_length = (uint16_t)p_at_ctxt->parser.current_atcmd.raw_cmd_size;
    }
    else
    {
      PRINT_ERR("Error with RAW command size = %ld", p_at_ctxt->parser.current_atcmd.raw_cmd_size)
      cmd_total_length = 0U;
    }
  }
  else if (cmd_type == ATTYPE_NO_CMD)
  {
    /* no command to send */
    cmd_total_length = 0U;
    PRINT_DBG("no command to send")
  }
  else /* default */
  {
    cmd_total_length = 0U;
    PRINT_ERR("invalid command type")
  }


  return (cmd_total_length);
}

at_action_send_t  ATParser_get_ATcmd(at_context_t *p_at_ctxt,
                                     uint8_t *p_ATcmdBuf,
                                     uint16_t ATcmdBuf_maxSize,
                                     uint16_t *p_ATcmdSize, uint32_t *p_ATcmdTimeout)
{
  at_action_send_t action = ATACTION_SEND_NO_ACTION;

  /* init command paramters */
  *p_ATcmdSize = 0U;
  reset_current_command(&p_at_ctxt->parser);

  /* get the next command to send and set timeout value */
  if (atcc_getCmd(p_at_ctxt, p_ATcmdTimeout) != ATSTATUS_OK)
  {
    PRINT_DBG("parser f_getCmd error")
    action = ATACTION_SEND_ERROR;
  }

  if (action != ATACTION_SEND_ERROR)
  {
    /* test if cmd is not invalid */
    if (p_at_ctxt->parser.current_atcmd.id != CMD_AT_INVALID)
    {
      /* build the command buffer */
      *p_ATcmdSize = build_command(p_at_ctxt, p_ATcmdBuf, ATcmdBuf_maxSize);
    }

    /* Prepare returned code (if no error) */
    if (p_at_ctxt->parser.answer_expected == CMD_MANDATORY_ANSWER_EXPECTED)
    {
      action |= ATACTION_SEND_WAIT_MANDATORY_RSP;
    }
    else if (p_at_ctxt->parser.answer_expected == CMD_OPTIONAL_ANSWER_EXPECTED)
    {
      action |= ATACTION_SEND_TEMPO;
    }
    else /* NO ANSWER EXPECTED */
    {
      PRINT_ERR("Invalid answer_expected value")
      action = ATACTION_SEND_ERROR;
    }
  }

  if (action != ATACTION_SEND_ERROR)
  {
    /* set last command flag if needed */
    if (p_at_ctxt->parser.is_final_cmd == 1U)
    {
      action |= ATACTION_SEND_FLAG_LAST_CMD;
    }

    /* DUMP SEND BUFFER */
    display_buffer(p_at_ctxt,
                   (uint8_t *)p_ATcmdBuf,
                   (uint16_t)*p_ATcmdSize, 1U);
  }

  PRINT_DBG("ATParser_get_ATcmd returned action = 0x%x", action)
  return (action);
}

at_action_rsp_t ATParser_parse_rsp(at_context_t *p_at_ctxt, IPC_RxMessage_t *p_message)
{
  at_action_rsp_t cmd_retval, param_retval, final_retval, clean_retval;
  at_endmsg_t msg_end;
  at_element_info_t element_infos = { .current_parse_idx = 0, .cmd_id_received = CMD_AT_INVALID, .param_rank = 0U,
                                      .str_start_idx = 0, .str_end_idx = 0, .str_size = 0
                                    };
  uint16_t data_mode;

  /* DUMP RECEIVE BUFFER */
  display_buffer(p_at_ctxt,
                 (uint8_t *)&p_message->buffer[0],
                 (uint16_t)p_message->size, 0U);

  /* extract next element to analyze */
  msg_end = atcc_extractElement(p_at_ctxt, p_message, &element_infos);

  /* Search for command name */
  cmd_retval = atcc_analyzeCmd(p_at_ctxt, p_message, &element_infos);

  /* extract return code only (remove data mode flag) */
  clean_retval = (at_action_rsp_t)(cmd_retval & ~(at_action_rsp_t)ATACTION_RSP_FLAG_DATA_MODE);
  data_mode = (((uint16_t)cmd_retval & (uint16_t)ATACTION_RSP_FLAG_DATA_MODE) != 0U) ? (uint16_t)1U : (uint16_t)0U;
  PRINT_DBG("analyzeCmd retval = %d (DATA mode=%d) msg_end = %d", clean_retval, data_mode, (msg_end == ATENDMSG_YES))

  if ((msg_end != ATENDMSG_YES) && (cmd_retval != ATACTION_RSP_ERROR))
  {
    PRINT_DBG("proceed to params")

    /* Search for command parameters */
    param_retval = atcc_analyzeParam(p_at_ctxt, p_message, &element_infos);
    if (param_retval != ATACTION_RSP_IGNORED)
    {
      /* action has been modified by analysis of parameters */
      PRINT_DBG("action modified by analysis of params: %d to %d", cmd_retval, param_retval)
      clean_retval = param_retval;
    }
  }

  /* if AT cmd treatment is finished, check if another AT cmd should be sent after */
  if (clean_retval == ATACTION_RSP_FRC_END)
  {
    /* final result code for current AT cmd: clean related context params */
    final_retval = atcc_terminateCmd(p_at_ctxt, &element_infos);
    if (final_retval == ATACTION_RSP_ERROR)
    {
      clean_retval = ATACTION_RSP_ERROR;
    }
    /* do we have another command to send for this SID ? */
    else if (p_at_ctxt->parser.is_final_cmd == 0U)
    {
      clean_retval = ATACTION_RSP_FRC_CONTINUE;
    }
    else
    {
      /* ignore */
    }

    /* current CMD treament is finished: reset command context */
    reset_current_command(&p_at_ctxt->parser);
  }

  /* reintegrate data mode flag if needed */
  if (data_mode == 1U)
  {
    cmd_retval = (at_action_rsp_t)(clean_retval | ATACTION_RSP_FLAG_DATA_MODE);
  }
  else
  {
    cmd_retval = clean_retval;
  }

  PRINT_DBG("ATParser_parse_rsp returned action = 0x%x", cmd_retval)
  return (cmd_retval);
}

at_status_t ATParser_get_rsp(at_context_t *p_at_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;

  retval = atcc_get_rsp(p_at_ctxt, p_rsp_buf);

  /* current SID treament is finished, reset parser context */
  reset_parser_context(&p_at_ctxt->parser);

  return (retval);
}

at_status_t ATParser_get_urc(at_context_t *p_at_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;

  retval = atcc_get_urc(p_at_ctxt, p_rsp_buf);
  return (retval);
}

at_status_t ATParser_get_error(at_context_t *p_at_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;

  retval = atcc_get_error(p_at_ctxt, p_rsp_buf);
  return (retval);
}

void ATParser_abort_request(at_context_t *p_at_ctxt)
{
  reset_parser_context(&p_at_ctxt->parser);
}

/* Private function Definition -----------------------------------------------*/
static void reset_parser_context(atparser_context_t *p_atp_ctxt)
{
  p_atp_ctxt->current_SID = SID_INVALID;

  p_atp_ctxt->step = 0U;
  p_atp_ctxt->answer_expected = CMD_MANDATORY_ANSWER_EXPECTED;
  p_atp_ctxt->is_final_cmd = 1U;
  p_atp_ctxt->cmd_timeout = 0U;

  reset_current_command(p_atp_ctxt);

  p_atp_ctxt->p_cmd_input = NULL;
}

static void reset_current_command(atparser_context_t *p_atp_ctxt)
{
  p_atp_ctxt->current_atcmd.id = CMD_AT_INVALID;
  p_atp_ctxt->current_atcmd.type = ATTYPE_UNKNOWN_CMD;
  (void) memset((void *)&p_atp_ctxt->current_atcmd.name[0], 0, sizeof(uint8_t) * (ATCMD_MAX_NAME_SIZE));
  (void) memset((void *)&p_atp_ctxt->current_atcmd.params[0], 0, sizeof(uint8_t) * (ATCMD_MAX_CMD_SIZE));
  p_atp_ctxt->current_atcmd.raw_cmd_size = 0U;
}

static void display_buffer(const at_context_t *p_at_ctxt, const uint8_t *p_buf, uint16_t buf_size, uint8_t is_TX_buf)
{
  uint8_t print_in_hexa = 0U; /* set default value (if 1, print in hexa otherwise, print in ascii) */
#if (USE_TRACE_ATPARSER == 0U)
  UNUSED(p_buf); /* for MISRA-2012 */
#endif /* USE_TRACE_ATPARSER */

  /* print header */
  PRINT_INDENT()
  if (is_TX_buf == 1U)
  {
    /* TX buffer */
    if (buf_size > 0U)
    {
      PRINT_INFO("*** SEND (size=%d) ***", buf_size)
      /* only RAW buffers are printed it in hexa */
      if ((p_at_ctxt->parser.current_atcmd.type == ATTYPE_RAW_CMD) &&
          (p_at_ctxt->parser.current_atcmd.id != (CMD_ID_t) CMD_AT_DIRECT_CMD))
      {
        print_in_hexa = 1U;
      }
    }
  }
  /* PRINT_INDENT() */

  if (buf_size != 0U)
  {
#if (FILTER_SOCKET_TRACES == 1U)
    if (buf_size > FILTER_DEFAULT_LENGH)
    {
      PRINT_INFO(" Big frame (display deactivated) ")
    }
    else
    {
      if (print_in_hexa == 1U)
      {
        PRINT_BUF_HEXA((const uint8_t *)p_buf, buf_size)
      }
      else
      {
        PRINT_BUF((const uint8_t *)p_buf, buf_size)
      }
    }
#else
    if (print_in_hexa == 1U)
    {
      PRINT_BUF_HEXA((const CRC_CHAR_t *)p_buf, buf_size)
    }
    else
    {
      PRINT_BUF((const CRC_CHAR_t *)p_buf, buf_size)
    }
#endif /* FILTER_SOCKET_TRACES */
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
