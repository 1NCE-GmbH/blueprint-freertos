/**
  ******************************************************************************
  * @file    at_custom_modem_specific.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code to the
  *          BG96 Quectel modem: LTE-cat-M1 or LTE-cat.NB1(=NB-IOT) or GSM
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

/* BG96 COMPILATION FLAGS to define in project option if needed:*/

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

#if defined(USE_MODEM_BG96)
#if defined(HWREF_B_CELL_BG96_V2)
#else
#error Hardware reference not specified
#endif /* HWREF_B_CELL_BG96_V2 */
#endif /* USE_MODEM_BG96 */

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCUSTOM_SPECIFIC == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "BG96:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "BG96:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P2, "BG96 API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "BG96 ERROR:" format "\n\r", ## args)
#else
#define PRINT_INFO(format, args...)  (void) printf("BG96:" format "\n\r", ## args);
#define PRINT_DBG(...) __NOP(); /* Nothing to do */
#define PRINT_API(...) __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("BG96 ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...) __NOP(); /* Nothing to do */
#define PRINT_DBG(...)  __NOP(); /* Nothing to do */
#define PRINT_API(...)  __NOP(); /* Nothing to do */
#define PRINT_ERR(...)  __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_SPECIFIC */


/* Private defines -----------------------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */
#define BG96_DEFAULT_TIMEOUT  ((uint32_t)15000U)
#define BG96_RDY_TIMEOUT      ((uint32_t)30000U)
#define BG96_SIMREADY_TIMEOUT ((uint32_t)3000U)
#define BG96_ESCAPE_TIMEOUT   ((uint32_t)1000U)  /* maximum time allowed to receive a response to an Escape command */
#define BG96_COPS_TIMEOUT     ((uint32_t)180000U) /* 180 sec */
#define BG96_CGATT_TIMEOUT    ((uint32_t)140000U) /* 140 sec */
#define BG96_CGACT_TIMEOUT    ((uint32_t)150000U) /* 150 sec */
#define BG96_ATH_TIMEOUT      ((uint32_t)90000U) /* 90 sec */
#define BG96_AT_TIMEOUT       ((uint32_t)1000U)  /* timeout for AT */
#define BG96_SOCKET_PROMPT_TIMEOUT ((uint32_t)10000U)
#define BG96_QIOPEN_TIMEOUT   ((uint32_t)150000U) /* 150 sec */
#define BG96_QICLOSE_TIMEOUT  ((uint32_t)150000U) /* 150 sec */
#define BG96_QIACT_TIMEOUT    ((uint32_t)150000U) /* 150 sec */
#define BG96_QIDEACT_TIMEOUT  ((uint32_t)40000U) /* 40 sec */
#define BG96_QNWINFO_TIMEOUT  ((uint32_t)1000U) /* 1000ms */
#define BG96_QIDNSGIP_TIMEOUT ((uint32_t)60000U) /* 60 sec */
#define BG96_QPING_TIMEOUT    ((uint32_t)150000U) /* 150 sec */

#define BG96_MODEM_SYNCHRO_AT_MAX_RETRIES ((uint8_t)30U)
#define BG96_MAX_SIM_STATUS_RETRIES       ((uint8_t)20U) /* maximum number of AT+QINISTAT retries to wait SIM ready
                                                          * multiply by BG96_SIMREADY_TIMEOUT to compute global
                                                          * timeout value
                                                          */

#if !defined(BG96_OPTION_NETWORK_INFO)
/* set default value */
#define BG96_OPTION_NETWORK_INFO (0)
#endif /* BG96_OPTION_NETWORK_INFO */

#if !defined(BG96_OPTION_ENGINEERING_MODE)
/* set default value */
#define BG96_OPTION_ENGINEERING_MODE (0)
#endif /* BG96_OPTION_ENGINEERING_MODE */

/* Global variables ----------------------------------------------------------*/
/* BG96 Modem device context */
static atcustom_modem_context_t BG96_ctxt;

/* shared variables specific to BG96 */
bg96_shared_variables_t bg96_shared =
{
  .QCFG_command_param              = QCFG_unknown,
  .QCFG_command_write              = AT_FALSE,
  .QINDCFG_command_param           = QINDCFG_unknown,
  .QIOPEN_waiting                  = AT_FALSE,
  .QIOPEN_current_socket_connected = 0U,
  .QICGSP_config_command           = AT_TRUE,
  .bg96_sim_status_retries         = 0U,
};

/* Private variables ---------------------------------------------------------*/

/* Socket Data receive: to analyze size received in data header */
static AT_CHAR_t SocketHeaderDataRx_Buf[4];
static uint8_t SocketHeaderDataRx_Cpt;
static uint8_t SocketHeaderDataRx_Cpt_Complete;

/* ###########################  END CUSTOMIZATION PART  ########################### */

/* Private function prototypes -----------------------------------------------*/
/* ###########################  START CUSTOMIZATION PART  ########################### */
static void bg96_modem_init(atcustom_modem_context_t *p_modem_ctxt);
static void bg96_modem_reset(atcustom_modem_context_t *p_modem_ctxt);
static void reinitSyntaxAutomaton_bg96(void);
static void reset_variables_bg96(void);
static void init_bg96_qiurc_dnsgip(void);
static void socketHeaderRX_reset(void);
static void SocketHeaderRX_addChar(CRC_CHAR_t *rxchar);
static uint16_t SocketHeaderRX_getSize(void);

static void display_decoded_GSM_bands(uint32_t gsm_bands);
static void display_decoded_CatM1_bands(uint32_t CatM1_bands_MsbPart, uint32_t CatM1_bands_LsbPart);
static void display_decoded_CatNB1_bands(uint32_t CatNB1_bands_MsbPart, uint32_t CatNB1_bands_LsbPart);
static void display_user_friendly_mode_and_bands_config(void);
static uint8_t display_if_active_band(ATCustom_BG96_QCFGscanseq_t scanseq,
                                      uint8_t rank, uint8_t catM1_on, uint8_t catNB1_on, uint8_t gsm_on);

/* ###########################  END CUSTOMIZATION PART  ########################### */

