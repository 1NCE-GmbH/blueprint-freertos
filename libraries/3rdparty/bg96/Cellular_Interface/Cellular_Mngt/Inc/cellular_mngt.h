/**
  ******************************************************************************
  * @file    cellular_mngt.h
  * @author  MCD Application Team
  * @brief   Header for cellular_mngt.c module
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
#ifndef CELLULAR_MNGT_H
#define CELLULAR_MNGT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"
#if (USE_NETWORK_LIBRARY == 1)
#include "net_connect.h"
#endif /* (USE_NETWORK_LIBRARY == 1) */
#include "cellular_datacache.h"


/** @defgroup CELLULAR_INTERFACE Cellular Interface
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                  ##### How to use Cellular Management module #####
  ==============================================================================

 Cellular Management allows Initialization and Start of X-Cube-Cellular components.

 Before to access any other services of X-Cube-Cellular component, initialization must be done.
 Then two start options are available:

 1) boot the modem with network registration (used to exchange data to with remote host)
 cellular_init() --> cellular_start()

 2) boot modem with no network registration  (used to configure modem)
 cellular_init() --> cellular_modem_start()

 @endverbatim
  */

/** @defgroup CELLULAR_MANAGEMENT Cellular Management module
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup CELLULAR_MANAGEMENT_Constants Constants
  * @{
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup CELLULAR_MANAGEMENT_Types Types
  * @{
  */

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup CELLULAR_MANAGEMENT_Variables Variables
  * @{
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup CELLULAR_MANAGEMENT_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup CELLULAR_MANAGEMENT_Functions Functions
  * @{
  */

/** @defgroup CELLULAR_MANAGEMENT_Functions_Group1  Cellular Initialization and Start services
  * @brief
  * @{
  */

/**
  * @brief  Initialize cellular features
  * @param  -
  * @retval -
  */
void cellular_init(void);

/**
  * @brief  Start cellular features with boot modem and network registration
  * @param  -
  * @retval -
  */
void cellular_start(void);

/**
  * @brief  Start cellular feature with boot modem (NO network registration)
  *         Usage: configure the modem
  * @param  -
  * @retval -
  */
void cellular_modem_start(void);

/**
  * @brief  Initialize dc_cellular_params_t structure with default value
  * @note   this function should be called by application before to store its own values
  * @param  cellular_params - cellular configuration
  * @retval -
  */
void cellular_set_default_setup_config(dc_cellular_params_t *cellular_params);

/**
  * @}
  */

#if (USE_NETWORK_LIBRARY == 1)

/** @defgroup CELLULAR_MANAGEMENT_Functions_Group3 Cellular Initialization and Start services Network Library
  * @brief    Initialization and Start services in case of Network Library use
  * @{
  */

/**
  * @brief  Registers a network notification function
  * @param  notify_func - notification function to record
  * @retval 0: OK - 1: NOK
  */
uint32_t cellular_net_register(net_if_notify_func notify_func);

/**
  * @brief  Initialize cellular features in case of Network Library use
  * @param  -
  * @retval -
  */
void cellular_net_init(void);

/**
  * @brief  Start cellular features in case of Network Library use
  * @param  -
  * @retval -
  */
void cellular_net_start(void);

/**
  * @}
 */

#endif  /* (USE_NETWORK_LIBRARY == 1) */

#if (!USE_DEFAULT_SETUP == 1)
/** @defgroup CELLULAR_MANAGEMENT_Functions_Group2 Cellular Other services
  * @brief
  * @{
  */

/**
  * @brief  Display Cellular FAQ
  *         Only available if Menu configuration is activated
  *         After FAQ display, waiting for a key and then reboot the board
  * @param  -
  * @retval -
  */
void cellular_faq_start(void);

/**
  * @}
 */

#endif /* !USE_DEFAULT_SETUP == 1 */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_MNGT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
