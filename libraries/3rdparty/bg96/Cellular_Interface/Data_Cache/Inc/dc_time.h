/**
  ******************************************************************************
  * @file    dc_time.h
  * @author  MCD Application Team
  * @brief   Header for dc_time.c module
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

#ifndef DC_TIME_H
#define DC_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "dc_common.h"


/** @addtogroup DC
  * @{
  */

/** @defgroup DC_TIME Data Cache Time services
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DC_TIME_Constants Constants
  * @{
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup DC_TIME_Types Types
  * @{
  */
typedef uint8_t dc_time_data_type_t;
#define DC_TIME          (dc_time_data_type_t)0U
#define DC_DATE          (dc_time_data_type_t)1U
#define DC_DATE_AND_TIME (dc_time_data_type_t)2U

typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t rt_state;
  uint32_t sec;    /*!< seconds 0-61 */
  uint32_t min;    /*!< minutes 0-59 */
  uint32_t hour;   /*!< hours 0-23 */
  uint32_t mday;   /*!< day of the month  1-31   (based on RTC HAL values)*/
  uint32_t month;  /*!< month since january 1-12 (based on RTC HAL values) */
  uint32_t year;   /*!< year since 1970 */
  uint32_t wday;   /*!< day since monday 1-7 (based on RTC HAL values) */
  uint32_t yday;   /*!< days since January 1 */
  uint32_t isdst;  /*!< daylight saving time 0-365 */
} dc_time_date_rt_info_t;

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup DC_TIME_Variables Variables
  * @{
  */

/** @brief Time Data Cache entries */
extern dc_com_res_id_t    DC_COM_TIME_DATE;

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup DC_TIME_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup DC_TIME_Functions Functions
  * @{
  */

/**
  * @brief  get system date and/or time
  * @param  time         (out) date and/or time to get
  * @param  time_date    (in) type of time parameter (date and/or time)
  * @retval bool  return status
  */
bool dc_time_get_time_date(dc_time_date_rt_info_t *time, dc_time_data_type_t time_date);

/**
  * @brief  set system date and/or time
  * @param  time           (in) date and/or time to set
  * @param  time_date      (in) type of time parameter (date and/or time)
  * @retval bool   return status
  */
bool dc_time_set_time_date(const dc_time_date_rt_info_t *time, dc_time_data_type_t time_date);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/*** Component Initialization/Start *******************************************/
/*** Used by Cellular-Service - Not an User Interface *************************/

/**
  * @brief  Init system date and time data cache entry
  * @param  -
  * @retval -
 */
void dc_time_init(void);

#ifdef __cplusplus
}
#endif


#endif /* __DC_TIME_H */

/***************************** (C) COPYRIGHT STMicroelectronics *******END OF FILE ************/