/* Functions Definition ------------------------------------------------------*/
void ATCustom_BG96_init(atparser_context_t *p_atp_ctxt)
{
  /* Commands Look-up table */
  static const atcustom_LUT_t ATCMD_BG96_LUT[] =
  {
    /* cmd enum - cmd string - cmd timeout (in ms) - build cmd ftion - analyze cmd ftion */
    {CMD_AT,             "",             BG96_AT_TIMEOUT,       fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_OK,          "OK",           BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CONNECT,     "CONNECT",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_RING,        "RING",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_CARRIER,  "NO CARRIER",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_ERROR,       "ERROR",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_BG96},
    {CMD_AT_NO_DIALTONE, "NO DIALTONE",  BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BUSY,        "BUSY",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_NO_ANSWER,   "NO ANSWER",    BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CME_ERROR,   "+CME ERROR",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_Error_BG96},
    {CMD_AT_CMS_ERROR,   "+CMS ERROR",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CmsErr},

    /* GENERIC MODEM commands */
    {CMD_AT_CGMI,        "+CGMI",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMI},
    {CMD_AT_CGMM,        "+CGMM",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMM},
    {CMD_AT_CGMR,        "+CGMR",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGMR},
    {CMD_AT_CGSN,        "+CGSN",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGSN_BG96,  fRspAnalyze_CGSN},
    {CMD_AT_GSN,         "+GSN",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_GSN},
    {CMD_AT_CIMI,        "+CIMI",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CIMI},
    {CMD_AT_CEER,        "+CEER",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEER},
    {CMD_AT_CMEE,        "+CMEE",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_CMEE,       fRspAnalyze_None},
    {CMD_AT_CPIN,        "+CPIN",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_CPIN,       fRspAnalyze_CPIN_BG96},
    {CMD_AT_CFUN,        "+CFUN",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_CFUN,       fRspAnalyze_CFUN_BG96},
    {CMD_AT_COPS,        "+COPS",        BG96_COPS_TIMEOUT,     fCmdBuild_COPS,       fRspAnalyze_COPS},
    {CMD_AT_CNUM,        "+CNUM",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CNUM},
    {CMD_AT_CGATT,       "+CGATT",       BG96_CGATT_TIMEOUT,    fCmdBuild_CGATT,      fRspAnalyze_CGATT},
    {CMD_AT_CGPADDR,     "+CGPADDR",     BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGPADDR,    fRspAnalyze_CGPADDR},
    {CMD_AT_CEREG,       "+CEREG",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_CEREG,      fRspAnalyze_CEREG},
    {CMD_AT_CREG,        "+CREG",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_CREG,       fRspAnalyze_CREG},
    {CMD_AT_CGREG,       "+CGREG",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGREG,      fRspAnalyze_CGREG},
    {CMD_AT_CSQ,         "+CSQ",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CSQ},
    {CMD_AT_CGDCONT,     "+CGDCONT",     BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGDCONT_BG96,    fRspAnalyze_None},
    {CMD_AT_CGACT,       "+CGACT",       BG96_CGACT_TIMEOUT,    fCmdBuild_CGACT,      fRspAnalyze_None},
    {CMD_AT_CGDATA,      "+CGDATA",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGDATA,     fRspAnalyze_None},
    {CMD_AT_CGEREP,      "+CGEREP",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_CGEREP,     fRspAnalyze_None},
    {CMD_AT_CGEV,        "+CGEV",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CGEV},
    {CMD_ATD,            "D",            BG96_DEFAULT_TIMEOUT,  fCmdBuild_ATD_BG96,   fRspAnalyze_None},
    {CMD_ATE,            "E",            BG96_DEFAULT_TIMEOUT,  fCmdBuild_ATE,        fRspAnalyze_None},
    {CMD_ATH,            "H",            BG96_ATH_TIMEOUT,      fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_ATO,            "O",            BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_ATV,            "V",            BG96_DEFAULT_TIMEOUT,  fCmdBuild_ATV,        fRspAnalyze_None},
    {CMD_ATX,            "X",            BG96_DEFAULT_TIMEOUT,  fCmdBuild_ATX,        fRspAnalyze_None},
    {CMD_AT_ESC_CMD,     "+++",          BG96_ESCAPE_TIMEOUT,   fCmdBuild_ESCAPE_CMD, fRspAnalyze_None},
    {CMD_AT_IPR,         "+IPR",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_IPR,        fRspAnalyze_IPR},
    {CMD_AT_IFC,         "+IFC",         BG96_DEFAULT_TIMEOUT,  fCmdBuild_IFC,        fRspAnalyze_None},
    {CMD_AT_AND_W,       "&W",           BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_AND_D,       "&D",           BG96_DEFAULT_TIMEOUT,  fCmdBuild_AT_AND_D,   fRspAnalyze_None},
    {CMD_AT_DIRECT_CMD,  "",             BG96_DEFAULT_TIMEOUT,  fCmdBuild_DIRECT_CMD, fRspAnalyze_DIRECT_CMD},

    /* MODEM SPECIFIC COMMANDS */
    {CMD_AT_QPOWD,       "+QPOWD",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_QPOWD_BG96, fRspAnalyze_None},
    {CMD_AT_QCFG,        "+QCFG",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_QCFG_BG96,  fRspAnalyze_QCFG_BG96},
    {CMD_AT_QIND,        "+QIND",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_QIND_BG96},
    {CMD_AT_QICSGP,      "+QICSGP",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_QICSGP_BG96, fRspAnalyze_None},
    {CMD_AT_QIURC,       "+QIURC",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QIURC_BG96},
    {CMD_AT_SOCKET_PROMPT, "> ",         BG96_SOCKET_PROMPT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SEND_OK,      "SEND OK",     BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_SEND_FAIL,    "SEND FAIL",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_QIDNSCFG,     "+QIDNSCFG",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_QIDNSCFG_BG96, fRspAnalyze_None},
    {CMD_AT_QIDNSGIP,     "+QIDNSGIP",   BG96_QIDNSGIP_TIMEOUT, fCmdBuild_QIDNSGIP_BG96, fRspAnalyze_None},
    {CMD_AT_QCCID,        "+QCCID",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QCCID_BG96},
    {CMD_AT_QICFG,       "+QICFG",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_QICFG_BG96, fRspAnalyze_None},
    {CMD_AT_QINDCFG,     "+QINDCFG",     BG96_DEFAULT_TIMEOUT,  fCmdBuild_QINDCFG_BG96,  fRspAnalyze_QINDCFG_BG96},
    {CMD_AT_QINISTAT,     "+QINISTAT",   BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QINISTAT_BG96},
    {CMD_AT_QCSQ,         "+QCSQ",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QCSQ_BG96},
    {CMD_AT_QUSIM,       "+QUSIM",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_CPSMS,       "+CPSMS",       BG96_DEFAULT_TIMEOUT,  fCmdBuild_CPSMS,  fRspAnalyze_CPSMS},
    {CMD_AT_CEDRXS,      "+CEDRXS",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_CEDRXS, fRspAnalyze_CEDRXS},
    {CMD_AT_CEDRXP,      "+CEDRXP",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEDRXP},
    {CMD_AT_CEDRXRDP,    "+CEDRXRDP",    BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams,   fRspAnalyze_CEDRXRDP},
    {CMD_AT_QNWINFO,     "+QNWINFO",     BG96_QNWINFO_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_None},
    {CMD_AT_QENG,        "+QENG",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_QENG_BG96, fRspAnalyze_None},
    {CMD_AT_QGMR,        "+QGMR",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_NoParams, fRspAnalyze_QGMR_BG96},

    /* MODEM SPECIFIC COMMANDS USED FOR SOCKET MODE */
    {CMD_AT_QIACT,       "+QIACT",       BG96_QIACT_TIMEOUT,    fCmdBuild_QIACT_BG96,   fRspAnalyze_QIACT_BG96},
    {CMD_AT_QIOPEN,      "+QIOPEN",      BG96_QIOPEN_TIMEOUT,   fCmdBuild_QIOPEN_BG96,  fRspAnalyze_QIOPEN_BG96},
    {CMD_AT_QICLOSE,     "+QICLOSE",     BG96_QICLOSE_TIMEOUT,  fCmdBuild_QICLOSE_BG96, fRspAnalyze_None},
    {CMD_AT_QISEND,      "+QISEND",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_QISEND_BG96,  fRspAnalyze_None},
    {CMD_AT_QISEND_WRITE_DATA,  "",      BG96_DEFAULT_TIMEOUT,  fCmdBuild_QISEND_WRITE_DATA_BG96, fRspAnalyze_None},
    {CMD_AT_QIRD,        "+QIRD",        BG96_DEFAULT_TIMEOUT,  fCmdBuild_QIRD_BG96,    fRspAnalyze_QIRD_BG96},
    {CMD_AT_QISTATE,     "+QISTATE",     BG96_DEFAULT_TIMEOUT,  fCmdBuild_QISTATE_BG96, fRspAnalyze_QISTATE_BG96},
    {CMD_AT_QPING,        "+QPING",      BG96_QPING_TIMEOUT,    fCmdBuild_QPING_BG96,   fRspAnalyze_QPING_BG96},

    /* MODEM SPECIFIC EVENTS */
    {CMD_AT_WAIT_EVENT,     "",          BG96_DEFAULT_TIMEOUT,        fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_BOOT_EVENT,     "",          BG96_RDY_TIMEOUT,            fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_RDY_EVENT,      "RDY",       BG96_RDY_TIMEOUT,            fCmdBuild_NoParams,   fRspAnalyze_None},
    {CMD_AT_POWERED_DOWN_EVENT, "POWERED DOWN",       BG96_RDY_TIMEOUT, fCmdBuild_NoParams,   fRspAnalyze_None},
  };
#define SIZE_ATCMD_BG96_LUT ((uint16_t) (sizeof (ATCMD_BG96_LUT) / sizeof (atcustom_LUT_t)))

  /* common init */
  bg96_modem_init(&BG96_ctxt);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  BG96_ctxt.modem_LUT_size = SIZE_ATCMD_BG96_LUT;
  BG96_ctxt.p_modem_LUT = (const atcustom_LUT_t *)ATCMD_BG96_LUT;

  /* override default termination string for AT command: <CR> */
  (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->endstr, "\r");

  /* ###########################  END CUSTOMIZATION PART  ########################### */
}

