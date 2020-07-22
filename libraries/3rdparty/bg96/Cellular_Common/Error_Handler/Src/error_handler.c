/**
  ******************************************************************************
  * @file    error_handler.c
  * @author  MCD Application Team
  * @brief   This file contains error utilities
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
#include "error_handler.h"
#include "plf_config.h"


/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ERROR_HANDLER == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ERROR_LOGGER, DBL_LVL_P0, "" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ERROR_LOGGER, DBL_LVL_P1, "" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_INFO(format, args...)  (void)printf("Error Handler:" format "\n\r", ## args);
#define PRINT_DBG(format, args...)   (void)printf("Error Handler" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)    __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ERROR_HANDLER */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define MAX_ERROR_ENTRIES (32U)     /* log only last MAX_ERROR_ENTRIES erros */
#define MAX_ERROR_COUNTER (0xFFFFU) /* count how many errors have been logged since the beginning */

/* Private variables ---------------------------------------------------------*/
static error_handler_decript_t errors_table[MAX_ERROR_ENTRIES]; /* error array */
static uint16_t error_counter = 0U; /* total number of errors */
static uint16_t error_index;        /* current error index */

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
void ERROR_Handler_Init(void)
{
  uint32_t i;

  /* initialize error array */
  for (i = 0U; i < MAX_ERROR_ENTRIES; i++)
  {
    errors_table[i].channel = DBG_CHAN_ERROR_LOGGER; /* default value = self (ie no error) */
    errors_table[i].errorId = 0;
    errors_table[i].gravity = ERROR_NO;
  }
  error_index = 0U; /* current error index */
}

void ERROR_Handler(dbg_channels_t chan, int32_t errorId, error_gravity_t gravity)
{
  /* if this is the very first error, init error array */
  if (error_counter == 0U)
  {
    ERROR_Handler_Init();
  }

  /* log the error */
  error_counter = (error_counter + 1U) % MAX_ERROR_COUNTER;
  errors_table[error_index].count = error_counter;
  errors_table[error_index].channel = chan;
  errors_table[error_index].errorId = errorId;
  errors_table[error_index].gravity = gravity;

  PRINT_INFO("LOG ERROR #%d: channel=%d / errorId=%ld / gravity=%d", error_counter, chan, errorId, gravity)

  /* endless loop if error is fatal */
  if (gravity == ERROR_FATAL)
  {
    HAL_Delay(1000U);
    NVIC_SystemReset();
    /* Infinite loop is done in NVIC_SystemReset(); */
  }

  /* increment error index */
  error_index = (error_index + 1U) %  MAX_ERROR_ENTRIES;
}

void ERROR_Dump_All(void)
{
  uint32_t i;
  if (error_counter > 0U)
  {
    /* Dump errors array */
    for (i = 0U; i < MAX_ERROR_ENTRIES; i++)
    {
      if (errors_table[i].gravity != ERROR_NO)
      {
        PRINT_INFO("DUMP ERROR[%ld] (#%ld): channel=%d / errorId=%ld / gravity=%d",
                   i,
                   errors_table[i].count,
                   errors_table[i].channel,
                   errors_table[i].errorId,
                   errors_table[i].gravity)
      }
    }
  }
}

void ERROR_Dump_Last(void)
{
#if (USE_TRACE_ERROR_HANDLER == 1U)
  if (error_counter != 0U)
  {
    /* get last error index */
    uint16_t previous_index; /* Last error index */

    if (error_index == 0U)
    {
      previous_index = MAX_ERROR_ENTRIES;
    }
    else
    {
      previous_index = error_index - 1U;
    }

    PRINT_INFO("DUMP LAST ERROR[%ld] (#%ld): channel=%d / errorId=%ld / gravity=%d",
               previous_index,
               errors_table[previous_index].count,
               errors_table[previous_index].channel,
               errors_table[previous_index].errorId,
               errors_table[previous_index].gravity)
  }
#endif  /* (USE_TRACE_ERROR_HANDLER == 1U) */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
