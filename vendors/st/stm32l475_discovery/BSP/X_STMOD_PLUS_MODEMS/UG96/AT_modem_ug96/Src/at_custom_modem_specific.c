/**
  ******************************************************************************
  * @file    at_custom_modem_specific.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code to the
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

/* AT commands format
 * AT+<X>=?    : TEST COMMAND
 * AT+<X>?     : READ COMMAND
 * AT+<X>=...  : WRITE COMMAND
 * AT+<X>      : EXECUTION COMMAND
*/

/* UG96 COMPILATION FLAGS to define in project option if needed:*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "at_modem_signalling.h"
#include "at_modem_socket.h"
#include "at_custom_modem_specific.h"
#include "at_custom_modem_signalling.h"
#include "at_custom_modem_socket.h"
#include "at_datapack.h"
#include "at_util.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"
#include "plf_modem_config.h"
#include "error_handler.h"

#if defined(USE_MODEM_UG96)
#if defined(HWREF_B_CELL_UG96)
#else
#error Hardware reference not specified
#endif /* HWREF_B_CELL_UG96 */
#endif /* USE_MODEM_UG96 */

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCUSTOM_SPECIFIC == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "UG96:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "UG96:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P2, "UG96 API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "UG96 ERROR:" format "\n\r", ## args)
#else
#define PRINT_INFO(format, args...)  (void) printf("UG96:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("UG96 ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC */

/* Private defines -----------------------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */
#define UG96_DEFAULT_TIMEOUT  ((uint32_t)15000U)
#define UG96_RDY_TIMEOUT      ((uint32_t)10000U)
#define UG96_SIMREADY_TIMEOUT ((uint32_t)30000U)
#define UG96_ESCAPE_TIMEOUT   ((uint32_t)1000U)  /* maximum time allowed to receive a response to an Escape command */
#define UG96_COPS_TIMEOUT     ((uint32_t)180000U) /* 180 sec */
#define UG96_CGATT_TIMEOUT    ((uint32_t)140000U) /* 140 sec (75 sec in doc but seems not always enough) */
#define UG96_CGACT_TIMEOUT    ((uint32_t)150000U) /* 15 sec */
#define UG96_ATH_TIMEOUT      ((uint32_t)90000U) /* 90 sec */
#define UG96_AT_TIMEOUT       ((uint32_t)1000U)  /* timeout for AT */
#define UG96_SOCKET_PROMPT_TIMEOUT ((uint32_t)10000U)
#define UG96_QIOPEN_TIMEOUT        ((uint32_t)150000U) /* 150 sec */
#define UG96_QICLOSE_TIMEOUT       ((uint32_t)150000U) /* 150 sec */
#define UG96_QIACT_TIMEOUT         ((uint32_t)150000U) /* 150 sec */
#define UG96_QIDEACT_TIMEOUT       ((uint32_t)40000U) /* 40 sec */
#define UG96_QIDNSGIP_TIMEOUT      ((uint32_t)60000U) /* 60 sec */
#define UG96_QPING_TIMEOUT         ((uint32_t)150000U) /* 150 sec */

#define UG96_MODEM_SYNCHRO_AT_MAX_RETRIES ((uint8_t)30U)

/* Global variables ----------------------------------------------------------*/
/* UG96 Modem device context */
static atcustom_modem_context_t UG96_ctxt;

/* shared variables specific to UG96 */
ug96_shared_variables_t ug96_shared =
{
  .waiting_sim_card_ready = AT_FALSE,
  .sim_card_ready = AT_FALSE,
  .change_ipr_baud_rate = AT_FALSE,
  .QIOPEN_waiting = AT_FALSE,
  .QICGSP_config_command = AT_TRUE,
};

/* Private variables ---------------------------------------------------------*/

/* Socket Data receive: to analyze size received in data header */
static AT_CHAR_t SocketHeaderDataRx_Buf[4];
static uint8_t SocketHeaderDataRx_Cpt;
static uint8_t SocketHeaderDataRx_Cpt_Complete;

/* ###########################  END CUSTOMIZATION PART  ########################### */

/* Private function prototypes -----------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */
static void ug96_modem_init(atcustom_modem_context_t *p_modem_ctxt);
static void ug96_modem_reset(atcustom_modem_context_t *p_modem_ctxt);
static void reinitSyntaxAutomaton_ug96(void);
static void reset_variables_ug96(void);
static void init_ug96_qiurc_dnsgip(void);
static void socketHeaderRX_reset(void);
static void SocketHeaderRX_addChar(CRC_CHAR_t *rxchar);
static uint16_t SocketHeaderRX_getSize(void);

/* ###########################  END CUSTOMIZATION PART  ########################### */