uint8_t ATCustom_BG96_checkEndOfMsgCallback(uint8_t rxChar)
{
  uint8_t last_char = 0U;

  /* static variables */
  static const uint8_t QIRD_string[] = "+QIRD";
  static uint8_t QIRD_Counter = 0U;

  /*---------------------------------------------------------------------------------------*/
  if (BG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_INIT_CR)
  {
    /* waiting for first valid <CR>, char received before are considered as trash */
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (BG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_CR)
  {
    if ((AT_CHAR_t)('\r') == rxChar)
    {
      /* current     : xxxxx<CR>
      *  command format : <CR><LF>xxxxxxxx<CR><LF>
      *  waiting for : <LF>
      */
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (BG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_LF)
  {
    /* waiting for <LF> */
    if ((AT_CHAR_t)('\n') == rxChar)
    {
      /*  current        : <CR><LF>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : x or <CR>
      */
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_FIRST_CHAR;
      last_char = 1U;
      QIRD_Counter = 0U;
    }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (BG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_FIRST_CHAR)
  {
    if (BG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_waiting_header)
    {
      /* Socket Data RX - waiting for Header: we are waiting for +QIRD
      *
      * +QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF><CR><LF>Serve...
      *    ^- waiting this string
      */
      if (rxChar == QIRD_string[QIRD_Counter])
      {
        QIRD_Counter++;
        if (QIRD_Counter == (uint8_t) strlen((const CRC_CHAR_t *)QIRD_string))
        {
          /* +QIRD detected, next step */
          socketHeaderRX_reset();
          BG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_header;
        }
      }
    }

    /* NOTE:
    * if we are in socket_RxData_state = SocketRxDataState_waiting_header, we are waiting for +QIRD (test above)
    * but if we receive another message, we need to evacuate it without modifying socket_RxData_state
    * That's why we are nt using "else if" here, if <CR> if received before +QIND, it means that we have received
    * something else
    */
    if (BG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_header)
    {
      /* Socket Data RX - Header received: we are waiting for second <CR>
      *
      * +QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF><CR><LF>Serve...
      *         ^- retrieving this size
      *             ^- waiting this <CR>
      */
      if ((AT_CHAR_t)('\r') == rxChar)
      {
        /* second <CR> detected, we have received data header
        *  now waiting for <LF>, then start to receive socket datas
        *  Verify that size received in header is the expected one
        */
        uint16_t size_from_header = SocketHeaderRX_getSize();
        if (BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size != size_from_header)
        {
          /* update buffer size received - should not happen */
          BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size = size_from_header;
        }
        BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
        BG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_receiving_data;
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
    else if (BG96_ctxt.socket_ctxt.socket_RxData_state == SocketRxDataState_receiving_data)
    {
      /* receiving socket data: do not analyze char, just count expected size
      *
      * +QIRD: 522<CR><LF>HTTP/1.1 200 OK<CR><LF><CR><LF>Date: Wed, 21 Feb 2018 14:56:54 GMT<CR><LF><CR><LF>Serve...
      *.                  ^- start to read data: count char
      */
      BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_SOCKET_DATA;
      /* check if full buffer has been received */
      if (BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received == BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size)
      {
        BG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
        BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
      }
    }
    /* waiting for <CR> or x */
    else if ((AT_CHAR_t)('\r') == rxChar)
    {
      /*   current        : <CR>
      *   command format : <CR><LF>xxxxxxxx<CR><LF>
      *   waiting for    : <LF>
      */
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_LF;
    }
    else {/* nothing to do */ }
  }
  /*---------------------------------------------------------------------------------------*/
  else if (BG96_ctxt.state_SyntaxAutomaton == WAITING_FOR_SOCKET_DATA)
  {
    BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received++;
    /* check if full buffer has been received */
    if (BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received == BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size)
    {
      BG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_data_received;
      BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_CR;
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
    /* BG96 special cases
    *
    *  SOCKET MODE: when sending DATA using AT+QISEND, we are waiting for socket prompt "<CR><LF>> "
    *               before to send DATA. Then we should receive "OK<CR><LF>".
    */

    if (BG96_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
    {
      switch (BG96_ctxt.socket_ctxt.socket_send_state)
      {
        case SocketSendState_WaitingPrompt1st_greaterthan:
        {
          /* detecting socket prompt first char: "greater than" */
          if ((AT_CHAR_t)('>') == rxChar)
          {
            BG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt2nd_space;
          }
          break;
        }

        case SocketSendState_WaitingPrompt2nd_space:
        {
          /* detecting socket prompt second char: "space" */
          if ((AT_CHAR_t)(' ') == rxChar)
          {
            BG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_Prompt_Received;
            last_char = 1U;
          }
          else
          {
            /* if char iommediatly after "greater than" is not a "space", reinit state */
            BG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
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

at_status_t ATCustom_BG96_getCmd(at_context_t *p_at_ctxt, uint32_t *p_ATcmdTimeout)
{
  /* static variables */
  /* memorize number of SIM status query retries (static) */

  /* local variables */
  at_status_t retval = ATSTATUS_OK;
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_msg_t curSID = p_atp_ctxt->current_SID;

  PRINT_API("enter ATCustom_BG96_getCmd() for SID %d", curSID)

  /* retrieve parameters from SID command (will update SID_ctxt) */
  if (atcm_retrieve_SID_parameters(&BG96_ctxt, p_atp_ctxt) != ATSTATUS_OK)
  {
    retval = ATSTATUS_ERROR;
    goto exit_ATCustom_BG96_getCmd;
  }

  /* new command: reset command context */
  atcm_reset_CMD_context(&BG96_ctxt.CMD_ctxt);

  /* For each SID, the sequence of AT commands to send id defined (it can be dynamic)
  * Determine and prepare the next command to send for this SID
  */

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  if (curSID == (at_msg_t) SID_CS_CHECK_CNX)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT, FINAL_CMD);
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
    uint8_t common_start_sequence_step = BG96_MODEM_SYNCHRO_AT_MAX_RETRIES + 1U;
    /* POWER_ON and RESET are almost the same, specific differences are managed case by case */
    /* for reset, only HW reset is supported */
    if ((curSID == (at_msg_t) SID_CS_RESET) &&
        (BG96_ctxt.SID_ctxt.reset_type != CS_RESET_HW))
    {
      PRINT_ERR("Reset type (%d) not supported", BG96_ctxt.SID_ctxt.reset_type)
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
        reset_variables_bg96();

        /* reinit modem at ready status */
        BG96_ctxt.persist.modem_at_ready = AT_FALSE;

        /* in case of RESET, reset all the contexts to start from a fresh state */
        if (curSID == (at_msg_t) SID_CS_RESET)
        {
          bg96_modem_reset(&BG96_ctxt);
        }

        /* force requested flow control */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
      }
      else if ((p_atp_ctxt->step >= 1U) && (p_atp_ctxt->step < BG96_MODEM_SYNCHRO_AT_MAX_RETRIES))
      {
        /* start a loop to wait for modem : send AT commands */
        if (BG96_ctxt.persist.modem_at_ready == AT_FALSE)
        {
          /* use optional as we are not sure to receive a response from the modem: this allows to avoid to return
            * an error to upper layer
            */
          PRINT_DBG("test connection [try number %d] ", p_atp_ctxt->step)
          atcm_program_AT_CMD_ANSWER_OPTIONAL(&BG96_ctxt, p_atp_ctxt,
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
      else if (p_atp_ctxt->step == BG96_MODEM_SYNCHRO_AT_MAX_RETRIES)
      {
        /* if we fall here and the modem is not ready, we have a communication problem */
        if (BG96_ctxt.persist.modem_at_ready == AT_FALSE)
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
        /* force flow control */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_IFC, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 1U))
      {
        /* disable echo */
        BG96_ctxt.CMD_ctxt.command_echo = AT_FALSE;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 2U))
      {
        /* request detailled error report */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CMEE, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 3U))
      {
        /* enable full response format */
        BG96_ctxt.CMD_ctxt.dce_full_resp_format = AT_TRUE;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATV, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 4U))
      {
        /* deactivate DTR */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_AND_D, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 5U))
      {
        /* Read baud rate settings */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_IPR, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 6U))
      {
        /* power on with AT+CFUN=0 */
        BG96_ctxt.CMD_ctxt.cfun_value = 0U;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
      }
      /* ----- start specific power ON sequence here ----
        * BG96_AT_Commands_Manual_V2.0
        */
      /* Check bands paramaters */
      else if (p_atp_ctxt->step == (common_start_sequence_step + 7U))
      {
        bg96_shared.QCFG_command_write = AT_FALSE;
        bg96_shared.QCFG_command_param = QCFG_band;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 8U))
      {
        bg96_shared.QCFG_command_write = AT_FALSE;
        bg96_shared.QCFG_command_param = QCFG_iotopmode;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 9U))
      {
        bg96_shared.QCFG_command_write = AT_FALSE;
        bg96_shared.QCFG_command_param = QCFG_nwscanseq;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, INTERMEDIATE_CMD);
      }
      else if (p_atp_ctxt->step == (common_start_sequence_step + 10U))
      {
        bg96_shared.QCFG_command_write = AT_FALSE;
        bg96_shared.QCFG_command_param = QCFG_nwscanmode;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QCFG, FINAL_CMD);
      }
      else if (p_atp_ctxt->step >= (common_start_sequence_step + 11U))
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QPOWD, FINAL_CMD);
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
      /* CFUN parameters here are coming from user settings in CS_init_modem() */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CFUN, INTERMEDIATE_CMD);
      bg96_shared.bg96_sim_status_retries = 0U;
      bg96_shared.QINISTAT_error = AT_FALSE;
    }
    else if (p_atp_ctxt->step == 1U)
    {
      if (BG96_ctxt.SID_ctxt.modem_init.init == CS_CMI_MINI)
      {
        /* minimum functionnality selected, no SIM access => leave now */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QCCID, INTERMEDIATE_CMD);
      }

      /* reset SIM ready status, we will check it later */
      BG96_ctxt.persist.modem_sim_ready  = AT_FALSE;
    }
    else if (p_atp_ctxt->step == 2U)
    {
      if (bg96_shared.bg96_sim_status_retries > BG96_MAX_SIM_STATUS_RETRIES)
      {
        if (bg96_shared.QINISTAT_error == AT_FALSE)
        {
          /* error, max sim status retries reached */
          atcm_program_NO_MORE_CMD(p_atp_ctxt);
          retval = ATSTATUS_ERROR;
        }
        else
        {
          /* special case: AT+QINISTAT reported an error because this cmd is not
            * supported by the modem FW version. Wait for maximum retries then
            * continue sequance
            */
          PRINT_INFO("warning, modem FW version certainly too old")
          BG96_ctxt.persist.modem_sim_ready  = AT_TRUE; /* assume that SIM is ready now */
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMR, INTERMEDIATE_CMD);
        }
      }
      else
      {
        /* if QINISTAT indicates that SIM is not ready, may be it's because there is a PIN code
          *  => check PIN code status regularly
          */
        if ((bg96_shared.bg96_sim_status_retries != 0U) &&
            ((bg96_shared.bg96_sim_status_retries % 3U) == 0U))
        {
          /* check PIN code status */
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
        }
        else
        {
          /* request SIM status */
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt,
                              ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QINISTAT, INTERMEDIATE_CMD);
        }
      }
    }
    else if (p_atp_ctxt->step == 3U)
    {
      if (BG96_ctxt.persist.modem_sim_ready == AT_FALSE)
      {
        if (BG96_ctxt.persist.sim_state == CS_SIMSTATE_SIM_PIN_REQUIRED)
        {
          /* SIM is ready but need PIN code, go to next step */
          atcm_program_SKIP_CMD(p_atp_ctxt);
          PRINT_INFO("SIM is ready, unlock sequence")
        }
        else
        {
          /* SIM not ready yet, wait before retry */
          atcm_program_TEMPO(p_atp_ctxt, BG96_SIMREADY_TIMEOUT, INTERMEDIATE_CMD);
          /* go back to previous step */
          p_atp_ctxt->step = p_atp_ctxt->step - 2U;
          PRINT_INFO("SIM not ready yet")
          bg96_shared.bg96_sim_status_retries++;
        }
      }
      else
      {
        /* SIM is ready, go to next step*/
        atcm_program_SKIP_CMD(p_atp_ctxt);
        PRINT_INFO("SIM is ready, unlock sequence")
      }
    }
    else if (p_atp_ctxt->step == 4U)
    {
      /* reset bg96_sim_status_retries */
      bg96_shared.bg96_sim_status_retries = 0U;
      /* check is CPIN is requested */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 5U)
    {
      if (BG96_ctxt.persist.sim_pin_code_ready == AT_FALSE)
      {
        if (strlen((const CRC_CHAR_t *)&BG96_ctxt.SID_ctxt.modem_init.pincode.pincode) != 0U)
        {
          /* send PIN value */
          PRINT_INFO("CPIN required, we send user value to modem")
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CPIN, INTERMEDIATE_CMD);
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
    else if (p_atp_ctxt->step == 6U)
    {
      /* check PDP context parameters */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGDCONT, FINAL_CMD);
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
      switch (BG96_ctxt.SID_ctxt.device_info->field_requested)
      {
        case CS_DIF_MANUF_NAME_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMI, FINAL_CMD);
          break;

        case CS_DIF_MODEL_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGMM, FINAL_CMD);
          break;

        case CS_DIF_REV_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QGMR, FINAL_CMD);
          break;

        case CS_DIF_SN_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CGSN, FINAL_CMD);
          break;

        case CS_DIF_IMEI_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_GSN, FINAL_CMD);
          break;

        case CS_DIF_IMSI_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CIMI, FINAL_CMD);
          break;

        case CS_DIF_PHONE_NBR_PRESENT:
          /* not AT+CNUM not supported by BG96 */
          retval = ATSTATUS_ERROR;
          break;

        case CS_DIF_ICCID_PRESENT:
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QCCID, FINAL_CMD);
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
      /* get signal strength */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_CSQ, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* get signal strength */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QCSQ, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
