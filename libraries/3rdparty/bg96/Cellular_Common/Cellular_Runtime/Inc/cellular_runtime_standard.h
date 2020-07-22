/**
  ******************************************************************************
  * @file    cellular_runtime_custom.h
  * @author  MCD Application Team
  * @brief   Header for cellular_runtime_custom.c module
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
#ifndef CELLULAR_RUNTIME_STANDARD_H
#define CELLULAR_RUNTIME_STANDARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"
#include <stdio.h>

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern uint8_t *crs_itoa(int32_t num, uint8_t *str, uint32_t base);
extern int32_t  crs_atoi(const uint8_t *string);
extern int32_t  crs_atoi_hex(const uint8_t *string);
extern uint32_t crs_strlen(const uint8_t *string);

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_RUNTIME_STANDARD_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
