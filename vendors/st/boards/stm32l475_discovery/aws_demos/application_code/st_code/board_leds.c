/**
  ******************************************************************************
  * @file    board_leds.c
  * @author  MCD Application Team
  * @brief   Implements functions for leds actions
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
#include <stdbool.h>
#include "plf_config.h"
#include "board_leds.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static bool BL_LED_get(uint8_t bl_led, Led_TypeDef *led);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Convert a led identifier to Led_TypeDef
  * @param  bl_led - led identifier see plf_hw_config.h
  * @param  led    - led identifier in Led_TypeDef format
  * @retval bool - true/false conversion bl_led to led OK/?OK
  */
static bool BL_LED_get(uint8_t bl_led,
                       Led_TypeDef *led)
{
  bool result;

  switch (bl_led)
  {
    case LED2:
    {
      *led = LED2;
      result = true;
      break;
    }
    case NO_LED:
    default:
    {
      result = false;
      break;
    }
  }

  return result;
}

/**
  * @brief  Initialize a Led
  * @param  bl_led - led identifier see plf_hw_config.h
  * @retval -
  */
void BL_LED_Init(uint8_t bl_led)
{
  Led_TypeDef led;

  if (BL_LED_get(bl_led,
                 &led)
      == true)
  {
    BSP_LED_Init(led);
  }
}

/**
  * @brief  Power Off a Led
  * @param  bl_led - led identifier see plf_hw_config.h
  * @retval -
  */
void BL_LED_Off(uint8_t bl_led)
{
  Led_TypeDef led;

  if (BL_LED_get(bl_led,
                 &led)
      == true)
  {
    BSP_LED_Off(led);
  }
}

/**
  * @brief  Power On a Led
  * @param  bl_led - led identifier see plf_hw_config.h
  * @retval -
  */
void BL_LED_On(uint8_t bl_led)
{
  Led_TypeDef led;

  if (BL_LED_get(bl_led,
                 &led)
      == true)
  {
    BSP_LED_On(led);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