#if (BG96_OPTION_NETWORK_INFO == 1)
      /* NB: cmd answer is optionnal ie no error will be raised if no answer received from modem
        *  indeed, if requested here, it's just a bonus and should not generate an error
        */
      atcm_program_AT_CMD_ANSWER_OPTIONAL(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_AT_QNWINFO,
                                          INTERMEDIATE_CMD);
#else
      atcm_program_SKIP_CMD(p_atp_ctxt);
#endif /* BG96_OPTION_NETWORK_INFO */
    }
    else if (p_atp_ctxt->step == 3U)
    {
#if (BG96_OPTION_ENGINEERING_MODE == 1)
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QENG, FINAL_CMD);
#else
      atcm_program_NO_MORE_CMD(p_atp_ctxt);
#endif /* BG96_OPTION_ENGINEERING_MODE */
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
#if 0
      /* check if actual registration status is the expected one */
      CS_OperatorSelector_t *operatorSelect = &(BG96_ctxt.SID_ctxt.write_operator_infos);
      if (BG96_ctxt.SID_ctxt.read_operator_infos.mode != operatorSelect->mode)
      {
        /* write registration status */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
      }
      else
      {
        /* skip this step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
#else
      /* due to problem observed on simu: does not register after reboot */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_COPS, INTERMEDIATE_CMD);
