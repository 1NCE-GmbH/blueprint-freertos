/**
  ******************************************************************************
  * @file    ppposif_client.h
  * @author  MCD Application Team
  * @brief   Header for ppposif_client.c module
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
#ifndef PPPOSIF_CLIENT_H
#define PPPOSIF_CLIENT_H


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "ppposif.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/**
  * @brief  component init
  * @param  none
  * @retval ppposif_status_t    return status
  */
extern ppposif_status_t ppposif_client_init(void);

/**
  * @brief  component start
  * @param  none
  * @retval ppposif_status_t    return status
  */
extern ppposif_status_t ppposif_client_start(void);

/**
  * @brief  Create a new PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
extern ppposif_status_t ppposif_client_config(void);

/**
  * @brief  close PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
extern ppposif_status_t ppposif_client_close(uint8_t cause);

#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#ifdef __cplusplus
}
#endif

#endif /* PPPOSIF_CLIENT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
