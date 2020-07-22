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

/* AT commands format
 * AT+<X>=?    : TEST COMMAND
 * AT+<X>?     : READ COMMAND
 * AT+<X>=...  : WRITE COMMAND
 * AT+<X>      : EXECUTION COMMAND
*/

/* GM01Q COMPILATION FLAGS to define in project option if needed:
*
*/
#define SEQUANS_WAIT_SIM_TEMPO ((uint32_t)8000U) /* time (in ms) allocated to wait SIM ready during modem init */

#define LP_TEST_MONARCH

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "at_modem_api.h"
#include "at_modem_common.h"
#include "at_modem_signalling.h"
#include "at_modem_socket.h"
#include "at_custom_modem_specific.h"
#include "at_custom_modem_signalling.h"
#include "at_custom_modem_socket.h"
#include "at_datapack.h"
#include "at_util.h"
#include "sysctrl_specific.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"
#include "plf_modem_config.h"
#include "error_handler.h"

#if defined(USE_MODEM_GM01Q)
#if defined(HWREF_B_CELL_GM01Q)
#else
#error Hardware reference not specified
#endif /* HWREF_B_CELL_GM01Q */
#endif /* USE_MODEM_GM01Q */

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
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC == 1 */

/* Private defines -----------------------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */

#define SEQMONARCH_DEFAULT_TIMEOUT  ((uint32_t)15000)
#define SEQMONARCH_AT_TIMEOUT       ((uint32_t)1000U)  /* timeout for AT */
#define SEQMONARCH_CMD_TIMEOUT      ((uint32_t)15000)
#define SEQMONARCH_BOOT_TIMEOUT     ((uint32_t)15000) /* MAXIMUM BOOT TIME ALLOWED */
#define SEQMONARCH_SYSSTART_TIMEOUT ((uint32_t)60000)
#define SEQMONARCH_SHUTDOWN_TIMEOUT ((uint32_t)30000) /* MAXIMUM TIME ALLOWED TO RECEIVE SHUTDOWN URC */


#define SEQMONARCH_DATA_SUSPEND_TIMEOUT  ((uint32_t)2000)   /* time before to send escape command */
#define SEQMONARCH_ESCAPE_TIMEOUT   ((uint32_t)2000)   /* maximum time allowed to receive a response to an Esc cmd */
#define SEQMONARCH_COPS_TIMEOUT     ((uint32_t)180000) /* 180 sec */
#define SEQMONARCH_CFUN_TIMEOUT     ((uint32_t)40000)  /* 40 sec: so long because in case of factory reset, modem
                                                       * answer can be very long, certainly because it takes time
                                                       * to save NV( Non Volatile) datas.
                                                       */
#define SEQMONARCH_SOCKET_PROMPT_TIMEOUT ((uint32_t)10000)
#define SEQMONARCH_SQNSD_TIMEOUT         ((uint32_t)150000) /* 150s */
#define SEQMONARCH_SQNSH_TIMEOUT         ((uint32_t)150000) /* 150s */
#define SEQMONARCH_RESTART_RADIO_TIMEOUT ((uint32_t)5000)

/* Global variables ----------------------------------------------------------*/
/* GM01Q Modem device context */
static atcustom_modem_context_t SEQMONARCH_ctxt;

/* shared variables specific to MONARCH */
monarch_shared_variables_t monarch_shared =
{
  .SMST_sim_error_status = 0U,
  .waiting_for_ring_irq = false,
};

/* Private variables ---------------------------------------------------------*/
/* Socket Data receive: to analyze size received in data header */
static AT_CHAR_t SocketHeaderDataRx_Buf[4];
static uint8_t SocketHeaderDataRx_Cpt;
static uint8_t SocketHeaderDataRx_Separator;

/* Private function prototypes -----------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */
static void monarch_modem_init(atcustom_modem_context_t *p_modem_ctxt);
static void monarch_modem_reset(atcustom_modem_context_t *p_modem_ctxt);
static void reinitSyntaxAutomaton_monarch(void);
static void reset_variables_monarch(void);
static void socketHeaderRX_reset(void);
static void SocketHeaderRX_separator_received(void);
static void SocketHeaderRX_addChar(CRC_CHAR_t *rxchar);
static uint16_t SocketHeaderRX_getSize(void);
static void init_dns_info(void);

static at_bool_t init_monarch_low_power(void);
static void low_power_state_requested(void);
static void low_power_state_enter(void);
static void low_power_state_cancel(void);
static bool is_waiting_modem_low_power_ack(void);

/* ###########################  END CUSTOMIZATION PART  ########################### */

