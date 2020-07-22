/**
  ******************************************************************************
  * @file    ping_client_config.h
  * @author  MCD Application Team
  * @brief   Default configuration parameters for ping client
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
#ifndef PING_CLIENT_CONFIG_H
#define PING_CLIENT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define PING_DEFAULT_REMOTE_HOSTIP1   ((uint8_t*)"8.8.8.8")         /* 8.8.8.8 */
#define PING_DEFAULT_REMOTE_HOSTIP2   ((uint8_t*)"173.236.12.163")  /* 173.236.12.163 */
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* PING_CLIENT_CONFIG_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

