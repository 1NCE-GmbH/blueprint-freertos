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
#ifndef CELLULAR_RUNTIME_CUSTOM_H
#define CELLULAR_RUNTIME_CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef char CRC_CHAR_t;

/* Exported functions ------------------------------------------------------- */
extern uint32_t crc_get_ip_addr(uint8_t *string, uint8_t *addr, uint16_t *port);

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_RUNTIME_CUSTOM_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