#endif /* 0 */
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CEREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 4U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGREG, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CEREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_CGREG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 3U)
    {
      /* read registration status */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_COPS, FINAL_CMD);
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
      CS_UrcEvent_t urcEvent = BG96_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CREG, CGREG or CEREG ? */
      if ((urcEvent == CS_URCEVENT_EPS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_EPS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_GPRS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_GPRS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_CS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_CS_LOCATION_INFO))
      {
        (void) atcm_subscribe_net_event(&BG96_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* if signal quality URC not yet suscbribe */
        if (BG96_ctxt.persist.urc_subscript_signalQuality == CELLULAR_FALSE)
        {
          /* set event as suscribed */
          BG96_ctxt.persist.urc_subscript_signalQuality = CELLULAR_TRUE;

          /* request the URC we want */
          bg96_shared.QINDCFG_command_param = QINDCFG_csq;
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QINDCFG, FINAL_CMD);
        }
        else
        {
          atcm_program_NO_MORE_CMD(p_atp_ctxt);
        }
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
      CS_UrcEvent_t urcEvent = BG96_ctxt.SID_ctxt.urcEvent;

      /* is an event linked to CREG, CGREG or CEREG ? */
      if ((urcEvent == CS_URCEVENT_EPS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_EPS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_GPRS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_GPRS_LOCATION_INFO) ||
          (urcEvent == CS_URCEVENT_CS_NETWORK_REG_STAT) || (urcEvent == CS_URCEVENT_CS_LOCATION_INFO))
      {
        (void) atcm_unsubscribe_net_event(&BG96_ctxt, p_atp_ctxt);
      }
      else if (urcEvent == CS_URCEVENT_SIGNAL_QUALITY)
      {
        /* if signal quality URC suscbribed */
        if (BG96_ctxt.persist.urc_subscript_signalQuality == CELLULAR_TRUE)
        {
          /* set event as unsuscribed */
          BG96_ctxt.persist.urc_subscript_signalQuality = CELLULAR_FALSE;

          /* request the URC we don't want */
          bg96_shared.QINDCFG_command_param = QINDCFG_csq;
          atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QINDCFG, FINAL_CMD);
        }
        else
        {
          atcm_program_NO_MORE_CMD(p_atp_ctxt);
        }
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
      if (BG96_ctxt.persist.urc_subscript_pdn_event == CELLULAR_FALSE)
      {
        /* set event as suscribed */
        BG96_ctxt.persist.urc_subscript_pdn_event = CELLULAR_TRUE;

        /* request PDN events */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
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
      if (BG96_ctxt.persist.urc_subscript_pdn_event == CELLULAR_TRUE)
      {
        /* set event as unsuscribed */
        BG96_ctxt.persist.urc_subscript_pdn_event = CELLULAR_FALSE;

        /* request PDN events */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGEREP, FINAL_CMD);
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
      BG96_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_ATTACHED;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
      BG96_ctxt.CMD_ctxt.cgatt_write_cmd_param = CGATT_DETACHED;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGATT, FINAL_CMD);
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
      bg96_shared.pdn_already_active = AT_FALSE;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* PDN activation */
      if (bg96_shared.pdn_already_active == AT_TRUE)
      {
        /* PDN already active - exit */
        PRINT_INFO("Skip PDN activation (already active)")
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        /* request PDN activation */
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIACT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* ckeck PDN state */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, FINAL_CMD);
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
      /* get IP address */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATX, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATD, FINAL_CMD);
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
      bg96_shared.QICGSP_config_command = AT_TRUE;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICSGP, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      bg96_shared.QICGSP_config_command = AT_FALSE;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICSGP, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGDCONT, FINAL_CMD);
      /* add AT+CGAUTH for username and password if required */
      /* could also use AT+QICSGP */
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
    /* get IP address */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
    /* SOCKET MODE */
    atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_READ_CMD, (CMD_ID_t) CMD_AT_QIACT, FINAL_CMD);
#else
    /* DATA MODE*/
    atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_CGPADDR, FINAL_CMD);
