/**
  ******************************************************************************
  * @file    plf_power_config.h
  * @author  MCD Application Team
  * @brief   This file contains the power default configuration
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
#ifndef PLF_POWER_CONFIG_H
#define PLF_POWER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Default power mode */
#define DC_POWER_MODE_DEFAULT                           DC_POWER_IDLE

/* power sleep request timeout default value */
#define DC_POWER_SLEEP_REQUEST_TIMEOUT_DEFAULT          20000U /* 20 s */

/* eDRX values definition */
#define DC_EDRX_WB_S1_PTW_1S_DRX_40S  (uint8_t)(0x03) /* "0000.0011" = 0x49 : WB-S1 mode PTW=1.28 sec, EDRX=40.96 sec */

/* PSM values definition */
#define DC_PSM_T3312_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */
#define DC_PSM_T3314_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */
#define DC_PSM_T3412_4_HOURS     (uint8_t)(0x24) /* "001.00100" = 0x24 */
#define DC_PSM_T3324_16_SEC      (uint8_t)(0x08) /* "000.01000" = 0x08 */

/* PSM default values */
#define DC_POWER_PSM_REQ_PERIODIC_RAU_DEFAULT           DC_PSM_T3312_DEACTIVATED
#define DC_POWER_PSM_REQ_GPRS_READY_TIMER_DEFAULT       DC_PSM_T3314_DEACTIVATED
#define DC_POWER_PSM_REQ_PERIODIC_TAU_DEFAULT           DC_PSM_T3412_4_HOURS
#define DC_POWER_PSM_REQ_ACTIVE_TIMER_DEFAULT           DC_PSM_T3324_16_SEC

/* eDRX default values */
#define DC_POWER_EDRX_ACT_TYPE_DEFAULT                  DC_EDRX_ACT_E_UTRAN_WB_S1
#define DC_POWER_EDRX_REQ_VALUE_DEFAULT                 DC_EDRX_WB_S1_PTW_1S_DRX_40S

#ifdef __cplusplus
}
#endif

#endif /* PLF_POWER_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
