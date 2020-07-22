/**
  ******************************************************************************
  * @file    dc_time.c
  * @author  MCD Application Team
  * @brief   This file contains data cache time services
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


/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"
#include "dc_time.h"
#if (USE_RTC == 1)
#include "rtc.h"
#endif /* (USE_RTC == 1) */
#include "dc_common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

#if (USE_RTC == 1)
static dc_time_date_rt_info_t dc_time_date;

/* Global variables ----------------------------------------------------------*/
dc_com_res_id_t DC_COM_TIME_DATE = DC_COM_INVALID_ENTRY;

/* Private function prototypes -----------------------------------------------*/

static uint16_t get_yday(RTC_DateTypeDef *Date);
#endif /* (USE_RTC == 1) */

/* Functions Definition ------------------------------------------------------*/

/* Private Functions Definition ------------------------------------------------------*/
#if (USE_RTC == 1)
/**
  * @brief  get day of year number
  * @param  Date     (in) date to set
  * @retval day od year
  */
static uint16_t get_yday(RTC_DateTypeDef *Date)
{
  static uint16_t month_day[13] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
  uint16_t i;
  uint16_t yday;
  yday = 0U;

  /* add number of day for each finisher month */
  for (i = 1U; i < Date->Month; i++)
  {
    yday += month_day[i];
  }

  /* add number of day of the current month */
  yday += Date->Date;

  /* add a day if it is a lead year after february */
  if (((Date->Year % 4U) == 0U) && (Date->Month > 1U))
  {
    yday++;
  }

  return yday;
}
#endif /* (USE_RTC == 1) */

/* Exported Functions Definition ------------------------------------------------------*/

/**
  * @brief  set system date and/or time
  * @param  time           (in) date and/or time to set
  * @param  time_date      (in) type of time parameter (date and/or time)
  * @retval bool   return status
  */

bool dc_time_set_time_date(const dc_time_date_rt_info_t *time,
                           dc_time_data_type_t time_date)
{
#if (USE_RTC == 1)
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
#endif /* (USE_RTC == 1) */
  bool ret;

  ret = true;
#if (USE_RTC == 1)

  if (time_date <= DC_DATE_AND_TIME)
  {
    /* Set Time */
    if (time_date != DC_DATE)
    {
      sTime.Hours   = (uint8_t)time->hour;
      sTime.Minutes = (uint8_t)time->min;
      sTime.Seconds = (uint8_t)time->sec;

      sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      sTime.StoreOperation = RTC_STOREOPERATION_RESET;

      if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
      {
        ret = false;
      }
    }

    /* Set Date */
    if ((time_date != DC_TIME)
        && (ret == true))
    {
      if ((time->wday > 7U) || (time->wday == 0U)
          || (time->year < 2000U) || (time->year > 2255U))
      {
        ret = false;
      }
      else
      {
        sDate.WeekDay = (uint8_t)time->wday;
        sDate.Date    = (uint8_t)time->mday;
        sDate.Month   = (uint8_t)time->month;
        sDate.Year    = (uint8_t)(time->year - 2000U);

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
        {
          ret = false;
        }
      }
    }

    if (ret == true)
    {
      /* Time and/or Date updated */
      dc_time_date_rt_info_t time_date_tmp;
      /* Update Time and Date */
      (void)dc_time_get_time_date(&time_date_tmp, DC_DATE_AND_TIME);
      if (time_date == DC_DATE_AND_TIME)
      {
        /* Time and Date updated */
        dc_time_date.rt_state = DC_SERVICE_ON;
      }
      else if (dc_time_date.rt_state != DC_SERVICE_ON)
      {
        /* Date and Time not yet fully initialized */
        /* DC_SERVICE_STARTING is used when time already updated */
        /* DC_SERVICE_RUN is used when date already updated */
        if (time_date == DC_DATE)
        {
          if (dc_time_date.rt_state == DC_SERVICE_STARTING)
          {
            dc_time_date.rt_state = DC_SERVICE_ON;
          }
          else
          {
            dc_time_date.rt_state = DC_SERVICE_RUN;
          }
        }
        else /* time_date == DC_TIME */
        {
          if (dc_time_date.rt_state == DC_SERVICE_RUN)
          {
            dc_time_date.rt_state = DC_SERVICE_ON;
          }
          else
          {
            dc_time_date.rt_state = DC_SERVICE_STARTING;
          }
        }
      }
      else
      {
        __NOP();
      }
      (void)dc_com_write(&dc_com_db,
                         DC_COM_TIME_DATE,
                         (void *)&dc_time_date,
                         sizeof(dc_time_date_rt_info_t));
    }
  }
  else
  {
    ret = false;
  }
#else
  ret = false;
#endif /* (USE_RTC == 1) */

  return ret;
}

/**
  * @brief  get system date and/or time
  * @param  time         (out) date and/or time to get
  * @param  time_date    (in) type of time parameter (date and/or time)
  * @retval bool  return status
  */
bool dc_time_get_time_date(dc_time_date_rt_info_t *time,
                           dc_time_data_type_t time_date)
{
#if (USE_RTC == 1)
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  bool ret;

  ret = false;

  if (time_date <= DC_DATE_AND_TIME)
  {
    /* WARNING : if HAL_RTC_GetTime is called it must be called before HAL_RTC_GetDate */
    /* HAL_RTC_GetTime return always HAL_OK */
    (void)HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    if (time_date != DC_DATE)
    {
      /* Get the RTC current Time */
      time->hour  = stimestructureget.Hours;
      time->min   = stimestructureget.Minutes;
      time->sec   = stimestructureget.Seconds;
      time->isdst = 0U;  /* not managed */
    }

    /* WARNING : HAL_RTC_GetDate must be called after HAL_RTC_GetTime even if date get is not necessary */
    (void)HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    if (time_date != DC_TIME)
    {
      /* Get the RTC current Date */
      time->wday  = sdatestructureget.WeekDay;
      time->mday  = sdatestructureget.Date;
      time->month = sdatestructureget.Month;
      time->year  = (uint32_t)sdatestructureget.Year + 2000U;
      time->yday  = get_yday(&sdatestructureget);
    }

    time->rt_state = dc_time_date.rt_state;
    ret = true;
  }

  return ret;
#else
  return false;
#endif /* (USE_RTC == 1) */
}


/**
  * @brief  service intialization
  * @param  -
  * @retval -
  */
void dc_time_init(void)
{
#if (USE_RTC == 1)
  DC_COM_TIME_DATE = dc_com_register_serv(&dc_com_db,
                                          (void *)&dc_time_date,
                                          (uint16_t)sizeof(dc_time_date));
  /* dc_time_date.rt_state is set to DC_SERVICE_OFF */
#endif /* (USE_RTC == 1) */
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