#endif /* USE_SOCKETS_TYPE */
  }
  else if (curSID == (at_msg_t) SID_CS_DIAL_COMMAND)
  {
    /* SOCKET CONNECTION FOR COMMAND DATA MODE */
    if (p_atp_ctxt->step == 0U)
    {
      /* reserve a modem CID for this socket_handle */
      bg96_shared.QIOPEN_current_socket_connected = 0U;
      socket_handle_t sockHandle = BG96_ctxt.socket_ctxt.socket_info->socket_handle;
      (void) atcm_socket_reserve_modem_cid(&BG96_ctxt, sockHandle);
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIOPEN, INTERMEDIATE_CMD);
      PRINT_INFO("For Client Socket Handle=%ld : MODEM CID affected=%d",
                 sockHandle,
                 BG96_ctxt.persist.socket[sockHandle].socket_connId_value)

    }
    else if (p_atp_ctxt->step == 1U)
    {
      if (bg96_shared.QIOPEN_current_socket_connected == 0U)
      {
        /* Waiting for +QIOPEN urc indicating that socket is open */
        atcm_program_TEMPO(p_atp_ctxt, BG96_QIOPEN_TIMEOUT, INTERMEDIATE_CMD);
      }
      else
      {
        /* socket opened */
        bg96_shared.QIOPEN_waiting = AT_FALSE;
        /* socket is connected */
        (void) atcm_socket_set_connected(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else  if (p_atp_ctxt->step == 2U)
    {
      if (bg96_shared.QIOPEN_current_socket_connected == 0U)
      {
        /* QIOPEN NOT RECEIVED,
          *  cf BG96 TCP/IP AT Commands Manual V1.0, paragraph 2.1.4 - 3/
          *  "if the URC cannot be received within 150 seconds, AT+QICLOSE should be used to close
          *   the socket".
          *
          *  then we will have to return an error to cellular service !!! (see next step)
          */
        bg96_shared.QIOPEN_waiting = AT_FALSE;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICLOSE, INTERMEDIATE_CMD);
      }
      else
      {
        /* socket opened */
        bg96_shared.QIOPEN_waiting = AT_FALSE;
        /* socket is connected */
        (void) atcm_socket_set_connected(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else  if (p_atp_ctxt->step == 3U)
    {
      /* if we fall here, it means we have send CMD_AT_QICLOSE on previous step
        *  now inform cellular service that opening has failed => return an error
        */
      /* release the modem CID for this socket_handle */
      (void) atcm_socket_release_modem_cid(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle);
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
      if (BG96_ctxt.SID_ctxt.socketSendData_struct.buffer_size > MODEM_MAX_SOCKET_TX_DATA_SIZE)
      {
        PRINT_ERR("Data size to send %ld exceed maximum size %ld",
                  BG96_ctxt.SID_ctxt.socketSendData_struct.buffer_size,
                  MODEM_MAX_SOCKET_TX_DATA_SIZE)
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
        retval = ATSTATUS_ERROR;
      }
      else
      {
        BG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_WaitingPrompt1st_greaterthan;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QISEND, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* waiting for socket prompt: "<CR><LF>> " */
      if (BG96_ctxt.socket_ctxt.socket_send_state == SocketSendState_Prompt_Received)
      {
        PRINT_DBG("SOCKET PROMPT ALREADY RECEIVED")
        /* go to next step */
        atcm_program_SKIP_CMD(p_atp_ctxt);
      }
      else
      {
        PRINT_DBG("WAITING FOR SOCKET PROMPT")
        atcm_program_WAIT_EVENT(p_atp_ctxt, BG96_SOCKET_PROMPT_TIMEOUT, INTERMEDIATE_CMD);
      }
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* socket prompt received, send DATA */
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_QISEND_WRITE_DATA, FINAL_CMD);

      /* reinit automaton to receive answer */
      reinitSyntaxAutomaton_bg96();
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
      BG96_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestSize;
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIRD, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* check that data size to receive is not null */
      if (BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size != 0U)
      {
        /* check that data size to receive does not exceed maximum size
          *  if it's the case, only request maximum size we can receive
          */
        if (BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size >
            BG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
        {
          PRINT_INFO("Data size available (%ld) exceed buffer maximum size (%ld)",
                     BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size,
                     BG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size)
          BG96_ctxt.socket_ctxt.socket_rx_expected_buf_size = BG96_ctxt.socket_ctxt.socketReceivedata.max_buffer_size;
        }

        /* receive datas */
        BG96_ctxt.socket_ctxt.socket_receive_state = SocketRcvState_RequestData_Header;
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIRD, FINAL_CMD);
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
        * due to BG96 socket connection mechanism (waiting URC QIOPEN), we can fall here but socket
        * has been already closed if error occurs during connection
        */
      if (atcm_socket_is_connected(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle) == AT_TRUE)
      {
        atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QICLOSE, INTERMEDIATE_CMD);
      }
      else
      {
        /* release the modem CID for this socket_handle */
        (void) atcm_socket_release_modem_cid(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle);
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* release the modem CID for this socket_handle */
      (void) atcm_socket_release_modem_cid(&BG96_ctxt, BG96_ctxt.socket_ctxt.socket_info->socket_handle);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_ESC_CMD, FINAL_CMD);
      reinitSyntaxAutomaton_bg96();
    }
    else
    {
      /* error, invalid step */
      retval = ATSTATUS_ERROR;
    }
  }
  else if (curSID == (at_msg_t) SID_CS_INIT_POWER_CONFIG)
  {
    BG96_ctxt.persist.psm_requested = AT_FALSE;
    /* TO BE CONFIRMED
      * BG96 switch directly to low power mode when sending PSM/EDRX commands
      * in this case, we should not send theses commands now
      *
      */

    /* Low Power modes not implemented yet for BG96 */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_SET_POWER_CONFIG)
  {
    /* TO BE CONFIRMED
      * BG96 switch directly to low power mode when sending PSM/EDRX commands
      * in this case, we should not send theses commands now
      *
      */

    /* Low Power modes not implemented yet for BG96 */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_SLEEP_COMPLETE)
  {
    /* Low Power modes not implemented yet for BG96 */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_SLEEP_CANCEL)
  {
    /* Low Power modes not implemented yet for BG96 */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_WAKEUP)
  {
    /* Low Power modes not implemented yet for BG96 */
    retval = ATSTATUS_ERROR;
  }
  else if (curSID == (at_msg_t) SID_CS_SOCKET_CNX_STATUS)
  {
    if (p_atp_ctxt->step == 0U)
    {
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QISTATE, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATX, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_EXECUTION_CMD, (CMD_ID_t) CMD_ATO, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIDNSCFG, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 1U)
    {
      /* initialize +QIURC "dnsgip" parameters */
      init_bg96_qiurc_dnsgip();
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QIDNSGIP, INTERMEDIATE_CMD);
    }
    else if (p_atp_ctxt->step == 2U)
    {
      /* do we have received a valid DNS response ? */
      if ((bg96_shared.QIURC_dnsgip_param.finished == AT_TRUE) && (bg96_shared.QIURC_dnsgip_param.error == 0U))
      {
        /* yes */
        atcm_program_NO_MORE_CMD(p_atp_ctxt);
      }
      else
      {
        /* not yet, waiting for DNS informations */
        atcm_program_TEMPO(p_atp_ctxt, BG96_QIDNSGIP_TIMEOUT, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_WRITE_CMD, (CMD_ID_t) CMD_AT_QPING, FINAL_CMD);
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
      atcm_program_AT_CMD(&BG96_ctxt, p_atp_ctxt, ATTYPE_RAW_CMD, (CMD_ID_t) CMD_AT_DIRECT_CMD, FINAL_CMD);
      atcm_program_CMD_TIMEOUT(&BG96_ctxt, p_atp_ctxt, BG96_ctxt.SID_ctxt.direct_cmd_tx->cmd_timeout);
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
      if (atcm_select_hw_simslot(BG96_ctxt.persist.sim_selected) != ATSTATUS_OK)
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
    retval = atcm_modem_build_cmd(&BG96_ctxt, p_atp_ctxt, p_ATcmdTimeout);
  }

exit_ATCustom_BG96_getCmd:
  return (retval);
}

at_endmsg_t ATCustom_BG96_extractElement(atparser_context_t *p_atp_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos)
{
  at_endmsg_t retval_msg_end_detected = ATENDMSG_NO;
  bool exit_loop;
  uint16_t idx;
  uint16_t start_idx;
  uint16_t *p_parseIndex = &(element_infos->current_parse_idx);

  PRINT_API("enter ATCustom_BG96_extractElement()")
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
      if ((p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_QIRD) &&
          (BG96_ctxt.socket_ctxt.socket_receive_state == SocketRcvState_RequestData_Payload) &&
          (BG96_ctxt.socket_ctxt.socket_RxData_state != SocketRxDataState_finished))
      {
        PRINT_DBG("receiving socket data (real size=%d)", SocketHeaderRX_getSize())
        element_infos->str_start_idx = 0U;
        element_infos->str_end_idx = (uint16_t) BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        element_infos->str_size = (uint16_t) BG96_ctxt.socket_ctxt.socket_rx_count_bytes_received;
        BG96_ctxt.socket_ctxt.socket_RxData_state = SocketRxDataState_finished;
        retval_msg_end_detected = ATENDMSG_YES;
        exit_loop = true;
      }
    }

    /* check if end of message has been detected */
    if (retval_msg_end_detected == ATENDMSG_YES)
    {
      goto exit_ATCustom_BG96_extractElement;
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
    goto exit_ATCustom_BG96_extractElement;
  }

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
    }
  } while (exit_loop == false);

  /* increase parameter rank */
  element_infos->param_rank = (element_infos->param_rank + 1U);

exit_ATCustom_BG96_extractElement:
  return (retval_msg_end_detected);
}

at_action_rsp_t ATCustom_BG96_analyzeCmd(at_context_t *p_at_ctxt,
                                         const IPC_RxMessage_t *p_msg_in,
                                         at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval;

  PRINT_API("enter ATCustom_BG96_analyzeCmd()")

  /* Analyze data received from the modem and
    * search in LUT the ID corresponding to command received
  */
  if (ATSTATUS_OK != atcm_searchCmdInLUT(&BG96_ctxt, p_atp_ctxt, p_msg_in, element_infos))
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
    retval = atcm_check_text_line_cmd(&BG96_ctxt, p_at_ctxt, p_msg_in, element_infos);

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
          if (fRspAnalyze_QIRD_data_BG96(p_at_ctxt, &BG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        case CMD_AT_QISTATE:
          if (fRspAnalyze_QISTATE_BG96(p_at_ctxt, &BG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        case CMD_AT_QGMR:
          if (fRspAnalyze_QGMR_BG96(p_at_ctxt, &BG96_ctxt, p_msg_in, element_infos) != ATACTION_RSP_ERROR)
          {
            /* received a valid intermediate answer */
            retval = ATACTION_RSP_INTERMEDIATE;
          }
          break;

        /* ###########################  END CUSTOMIZED PART  ########################### */
        default:
          /* this is not one of modem common command, need to check if this is an answer to a modem's specific cmd */
          PRINT_DBG("receive an un-expected line... is it a text line ?")
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
      {
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
            BG96_ctxt.persist.modem_at_ready = AT_TRUE;
          }
          if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_ATE)
          {
            PRINT_DBG("Echo successfully disabled")
          }
        }
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_PING_IP_ADDRESS)
        {
          /* PING requests for BG96 are asynchronous.
          * If the PING request is valid (no other ongoing ping request for example), the
          * modem will answer OK.
          * At this point, initialize the ping response structure.
          */
          PRINT_DBG("this is a valid PING request")
          atcm_validate_ping_request(&BG96_ctxt);
        }
        retval = ATACTION_RSP_FRC_END;
        break;
      }

      case CMD_AT_NO_CARRIER:
      case CMD_AT_NO_ANSWER:
      {
        retval = ATACTION_RSP_ERROR;
        break;
      }

      case CMD_AT_RING:
      case CMD_AT_NO_DIALTONE:
      case CMD_AT_BUSY:
      {
        /* VALUES NOT MANAGED IN CURRENT IMPLEMENTATION BECAUSE NOT EXPECTED */
        retval = ATACTION_RSP_ERROR;
        break;
      }

      case CMD_AT_CONNECT:
      {
        PRINT_INFO("MODEM SWITCHES TO DATA MODE")
        retval = (at_action_rsp_t)(ATACTION_RSP_FLAG_DATA_MODE | ATACTION_RSP_FRC_END);
        break;
      }

      /* ###########################  START CUSTOMIZATION PART  ########################### */
      case CMD_AT_CEREG:
      case CMD_AT_CREG:
      case CMD_AT_CGREG:
      {
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
      }

      case CMD_AT_RDY_EVENT:
      {
        /* We received RDY event from the modem.
        * If received during Power ON or RESET, it is indicating that the modem is ready.
        * If received in another state, we report to upper layer a modem reboot event.
        */
        BG96_ctxt.persist.modem_at_ready = AT_TRUE;
        if ((p_atp_ctxt->current_SID == (at_msg_t) SID_CS_POWER_ON) ||
            (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_RESET))
        {
          /* ignore the RDY event during POWER ON or RESET */
          retval = ATACTION_RSP_URC_IGNORED;
        }
        else
        {
          /* if event is received in other states, it's an unexpected modem reboot */
          if (atcm_modem_event_received(&BG96_ctxt, CS_MDMEVENT_BOOT) == AT_TRUE)
          {
            retval = ATACTION_RSP_URC_FORWARDED;
          }
          else
          {
            retval = ATACTION_RSP_URC_IGNORED;
          }
        }
        break;
      }

      case CMD_AT_POWERED_DOWN_EVENT:
      {
        PRINT_DBG("MODEM POWERED DOWN EVENT DETECTED")
        if (atcm_modem_event_received(&BG96_ctxt, CS_MDMEVENT_POWER_DOWN) == AT_TRUE)
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
      {
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
      }

      case CMD_AT_SEND_OK:
      {
        if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SEND_DATA)
        {
          retval = ATACTION_RSP_FRC_END;
        }
        else
        {
          retval = ATACTION_RSP_ERROR;
        }
        break;
      }

      case CMD_AT_SEND_FAIL:
      {
        retval = ATACTION_RSP_ERROR;
        break;
      }

      case CMD_AT_QIURC:
      {
        /* retval will be override in analyze of +QUIRC content
        *  indeed, QIURC can be considered as an URC or a normal msg (for DNS request)
        */
        retval = ATACTION_RSP_INTERMEDIATE;
        break;
      }

      case CMD_AT_QIOPEN:
        /* now waiting for an URC  */
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_QIND:
        retval = ATACTION_RSP_URC_FORWARDED;
        break;

      case CMD_AT_CFUN:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_CPIN:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_QCFG:
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_QUSIM:
        retval = ATACTION_RSP_URC_IGNORED;
        break;

      case CMD_AT_CPSMS:
        retval = ATACTION_RSP_INTERMEDIATE;
        break;

      case CMD_AT_CEDRXS:
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
        /* ERROR does not contains parameters, so call the analyze function explicity
        *  otherwise it will not ne called
        */
        retval = fRspAnalyze_Error_BG96(p_at_ctxt, &BG96_ctxt, p_msg_in, element_infos);
        break;

      case CMD_AT_CME_ERROR:
      case CMD_AT_CMS_ERROR:
        /* do the analyze here because will not be called by parser if the command
        * has no parameters (+CME ERROR only without parameters)
        */
        retval = fRspAnalyze_Error_BG96(p_at_ctxt, &BG96_ctxt, p_msg_in, element_infos);
        break;

      default:
      {
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
  }
  return (retval);
}

at_action_rsp_t ATCustom_BG96_analyzeParam(at_context_t *p_at_ctxt,
                                           const IPC_RxMessage_t *p_msg_in,
                                           at_element_info_t *element_infos)
{
  at_action_rsp_t retval;
  PRINT_API("enter ATCustom_BG96_analyzeParam()")

  /* analyse parameters of the command we received:
  * call the corresponding function from the LUT
  */
  retval = (atcm_get_CmdAnalyzeFunc(&BG96_ctxt, element_infos->cmd_id_received))(p_at_ctxt,
                                                                                 &BG96_ctxt,
                                                                                 p_msg_in,
                                                                                 element_infos);

  return (retval);
}

/* function called to finalize an AT command */
at_action_rsp_t ATCustom_BG96_terminateCmd(atparser_context_t *p_atp_ctxt, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter ATCustom_BG96_terminateCmd()")

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* additional tests */
  if (BG96_ctxt.socket_ctxt.socket_send_state != SocketSendState_No_Activity)
  {
    /* special case for SID_CS_SEND_DATA
    * indeed, this function is called when an AT cmd is finished
    * but for AT+QISEND, it is called a 1st time when prompt is received
    * and a second time when data have been sent.
    */
    if (p_atp_ctxt->current_SID != (at_msg_t) SID_CS_SEND_DATA)
    {
      /* reset socket_send_state */
      BG96_ctxt.socket_ctxt.socket_send_state = SocketSendState_No_Activity;
    }
  }

  if ((p_atp_ctxt->current_atcmd.id == (at_msg_t) CMD_ATD) ||
      (p_atp_ctxt->current_atcmd.id == (at_msg_t) CMD_ATO) ||
      (p_atp_ctxt->current_atcmd.id == (at_msg_t) CMD_AT_CGDATA))
  {
    if (element_infos->cmd_id_received == (at_msg_t) CMD_AT_CONNECT)
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
at_status_t ATCustom_BG96_get_rsp(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_BG96_get_rsp()")

  /* prepare response for a SID - common part */
  retval = atcm_modem_get_rsp(&BG96_ctxt, p_atp_ctxt, p_rsp_buf);

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
                               (uint16_t) sizeof(bg96_shared.QIURC_dnsgip_param.hostIPaddr),
                               (void *)bg96_shared.QIURC_dnsgip_param.hostIPaddr) != DATAPACK_OK)
      {
        retval = ATSTATUS_OK;
      }
      break;

    case SID_CS_POWER_ON:
    case SID_CS_RESET:
      display_user_friendly_mode_and_bands_config();
      break;

    case SID_CS_POWER_OFF:
      /* reinit context for power off case */
      bg96_modem_reset(&BG96_ctxt);
      break;

#if (BG96_ACTIVATE_PING_REPORT == 1)
    case SID_CS_PING_IP_ADDRESS:
    {
      /* SID_CS_PING_IP_ADDRESS is waiting for a ping structure
       * For this modem, PING will be received later (as URC).
       * Just indicate that no ping report is available for now.
       */
      PRINT_DBG("Ping no report available yet - use PING_INVALID_INDEX")
      (void) memset((void *)&BG96_ctxt.persist.ping_resp_urc, 0, sizeof(CS_Ping_response_t));
      BG96_ctxt.persist.ping_resp_urc.index = PING_INVALID_INDEX;
      if (DATAPACK_writeStruct(p_rsp_buf,
                               (uint16_t) CSMT_URC_PING_RSP,
                               (uint16_t) sizeof(CS_Ping_response_t),
                               (void *)&BG96_ctxt.persist.ping_resp_urc) != DATAPACK_OK)
      {
        retval = ATSTATUS_OK;
      }
      break;
    }
#endif /* (BG96_ACTIVATE_PING_REPORT == 1) */

    default:
      break;
  }
  /* ###########################  END CUSTOMIZATION PART  ########################### */

  /* reset SID context */
  atcm_reset_SID_context(&BG96_ctxt.SID_ctxt);

  /* reset socket context */
  atcm_reset_SOCKET_context(&BG96_ctxt);

  return (retval);
}

