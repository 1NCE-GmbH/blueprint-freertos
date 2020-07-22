/**
  ******************************************************************************
  * @file    cmsis_os_misrac2012.h
  * @author  MCD Application Team
  * @brief   This file is used to disable FreeRTOS MISRAC 2012 messages
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
#ifndef CMSIS_OS_MISRAC2012_H
#define CMSIS_OS_MISRAC2012_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* FreeRTOS is a Third Party so MISRAC messages linked to it are ignored */
/*cstat -MISRAC2012-* */
#include "cmsis_os.h"
/*cstat +MISRAC2012-* */

/* Exported constants --------------------------------------------------------*/

/* Platform defines ----------------------------------------------------------*/
/* MISRAC 2012 issue link to osWaitForever usage */
/* Adding U in order to solve MISRAC2012-Dir-7.2 */
#define RTOS_WAIT_FOREVER 0xFFFFFFFFU

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* CMSIS_OS_MISRAC2012_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
