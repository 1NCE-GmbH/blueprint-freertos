/**
  ******************************************************************************
  * @file    at_custom_modem_api.c
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "at_custom_modem_api.h"
#include "at_custom_modem_specific.h"
#include "sysctrl_specific.h"
#include "plf_config.h"

/* BG96 COMPILATION FLAGS to define in project option if needed:
*
*/

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
void atcma_init_at_func_ptrs(atcustom_funcPtrs_t *funcPtrs)
{
#if defined(USE_MODEM_BG96)
  /* init function pointers with BG96 functions */
  funcPtrs->f_init = ATCustom_BG96_init;
  funcPtrs->f_checkEndOfMsgCallback = ATCustom_BG96_checkEndOfMsgCallback;
  funcPtrs->f_getCmd = ATCustom_BG96_getCmd;
  funcPtrs->f_extractElement = ATCustom_BG96_extractElement;
  funcPtrs->f_analyzeCmd = ATCustom_BG96_analyzeCmd;
  funcPtrs->f_analyzeParam = ATCustom_BG96_analyzeParam;
  funcPtrs->f_terminateCmd = ATCustom_BG96_terminateCmd;
  funcPtrs->f_get_rsp = ATCustom_BG96_get_rsp;
  funcPtrs->f_get_urc = ATCustom_BG96_get_urc;
  funcPtrs->f_get_error = ATCustom_BG96_get_error;
  funcPtrs->f_hw_event = ATCustom_BG96_hw_event;
#else
#error AT custom does not match with selected modem
#endif /* USE_MODEM_BG96 */
}

void atcma_init_sysctrl_func_ptrs(sysctrl_funcPtrs_t *funcPtrs)
{
#if defined(USE_MODEM_BG96)
  /* init function pointers with BG96 functions */
  funcPtrs->f_getDeviceDescriptor = SysCtrl_BG96_getDeviceDescriptor;
  funcPtrs->f_open_channel =  SysCtrl_BG96_open_channel;
  funcPtrs->f_close_channel =  SysCtrl_BG96_close_channel;
  funcPtrs->f_power_on =  SysCtrl_BG96_power_on;
  funcPtrs->f_power_off = SysCtrl_BG96_power_off;
  funcPtrs->f_reset_device = SysCtrl_BG96_reset;
  funcPtrs->f_sim_select = SysCtrl_BG96_sim_select;
#else
#error SysCtrl does not match with selected modem
#endif /* USE_MODEM_BG96 */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
