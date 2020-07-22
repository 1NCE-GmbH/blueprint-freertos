/**
  ******************************************************************************
  * @file    at_custom_modem_api.h
  * @author  MCD Application Team
  * @brief   Header for at_custom_modem_api.c module for BG96
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
#ifndef AT_CUSTOM_MODEM_API_H
#define AT_CUSTOM_MODEM_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "at_modem_api.h"
#include "sysctrl.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void atcma_init_at_func_ptrs(atcustom_funcPtrs_t *funcPtrs);
void atcma_init_sysctrl_func_ptrs(sysctrl_funcPtrs_t *funcPtrs);

#ifdef __cplusplus
}
#endif

#endif /* AT_CUSTOM_MODEM_API_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