at_status_t ATCustom_BG96_get_urc(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_BG96_get_urc()")

  /* prepare response for an URC - common part */
  retval = atcm_modem_get_urc(&BG96_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /* prepare response for an URC
  *  all specific behaviors for an URC have to be implemented here
  */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_BG96_get_error(atparser_context_t *p_atp_ctxt, at_buf_t *p_rsp_buf)
{
  at_status_t retval;
  PRINT_API("enter ATCustom_BG96_get_error()")

  /* prepare response when an error occured - common part */
  retval = atcm_modem_get_error(&BG96_ctxt, p_atp_ctxt, p_rsp_buf);

  /* ###########################  START CUSTOMIZATION PART  ########################### */
  /*  prepare response when an error occured
  *  all specific behaviors for an error have to be implemented here
  */

  /* ###########################  END CUSTOMIZATION PART  ########################### */

  return (retval);
}

at_status_t ATCustom_BG96_hw_event(sysctrl_device_type_t deviceType, at_hw_event_t hwEvent, GPIO_PinState gstate)
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

/* BG96 modem init fonction
*  call common init function and then do actions specific to this modem
*/
static void bg96_modem_init(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter bg96_modem_init")

  /* common init function (reset all contexts) */
  atcm_modem_init(p_modem_ctxt);

  /* modem specific actions if any */
}

/* BG96 modem reset fonction
*  call common reset function and then do actions specific to this modem
*/
static void bg96_modem_reset(atcustom_modem_context_t *p_modem_ctxt)
{
  PRINT_API("enter bg96_modem_reset")

  /* common reset function (reset all contexts except SID) */
  atcm_modem_reset(p_modem_ctxt);

  /* modem specific actions if any */
}

static void reinitSyntaxAutomaton_bg96(void)
{
  BG96_ctxt.state_SyntaxAutomaton = WAITING_FOR_INIT_CR;
}

static void reset_variables_bg96(void)
{
  /* Set default values of BG96 specific variables after SWITCH ON or RESET */
  bg96_shared.mode_and_bands_config.nw_scanseq = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.nw_scanmode = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.iot_op_mode = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.gsm_bands = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.CatM1_bands_MsbPart = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.CatM1_bands_LsbPart = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.CatNB1_bands_MsbPart = 0xFFFFFFFFU;
  bg96_shared.mode_and_bands_config.CatNB1_bands_LsbPart = 0xFFFFFFFFU;
}

static void init_bg96_qiurc_dnsgip(void)
{
  bg96_shared.QIURC_dnsgip_param.finished = AT_FALSE;
  bg96_shared.QIURC_dnsgip_param.wait_header = AT_TRUE;
  bg96_shared.QIURC_dnsgip_param.error = 0U;
  bg96_shared.QIURC_dnsgip_param.ip_count = 0U;
  bg96_shared.QIURC_dnsgip_param.ttl = 0U;
  (void) memset((void *)bg96_shared.QIURC_dnsgip_param.hostIPaddr, 0, MAX_SIZE_IPADDR);
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

static void display_decoded_GSM_bands(uint32_t gsm_bands)
{
  PRINT_INFO("GSM BANDS config = 0x%lx", gsm_bands)

  if ((gsm_bands & QCFGBANDGSM_900) != 0U)
  {
    PRINT_INFO("GSM_900")
  }
  if ((gsm_bands & QCFGBANDGSM_1800) != 0U)
  {
    PRINT_INFO("GSM_1800")
  }
  if ((gsm_bands & QCFGBANDGSM_850) != 0U)
  {
    PRINT_INFO("GSM_850")
  }
  if ((gsm_bands & QCFGBANDGSM_1900) != 0U)
  {
    PRINT_INFO("GSM_1900")
  }
}

static void display_decoded_CatM1_bands(uint32_t CatM1_bands_MsbPart, uint32_t CatM1_bands_LsbPart)
{
  PRINT_INFO("Cat.M1 BANDS config = 0x%lx%lx", CatM1_bands_MsbPart, CatM1_bands_LsbPart)

  /* LSB bitmap part: ........XXXXXXXX */
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B1_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B1")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B2_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B2")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B3_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B3")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B4_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B4")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B5_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B5")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B8_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B8")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B12_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B12")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B13_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B13")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B18_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B18")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B19_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B19")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B20_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B20")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B26_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B26")
  }
  if ((CatM1_bands_LsbPart & (uint32_t) QCFGBANDCATM1_B28_LSB) != 0U)
  {
    PRINT_INFO("CatM1_B28")
  }
  /* MSB bitmap part: XXXXXXXX........ */
  if ((CatM1_bands_MsbPart & (uint32_t) QCFGBANDCATM1_B39_MSB) != 0U)
  {
    PRINT_INFO("CatM1_B39")
  }
}

