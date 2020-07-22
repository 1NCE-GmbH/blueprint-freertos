/**
  ******************************************************************************
  * @file    httpclient.h
  * @author  MCD Application Team
  * @brief   Header for httpclient.c module
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
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/**
  * @brief  Initialization
  * @note   HTTP client initialization - first function to call
  * @param  -
  * @retval -
  */
void httpclient_init(void);

/**
  * @brief  Start
  * @note   HTTP client start - must be called after httpclient_init
  * @param  -
  * @retval -
  */
void httpclient_start(void);


#ifdef __cplusplus
}
#endif

#endif /* HTTP_CLIENT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