/* Functions Definition ------------------------------------------------------*/
void ATCustom_MONARCH_init(atparser_context_t *p_atp_ctxt)
{
  /* Commands Look-up table : list of commands supported for this modem */
  static const atcustom_LUT_t ATCMD_SEQMONARCH_LUT[] =
  {
    /* cmd enum - cmd string - cmd timeout (in ms) - build cmd ftion - analyze cmd ftion */
    {CMD_AT,             "",             SEQMONARCH_AT_TIMEOUT,       fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_OK,          "OK",           SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CONNECT,     "CONNECT",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_RING,        "RING",         SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_CARRIER,  "NO CARRIER",   SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_ERROR,       "ERROR",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_MONARCH},
    {CMD_AT_NO_DIALTONE, "NO DIALTONE",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BUSY,        "BUSY",         SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_ANSWER,   "NO ANSWER",    SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CME_ERROR,   "+CME ERROR",   SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_MONARCH},
    {CMD_AT_CMS_ERROR,   "+CMS ERROR",   SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CmsErr},

    /* GENERIC MODEM commands */
    {CMD_AT_CGMI,        "+CGMI",        SEQMONARCH_CMD_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMI},
    {CMD_AT_CGMM,        "+CGMM",        SEQMONARCH_CMD_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMM},
    {CMD_AT_CGMR,        "+CGMR",        SEQMONARCH_CMD_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMR},
    {CMD_AT_CGSN,        "+CGSN",        SEQMONARCH_CMD_TIMEOUT,  fCmdBuild_CGSN,       fRspAnalyze_CGSN},
    {CMD_AT_CIMI,        "+CIMI",        SEQMONARCH_CMD_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CIMI},
    {CMD_AT_CEER,        "+CEER",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEER},
    {CMD_AT_CMEE,        "+CMEE",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CMEE,       fRspAnalyze_None},
    {CMD_AT_CPIN,        "+CPIN",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CPIN,       fRspAnalyze_CPIN},
    {CMD_AT_CFUN,        "+CFUN",        SEQMONARCH_CFUN_TIMEOUT,     fCmdBuild_CFUN,       fRspAnalyze_None},
    {CMD_AT_COPS,        "+COPS",        SEQMONARCH_COPS_TIMEOUT,     fCmdBuild_COPS,       fRspAnalyze_COPS},
    {CMD_AT_CNUM,        "+CNUM",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CNUM},
    {CMD_AT_CGATT,       "+CGATT",       SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGATT,      fRspAnalyze_CGATT},
    {CMD_AT_CEREG,       "+CEREG",       SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CEREG,      fRspAnalyze_CEREG},
    {CMD_AT_CGEREP,      "+CGEREP",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGEREP,     fRspAnalyze_None},
    {CMD_AT_CGEV,        "+CGEV",        SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGEV},
    {CMD_AT_CSQ,         "+CSQ",         SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CSQ},
    {CMD_AT_CGDCONT,     "+CGDCONT",     SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGDCONT,    fRspAnalyze_None},
    {CMD_AT_CGACT,       "+CGACT",       SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGACT,      fRspAnalyze_None},
    {CMD_AT_CGAUTH,      "+CGAUTH",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGAUTH,     fRspAnalyze_None},
    {CMD_AT_CGDATA,      "+CGDATA",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGDATA,     fRspAnalyze_None},
    {CMD_AT_CGPADDR,     "+CGPADDR",     SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGPADDR,    fRspAnalyze_CGPADDR},
    {CMD_ATD,            "D",            SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_ATD_MONARCH, fRspAnalyze_None},
    {CMD_ATO,            "O",            SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_ATE,            "E",            SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_ATE,        fRspAnalyze_None},
    {CMD_ATV,            "V",            SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_ATV,        fRspAnalyze_None},
    {CMD_AT_ESC_CMD,     "+++",          SEQMONARCH_ESCAPE_TIMEOUT,   fCmdBuild_ESCAPE_CMD, fRspAnalyze_None},
    {CMD_AT_DIRECT_CMD,  "",             SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_DIRECT_CMD, fRspAnalyze_DIRECT_CMD},
    {CMD_AT_IPR,         "+IPR",         SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_IPR,        fRspAnalyze_IPR},
    {CMD_AT_IFC,         "+IFC",         SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_IFC,        fRspAnalyze_None},
    {CMD_AT_CPSMS,       "+CPSMS",       SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CPSMS,      fRspAnalyze_CPSMS},
    {CMD_AT_CEDRXS,      "+CEDRXS",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CEDRXS,     fRspAnalyze_CEDRXS},
    {CMD_AT_CEDRXP,      "+CEDRXP",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEDRXP},
    {CMD_AT_CEDRXRDP,    "+CEDRXRDP",    SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEDRXRDP},

    /* MODEM SPECIFIC COMMANDS */
    {CMD_AT_RESET,       "^RESET",       SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,           fRspAnalyze_None},
    {CMD_AT_SQNCTM,      "+SQNCTM",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SQNCTM_MONARCH,     fRspAnalyze_None},
    {CMD_AT_AUTOATT,     "^AUTOATT",     SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_AUTOATT_MONARCH,    fRspAnalyze_None},
    {CMD_AT_CGDCONT_REPROGRAM, "",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_CGDCONT_REPROGRAM_MONARCH, fRspAnalyze_None},
    {CMD_AT_ICCID,       "+SQNCCID",   SEQMONARCH_SQNSH_TIMEOUT,    fCmdBuild_NoParams, fRspAnalyze_SQNCCID_MONARCH},
    {
      CMD_AT_SQNDNSLKUP, "+SQNDNSLKUP", SEQMONARCH_DEFAULT_TIMEOUT,
      fCmdBuild_SQNDNSLKUP_MONARCH, fRspAnalyze_SQNDNSLKUP_MONARCH
    },
    {CMD_AT_SMST,        "+SMST",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SMST_MONARCH, fRspAnalyze_SMST_MONARCH},
    {CMD_AT_CESQ,        "+CESQ",      SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,  fRspAnalyze_CESQ_MONARCH},
    {CMD_AT_SQNSSHDN,    "+SQNSSHDN",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,            fRspAnalyze_None},
    {CMD_AT_SHUTDOWN,    "+SHUTDOWN",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,            fRspAnalyze_None},

    /* MODEM SPECIFIC COMMANDS USED FOR SOCKET MODE */
    {CMD_AT_SQNSRING,    "+SQNSRING",    SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_SQNSRING_MONARCH},
    {CMD_AT_SQNSCFG,     "+SQNSCFG",     SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SQNSCFG_MONARCH,    fRspAnalyze_None},
    {CMD_AT_SQNSCFGEXT,  "+SQNSCFGEXT",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SQNSCFGEXT_MONARCH, fRspAnalyze_None},
    {CMD_AT_SQNSD,       "+SQNSD",       SEQMONARCH_SQNSD_TIMEOUT,    fCmdBuild_SQNSD_MONARCH,      fRspAnalyze_None},
    {CMD_AT_SQNSH,       "+SQNSH", SEQMONARCH_SQNSH_TIMEOUT,    fCmdBuild_SQNSH_MONARCH, fRspAnalyze_SQNSH_MONARCH},
    {CMD_AT_SQNSI,       "+SQNSI", SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SQNSI_MONARCH, fRspAnalyze_SQNSI_MONARCH},
    {CMD_AT_SQNSS,       "+SQNSS", SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_SQNSS_MONARCH},
    {
      CMD_AT_SQNSRECV, "+SQNSRECV", SEQMONARCH_DEFAULT_TIMEOUT,
      fCmdBuild_SQNSRECV_MONARCH, fRspAnalyze_SQNSRECV_MONARCH
    },
    {CMD_AT_SQNSSENDEXT, "+SQNSSENDEXT", SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_SQNSSENDEXT_MONARCH, fRspAnalyze_None},
    {
      CMD_AT_SQNSSEND_WRITE_DATA, "", SEQMONARCH_DEFAULT_TIMEOUT,
      fCmdBuild_SQNSSEND_WRITE_DATA_MONARCH, fRspAnalyze_None
    },
    {CMD_AT_PING,        "+PING", SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_PING_MONARCH,       fRspAnalyze_PING_MONARCH},

    /* MODEM SPECIFIC EVENTS */
    {CMD_AT_WAIT_EVENT,     "",          SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BOOT_EVENT,     "",          SEQMONARCH_BOOT_TIMEOUT,     fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SYSSTART_TYPE1, "+SYSSTART", SEQMONARCH_SYSSTART_TIMEOUT, fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SYSSTART_TYPE2, "^SYSSTART", SEQMONARCH_SYSSTART_TIMEOUT, fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SYSSHDN,        "+SYSSHDN",  SEQMONARCH_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SOCKET_PROMPT,  "> ",        SEQMONARCH_SOCKET_PROMPT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    /* */
  };
#define SIZE_ATCMD_SEQMONARCH_LUT ((uint16_t) (sizeof (ATCMD_SEQMONARCH_LUT) / sizeof (atcustom_LUT_t)))
  /* common init */
  monarch_modem_init(&SEQMONARCH_ctxt);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  SEQMONARCH_ctxt.modem_LUT_size = SIZE_ATCMD_SEQMONARCH_LUT;
  SEQMONARCH_ctxt.p_modem_LUT = (const atcustom_LUT_t *)ATCMD_SEQMONARCH_LUT;

  /* set default termination char for AT command: <CR> */
  (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->endstr, "\r");
  /* ###########################  END CUSTOMIZATION PART  ########################### */
}

uint8_t ATCustom_MONARCH_checkEndOfMsgCallback(uint8_t rxChar)
{
  uint8_t last_char = 0U;

  /*---------------------------------------------------------------------------------------*/
  if (SEQMONARCH_ctxt.state_SyntaxAutomaton == WAITING_FOR_INIT_CR)
  {
    /* waiting for first valid <CR>, char received before are considered as trash */
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (SEQMONARCH_ctxt.state_SyntaxAutomaton == WAITING_FOR_CR)
  {
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (SEQMONARCH_ctxt.state_SyntaxAutomaton == WAITING_FOR_LF)
  {
    /* waiting for <LF> */
    if ((AT_CHAR_t)('\n') == rxChar)
    {
      /*  current        : <CR><LF>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : x or <CR>
      */
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_FIRST_CHAR;
      last_char = 1U;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (SEQMONARCH_ctxt.state_SyntaxAutomaton == WAITING_FOR_FIRST_CHAR)
  {
    if (SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_waiting_header)
    {
      /* Socket Data RX - waiting for Header: we are waiting for first <CR>
      *
      * <CR><LF>+SQNSRECV: 1,522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *  ^- waiting this <CR>
      */
      if ((AT_CHAR_t)('\r') == rxChar)
      {
        /* first <CR> detected, next step */
        socketHeaderRX_reset();
        SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_header;
      }
    }
    else if (SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_header)
    {
      /* Socket Data RX - Header received: we are waiting for second <CR>
      *
      * <CR><LF>+SQNSRECV: 1,522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *                          ^- waiting this <CR>
      */
      if ((AT_CHAR_t)('\r') == rxChar)
      {
        /* second <CR> detected, we have received data header
        *  now waiting for <LF>, then start to receive socket datas
        *  Verify that size received in header is the expected one
        */
        uint16_t size_from_header = SocketHeaderRX_getSize();
        if (SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size != size_from_header)
        {
          /* update buffer size received - should not happen */
          SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size = size_from_header;
        }
        SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
        SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_data;
      }
      else if (rxChar == (AT_CHAR_t)(','))
      {
        SocketHeaderRX_separator_received();
      }
      else if ((rxChar >= (AT_CHAR_t)('0')) && (rxChar <= (AT_CHAR_t)('9')))
      {
        /* receiving size of data in header */
        SocketHeaderRX_addChar((CRC_CHAR_t *)&rxChar);
      }
      else {/* nothing to do */ }
    }
    else if (SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_data)
    {
      /* receiving socket data: do not analyze char, just count expected size
      *
      * <CR><LF>+SQNSRECV: 1,522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF>...
      *.                                ^- start to read data: count char
      */
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_SOCKET_DATA;
      SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
      /* check if full buffer has been received */
      if (SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received ==
          SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size)
      {
        SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
        SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
      }
    }
    /* waiting for <CR> or x */
    else if ((AT_CHAR_t)('\r') == rxChar)
    {
      /*   current        : <CR>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : <LF>
      */
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
    else {/* nothing to do */ }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (SEQMONARCH_ctxt.state_SyntaxAutomaton == WAITING_FOR_SOCKET_DATA)
  {
    SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
    /* check if full buffer has been received */
    if (SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received ==
        SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size)
    {
      SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
      SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
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
    /* MONARCH special cases
    *
    *  SOCKET MODE: when sending DATA using AT+SQNSSENDEXT, we are waiting for socket prompt "<CR><LF>> "
    *               before to send DATA. Then we should receive "OK<CR><LF>".
    */

    if (SEQMONARCH_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
    {
      switch (SEQMONARCH_ctxt.socket_ctxt.socket_send_state)
      {
        case SocketSendState_WaitingPrompt1st_greaterthan:
        {
          /* detecting socket prompt first char: "greater than" */
          if ((AT_CHAR_t)('>') == rxChar)
          {
            SEQMONARCH_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt2nd_space;
          }
          break;
        }

        case SocketSendState_WaitingPrompt2nd_space:
        {
          /* detecting socket prompt second char: "space" */
          if ((AT_CHAR_t)(' ') == rxChar)
          {
            SEQMONARCH_ctxt.socket_ctxt.socket_send_state = SocketSendState_Prompt_Received;
            last_char = 1U;
          }
          else
          {
            /* if char iommediatly after "greater than" is not a "space", reinit state */
            SEQMONARCH_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
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

at_status_t ATCustom_MONARCH_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout)
{
  /********************************************************************
  Modem Model Selection
    *********************************************************************/
  at_status_t retval = ATSTATUS_OK;
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_msg_t curSID = p_atp_ctxt->current_SID;

  PRINT_API("enter ATCustom_MONARCH_getCmd() for SID %d", curSID)

  /* retrieve parameters from SID command (will update SID_ctxt) */
  if (atcm_retrieve_SID_parameters(&SEQMONARCH_ctxt, p_atp_ctxt) != ATSTATUS_OK)
  {
    retval = ATSTATUS_ERROR;
    goto exit_ATCustom_MONARCH_getCmd;
  }

  /* new command: reset command context */
  atcm_reset_CMD_context(&SEQMONARCH_ctxt.CMD_ctxt);

  /* For each SID, athe sequence of AT commands to send id defined (it can be dynamic)
    * Determine and prepare the next command to send for this SID
    */

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  if (curSID == (at_msg_t) SID_CS_CHECK_CNX)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
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
      /* NOT IMPLEMENTED */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
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
    uint8_t common_start_sequence_step = 2U;

    /* POWER_ON and RESET are almost the same, specific differences are managed case by case */

    /* for reset, only HW reset is supported and treated as Power ON  */
    if ((curSID == (at_msg_t) SID_CS_POWER_ON) ||
        ((curSID == (at_msg_t) SID_CS_RESET) && (SEQMONARCH_ctxt.SID_ctxt.reset_type == CS_RESET_HW)))
    {
      /****************************************************************************
        * POWER_ON and RESET first steps
        * try to establish the communiction with the modem by sending "AT" commands
        ****************************************************************************/
      if (p_atp_ctxt->step == 0U)
      {
        /* reset modem specific variables */
        reset_variables_monarch();

        /* check if +SYSSTART has been received */
        if (SEQMONARCH_ctxt.persist.modem_at_ready == AT_TRUE)
        {
          PRINT_DBG("Modem START indication already received, continue init sequence...")
          /* now reset modem_at_ready (in case of reception of reset indication) */
          SEQMONARCH_ctxt.persist.modem_at_ready = AT_FALSE;

          if (curSID == (at_msg_t) SID_CS_RESET)
          {
            /* reinit context for reset case */
            monarch_modem_reset(&SEQMONARCH_ctxt);
          }

          /* force flow control */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
        }
        else
        {
          if (curSID == (at_msg_t) SID_CS_RESET)
          {
            /* reinit context for reset case */
            monarch_modem_reset(&SEQMONARCH_ctxt);
          }

          PRINT_DBG("Modem START indication not received yet, waiting for it...")
          /* wait for +SYSSTART */
          atcm_program_WAIT_EVENT(p_atp_ctxt, SEQMONARCH_BOOT_TIMEOUT, INTERMEDIATE_CMD);
        }
      }
      else if (p_atp_ctxt->step == 1U)
      {
        /* check if SIM is ready */
        if (SEQMONARCH_ctxt.persist.modem_sim_ready == AT_TRUE)
        {
          PRINT_DBG("Modem SIM indication already received, continue init sequence...")
          SEQMONARCH_ctxt.persist.modem_sim_ready = AT_FALSE;

          /* skip this step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
        else
        {
          /* skip this step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
      }
      /********************************************************************
        * common power ON/RESET sequence starts here
        * when communication with modem has been successfully established
        ********************************************************************/
      else if (p_atp_ctxt->step == common_start_sequence_step)
      {
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 1U))
      {
        /* force flow control */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 2U))
      {
        /* disable echo */
        SEQMONARCH_ctxt.CMD_ctxt.command_echo = AT_FALSE;
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 3U))
      {
        /* Read baud rate settings */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_IPR, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 4U))
      {
        /* request detailled error report */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CMEE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 5U))
      {
        /* enable full response format */
        SEQMONARCH_ctxt.CMD_ctxt.dce_full_resp_format = AT_TRUE;
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATV, INTERMEDIATE_CMD);
      }
      /* request detailled error report */
      else if (p_atp_ctxt->step == (common_start_sequence_step + 6U))
      {
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CMEE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 7U))
      {
#if defined(MODEM_TEST_FIRMWARE)
        /* skip this step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
#else
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGDCONT, INTERMEDIATE_CMD);
#endif /* MODEM_TEST_FIRMWARE */
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 8U))
      {
        /* set CFUN = 0 for GM01Q */
        SEQMONARCH_ctxt.CMD_ctxt.cfun_value = 0U;
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, FINAL_CMD);
      }
      else
      {
        /* error, invalid step */
        retval = ATSTATUS_ERROR;
      }
    }
    else
    {
      /* manage other RESET types (different from HW)
        */
      if (SEQMONARCH_ctxt.SID_ctxt.reset_type == CS_RESET_FACTORY_RESET)
      {
#define FACTORY_RESET_ENABLED
#if defined(FACTORY_RESET_ENABLED)
        /* IN CASE OF FACTORY RESET:
        set conformance test mode (replace ??? by standard, 3gpp-conformance or a supported operator name)
        AT+SQNCTM="???"
        -> OK
        AT^RESET
        -> OK
        modem will restart automatically : +SYSSTART
        THEN
        AT+CGDCONT=3,"IP",""
        -> OK
        AT^AUTOATT=1
        -> OK
        AT+CFUN=0  (IT IS MANDATORY TO MAKE PREVIOUS CONFIG PERSISTANT !!!)
        -> OK
        AT^RESET
        -> OK
        modem will restart automatically : +SYSSTART
          */
        if (p_atp_ctxt->step == 0U)
        {
          PRINT_INFO("***** START FACTORY RESET *****")
          /* reprogram Conformance Test Mode with AT+SQNCTM */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                              ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNCTM, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 1U)
        {
          PRINT_INFO("***** trigger RESET *****")

          /* reset modem specific variables */
          reset_variables_monarch();

          /* reset sim state */
          SEQMONARCH_ctxt.persist.modem_sim_ready = AT_FALSE;

          /* trigger reset with AT^RESET */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                              ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_RESET, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 2U)
        {
          PRINT_INFO("***** wait for SYSSTART *****")
          /* wait for +SYSSTART */
          atcm_program_WAIT_EVENT(p_atp_ctxt, SEQMONARCH_BOOT_TIMEOUT, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 3U)
        {
          /* skip this step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
        else if (p_atp_ctxt->step == 4U)
        {
          PRINT_INFO("***** disable echo *****")
          SEQMONARCH_ctxt.CMD_ctxt.command_echo = AT_FALSE;
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATE, FINAL_CMD);
        }
        else if (p_atp_ctxt->step == 5U)
        {
#if defined(MODEM_TEST_FIRMWARE)
          /* skip this step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
#else
          PRINT_INFO("***** reprogam CGDCONT *****")
          /* reprogram AT+CGDCONT, for example: AT+CGDCONT=3,"IPV4V6","" */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_CGDCONT_REPROGRAM,
                              INTERMEDIATE_CMD);
#endif /* MODEM_TEST_FIRMWARE */
        }
        else if (p_atp_ctxt->step == 6U)
        {
          PRINT_INFO("***** stop RADIO to save paramaters *****")
          /* stop the radio with AT+CFUN=0*/
          SEQMONARCH_ctxt.CMD_ctxt.cfun_value = 0U;
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                              ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 7U)
        {
          PRINT_INFO("***** reprogam AUTOATTACH *****")
          /* reprogram auto attach mode with AT^AUTOATT */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                              ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_AUTOATT, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 8U)
        {
          /* reset modem specific variables */
          reset_variables_monarch();

          /* reset sim state */
          SEQMONARCH_ctxt.persist.modem_sim_ready = AT_FALSE;

          PRINT_INFO("***** trigger RESET *****")
          /* trigger reset with AT^RESET */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                              ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_RESET, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 9U)
        {
          PRINT_INFO("***** wait for SYSSTART *****")
          /* wait for +SYSSTART */
          atcm_program_WAIT_EVENT(p_atp_ctxt, SEQMONARCH_BOOT_TIMEOUT, INTERMEDIATE_CMD);
        }
        else if (p_atp_ctxt->step == 10U)
        {
          /* skip this step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
        }
        else if (p_atp_ctxt->step == 11U)
        {
          PRINT_INFO("***** disable echo *****")
          SEQMONARCH_ctxt.CMD_ctxt.command_echo = AT_FALSE;
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATE, FINAL_CMD);
        }
        else
        {
          PRINT_ERR("invalid step")
          /* error, invalid step */
          retval = ATSTATUS_ERROR;
        }
#else
        PRINT_DBG("Reset type (%d) not supported", SEQMONARCH_ctxt.SID_ctxt.reset_type)
        retval = ATSTATUS_ERROR;
#endif /* FACTORY_RESET_ENABLED */
      }
      else
      {
        PRINT_DBG("Reset type (%d) not supported", SEQMONARCH_ctxt.SID_ctxt.reset_type)
        retval = ATSTATUS_ERROR;
      }
    }
  }
  else if (curSID == (at_msg_t) SID_CS_POWER_OFF)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* hardware power off for this modem
        * will be done by cellular service just after CMD_AT_SQNSSHDN/CMD_AT_SHUTDOWN
        */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                          ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_SQNSSHDN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      PRINT_INFO("***** waiting for shutdown confirmation urc *****")
      /* wait for +SHUTDOWN or +SQNSSSHDN URC before to safely cut device power */
      atcm_program_WAIT_EVENT(p_atp_ctxt, SEQMONARCH_SHUTDOWN_TIMEOUT, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      if (SEQMONARCH_ctxt.SID_ctxt.modem_init.init == CS_CMI_MINI)
      {
        /* minimum functionnality selected, no SIM access => leave now */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      /* otherwise, wait SIM for a few seconds */
      else if (SEQUANS_WAIT_SIM_TEMPO == 0U)
      {
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        atcm_program_TEMPO(p_atp_ctxt, SEQUANS_WAIT_SIM_TEMPO, INTERMEDIATE_CMD);
        PRINT_INFO("waiting %ld msec before to continue (GM01Q specific)", SEQUANS_WAIT_SIM_TEMPO)
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* request for CCID - only possible under CFUN = 1 or 4 */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_ICCID, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* check is CPIN is requested */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 4U)
    {
      if (SEQMONARCH_ctxt.persist.sim_pin_code_ready == AT_FALSE)
      {
        PRINT_DBG("CPIN required, we send user value to modem")

        if (strlen((const CRC_CHAR_t *)&SEQMONARCH_ctxt.SID_ctxt.modem_init.pincode.pincode) != 0U)
        {
          /* send PIN value */
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CPIN, FINAL_CMD);
        }
        else
        {
          /* no PIN provided by user */
          retval = ATSTATUS_ERROR;
        }
      }
      else
      {
        PRINT_DBG("CPIN not required");
        /* no PIN required, end of init */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
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
      switch (SEQMONARCH_ctxt.SID_ctxt.device_info->field_requested)
      {
        case CS_DIF_MANUF_NAME_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMI, FINAL_CMD);
          break;

        case CS_DIF_MODEL_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMM, FINAL_CMD);
          break;

        case CS_DIF_REV_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMR, FINAL_CMD);
          break;

        case CS_DIF_SN_PRESENT:
          SEQMONARCH_ctxt.CMD_ctxt.cgsn_write_cmd_param = CGSN_SN;
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGSN, FINAL_CMD);
          break;

        case CS_DIF_IMEI_PRESENT:
          SEQMONARCH_ctxt.CMD_ctxt.cgsn_write_cmd_param = CGSN_IMEI;
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGSN, FINAL_CMD);
          break;

        case CS_DIF_IMSI_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CIMI, FINAL_CMD);
          break;

        case CS_DIF_PHONE_NBR_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CNUM, FINAL_CMD);
          break;

        case CS_DIF_ICCID_PRESENT:
          atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_ICCID, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CSQ, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CESQ, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_GET_ATTACHSTATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check if actual registration status is the expected one */
      const CS_OperatorSelector_t *operatorSelect = &(SEQMONARCH_ctxt.SID_ctxt.write_operator_infos);
      if (SEQMONARCH_ctxt.SID_ctxt.read_operator_infos.mode != operatorSelect->mode)
      {
        /* write registration status */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CEREG, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CEREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, FINAL_CMD);
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
      CS_UrcEvent_t urcEvent = SEQMONARCH_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CEREG ? */
      if ((urcEvent == CS_URCEVENT_EPS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_EPS_LOCATION_INFO))
      {
        (void) atcm_subscribe_net_event(&SEQMONARCH_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* no command to monitor signal quality with URC in MONARCH */
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
      CS_UrcEvent_t urcEvent = SEQMONARCH_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CEREG ? */
      if ((urcEvent == CS_URCEVENT_EPS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_EPS_LOCATION_INFO))
      {
        (void) atcm_unsubscribe_net_event(&SEQMONARCH_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* no command to monitor signal quality with URC in MONARCH */
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
      if (SEQMONARCH_ctxt.persist.urc_subscript_pdn_event == CELLULAR_FALSE)
      {
        /* set event as suscribed */
        SEQMONARCH_ctxt.persist.urc_subscript_pdn_event = CELLULAR_TRUE;

        /* request PDN events */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
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
      if (SEQMONARCH_ctxt.persist.urc_subscript_pdn_event == CELLULAR_TRUE)
      {
        /* set event as unsuscribed */
        SEQMONARCH_ctxt.persist.urc_subscript_pdn_event = CELLULAR_FALSE;

        /* request PDN events */
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
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
      SEQMONARCH_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_ATTACHED;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
      SEQMONARCH_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_DETACHED;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
    if (p_atp_ctxt->step == 0U)
    {
      /* PDN activation */
      SEQMONARCH_ctxt.CMD_ctxt.pdn_state = PDN_STATE_ACTIVATE;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGACT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* get IP address */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
#else
    if (p_atp_ctxt->step == 0U)
    {
      /* PDN activation */
      SEQMONARCH_ctxt.CMD_ctxt.pdn_state = PDN_STATE_ACTIVATE;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, CMD_AT_CGACT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* get IP address */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, CMD_AT_CGPADDR, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, CMD_AT_CGDATA, FINAL_CMD);
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
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGDCONT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGAUTH, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* nothing to do here */
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SET_DEFAULT_PDN)
  {
    /* nothing to do here */
    atcm_program_NO_MORE_CMD(p_atp_ctxt);
  }
  else if (curSID == (at_msg_t) SID_CS_GET_IP_ADDRESS)
  {
    atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, FINAL_CMD);
  }
  else if (curSID == (at_msg_t) SID_CS_DIAL_COMMAND)
  {
    /* SOCKET CONNECTION FOR COMMAND DATA MODE */
    if (p_atp_ctxt->step == 0U)
    {
      /* reserve a modem CID for this socket_handle */
      socket_handle_t sockHandle = SEQMONARCH_ctxt.socket_ctxt.socket_info->socket_handle;
      (void) atcm_socket_reserve_modem_cid(&SEQMONARCH_ctxt, sockHandle);
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSCFG, INTERMEDIATE_CMD);
      PRINT_INFO("For Client Socket Handle=%ld : MODEM CID affected=%d",
                 sockHandle,
                 SEQMONARCH_ctxt.persist.socket[sockHandle].socket_connId_value)
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                          ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSCFGEXT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSD, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* socket is connected */
      (void) atcm_socket_set_connected(&SEQMONARCH_ctxt, SEQMONARCH_ctxt.socket_ctxt.socket_info->socket_handle);
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
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
      if (SEQMONARCH_ctxt.SID_ctxt.socketSendData_struct.buffer_size > MODEM_MAX_SOCKET_TX_DATA_SIZE)
      {
        PRINT_ERR("Data size to send %ld exceed maximum size %ld",
                  SEQMONARCH_ctxt.SID_ctxt.socketSendData_struct.buffer_size,
                  MODEM_MAX_SOCKET_TX_DATA_SIZE)
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
        retval = ATSTATUS_ERROR;
      }
      else
      {
        SEQMONARCH_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                            ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSSENDEXT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* waiting for socket prompt: "<CR><LF>> " */
      if (SEQMONARCH_ctxt.socket_ctxt.socket_send_state == SocketSendState_Prompt_Received)
      {
        PRINT_DBG("SOCKET PROMPT ALREADY RECEIVED")
        /* skip this step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        PRINT_DBG("WAITING FOR SOCKET PROMPT")
        atcm_program_WAIT_EVENT(p_atp_ctxt, SEQMONARCH_SOCKET_PROMPT_TIMEOUT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* socket prompt received, send DATA */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                          ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_SQNSSEND_WRITE_DATA, FINAL_CMD);

      /* reinit automaton to receive answer */
      reinitSyntaxAutomaton_monarch();
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
      SEQMONARCH_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestSize;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSI, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check that data size to receive is not null */
      if (SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size != 0U)
      {
        /* check that data size to receive does not exceed maximum size
          *  if it's the case, only request maximum size we can receive
          */
        if (SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size >
            SEQMONARCH_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
        {
          PRINT_INFO("Data size available (%ld) exceed buffer maximum size (%ld)",
                     SEQMONARCH_ctxt.socket_ctxt.socket_rx_expected_buf_size,
                     SEQMONARCH_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
        }

        /* receive datas */
        SEQMONARCH_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestData_Header;
        atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSRECV, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNSH, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* release the modem CID for this socket_handle */
      (void) atcm_socket_release_modem_cid(&SEQMONARCH_ctxt, SEQMONARCH_ctxt.socket_ctxt.socket_info->socket_handle);
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
      atcm_program_TEMPO(p_atp_ctxt, SEQMONARCH_DATA_SUSPEND_TIMEOUT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* send escape sequence +++ (RAW command type)
        *  CONNECT expected before 1000 ms
        */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_ESC_CMD, FINAL_CMD);
      reinitSyntaxAutomaton_monarch();
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATO, FINAL_CMD);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_INIT_POWER_CONFIG)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* Init parameters are available in to SID_ctxt.init_power_config
        * SID_ctxt.init_power_config  is used to build AT+CPSMS and AT+CEDRX commands
        * Built it from SID_ctxt.init_power_config  and modem specificities
        */
      if (init_monarch_low_power() == AT_FALSE)
      {
        /* Low Power not enabled, stop here the SID */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        /* Low Power enabled, send commands */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* read EDRX params */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CEDRXS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* read PSM params */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CPSMS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* set EDRX params (default) */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CEDRXS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 4U)
    {
      /* set PSM params (default) */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CPSMS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 5U)
    {
      /* note: keep this as final command (previous command may be skipped if no valid PSM parameters) */
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SET_POWER_CONFIG)
  {
    if (p_atp_ctxt->step == 0U)
    {
      /* set EDRX params (if available) */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CEDRXS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* set PSM params (if available) */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CPSMS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* TODO temporary step for test purpose, a read function should be implemented */
      /* eDRX Read Dynamix Parameters */
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt,
                          ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CEDRXRDP, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* note: keep this as final command (previous command may be skipped if no valid PSM parameters) */
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SLEEP_REQUEST)
  {
#if !defined(LP_TEST_MONARCH) /* LP_TEST_MONARCH */
    if (p_atp_ctxt->step == 0U)
    {
      /* AT+CFUN=0 is required to enable modem low power*/
      SEQMONARCH_ctxt.CMD_ctxt.cfun_value = 0U;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* wait for some seconds (if network attached...) TODO temporary solution */
      atcm_program_TEMPO(p_atp_ctxt, 4000U, INTERMEDIATE_CMD);
    }
#else
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
#endif /* LP_TEST_MONARCH */
    else if (p_atp_ctxt->step == 2U)
    {
      low_power_state_requested();
      (void) SysCtrl_MONARCH_suspend_channel_request(p_at_ctxt->ipc_handle, DEVTYPE_MODEM_CELLULAR);
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* end of SID */
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SLEEP_COMPLETE)
  {
    if (p_atp_ctxt->step == 0U)
    {
      (void) SysCtrl_MONARCH_suspend_channel_complete(p_at_ctxt->ipc_handle, DEVTYPE_MODEM_CELLULAR);
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SLEEP_CANCEL)
  {
    if (p_atp_ctxt->step == 0U)
    {
      low_power_state_cancel();

      /* wake up modem (in case modem already enters in Low Power or we missed the URC from modem) */
      (void) SysCtrl_MONARCH_resume_channel(p_at_ctxt->ipc_handle, DEVTYPE_MODEM_CELLULAR);
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
#if !defined(LP_TEST_MONARCH) /* LP_TEST_MONARCH */
    else if (p_atp_ctxt->step == 1U)
    {
      PRINT_INFO("restart radio...")
      SEQMONARCH_ctxt.CMD_ctxt.cfun_value = 1U;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* NOTE:
        *   in current implementation, GM01Q needs CFUN=0 to enter in low power
        *   When leaving LP, some time is needed after CFUN=1 to recover a "normal" state
        *   Otherwise, NO CARRIER error can occur
        */
      atcm_program_TEMPO(p_atp_ctxt, SEQMONARCH_RESTART_RADIO_TIMEOUT, INTERMEDIATE_CMD);
      PRINT_INFO("wait before to continue (GM01Q specific)")
    }
    else if (p_atp_ctxt->step == 3U)
    {
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
#else
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
#endif /* LP_TEST_MONARCH */
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_WAKEUP)
  {
    if (p_atp_ctxt->step == 0U)
    {
      (void) SysCtrl_MONARCH_resume_channel(p_at_ctxt->ipc_handle, DEVTYPE_MODEM_CELLULAR);
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
#if !defined(LP_TEST_MONARCH) /* LP_TEST_MONARCH */
    else if (p_atp_ctxt->step == 1U)
    {
      PRINT_INFO("restart radio...")
      SEQMONARCH_ctxt.CMD_ctxt.cfun_value = 1U;
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* NOTE:
        *   in current implementation, GM01Q needs CFUN=0 to enter in low power
        *   When leaving LP, some time is needed after CFUN=1 to recover a "normal" state
        *   Otherwise, NO CARRIER error can occur
        */
      atcm_program_TEMPO(p_atp_ctxt, SEQMONARCH_RESTART_RADIO_TIMEOUT, INTERMEDIATE_CMD);
      PRINT_INFO("wait before to continue (GM01Q specific)")
    }
    else if (p_atp_ctxt->step == 3U)
    {
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
#else
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
    }
#endif /* LP_TEST_MONARCH */
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_SOCKET_CNX_STATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_SQNSS, FINAL_CMD);
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
      init_dns_info();
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_SQNDNSLKUP, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_PING, FINAL_CMD);
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
      atcm_program_AT_CMD(&SEQMONARCH_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_DIRECT_CMD, FINAL_CMD);
      atcm_program_CMD_TIMEOUT(&SEQMONARCH_ctxt, p_atp_ctxt, SEQMONARCH_ctxt.SID_ctxt.direct_cmd_tx->cmd_timeout);
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
      if (atcm_select_hw_simslot(SEQMONARCH_ctxt.persist.sim_selected) != ATSTATUS_OK)
      {
        retval = ATSTATUS_ERROR;
      }
      /* skip this step */
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
    else if (p_atp_ctxt->step == 1U)
    {
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
    PRINT_ERR("Error, invalid command ID")
    retval = ATSTATUS_ERROR;
  }

  /* if no error, build the command to send */
  if (retval == ATSTATUS_OK)
  {
    retval = atcm_modem_build_cmd(&SEQMONARCH_ctxt, p_atp_ctxt, p_ATcmdTimeout);
  }

exit_ATCustom_MONARCH_getCmd:
  return (retval);
}

at_endmsg_t ATCustom_MONARCH_extractElement(atparser_context_t *p_atp_ctxt,
                                            const IPC_RxMessage_t *p_msg_in,
                                            at_element_info_t *element_infos)
{
  at_endmsg_t retval_msg_end_detected = ATENDMSG_NO;
  bool exit_loop;
  uint16_t idx;
  uint16_t start_idx;
  uint16_t *p_parseIndex = &(element_infos->current_parse_idx);

  PRINT_API("enter ATCustom_MONARCH_extractElement()")
  PRINT_DBG("input message: size=%d ", p_msg_in->size)

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
      if ((p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_SQNSRECV) &&
          (SEQMONARCH_ctxt.socket_ctxt.socket_receive_state == SocketRcvState_RequestData_Payload) &&
          (SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state != SocketRxDataState_finished))
      {
        PRINT_DBG("receiving socket data (real size=%d)", SocketHeaderRX_getSize())
        element_infos->str_start_idx = 0U;
        element_infos->str_end_idx = (uint16_t) SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        element_infos->str_size = (uint16_t) SEQMONARCH_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        SEQMONARCH_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_finished;
        retval_msg_end_detected = ATENDMSG_YES;
        exit_loop = true;
      }
      else if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_SQNSS)
      {
        /* SPECIAL CASE:
          * SNQNSS without <CR><LF> sequence has been found, it is a command line
          */
        PRINT_DBG("SQNSS command detected")
        *p_parseIndex = 0U;
        exit_loop = true;
      }
      else { /* nothing to do */ }
    }

    /* check if end of message has been detected */
    if (retval_msg_end_detected == ATENDMSG_YES)
    {
      goto exit_ATCustom_MONARCH_extractElement;
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
    goto exit_ATCustom_MONARCH_extractElement;
  };

  /* extract parameter from message */
  exit_loop = false;
  do
  {
    switch (p_msg_in->buffer[*p_parseIndex])
    {
      /* ###########################  START CUSTOMIZATION PART  ########################### */
      /* ----- test separators ----- */
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
    };
  } while (exit_loop == false);

  /* increase parameter rank */
  element_infos->param_rank = (element_infos->param_rank + 1U);

exit_ATCustom_MONARCH_extractElement:
  return (retval_msg_end_detected);
}

at_action_rsp_t ATCustom_MONARCH_analyzeCmd(at_context_t *p_at_ctxt,
                                            const IPC_RxMessage_t *p_msg_in,
                                            at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval;

  PRINT_API("enter ATCustom_MONARCH_analyzeCmd()")

  /* Analyze data received from the modem and
    * search in LUT the ID corresponding to command received
    */
  if (ATSTATUS_OK != atcm_searchCmdInLUT(&SEQMONARCH_ctxt, p_atp_ctxt, p_msg_in, element_infos))
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
    retval = atcm_check_text_line_cmd(&SEQMONARCH_ctxt, p_at_ctxt, p_msg_in, element_infos);

    /* 2nd STEP: search in specific modems commands if not found at 1st STEP
      *
      * This is the case in socket mode when receiving data.
      * The command is decomposed in 2 lines:
      * The 1st part of the response is analyzed by atcm_searchCmdInLUT:
      *   <CR><LF>+SQNSRECV: 1,522<CR><LF>
      * The 2nd part of the response, corresponding to the datas, falls here:
      *   HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF><CR><LF>Serve...
      */
    if (retval == ATACTION_RSP_NO_ACTION)
    {
      switch (p_atp_ctxt->current_atcmd.id)
      {
        /* ###########################  START CUSTOMIZED PART  ########################### */
        case CMD_AT_SQNSRECV:
          /* receive data in SQNSRECV command */
          if (fRspAnalyze_SQNSRECV_data_MONARCH(p_at_ctxt, &SEQMONARCH_ctxt, p_msg_in, element_infos)
              != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        /* ###########################  END CUSTOMIZED PART  ########################### */
        default:
          /* this is not one of modem common command, need to check if this is an answer to a modem's specific cmd */
          PRINT_ERR("receive an un-expected line... is it a text line ?")
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
        else if ((p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_ON)
                 || (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RESET))
        {
          if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT)
          {
            /* modem is synchronized */
            SEQMONARCH_ctxt.persist.modem_at_ready = AT_TRUE;
          }
          if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_ATE)
          {
            PRINT_DBG("Echo successfully disabled")
          }
        }
        else
        {
          /* nothing to do */
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
      case CMD_AT_CEREG:
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

      case CMD_AT_SYSSTART_TYPE1:
      case CMD_AT_SYSSTART_TYPE2:
        /* We received SYSSTART event from the modem.
          * If received during Power ON or RESET, it is indicating that the modem is ready.
          * If received in another state, we report to upper layer a modem reboot event.
          */

        /* modem is ready */
        SEQMONARCH_ctxt.persist.modem_at_ready = AT_TRUE;

        if ((p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_ON) ||
            (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RESET))
        {
          PRINT_DBG("continue INIT modem sequence  (final = %d)", p_atp_ctxt->is_final_cmd)
          SEQMONARCH_ctxt.persist.modem_at_ready = AT_FALSE;
          /* UNLOCK the WAIT EVENT */
          retval = ATACTION_RSP_FRC_END;
        }
        else
        {
          /* if event is received in other states, it's an unexpected modem reboot */
          if (atcm_modem_event_received(&SEQMONARCH_ctxt, CS_MDMEVENT_BOOT) == AT_TRUE)
          {
            retval = ATACTION_RSP_URC_FORWARDED;
          }
          else
          {
            retval = ATACTION_RSP_URC_IGNORED;
          }
        }
        break;

      case CMD_AT_SHUTDOWN:
      case CMD_AT_SQNSSHDN:
        /* if we were waiting for this event, we can continue the sequence */
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_OFF)
        {
          PRINT_DBG("modem shutdown confirmation received")
          /* UNLOCK the WAIT EVENT */
          retval = ATACTION_RSP_FRC_END;
        }
        else
        {
          /* unexpected modem POWER DOWN EVENT DETECTED
            * forward it to upper layer if it has subscribed to this event
            */
          if (atcm_modem_event_received(&SEQMONARCH_ctxt, CS_MDMEVENT_POWER_DOWN) == AT_TRUE)
          {
            retval = ATACTION_RSP_URC_FORWARDED;
          }
          else
          {
            retval = ATACTION_RSP_URC_IGNORED;
          }
        }
        break;

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

      case CMD_AT_SYSSHDN:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_SQNSH:
        /* check if response received corresponds to the command we have send
          *  if not => this is an URC
          */
        if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
        {
          PRINT_INFO("+SQNSH intermediate")
          retval = ATACTION_RSP_INTERMEDIATE;
        }
        else
        {
          PRINT_INFO("+SQNSH URC")
          retval = ATACTION_RSP_URC_FORWARDED;
        }
        break;

      case CMD_AT_SQNSRING:
        /* this is an URC, socket RING (format depends on the last +SQNSCFGEXT setting)  */
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case CMD_AT_CGEV:
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case CMD_AT_SMST:
        retval = ATACTION_RSP_INTERMEDIATE;
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
        retval = fRspAnalyze_Error_MONARCH(p_at_ctxt, &SEQMONARCH_ctxt, p_msg_in, element_infos);
        break;

      case CMD_AT_CME_ERROR:
      case CMD_AT_CMS_ERROR:
        /* do the analyze here because will not be called by the parser */
        retval = fRspAnalyze_Error_MONARCH(p_at_ctxt, &SEQMONARCH_ctxt, p_msg_in, element_infos);
        break;

      default:
        /* check if response received corresponds to the command we have send
          *  if not => this is an ERROR
          */
        if (element_infos->cmd_id_received == p_atp_ctxt->current_atcmd.id)
        {
          retval = ATACTION_RSP_INTERMEDIATE;
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

at_action_rsp_t ATCustom_MONARCH_analyzeParam(at_context_t *p_at_ctxt,
                                              const IPC_RxMessage_t *p_msg_in,
                                              at_element_info_t *element_infos)
{
  at_action_rsp_t retval;
  PRINT_API("enter ATCustom_MONARCH_analyzeParam()")

  /* analyse parameters of the command we received:
    * call the corresponding function from the LUT
    */
  retval = (atcm_get_CmdAnalyzeFunc(&SEQMONARCH_ctxt, element_infos->cmd_id_received))(p_at_ctxt,
           &SEQMONARCH_ctxt,
           p_msg_in,
           element_infos);
  return (retval);
}

/* function called to finalize an AT command */
at_action_rsp_t ATCustom_MONARCH_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter ATCustom_MONARCH_terminateCmd()")

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  if (SEQMONARCH_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
  {
    /* special case for SID_CS_SEND_DATA
      * indeed, this function is called when an AT cmd is finished
      * but for AT+SQNSSENDEXT, it is called a 1st time when prompt is received
      * and a second time when data have been sent.
      */
    if (p_atp_ctxt->current_SID != (at_msg_t) SID_CS_SEND_DATA)
    {
      /* reset socket_send_state */
      SEQMONARCH_ctxt.socket_ctxt.socket_send_state = SocketSendState_No_Activity;
    }
  }

  /* additional tests */
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

  /* test +SMST result
    * note: works also if AT+SMST sent through direct command
    */
  if ((p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_SMST) ||
      (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_DIRECT_CMD))
  {
    if (monarch_shared.SMST_sim_error_status != 0U)
    {
      monarch_shared.SMST_sim_error_status = 0U;
      retval = ATACTION_RSP_ERROR;
    }
  }
#if (GM01Q_ACTIVATE_PING_REPORT == 0)
  if (p_atp_ctxt->current_atcmd.id == CMD_AT_PING)
  {
    PRINT_ERR("PING not supported actually, force an ERROR !!!")
    retval = ATACTION_RSP_ERROR;
  }
#endif /* (GM01Q_ACTIVATE_PING_REPORT == 0) */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

/* function called to finalize a SID */
at_status_t ATCustom_MONARCH_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_MONARCH_get_rsp()")

  /* prepare response for a SID - common part */
  retval = atcm_modem_get_rsp(&SEQMONARCH_ctxt, p_atp_ctxt, p_rsp_buf);

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
                               (uint16_t) sizeof(monarch_shared.SQNDNSLKUP_dns_info.hostIPaddr),
                               (void *)monarch_shared.SQNDNSLKUP_dns_info.hostIPaddr) != DATAPACK_OK)
      {
        retval = ATSTATUS_OK;
      }
      break;

    case SID_CS_POWER_OFF:
      /* reinit context for power off case */
      monarch_modem_reset(&SEQMONARCH_ctxt);
      break;

#if (GM01Q_ACTIVATE_PING_REPORT == 1)
    case SID_CS_PING_IP_ADDRESS:
    {
      /* PING responses from the modem are synchronous for this modem.
        * It is not possible to trigger an URC now, so update the PING report now.
        * Final reports (needed by upper layer) will be send by cellular service.
        *  note: only the last PING received will be sent in this case.
        */
      PRINT_INFO("Ping final report")

      /* prepare response */
      clear_ping_resp_struct(&SEQMONARCH_ctxt);
      SEQMONARCH_ctxt.persist.ping_resp_urc.ping_status = CELLULAR_OK;
      /* simulate final report datas */
      SEQMONARCH_ctxt.persist.ping_resp_urc.is_final_report = CELLULAR_TRUE;
      /* index expected by COM  for final report = number of pings requested + 1 */
      SEQMONARCH_ctxt.persist.ping_resp_urc.index = SEQMONARCH_ctxt.persist.ping_infos.ping_params.pingnum + 1U;
      if (DATAPACK_writeStruct(p_rsp_buf,
                               (uint16_t) CSMT_URC_PING_RSP,
                               (uint16_t) sizeof(CS_Ping_response_t),
                               (void *)&SEQMONARCH_ctxt.persist.ping_resp_urc) != DATAPACK_OK)
      {
        retval = ATSTATUS_OK;
      }
      break;
    }
#endif /* (GM01Q_ACTIVATE_PING_REPORT == 1) */

    default:
      break;
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */

  /* reset SID context */
  atcm_reset_SID_context(&SEQMONARCH_ctxt.SID_ctxt);

  /* reset socket context */
  atcm_reset_SOCKET_context(&SEQMONARCH_ctxt);

  return (retval);
}

at_status_t ATCustom_MONARCH_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_MONARCH_get_urc()")

  /* prepare response for an URC - common part */
  retval = atcm_modem_get_urc(&SEQMONARCH_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* prepare response for an URC
    *  all specific behaviors for an URC have to be implemented here
    */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_MONARCH_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_MONARCH_get_error()")

  /* prepare response when an error occured - common part */
  retval = atcm_modem_get_error(&SEQMONARCH_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /*  prepare response when an error occured
    *  all specific behaviors for an error have to be implemented here
    */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_MONARCH_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate)
{
  UNUSED(gstate);
  at_status_t retval = ATSTATUS_ERROR;
  /* Do not add traces (called under interrupt if GPIO event) */

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  if (hwEvent == HWEVT_MODEM_RING)
  {
    /* check that:
      *  - RING state is low (filter "bad" interrupts)
      *  - this event was susbribed
      * NOTE: for Sequans modem, rebounds are observed on RING gpio.
      *       Do not take into account RING gpio state retrieved immediatly after the interrupt as it can
      *       be wrong due to rebounds. Check it now again and use this value to determine if RING is at low state.
      */
    if (is_waiting_modem_low_power_ack() == true)
    {
      if (atcm_modem_event_received(&SEQMONARCH_ctxt, CS_MDMEVENT_LP_ENTER) == AT_TRUE)
      {
        GPIO_PinState gpiostate = HAL_GPIO_ReadPin(MODEM_RING_GPIO_PORT, MODEM_RING_PIN);
        if (gpiostate == GPIO_PIN_RESET)
        {
          low_power_state_enter();
          AT_internalEvent(deviceType);
          retval = ATSTATUS_OK;
        }
      }
    }
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

/* Private function Definition -----------------------------------------------*/

/* MONARCH modem init fonction
  *  call common init function and then do actions specific to this modem
  */
static void monarch_modem_init(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter monarch_modem_init")

  /* common init function (reset all contexts) */
  atcm_modem_init(p_modem_ctxt);

  /* modem specific actions if any */
}

/* MONARCH modem reset fonction
  *  call common reset function and then do actions specific to this modem
  */
static void monarch_modem_reset(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter monarch_modem_reset")

  /* common reset function (reset all contexts except SID) */
  atcm_modem_reset(p_modem_ctxt);

  /* modem specific actions if any */
}

static void reinitSyntaxAutomaton_monarch(void)
{
  SEQMONARCH_ctxt.state_SyntaxAutomaton = WAITING_FOR_INIT_CR;
}

static void reset_variables_monarch(void)
{
  /* Set default values of MONARCH specific variables after SWITCH ON or RESET */
  /* add default variables value if needed */
  monarch_shared.SMST_sim_error_status = 0U;
}

static void socketHeaderRX_reset(void)
{
  (void) memset((void *)SocketHeaderDataRx_Buf, 0, 4);
  SocketHeaderDataRx_Cpt = 0U;
  SocketHeaderDataRx_Separator = 0U;
}
static void SocketHeaderRX_separator_received(void)
{
  SocketHeaderDataRx_Separator = 1U;
}
static void SocketHeaderRX_addChar(CRC_CHAR_t *rxchar)
{
  if ((SocketHeaderDataRx_Separator == 1U) &&
      (SocketHeaderDataRx_Cpt < 4U))
  {
    (void) memcpy((void *)&SocketHeaderDataRx_Buf[SocketHeaderDataRx_Cpt],
                  (const void *)rxchar,
                  (size_t) sizeof(char));
    SocketHeaderDataRx_Cpt++;
  }
}
static uint16_t SocketHeaderRX_getSize(void)
{
  uint16_t retval = (uint16_t) ATutil_convertStringToInt((uint8_t *)SocketHeaderDataRx_Buf,
                                                         (uint16_t)SocketHeaderDataRx_Cpt);
  return (retval);
}

static void init_dns_info(void)
{
  (void) memset((void *)monarch_shared.SQNDNSLKUP_dns_info.hostIPaddr, 0, MAX_SIZE_IPADDR);
}

static at_bool_t init_monarch_low_power(void)
{
  at_bool_t lp_enabled;

  if (SEQMONARCH_ctxt.SID_ctxt.init_power_config.low_power_enable == CELLULAR_TRUE)
  {
    /* enable PSM in CGREG/CEREG (value = 4) */
    SEQMONARCH_ctxt.persist.psm_requested = AT_TRUE;

    /* send PSM and EDRX commands: need to populate SID_ctxt.set_power_config
     * Provide psm and edrx default parameters provided but disable them for the moment
     */
    SEQMONARCH_ctxt.SID_ctxt.set_power_config.psm_present = CELLULAR_TRUE;
    SEQMONARCH_ctxt.SID_ctxt.set_power_config.psm_mode = PSM_MODE_DISABLE;
    (void) memcpy((void *) &SEQMONARCH_ctxt.SID_ctxt.set_power_config.psm,
                  (void *) &SEQMONARCH_ctxt.SID_ctxt.init_power_config.psm,
                  sizeof(CS_PSM_params_t));

    SEQMONARCH_ctxt.SID_ctxt.set_power_config.edrx_present = CELLULAR_TRUE;
    SEQMONARCH_ctxt.SID_ctxt.set_power_config.edrx_mode = EDRX_MODE_DISABLE;
    (void) memcpy((void *) &SEQMONARCH_ctxt.SID_ctxt.set_power_config.edrx,
                  (void *) &SEQMONARCH_ctxt.SID_ctxt.init_power_config.edrx,
                  sizeof(CS_EDRX_params_t));

    lp_enabled = AT_TRUE;

  }
  else
  {
    /* disable PSM in CGREG/CEREG (value = 2) */
    SEQMONARCH_ctxt.persist.psm_requested = AT_FALSE;

    /* do not send PSM and EDRX commands */
    lp_enabled = AT_FALSE;
  }

  return (lp_enabled);
}

static void low_power_state_requested(void)
{
  /* activate interrupt detection for RING */
  monarch_shared.waiting_for_ring_irq = true;
  (void) SysCtrl_MONARCH_setup_LowPower_Int(1U);
}

static void low_power_state_enter(void)
{
  monarch_shared.waiting_for_ring_irq = false;
}

static void low_power_state_cancel(void)
{
  /* deactivate interrupt detection for RING */
  monarch_shared.waiting_for_ring_irq = false;
  (void) SysCtrl_MONARCH_setup_LowPower_Int(0U);
}

static bool is_waiting_modem_low_power_ack(void)
{
  return (monarch_shared.waiting_for_ring_irq);
}
/* ###########################  END CUSTOMIZATION PART  ########################### */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