/* Functions Definition ------------------------------------------------------*/
void ATCustom_UG96_init(atparser_context_t *p_atp_ctxt)
{
  /* Commands Look-up table */
  static const atcustom_LUT_t ATCMD_UG96_LUT[] =
  {
    /* cmd enum - cmd string - cmd timeout (in ms) - build cmd ftion - analyze cmd ftion */
    {CMD_AT,             "",             UG96_AT_TIMEOUT,       fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_OK,          "OK",           UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CONNECT,     "CONNECT",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_RING,        "RING",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_CARRIER,  "NO CARRIER",   UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_ERROR,       "ERROR",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_UG96},
    {CMD_AT_NO_DIALTONE, "NO DIALTONE",  UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BUSY,        "BUSY",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_ANSWER,   "NO ANSWER",    UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CME_ERROR,   "+CME ERROR",   UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_UG96},
    {CMD_AT_CMS_ERROR,   "+CMS ERROR",   UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CmsErr},

    /* GENERIC MODEM commands */
    {CMD_AT_CGMI,        "+CGMI",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMI},
    {CMD_AT_CGMM,        "+CGMM",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMM},
    {CMD_AT_CGMR,        "+CGMR",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMR},
    {CMD_AT_CGSN,        "+CGSN",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGSN_UG96,  fRspAnalyze_CGSN},
    {CMD_AT_GSN,         "+GSN",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_GSN},
    {CMD_AT_CIMI,        "+CIMI",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CIMI},
    {CMD_AT_CEER,        "+CEER",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEER},
    {CMD_AT_CMEE,        "+CMEE",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_CMEE,       fRspAnalyze_None},
    {CMD_AT_CPIN,        "+CPIN",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_CPIN,       fRspAnalyze_CPIN_UG96},
    {CMD_AT_CFUN,        "+CFUN",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_CFUN,       fRspAnalyze_CFUN_UG96},
    {CMD_AT_COPS,        "+COPS",        UG96_COPS_TIMEOUT,     fCmdBuild_COPS,       fRspAnalyze_COPS},
    {CMD_AT_CNUM,        "+CNUM",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CNUM},
    {CMD_AT_CGATT,       "+CGATT",       UG96_CGATT_TIMEOUT,    fCmdBuild_CGATT,      fRspAnalyze_CGATT},
    {CMD_AT_CGPADDR,     "+CGPADDR",     UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGPADDR,    fRspAnalyze_CGPADDR},
    {CMD_AT_CREG,        "+CREG",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_CREG,       fRspAnalyze_CREG},
    {CMD_AT_CGREG,       "+CGREG",       UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGREG,      fRspAnalyze_CGREG},
    {CMD_AT_CGEREP,      "+CGEREP",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGEREP,     fRspAnalyze_None},
    {CMD_AT_CGEV,        "+CGEV",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGEV},
    {CMD_AT_CSQ,         "+CSQ",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CSQ},
    {CMD_AT_CGDCONT,     "+CGDCONT",     UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGDCONT,    fRspAnalyze_None},
    {CMD_AT_CGACT,       "+CGACT",       UG96_CGACT_TIMEOUT,    fCmdBuild_CGACT,      fRspAnalyze_None},
    {CMD_AT_CGDATA,      "+CGDATA",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_CGDATA,     fRspAnalyze_None},
    {CMD_ATD,            "D",            UG96_DEFAULT_TIMEOUT,  fCmdBuild_ATD_UG96,   fRspAnalyze_None},
    {CMD_ATE,            "E",            UG96_DEFAULT_TIMEOUT,  fCmdBuild_ATE,        fRspAnalyze_None},
    {CMD_ATH,            "H",            UG96_ATH_TIMEOUT,      fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_ATO,            "O",            UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_ATV,            "V",            UG96_DEFAULT_TIMEOUT,  fCmdBuild_ATV,        fRspAnalyze_None},
    {CMD_ATX,            "X",            UG96_DEFAULT_TIMEOUT,  fCmdBuild_ATX,        fRspAnalyze_None},
    {CMD_AT_ESC_CMD,     "+++",          UG96_ESCAPE_TIMEOUT,   fCmdBuild_ESCAPE_CMD, fRspAnalyze_None},
    {CMD_AT_IPR,         "+IPR",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_IPR,        fRspAnalyze_IPR},
    {CMD_AT_IFC,         "+IFC",         UG96_DEFAULT_TIMEOUT,  fCmdBuild_IFC,        fRspAnalyze_None},
    {CMD_AT_AND_W,       "&W",           UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_DIRECT_CMD,  "",             UG96_DEFAULT_TIMEOUT,  fCmdBuild_DIRECT_CMD, fRspAnalyze_DIRECT_CMD},

    /* MODEM SPECIFIC COMMANDS */
    {CMD_AT_QPOWD,       "+QPOWD",       UG96_DEFAULT_TIMEOUT,  fCmdBuild_QPOWD_UG96, fRspAnalyze_None},
    {CMD_AT_QCFG,        "+QCFG",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_QCFG_UG96,  fRspAnalyze_QCFG_UG96},
    {CMD_AT_QIND,        "+QIND",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_QIND_UG96},
    {CMD_AT_QUSIM,       "+QUSIM",       UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_QUSIM_UG96},
    {CMD_AT_QICSGP,      "+QICSGP",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_QICSGP_UG96,   fRspAnalyze_None},
    {CMD_AT_QIURC,       "+QIURC",       UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QIURC_UG96},
    {CMD_AT_SOCKET_PROMPT, "> ",         UG96_SOCKET_PROMPT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SEND_OK,      "SEND OK",     UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SEND_FAIL,    "SEND FAIL",   UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_QIDNSCFG,     "+QIDNSCFG",   UG96_DEFAULT_TIMEOUT,  fCmdBuild_QIDNSCFG_UG96, fRspAnalyze_None},
    {CMD_AT_QIDNSGIP,     "+QIDNSGIP",   UG96_QIDNSGIP_TIMEOUT, fCmdBuild_QIDNSGIP_UG96, fRspAnalyze_None},
    {CMD_AT_QCCID,        "+QCCID",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QCCID_UG96},

    /* MODEM SPECIFIC COMMANDS USED FOR SOCKET MODE */
    {CMD_AT_QIACT,       "+QIACT",       UG96_QIACT_TIMEOUT,    fCmdBuild_QIACT_UG96,   fRspAnalyze_QIACT_UG96},
    /* {CMD_AT_QIDEACT,     "+QIDEACT",     UG96_QIDEACT_TIMEOUT,  fCmdBuild_QIDEACT_UG96, fRspAnalyze_QIDEACT_UG96},*/
    {CMD_AT_QIOPEN,      "+QIOPEN",      UG96_QIOPEN_TIMEOUT,   fCmdBuild_QIOPEN_UG96, fRspAnalyze_QIOPEN_UG96},
    {CMD_AT_QICLOSE,     "+QICLOSE",     UG96_QICLOSE_TIMEOUT,  fCmdBuild_QICLOSE_UG96, fRspAnalyze_None},
    {CMD_AT_QISEND,      "+QISEND",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_QISEND_UG96, fRspAnalyze_None},
    {CMD_AT_QISEND_WRITE_DATA,  "",      UG96_DEFAULT_TIMEOUT,  fCmdBuild_QISEND_WRITE_DATA_UG96, fRspAnalyze_None},
    {CMD_AT_QIRD,        "+QIRD",        UG96_DEFAULT_TIMEOUT,  fCmdBuild_QIRD_UG96, fRspAnalyze_QIRD_UG96},
    {CMD_AT_QISTATE,     "+QISTATE",     UG96_DEFAULT_TIMEOUT,  fCmdBuild_QISTATE_UG96, fRspAnalyze_QISTATE_UG96},
    {CMD_AT_QPING,        "+QPING",      UG96_QPING_TIMEOUT,    fCmdBuild_QPING_UG96,    fRspAnalyze_QPING_UG96},

    /* MODEM SPECIFIC EVENTS */
    {CMD_AT_WAIT_EVENT,     "",          UG96_DEFAULT_TIMEOUT,        fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BOOT_EVENT,     "",          UG96_RDY_TIMEOUT,            fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_RDY_EVENT,      "RDY",       UG96_RDY_TIMEOUT,            fCmdBuild_NoParams,   fRspAnalyze_None},
    /* {CMD_AT_SIMREADY_EVENT, "",          UG96_SIMREADY_TIMEOUT,       fCmdBuild_NoParams,   fRspAnalyze_None}, */
    {CMD_AT_POWERED_DOWN_EVENT, "POWERED DOWN", UG96_RDY_TIMEOUT, fCmdBuild_NoParams,   fRspAnalyze_None},
  };
#define SIZE_ATCMD_UG96_LUT ((uint16_t) (sizeof (ATCMD_UG96_LUT) / sizeof (atcustom_LUT_t)))

  /* common init */
  ug96_modem_init(&UG96_ctxt);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  UG96_ctxt.modem_LUT_size = SIZE_ATCMD_UG96_LUT;
  UG96_ctxt.p_modem_LUT = (const atcustom_LUT_t *)ATCMD_UG96_LUT;

  /* override default termination string for AT command: <CR> */
  (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->endstr, "\r");

  /* ###########################  END CUSTOMIZATION PART  ########################### */
}