static void display_decoded_CatNB1_bands(uint32_t CatNB1_bands_MsbPart, uint32_t CatNB1_bands_LsbPart)
{
  UNUSED(CatNB1_bands_MsbPart); /* MSB part of NB1 band bitmap is not used */

  PRINT_INFO("Cat.NB1 BANDS config = 0x%lx%lx", CatNB1_bands_MsbPart, CatNB1_bands_LsbPart)

  /* LSB bitmap part: ........XXXXXXXX */
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B1_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B1")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B2_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B2")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B3_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B3")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B4_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B4")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B5_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B5")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B8_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B8")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B12_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B12")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B13_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B13")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B18_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B18")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B19_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B19")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B20_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B20")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B26_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B26")
  }
  if ((CatNB1_bands_LsbPart & (uint32_t) QCFGBANDCATNB1_B28_LSB) != 0U)
  {
    PRINT_INFO("CatNB1_B28")
  }
  /* MSB bitmap part: XXXXXXXX........ */
  /* no bands in MSB bitmap part */
}

static void display_user_friendly_mode_and_bands_config(void)
{
  uint8_t catM1_on = 0U;
  uint8_t catNB1_on = 0U;
  uint8_t gsm_on = 0U;
  ATCustom_BG96_QCFGscanseq_t scanseq_1st, scanseq_2nd, scanseq_3rd;

#if 0 /* for DEBUG */
  /* display modem raw values */
  display_decoded_CatM1_bands(bg96_shared.mode_and_bands_config.CatM1_bands);
  display_decoded_CatNB1_bands(bg96_shared.mode_and_bands_config.CatNB1_bands);
  display_decoded_GSM_bands(bg96_shared.mode_and_bands_config.gsm_bands);

  PRINT_INFO("nw_scanmode = 0x%x", bg96_shared.mode_and_bands_config.nw_scanmode)
  PRINT_INFO("iot_op_mode = 0x%x", bg96_shared.mode_and_bands_config.iot_op_mode)
  PRINT_INFO("nw_scanseq = 0x%x", bg96_shared.mode_and_bands_config.nw_scanseq)
#endif /* 0, for DEBUG */

  PRINT_INFO(">>>>> BG96 mode and bands configuration <<<<<")
  /* LTE bands */
  if ((bg96_shared.mode_and_bands_config.nw_scanmode == QCFGSCANMODE_AUTO) ||
      (bg96_shared.mode_and_bands_config.nw_scanmode == QCFGSCANMODE_LTEONLY))
  {
    if ((bg96_shared.mode_and_bands_config.iot_op_mode == QCFGIOTOPMODE_CATM1CATNB1) ||
        (bg96_shared.mode_and_bands_config.iot_op_mode == QCFGIOTOPMODE_CATM1))
    {
      /* LTE Cat.M1 active */
      catM1_on = 1U;
    }
    if ((bg96_shared.mode_and_bands_config.iot_op_mode == QCFGIOTOPMODE_CATM1CATNB1) ||
        (bg96_shared.mode_and_bands_config.iot_op_mode == QCFGIOTOPMODE_CATNB1))
    {
      /* LTE Cat.NB1 active */
      catNB1_on = 1U;
    }
  }

  /* GSM bands */
  if ((bg96_shared.mode_and_bands_config.nw_scanmode == QCFGSCANMODE_AUTO) ||
      (bg96_shared.mode_and_bands_config.nw_scanmode == QCFGSCANMODE_GSMONLY))
  {
    /* GSM active */
    gsm_on = 1U;
  }

  /* Search active techno */
  scanseq_1st = ((ATCustom_BG96_QCFGscanseq_t)bg96_shared.mode_and_bands_config.nw_scanseq &
                 (ATCustom_BG96_QCFGscanseq_t)0x00FF0000U) >> 16;
  scanseq_2nd = ((ATCustom_BG96_QCFGscanseq_t)bg96_shared.mode_and_bands_config.nw_scanseq &
                 (ATCustom_BG96_QCFGscanseq_t)0x0000FF00U) >> 8;
  scanseq_3rd = ((ATCustom_BG96_QCFGscanseq_t)bg96_shared.mode_and_bands_config.nw_scanseq &
                 (ATCustom_BG96_QCFGscanseq_t)0x000000FFU);
  PRINT_DBG("decoded scanseq: 0x%lx -> 0x%lx -> 0x%lx", scanseq_1st, scanseq_2nd, scanseq_3rd)

  uint8_t rank = 1U;
  /* display active bands by rank
   * rank is incremented only if a band is really active
   */
  /* 1st band */
  if (1U == display_if_active_band(scanseq_1st, rank, catM1_on, catNB1_on, gsm_on))
  {
    rank++;
  }
  /* 2nd band */
  if (1U == display_if_active_band(scanseq_2nd, rank, catM1_on, catNB1_on, gsm_on))
  {
    rank++;
  }
  /* 3rd band (do not need to increment rank as this is the last band */
  (void) display_if_active_band(scanseq_3rd, rank, catM1_on, catNB1_on, gsm_on);

  PRINT_INFO(">>>>> ................................. <<<<<")
}

static uint8_t display_if_active_band(ATCustom_BG96_QCFGscanseq_t scanseq,
                                      uint8_t rank,
                                      uint8_t catM1_on,
                                      uint8_t catNB1_on,
                                      uint8_t gsm_on)
{
  UNUSED(rank);
  uint8_t retval = 0U;

  if (scanseq == (ATCustom_BG96_QCFGscanseq_t) QCFGSCANSEQ_GSM)
  {
    if (gsm_on == 1U)
    {
      PRINT_INFO("GSM band active (scan rank = %d)", rank)
      display_decoded_GSM_bands(bg96_shared.mode_and_bands_config.gsm_bands);

      retval = 1U;
    }
  }
  else if (scanseq == (ATCustom_BG96_QCFGscanseq_t) QCFGSCANSEQ_LTECATM1)
  {
    if (catM1_on == 1U)
    {
      PRINT_INFO("LTE Cat.M1 band active (scan rank = %d)", rank)
      display_decoded_CatM1_bands(bg96_shared.mode_and_bands_config.CatM1_bands_MsbPart,
                                  bg96_shared.mode_and_bands_config.CatM1_bands_LsbPart);
      retval = 1U;
    }
  }
  else if (scanseq == (ATCustom_BG96_QCFGscanseq_t) QCFGSCANSEQ_LTECATNB1)
  {
    if (catNB1_on == 1U)
    {
      PRINT_INFO("LTE Cat.NB1 band active (scan rank = %d)", rank)
      display_decoded_CatNB1_bands(bg96_shared.mode_and_bands_config.CatNB1_bands_MsbPart,
                                   bg96_shared.mode_and_bands_config.CatNB1_bands_LsbPart);
      retval = 1U;
    }
  }
  else
  {
    /* scanseq value not managed */
  }

  return (retval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