uint8_t ATCustom_UG96_checkEndOfMsgCallback(uint8_t rxChar)
{
  uint8_t last_char = 0U;

  /* static variables */
  static const uint8_t QIRD_string[] = "+QIRD";
  static uint8_t QIRD_Counter = 0U;

  /*---------------------------------------------------------------------------------------*/
  if (UG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_INIT_CR)
  {
    /* waiting for first valid <CR>, char received before are considered as trash */
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (UG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_CR)
  {
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (UG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_LF)
  {
    /* waiting for <LF> */
    if ((AT_CHAR_t)('\n') == rxChar)
    {
      /*  current        : <CR><LF>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : x or <CR>
      */
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_FIRST_CHAR;
      last_char = 1U;
      QIRD_Counter = 0U;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (UG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_FIRST_CHAR)
  {
    if (UG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_waiting_header)
    {
      /* Socket Data RX - waiting for Header: we are waiting for +QIRD
      *
      * <CR><LF>+QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *          ^- waiting this string
      */
      if (rxChar == QIRD_string[QIRD_Counter])
      {
        QIRD_Counter++;
        if (QIRD_Counter == (uint8_t) strlen((const CRC_CHAR_t *)QIRD_string))
        {
          /* +QIRD detected, next step */
          socketHeaderRX_reset();
          UG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_header;
        }
      }
    }

    /* NOTE:
    * if we are in socket_RxData_state = SocketRxDataState_waiting_header, we are waiting for +QIRD (test above)
    * but if we receive another message, we need to evacuate it without modifying socket_RxData_state
    * That's why we are nt using "else if" here, if <CR> if received before +QIND, it means that we have received
    * something else
    */
    if (UG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_header)
    {
      /* Socket Data RX - Header received: we are waiting for second <CR>
      *
      * <CR><LF>+QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *                    ^- waiting this <CR>
      */
      if ((AT_CHAR_t)('\r') == rxChar)
      {
        /* second <CR> detected, we have received data header
        *  now waiting for <LF>, then start to receive socket datas
        *  Verify that size received in header is the expected one
        */
        uint16_t size_from_header = SocketHeaderRX_getSize();
        if (UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size != size_from_header)
        {
          /* update buffer size received - should not happen */
          UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size = size_from_header;
        }
        UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
        UG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_data;
      }
      else if ((rxChar >= (AT_CHAR_t)('0')) && (rxChar <= (AT_CHAR_t)('9')))
      {
        /* receiving size of data in header */
        SocketHeaderRX_addChar((CRC_CHAR_t *)&rxChar);
      }
      else if (rxChar == (AT_CHAR_t)(','))
      {
        /* receiving data field separator in header: +QIRD: 4,"10.7.76.34",7678
        *  data size field has been receied, now ignore all chars until <CR><LF>
        *  additonal fields (remote IP address and port) will be analyzed later
        */
        SocketHeaderDataRx_Cpt_Complete = 1U;
      }
      else {/* nothing to do */ }
    }
    else if (UG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_data)
    {
      /* receiving socket data: do not analyze char, just count expected size
      *
      * <CR><LF>+QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *.                          ^- start to read data: count char
      */
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_SOCKET_DATA;
      UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
      /* check if full buffer has been received */
      if (UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received == UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size)
      {
        UG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
        UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
      }
    }
    /* waiting for <CR> or x */
    else if ((AT_CHAR_t)('\r') == rxChar)
    {
      /*   current        : <CR>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : <LF>
      */
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
    else {/* nothing to do */ }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (UG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_SOCKET_DATA)
  {
    UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
    /* check if full buffer has been received */
    if (UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received == UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size)
    {
      UG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
      UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else
  {
    /* should not happen */
  }

  /* ###########################  START CUSTOMIZATION PART  ######################### */
  /* if modem does not use standard syntax or has some specificities, replace previous
  *  function by a custom function
  */
  if (last_char == 0U)
  {
    /* UG96 special cases
    *
    *  SOCKET MODE: when sending DATA using AT+QISEND, we are waiting for socket prompt "<CR><LF>> "
    *               before to send DATA. Then we should receive "OK<CR><LF>".
    */

    if (UG96_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
    {
      switch (UG96_ctxt.socket_ctxt.socket_send_state)
      {
        case SocketSendState_WaitingPrompt1st_greaterthan:
        {
          /* detecting socket prompt first char: "greater than" */
          if ((AT_CHAR_t)('>') == rxChar)
          {
            UG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt2nd_space;
          }
          break;
        }

        case SocketSendState_WaitingPrompt2nd_space:
        {
          /* detecting socket prompt second char: "space" */
          if ((AT_CHAR_t)(' ') == rxChar)
          {
            UG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_Prompt_Received;
            last_char = 1U;
          }
          else
          {
            /* if char iommediatly after "greater than" is not a "space", reinit state */
            UG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
          }
          break;
        }

        default:
          break;
      }
    }
  }

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (last_char);
}

at_status_t ATCustom_UG96_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout)
{
  /* static variables */

  /* local variables */
  at_status_t retval = ATSTATUS_OK;
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_msg_t curSID = p_atp_ctxt->current_SID;

  PRINT_API("enter ATCustom_UG96_getCmd() for SID %d", curSID)

  /* retrieve parameters from SID command (will update SID_ctxt) */
  if (atcm_retrieve_SID_parameters(&UG96_ctxt, p_atp_ctxt) != ATSTATUS_OK)
  {
    retval = ATSTATUS_ERROR;
    goto exit_ATCustom_UG96_getCmd;
  }

  /* new command: reset command context */
  atcm_reset_CMD_context(&UG96_ctxt.CMD_ctxt);

  /* For each SID, the sequence of AT commands to send id defined (it can be dynamic)
  * Determine and prepare the next command to send for this SID
  */

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  if (curSID == (at_msg_t) SID_CS_CHECK_CNX)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_MODEM_CONFIG)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if ((curSID == (at_msg_t) SID_CS_POWER_ON) ||
           (curSID == (at_msg_t) SID_CS_RESET))
  {
    uint8_t common_start_sequence_step = UG96_MODEM_SYNCHRO_AT_MAX_RETRIES + 1U;

    /* POWER_ON and RESET are almost the same, specific differences are managed case by case */

    /* for reset, only HW reset is supported */
    if ((curSID == (at_msg_t) SID_CS_RESET) &&
        (UG96_ctxt.SID_ctxt.reset_type != CS_RESET_HW))
    {
      PRINT_ERR("Reset type (%d) not supported", UG96_ctxt.SID_ctxt.reset_type)
      retval = ATSTATUS_ERROR;
    }
    else
    {
      /****************************************************************************
        * POWER_ON and RESET first steps
        * try to establish the communiction with the modem by sending "AT" commands
        ****************************************************************************/
      if (p_atp_ctxt->step == 0U)
      {
        /* reset modem specific variables */
        reset_variables_ug96();

        /* reinit modem at ready status */
        UG96_ctxt.persist.modem_at_ready = AT_FALSE;

        /* in case of RESET, reset all the contexts to start from a fresh state */
        if (curSID == (at_msg_t) SID_CS_RESET)
        {
          ug96_modem_reset(&UG96_ctxt);
        }

        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else if ((p_atp_ctxt->step >= 1U) && (p_atp_ctxt->step < UG96_MODEM_SYNCHRO_AT_MAX_RETRIES))
      {
        /* start a loop to wait for modem : send AT commands */
        if (UG96_ctxt.persist.modem_at_ready == AT_FALSE)
        {
          /* use optional as we are not sure to receive a response from the modem: this allows to avoid to return
            * an error to upper layer
            */
          PRINT_DBG("test connection [try number %d] ", p_atp_ctxt->step)
          atcm_program_AT_CMD_ANSWER_OPTIONAL(&UG96_ctxt, p_atp_ctxt,
                                              ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, INTERMEDIATE_CMD);
        }
        else
        {
          /* modem has answered to the command AT: it is ready */
          PRINT_INFO("modem synchro established, proceed to normal power sequence")

          /* go to next step: jump to POWER ON sequence step */
          p_atp_ctxt->step = common_start_sequence_step;
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
      }
      else if (p_atp_ctxt->step == UG96_MODEM_SYNCHRO_AT_MAX_RETRIES)
      {
        /* if we fall here and the modem is not ready, we have a communication problem */
        if (UG96_ctxt.persist.modem_at_ready == AT_FALSE)
        {
          /* error, impossible to synchronize with modem */
          PRINT_ERR("Impossible to sync with modem")
          retval = ATSTATUS_ERROR;
        }
        else
        {
          /* continue the boot sequence */
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
      }
      /********************************************************************
        * common power ON/RESET sequence starts here
        * when communication with modem has been successfully established
        ********************************************************************/
      else if (p_atp_ctxt->step == common_start_sequence_step)
      {
        /*  set flow control to be sure that config is correct */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 1U))
      {
        /* disable echo */
        UG96_ctxt.CMD_ctxt.command_echo = AT_FALSE;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 2U))
      {
        /* request detailled error report */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CMEE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 3U))
      {
        /* enable full response format */
        UG96_ctxt.CMD_ctxt.dce_full_resp_format = AT_TRUE;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATV, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 4U))
      {
        /* check flow control */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 5U))
      {
        /* Read baud rate settings */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_IPR, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 6U))
      {
        /* power on with AT+CFUN=0 */
        UG96_ctxt.CMD_ctxt.cfun_value = 0U;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 7U))
      {
        /* check URC state */
        ug96_shared.QCFG_received_param_name = QCFG_unknown;
        ug96_shared.QCFG_received_param_value = QCFG_unknown;
        ug96_shared.QCFG_command_param = QCFG_stateurc_check;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 8U))
      {
        if ((ug96_shared.QCFG_received_param_name == QCFG_stateurc_check) &&
            (ug96_shared.QCFG_received_param_value == QCFG_stateurc_disable))
        {
          /* enable URC state */
          ug96_shared.QCFG_received_param_name = QCFG_unknown;
          ug96_shared.QCFG_received_param_value = QCFG_unknown;
          ug96_shared.QCFG_command_param = QCFG_stateurc_enable;
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, FINAL_CMD);
        }
        else
        {
          /* jump to next step */
          atcm_program_NO_MORE_CMD(p_atp_ctxt);
        }
      }
      else if (p_atp_ctxt->step > (common_start_sequence_step + 8U))
      {
        /* error, invalid step */
        retval = ATSTATUS_ERROR;
      }
      else
      {
        /* ignore */
      }
    }
  }
  else if (curSID == (at_msg_t) SID_CS_POWER_OFF)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* software power off for this modem
        * hardware power off will just reset modem GPIOs
        */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QPOWD, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_INIT_MODEM)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* read CFUN status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check if requested CFUN status if different from current modem status */
      if (UG96_ctxt.SID_ctxt.modem_init.init != UG96_ctxt.SID_ctxt.cfun_status)
      {
        ug96_shared.sim_card_ready = AT_FALSE;
        /* CFUN parameters here are coming from user settings in CS_init_modem() */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
      }
      else
      {
        /* requested CFUN status is already applied */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      if (UG96_ctxt.SID_ctxt.modem_init.init == CS_CMI_MINI)
      {
        /* minimum functionnality selected, no SIM access => leave now */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      /* check if +QIND: PB DONE has been received */
      else if (ug96_shared.sim_card_ready == AT_TRUE)
      {
        PRINT_INFO("Modem SIM 'PB DONE' indication already received, continue init sequence...")
        ug96_shared.sim_card_ready = AT_FALSE;
        /* go to next step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        PRINT_INFO("***** wait a few seconds for optional 'PB DONE' *****")
        ug96_shared.waiting_sim_card_ready = AT_TRUE;
        /* wait for a few seconds */
        atcm_program_TEMPO(p_atp_ctxt, UG96_SIMREADY_TIMEOUT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* check is CPIN is requested */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 4U)
    {
      if (UG96_ctxt.persist.sim_pin_code_ready == AT_FALSE)
      {
        if (strlen((const CRC_CHAR_t *)&UG96_ctxt.SID_ctxt.modem_init.pincode.pincode) != 0U)
        {
          /* send PIN value */
          PRINT_INFO("CPIN required, we send user value to modem")
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
        }
        else
        {
          /* no PIN provided by user */
          PRINT_INFO("CPIN required but not provided by user")
          retval = ATSTATUS_ERROR;
        }
      }
      else
      {
        PRINT_INFO("CPIN not required")
        /* no PIN required, skip cmd and go to next step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 5U)
    {
      /* check PDP context parameters */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGDCONT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_GET_DEVICE_INFO)
  {
    if (p_atp_ctxt->step == 0U)
    {
      switch (UG96_ctxt.SID_ctxt.device_info->field_requested)
      {
        case CS_DIF_MANUF_NAME_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMI, FINAL_CMD);
          break;

        case CS_DIF_MODEL_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMM, FINAL_CMD);
          break;

        case CS_DIF_REV_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMR, FINAL_CMD);
          break;

        case CS_DIF_SN_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGSN, FINAL_CMD);
          break;

        case CS_DIF_IMEI_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_GSN, FINAL_CMD);
          break;

        case CS_DIF_IMSI_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CIMI, FINAL_CMD);
          break;

        case CS_DIF_PHONE_NBR_PRESENT:
          /* not AT+CNUM not supported by UG96 */
          retval = ATSTATUS_ERROR;
          break;

        case CS_DIF_ICCID_PRESENT:
          atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QCCID, FINAL_CMD);
          break;

        default:
          /* error, invalid step */
          retval = ATSTATUS_ERROR;
          break;
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_GET_SIGNAL_QUALITY)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CSQ, FINAL_CMD);
    }
  }
  else if (curSID == (at_msg_t) SID_CS_GET_ATTACHSTATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_REGISTER_NET)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check if actual registration status is the expected one */
      const CS_OperatorSelector_t *operatorSelect = &(UG96_ctxt.SID_ctxt.write_operator_infos);
      if (UG96_ctxt.SID_ctxt.read_operator_infos.mode != operatorSelect->mode)
      {
        /* write registration status */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
      }
      else
      {
        /* skip this step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGREG, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_GET_NETSTATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SUSBCRIBE_NET_EVENT)
  {
    if (p_atp_ctxt->step == 0U)
    {
      CS_UrcEvent_t urcEvent = UG96_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CREG or CGREG ? */
      if ((urcEvent == CS_URCEVENT_GPRS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_GPRS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_CS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_CS_LOCATION_INFO))
      {
        (void) atcm_subscribe_net_event(&UG96_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* no command to monitor signal quality with URC in UG96 */
        PRINT_ERR("Signal quality monitoring no supported by this modem")
        retval = ATSTATUS_ERROR;
      }
      else
      {
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_UNSUSBCRIBE_NET_EVENT)
  {
    if (p_atp_ctxt->step == 0U)
    {
      CS_UrcEvent_t urcEvent = UG96_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CREG or CGREG ? */
      if ((urcEvent == CS_URCEVENT_GPRS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_GPRS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_CS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_CS_LOCATION_INFO))
      {
        (void) atcm_unsubscribe_net_event(&UG96_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* no command to monitor signal quality with URC in UG96 */
        PRINT_ERR("Signal quality monitoring no supported by this modem")
        retval = ATSTATUS_ERROR;
      }
      else
      {
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_REGISTER_PDN_EVENT)
  {
    if (p_atp_ctxt->step == 0U)
    {
      if (UG96_ctxt.persist.urc_subscript_pdn_event == CELLULAR_FALSE)
      {
        /* set event as suscribed */
        UG96_ctxt.persist.urc_subscript_pdn_event = CELLULAR_TRUE;

        /* request PDN events */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
      }
      else
      {
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DEREGISTER_PDN_EVENT)
  {
    if (p_atp_ctxt->step == 0U)
    {
      if (UG96_ctxt.persist.urc_subscript_pdn_event == CELLULAR_TRUE)
      {
        /* set event as unsuscribed */
        UG96_ctxt.persist.urc_subscript_pdn_event = CELLULAR_FALSE;

        /* request PDN events */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
      }
      else
      {
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_ATTACH_PS_DOMAIN)
  {
    if (p_atp_ctxt->step == 0U)
    {
      UG96_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_ATTACHED;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_DETACH_PS_DOMAIN)
  {
    if (p_atp_ctxt->step == 0U)
    {
      UG96_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_DETACHED;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_ACTIVATE_PDN)
  {
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
    /* SOCKET MODE */
    if (p_atp_ctxt->step == 0U)
    {
      /* ckeck PDN state */
      ug96_shared.pdn_already_active = AT_FALSE;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* PDN activation */
      if (ug96_shared.pdn_already_active == AT_TRUE)
      {
        /* PDN already active - exit */
        PRINT_INFO("Skip PDN activation (already active)")
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        /* request PDN activation */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIACT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* ckeck PDN state */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
#else
    /* DATA MODE*/
    if (p_atp_ctxt->step == 0U)
    {
      /* PDN activation */
      UG96_ctxt.CMD_ctxt.pdn_state = PDN_STATE_ACTIVATE;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGACT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* get IP address */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATX, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGDATA, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
#endif /* USE_SOCKETS_TYPE */
  }
  else if (curSID == (at_msg_t) SID_CS_DEACTIVATE_PDN)
  {
    /* not implemented yet */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_DEFINE_PDN)
  {
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
    /* SOCKET MODE */
    if (p_atp_ctxt->step == 0U)
    {
      ug96_shared.QICGSP_config_command = AT_TRUE;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICSGP, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      ug96_shared.QICGSP_config_command = AT_FALSE;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICSGP, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
#else
    /* DATA MODE
      * NOTE: UG96 returns an error to CGDCONT if SIM error (in our case if CFUN = 0)
      *       this is the reason why we are starting radio before
      */
    if (p_atp_ctxt->step == 0U)
    {
      /* read CFUN status */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check if need to switch on radio */
      if (UG96_ctxt.SID_ctxt.cfun_status == CS_CMI_MINI)
      {
        ug96_shared.sim_card_ready = AT_FALSE;
        /* CFUN parameters here are coming from user settings in CS_init_modem() */
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
      }
      else
      {
        /* requested CFUN status is already applied */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* check if +QIND: PB DONE has been received */
      if (ug96_shared.sim_card_ready == AT_TRUE)
      {
        PRINT_INFO("Modem SIM 'PB DONE' indication already received, continue init sequence...")
        /* go to next step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        PRINT_INFO("***** wait a few seconds for optional 'PB DONE' *****")
        ug96_shared.waiting_sim_card_ready = AT_TRUE;
        /* wait for a few seconds */
        atcm_program_TEMPO(p_atp_ctxt, UG96_SIMREADY_TIMEOUT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 3U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGDCONT, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
#endif /* USE_SOCKETS_TYPE */
  }
  else if (curSID == (at_msg_t) SID_CS_SET_DEFAULT_PDN)
  {
    /* nothing to do here
      * Indeed, default PDN has been saved automatically during analysis of SID command
      * cf function: atcm_retrieve_SID_parameters()
      */
    atcm_program_NO_MORE_CMD(p_atp_ctxt);
  }
  else if (curSID == (at_msg_t) SID_CS_GET_IP_ADDRESS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* get IP address */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
      /* SOCKET MODE */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, FINAL_CMD);
#else
      /* DATA MODE*/
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, FINAL_CMD);
#endif /* USE_SOCKETS_TYPE */
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DIAL_COMMAND)
  {
    /* SOCKET CONNECTION FOR COMMAND DATA MODE */
    if (p_atp_ctxt->step == 0U)
    {
      /* reserve a modem CID for this socket_handle */
      ug96_shared.QIOPEN_current_socket_connected = 0U;
      socket_handle_t sockHandle = UG96_ctxt.socket_ctxt.socket_info->socket_handle;
      (void) atcm_socket_reserve_modem_cid(&UG96_ctxt, sockHandle);
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIOPEN, INTERMEDIATE_CMD);
      PRINT_INFO("For Client Socket Handle=%ld : MODEM CID affected=%d",
                 sockHandle,
                 UG96_ctxt.persist.socket[sockHandle].socket_connId_value)

    }
    else if (p_atp_ctxt->step == 1U)
    {
      if (ug96_shared.QIOPEN_current_socket_connected == 0U)
      {
        /* Waiting for +QIOPEN urc indicating that socket is open */
        atcm_program_TEMPO(p_atp_ctxt, UG96_QIOPEN_TIMEOUT, INTERMEDIATE_CMD);
      }
      else
      {
        /* socket opened */
        ug96_shared.QIOPEN_waiting = AT_FALSE;
        /* socket is connected */
        (void) atcm_socket_set_connected(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else  if (p_atp_ctxt->step == 2U)
    {
      if (ug96_shared.QIOPEN_current_socket_connected == 0U)
      {
        /* QIOPEN NOT RECEIVED,
          *  cf TCP/IP AT Commands Manual V1.0, paragraph 2.1.4 - 3/
          *  "if the URC cannot be received within 150 seconds, AT+QICLOSE should be used to close
          *   the socket".
          *
          *  then we will have to return an error to cellular service !!! (see next step)
          */
        ug96_shared.QIOPEN_waiting = AT_FALSE;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICLOSE, INTERMEDIATE_CMD);
      }
      else
      {
        /* socket opened */
        ug96_shared.QIOPEN_waiting = AT_FALSE;
        /* socket is connected */
        (void) atcm_socket_set_connected(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else  if (p_atp_ctxt->step == 3U)
    {
      /* if we fall here, it means we have send CMD_AT_QICLOSE on previous step
        *  now inform cellular service that opening has failed => return an error
        */
      /* release the modem CID for this socket_handle */
      (void) atcm_socket_release_modem_cid(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle);
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
      retval = ATSTATUS_ERROR;
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SEND_DATA)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* Check data size to send */
      if (UG96_ctxt.SID_ctxt.socketSendData_struct.buffer_size > MODEM_MAX_SOCKET_TX_DATA_SIZE)
      {
        PRINT_ERR("Data size to send %ld exceed maximum size %ld",
                  UG96_ctxt.SID_ctxt.socketSendData_struct.buffer_size,
                  MODEM_MAX_SOCKET_TX_DATA_SIZE)
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
        retval = ATSTATUS_ERROR;
      }
      else
      {
        UG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QISEND, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* waiting for socket prompt: "<CR><LF>> " */
      if (UG96_ctxt.socket_ctxt.socket_send_state == SocketSendState_Prompt_Received)
      {
        PRINT_DBG("SOCKET PROMPT ALREADY RECEIVED")
        /* go to next step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        PRINT_DBG("WAITING FOR SOCKET PROMPT")
        atcm_program_WAIT_EVENT(p_atp_ctxt, UG96_SOCKET_PROMPT_TIMEOUT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* socket prompt received, send DATA */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_QISEND_WRITE_DATA, FINAL_CMD);

      /* reinit automaton to receive answer */
      reinitSyntaxAutomaton_ug96();
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if ((curSID == (at_msg_t) SID_CS_RECEIVE_DATA) ||
           (curSID == (at_msg_t) SID_CS_RECEIVE_DATA_FROM))
  {
    if (p_atp_ctxt->step == 0U)
    {
      UG96_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestSize;
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIRD, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check that data size to receive is not null */
      if (UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size != 0U)
      {
        /* check that data size to receive does not exceed maximum size
          *  if it's the case, only request maximum size we can receive
          */
        if (UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size >
            UG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
        {
          PRINT_INFO("Data size available (%ld) exceed buffer maximum size (%ld)",
                     UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size,
                     UG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
          UG96_ctxt.socket_ctxt.socket_rx_expected_buf_size = UG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size;
        }

        /* receive datas */
        UG96_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestData_Header;
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIRD, FINAL_CMD);
      }
      else
      {
        /* no datas to receive */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SOCKET_CLOSE)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* is socket connected ?
        * due to UG96 socket connection mechanism (waiting URC QIOPEN), we can fall here but socket
        * has been already closed if error occurs during connection
        */
      if (atcm_socket_is_connected(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle) == AT_TRUE)
      {
        atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICLOSE, INTERMEDIATE_CMD);
      }
      else
      {
        /* release the modem CID for this socket_handle */
        (void) atcm_socket_release_modem_cid(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* release the modem CID for this socket_handle */
      (void) atcm_socket_release_modem_cid(&UG96_ctxt, UG96_ctxt.socket_ctxt.socket_info->socket_handle);
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DATA_SUSPEND)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* wait for 1 second */
      atcm_program_TEMPO(p_atp_ctxt, 1000U, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* send escape sequence +++ (RAW command type)
        *  CONNECT expected before 1000 ms
        */
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_ESC_CMD, FINAL_CMD);
      reinitSyntaxAutomaton_ug96();
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if ((curSID == (at_msg_t) SID_CS_INIT_POWER_CONFIG) ||
           (curSID == (at_msg_t) SID_CS_SET_POWER_CONFIG) ||
           (curSID == (at_msg_t) SID_CS_SLEEP_REQUEST) ||
           (curSID == (at_msg_t) SID_CS_SLEEP_COMPLETE) ||
           (curSID == (at_msg_t) SID_CS_SLEEP_CANCEL) ||
           (curSID == (at_msg_t) SID_CS_WAKEUP))
  {
    /* Low Power modes not implemented for UG96 */
    PRINT_INFO("UG96 power management not supported")
    UG96_ctxt.persist.psm_requested = AT_FALSE;
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_SOCKET_CNX_STATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QISTATE, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DATA_RESUME)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATX, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATO, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DNS_REQ)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIDNSCFG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* initialize +QIURC "dnsgip" parameters */
      init_ug96_qiurc_dnsgip();
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIDNSGIP, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* do we have received a valid DNS response ? */
      if ((ug96_shared.QIURC_dnsgip_param.finished == AT_TRUE) && (ug96_shared.QIURC_dnsgip_param.error == 0U))
      {
        /* yes */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        /* not yet, waiting for DNS informations */
        atcm_program_TEMPO(p_atp_ctxt, UG96_QIDNSGIP_TIMEOUT, FINAL_CMD);
      }
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SUSBCRIBE_MODEM_EVENT)
  {
    /* nothing to do here
      * Indeed, default modem events subscribed havebeen saved automatically during analysis of SID command
      * cf function: atcm_retrieve_SID_parameters()
      */
    atcm_program_NO_MORE_CMD(p_atp_ctxt);
  }
  else if (curSID == (at_msg_t) SID_CS_PING_IP_ADDRESS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QPING, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_DIRECT_CMD)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&UG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_DIRECT_CMD, FINAL_CMD);
      atcm_program_CMD_TIMEOUT(&UG96_ctxt, p_atp_ctxt, UG96_ctxt.SID_ctxt.direct_cmd_tx->cmd_timeout);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SIM_SELECT)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* select the SIM slot */
      if (atcm_select_hw_simslot(UG96_ctxt.persist.sim_selected) != ATSTATUS_OK)
      {
        retval = ATSTATUS_ERROR;
      }
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */
  else
  {
    PRINT_ERR("Error, invalid command ID %d", curSID)
    retval = ATSTATUS_ERROR;
  }

  /* if no error, build the command to send */
  if (retval == ATSTATUS_OK)
  {
    retval = atcm_modem_build_cmd(&UG96_ctxt, p_atp_ctxt, p_ATcmdTimeout);
  }

exit_ATCustom_UG96_getCmd:
  return (retval);
}

at_endmsg_t ATCustom_UG96_extractElement(atparser_context_t *p_atp_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos)
{
  at_endmsg_t retval_msg_end_detected = ATENDMSG_NO;
  bool exit_loop;
  uint16_t idx;
  uint16_t start_idx;
  uint16_t *p_parseIndex = &(element_infos->current_parse_idx);

  PRINT_API("enter ATCustom_UG96_extractElement()")
  PRINT_API("input message: size=%d ", p_msg_in->size)

  /* if this is beginning of message, check that message header is correct and jump over it */
  if (*p_parseIndex == 0U)
  {
    /* ###########################  START CUSTOMIZATION PART  ########################### */
    /* MODEM RESPONSE SYNTAX:
      * <CR><LF><response><CR><LF>
      *
      */
    start_idx = 0U;
    /* search initial <CR><LF> sequence (for robustness) */
    if ((p_msg_in->buffer[0] == (AT_CHAR_t)('\r')) && (p_msg_in->buffer[1] == (AT_CHAR_t)('\n')))
    {
      /* <CR><LF> sequence has been found, it is a command line */
      PRINT_DBG("cmd init sequence <CR><LF> found - break")
      *p_parseIndex = 2U;
      start_idx = 2U;
    }

    exit_loop = false;
    for (idx = start_idx; (idx < (p_msg_in->size - 1U)) && (exit_loop == false); idx++)
    {
      if ((p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_QIRD) &&
          (UG96_ctxt.socket_ctxt.socket_receive_state == SocketRcvState_RequestData_Payload) &&
          (UG96_ctxt.socket_ctxt.socket_RxData_state != SocketRxDataState_finished))
      {
        PRINT_DBG("receiving socket data (real size=%d)", SocketHeaderRX_getSize())
        element_infos->str_start_idx = 0U;
        element_infos->str_end_idx = (uint16_t) UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        element_infos->str_size = (uint16_t) UG96_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        UG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_finished;
        retval_msg_end_detected = ATENDMSG_YES;
        exit_loop = true;
      }
    }

    /* check if end of message has been detected */
    if (retval_msg_end_detected == ATENDMSG_YES)
    {
      goto exit_ATCustom_UG96_extractElement;
    }
    /* ###########################  END CUSTOMIZATION PART  ########################### */
  }

  element_infos->str_start_idx = *p_parseIndex;
  element_infos->str_end_idx = *p_parseIndex;
  element_infos->str_size = 0U;

  /* reach limit of input buffer ? (empty message received) */
  if (*p_parseIndex >= p_msg_in->size)
  {
    retval_msg_end_detected = ATENDMSG_YES;
    goto exit_ATCustom_UG96_extractElement;
  }

  /* extract parameter from message */
  exit_loop = false;
  do
  {
    switch (p_msg_in->buffer[*p_parseIndex])
    {
      /* ###########################  START CUSTOMIZATION PART  ########################### */
      /* ----- test separators ----- */
      case ' ':
        if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CGDATA)
        {
          /* specific case for UG96 which answer CONNECT <value> even when ATX0 sent before (UG96 bug)
            *  as a consequence, if NO CARRIER is received, it will not be detected (=> timeout will expire)
            */
          exit_loop = true;
        }
        else
        {
          element_infos->str_end_idx = *p_parseIndex;
          element_infos->str_size++;
        }
        break;

      case ':':
      case ',':
        exit_loop = true;
        break;

      /* ----- test end of message ----- */
      case '\r':
        exit_loop = true;
        retval_msg_end_detected = ATENDMSG_YES;
        break;

      default:
        /* increment end position */
        element_infos->str_end_idx = *p_parseIndex;
        element_infos->str_size++;
        break;
        /* ###########################  END CUSTOMIZATION PART  ########################### */
    }

    /* increment index */
    (*p_parseIndex)++;

    /* reach limit of input buffer ? */
    if (*p_parseIndex >= p_msg_in->size)
    {
      exit_loop = true;
      retval_msg_end_detected = ATENDMSG_YES;
    }
  } while (exit_loop == false);

  /* increase parameter rank */
  element_infos->param_rank = (element_infos->param_rank + 1U);

exit_ATCustom_UG96_extractElement:
  return (retval_msg_end_detected);
}

at_action_rsp_t ATCustom_UG96_analyzeCmd(at_context_t *p_at_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval ;

  PRINT_API("enter ATCustom_UG96_analyzeCmd()")

  /* Analyze data received from the modem and
    * search in LUT the ID corresponding to command received
    */
  if (ATSTATUS_OK != atcm_searchCmdInLUT(&UG96_ctxt, p_atp_ctxt, p_msg_in, element_infos))
  {
    /* No command corresponding to a LUT entry has been found.
      * May be we received a text line without command prefix.
      *
      * This is the case for some commands which are using following syntax:
      *    AT+MYCOMMAND
      *    parameters
      *    OK
      *
      * ( usually, command are using the syntax:
      *    AT+MYCOMMAND
      *    +MYCOMMAND=parameters
      *    OK
      *  )
      */

    /* 1st STEP: search in common modems commands
      * (CGMI, CGMM, CGMR, CGSN, GSN, IPR, CIMI, CGPADDR, ...)
      */
    retval = atcm_check_text_line_cmd(&UG96_ctxt, p_at_ctxt, p_msg_in, element_infos);

    /* 2nd STEP: search in specific modems commands if not found at 1st STEP
      *
      * This is the case in socket mode when receiving data.
      * The command is decomposed in 2 lines:
      * The 1st part of the response is analyzed by atcm_searchCmdInLUT:
      *   +QIRD: 522<CR><LF>
      * The 2nd part of the response, corresponding to the datas, falls here:
      *   HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF><CR><LF>Serve...
      */
    if (retval == ATACTION_RSP_NO_ACTION)
    {
      switch (p_atp_ctxt->current_atcmd.id)
      {
        /* ###########################  START CUSTOMIZED PART  ########################### */
        case CMD_AT_QIRD:
          if (fRspAnalyze_QIRD_data_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        case CMD_AT_QISTATE:
          if (fRspAnalyze_QISTATE_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        /* ###########################  END CUSTOMIZED PART  ########################### */
        default:
          /* this is not one of modem common command, need to check if this is an answer to a modem's specific cmd */
          PRINT_INFO("receive an un-expected line... is it a text line ?")
          retval = ATACTION_RSP_IGNORED;
          break;
      }
    }

    /* we fall here when cmd_id not found in the LUT
      * 2 cases are possible:
      *  - this is a valid line: ATACTION_RSP_INTERMEDIATE
      *  - this is an invalid line: ATACTION_RSP_ERROR
      */
  }
  else
  {
    /* cmd_id has been found in the LUT: determine next action */
    switch (element_infos->cmd_id_received)
    {
      case CMD_AT_OK:
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_DATA_SUSPEND)
        {
          PRINT_INFO("MODEM SWITCHES TO COMMAND MODE")
        }
        if ((p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_ON) ||
            (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RESET))
        {
          if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT)
          {
            /* modem is synchronized */
            UG96_ctxt.persist.modem_at_ready = AT_TRUE;
          }
          if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_ATE)
          {
            PRINT_DBG("Echo successfully disabled")
          }
        }
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_PING_IP_ADDRESS)
        {
          /* PING requests for UG96 are asynchronous.
            * If the PING request is valid (no other ongoing ping request for example), the
            * modem will answer OK.
            * At this point, initialize the ping response structure.
            */
          PRINT_DBG("This is a valid PING request")
          atcm_validate_ping_request(&UG96_ctxt);
        }
        retval = ATACTION_RSP_FRC_END;
        break;

      case CMD_AT_NO_CARRIER:
      case CMD_AT_NO_ANSWER:
        retval = ATACTION_RSP_ERROR;
        break;

      case CMD_AT_RING:
      case CMD_AT_NO_DIALTONE:
      case CMD_AT_BUSY:
        /* VALUES NOT MANAGED IN CURRENT IMPLEMENTATION BECAUSE NOT EXPECTED */
        retval = ATACTION_RSP_ERROR;
        break;

      case CMD_AT_CONNECT:
        PRINT_INFO("MODEM SWITCHES TO DATA MODE")
        retval = (at_action_rsp_t)(ATACTION_RSP_FLAG_DATA_MODE | ATACTION_RSP_FRC_END);
        break;

      /* ###########################  START CUSTOMIZATION PART  ########################### */
      case CMD_AT_CREG:
        /* check if response received corresponds to the command we have send
          *  if not => this is an URC
          */
        if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
        {
          retval = ATACTION_RSP_INTERMEDIATE;
        }
        else
        {
          retval = ATACTION_RSP_URC_FORWARDED;
        }
        break;

      case CMD_AT_CGREG:
        /* check if response received corresponds to the command we have send
          *  if not => this is an URC
          */
        if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
        {
          retval = ATACTION_RSP_INTERMEDIATE;
        }
        else
        {
          retval = ATACTION_RSP_URC_FORWARDED;
        }
        break;

      case CMD_AT_RDY_EVENT:
        /* We received RDY event from the modem.
          * If received during Power ON or RESET, it is indicating that the modem is ready.
          * If received in another state, we report to upper layer a modem reboot event.
          */

        if ((p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_ON) ||
            (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RESET))
        {
          /* ignore the RDY event during POWER ON or RESET */
          retval = ATACTION_RSP_URC_IGNORED;
        }
        else
        {
          /* if event is received in other states, it's an unexpected modem reboot */
          if (atcm_modem_event_received(&UG96_ctxt, CS_MDMEVENT_BOOT) == AT_TRUE)
          {
            retval = ATACTION_RSP_URC_FORWARDED;
          }
          else
          {
            retval = ATACTION_RSP_URC_IGNORED;
          }
        }
        break;

      case CMD_AT_POWERED_DOWN_EVENT:
      {
        PRINT_DBG("MODEM POWERED DOWN EVENT DETECTED")
        if (atcm_modem_event_received(&UG96_ctxt, CS_MDMEVENT_POWER_DOWN) == AT_TRUE)
        {
          retval = ATACTION_RSP_URC_FORWARDED;
        }
        else
        {
          retval = ATACTION_RSP_URC_IGNORED;
        }
        break;
      }

      case CMD_AT_SOCKET_PROMPT:
        PRINT_INFO(" SOCKET PROMPT RECEIVED")
        /* if we were waiting for this event, we can continue the sequence */
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SEND_DATA)
        {
          /* UNLOCK the WAIT EVENT */
          retval = ATACTION_RSP_FRC_END;
        }
        else
        {
          retval = ATACTION_RSP_URC_IGNORED;
        }
        break;

      case CMD_AT_SEND_OK:
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SEND_DATA)
        {
          retval = ATACTION_RSP_FRC_END;
        }
        else
        {
          retval = ATACTION_RSP_ERROR;
        }
        break;

      case CMD_AT_SEND_FAIL:
        retval = ATACTION_RSP_ERROR;
        break;

      case CMD_AT_QIURC:
        /* retval will be override in analyze of +QUIRC content
          *  indeed, QIURC can be considered as an URC or a normal msg (for DNS request)
          */
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_QIOPEN:
        /* now waiting for an URC  */
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_QUSIM:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_QIND:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_CFUN:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_CPIN:
        PRINT_DBG(" SIM STATE RECEIVED")
        /* retval will be override in analyze of +CPIN content if needed
          */
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_QCFG:
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_CGEV:
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case CMD_AT_QPING:
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      /* ###########################  END CUSTOMIZATION PART  ########################### */

      case CMD_AT:
        retval = ATACTION_RSP_IGNORED;
        break;

      case CMD_AT_INVALID:
        retval = ATACTION_RSP_ERROR;
        break;

      case CMD_AT_ERROR:
        /* ERROR does not contains parameters, call the analyze function explicity */
        retval = fRspAnalyze_Error_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos);
        break;

      case CMD_AT_CME_ERROR:
      case CMD_AT_CMS_ERROR:
        /* do the analyze here because will not be called by parser */
        retval = fRspAnalyze_Error_UG96(p_at_ctxt, &UG96_ctxt, p_msg_in, element_infos);
        break;

      default:
        /* check if response received corresponds to the command we have send
          *  if not => this is an ERROR
          */
        if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
        {
          retval = ATACTION_RSP_INTERMEDIATE;
        }
        else if (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD)
        {
          retval = ATACTION_RSP_IGNORED;
        }
        else
        {
          PRINT_INFO("UNEXPECTED MESSAGE RECEIVED")
          retval = ATACTION_RSP_IGNORED;
        }
        break;
    }
  }
  return (retval);
}

at_action_rsp_t ATCustom_UG96_analyzeParam(at_context_t *p_at_ctxt,
                                           const IPC_RxMessage_t *p_msg_in,
                                           at_element_info_t *element_infos)
{
  at_action_rsp_t retval;
  PRINT_API("enter ATCustom_UG96_analyzeParam()")

  /* analyse parameters of the command we received:
    * call the corresponding function from the LUT
  */
  retval = (atcm_get_CmdAnalyzeFunc(&UG96_ctxt, element_infos->cmd_id_received))(p_at_ctxt,
                                                                                 &UG96_ctxt,
                                                                                 p_msg_in,
                                                                                 element_infos);

  return (retval);
}

/* function called to finalize an AT command */
at_action_rsp_t ATCustom_UG96_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter ATCustom_UG96_terminateCmd()")

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* additional tests */
  if (UG96_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
  {
    /* special case for SID_CS_SEND_DATA
    * indeed, this function is called when an AT cmd is finished
    * but for AT+QISEND, it is called a 1st time when prompt is received
    * and a second time when data have been sent.
    */
    if (p_atp_ctxt->current_SID != (at_msg_t) SID_CS_SEND_DATA)
    {
      /* reset socket_send_state */
      UG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_No_Activity;
    }
  }

  if ((p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_ATD) ||
      (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_ATO) ||
      (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CGDATA))
  {
    if (element_infos->cmd_id_received == (CMD_ID_t) CMD_AT_CONNECT)
    {
      /* force last command (no command can be sent in DATA mode) */
      p_atp_ctxt->is_final_cmd = 1U;
      PRINT_DBG("CONNECT received")
    }
    else
    {
      PRINT_ERR("expected CONNECT not received !!!")
      retval = ATACTION_RSP_ERROR;
    }
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */
  return (retval);
}

/* function called to finalize a SID */
at_status_t ATCustom_UG96_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_UG96_get_rsp()")

  /* prepare response for a SID - common part */
  retval = atcm_modem_get_rsp(&UG96_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* prepare response for a SID
  *  all specific behaviors for SID which are returning datas in rsp_buf have to be implemented here
  */
  switch (p_atp_ctxt->current_SID)
  {
    case SID_CS_DNS_REQ:
      /* PACK data to response buffer */
      if (DATAPACK_writeStruct(p_rsp_buf,
                               (uint16_t) CSMT_DNS_REQ,
                               (uint16_t) sizeof(ug96_shared.QIURC_dnsgip_param.hostIPaddr),
                               (void *)ug96_shared.QIURC_dnsgip_param.hostIPaddr) != DATAPACK_OK)
      {
        retval = ATSTATUS_ERROR;
      }
      break;

    case SID_CS_POWER_OFF:
      /* reinit context for power off case */
      ug96_modem_reset(&UG96_ctxt);
      break;

#if (UG96_ACTIVATE_PING_REPORT == 1)
    case SID_CS_PING_IP_ADDRESS:
    {
      /* SID_CS_PING_IP_ADDRESS is waiting for a ping structure
       * For this modem, PING will be received later (as URC).
       * Just indicate that no ping report is available for now.
       */
      PRINT_DBG("Ping no report available yet - use PING_INVALID_INDEX")
      (void) memset((void *)&UG96_ctxt.persist.ping_resp_urc, 0, sizeof(CS_Ping_response_t));
      UG96_ctxt.persist.ping_resp_urc.index = PING_INVALID_INDEX;
      if (DATAPACK_writeStruct(p_rsp_buf,
                               (uint16_t) CSMT_URC_PING_RSP,
                               (uint16_t) sizeof(CS_Ping_response_t),
                               (void *)&UG96_ctxt.persist.ping_resp_urc) != DATAPACK_OK)
      {
        retval = ATSTATUS_OK;
      }
      break;
    }
#endif /* (UG96_ACTIVATE_PING_REPORT == 1) */

    default:
      break;
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */

  /* reset SID context */
  atcm_reset_SID_context(&UG96_ctxt.SID_ctxt);

  /* reset socket context */
  atcm_reset_SOCKET_context(&UG96_ctxt);

  return (retval);
}

at_status_t ATCustom_UG96_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_UG96_get_urc()")

  /* prepare response for an URC - common part */
  retval = atcm_modem_get_urc(&UG96_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* prepare response for an URC
  *  all specific behaviors for an URC have to be implemented here
  */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_UG96_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_UG96_get_error()")

  /* prepare response when an error occured - common part */
  retval = atcm_modem_get_error(&UG96_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /*  prepare response when an error occured
  *  all specific behaviors for an error have to be implemented here
  */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_UG96_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate)
{
  UNUSED(deviceType);
  UNUSED(hwEvent);
  UNUSED(gstate);

  at_status_t retval = ATSTATUS_ERROR;
  /* Do not add traces (called under interrupt if GPIO event) */

  /* ###########################  START CUSTOMIZATION PART  ########################### */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

/* Private function Definition -----------------------------------------------*/

/* UG96 modem init fonction
*  call common init function and then do actions specific to this modem
*/
static void ug96_modem_init(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter ug96_modem_init")

  /* common init function (reset all contexts) */
  atcm_modem_init(p_modem_ctxt);

  /* modem specific actions if any */
}

/* UG96 modem reset fonction
*  call common reset function and then do actions specific to this modem
*/
static void ug96_modem_reset(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter ug96_modem_reset")

  /* common reset function (reset all contexts except SID) */
  atcm_modem_reset(p_modem_ctxt);

  /* modem specific actions if any */
}

static void reinitSyntaxAutomaton_ug96(void)
{
  UG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_INIT_CR;
}

static void reset_variables_ug96(void)
{
  /* Set default values of UG96 specific variables after SWITCH ON or RESET */
  ug96_shared.change_ipr_baud_rate = AT_FALSE;
  ug96_shared.sim_card_ready = AT_FALSE;
  ug96_shared.waiting_sim_card_ready = AT_FALSE;
}

static void init_ug96_qiurc_dnsgip(void)
{
  ug96_shared.QIURC_dnsgip_param.finished = AT_FALSE;
  ug96_shared.QIURC_dnsgip_param.wait_header = AT_TRUE;
  ug96_shared.QIURC_dnsgip_param.error = 0U;
  ug96_shared.QIURC_dnsgip_param.ip_count = 0U;
  ug96_shared.QIURC_dnsgip_param.ttl = 0U;
  (void) memset((void *)ug96_shared.QIURC_dnsgip_param.hostIPaddr, 0, MAX_SIZE_IPADDR);
}

static void socketHeaderRX_reset(void)
{
  (void) memset((void *)SocketHeaderDataRx_Buf, 0, 4U);
  SocketHeaderDataRx_Cpt = 0U;
  SocketHeaderDataRx_Cpt_Complete = 0U;
}
static void SocketHeaderRX_addChar(CRC_CHAR_t *rxchar)
{
  if ((SocketHeaderDataRx_Cpt_Complete == 0U) && (SocketHeaderDataRx_Cpt < 4U))
  {
    (void) memcpy((void *)&SocketHeaderDataRx_Buf[SocketHeaderDataRx_Cpt], (void *)rxchar, sizeof(char));
    SocketHeaderDataRx_Cpt++;
  }
}
static uint16_t SocketHeaderRX_getSize(void)
{
  uint16_t retval = (uint16_t) ATutil_convertStringToInt((uint8_t *)SocketHeaderDataRx_Buf,
                                                         (uint16_t)SocketHeaderDataRx_Cpt);
  return (retval);
}



/* ###########################  END CUSTOMIZATION PART  ########################### */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

