/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOC
#define BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define MDM_SIM_CLK_Pin GPIO_PIN_0
#define MDM_SIM_CLK_GPIO_Port GPIOC
#define MDM_SIM_SELECT_1_Pin GPIO_PIN_3
#define MDM_SIM_SELECT_1_GPIO_Port GPIOA
#define MDM_SIM_SELECT_0_Pin GPIO_PIN_4
#define MDM_SIM_SELECT_0_GPIO_Port GPIOA
#define MDM_DTR_Pin GPIO_PIN_1
#define MDM_DTR_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOB
#define MDM_SIM_DATA_Pin GPIO_PIN_14
#define MDM_SIM_DATA_GPIO_Port GPIOD
#define MDM_RST_Pin GPIO_PIN_15
#define MDM_RST_GPIO_Port GPIOA
#define MDM_PWR_EN_Pin GPIO_PIN_4
#define MDM_PWR_EN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

extern RTC_HandleTypeDef xHrtc;
extern RNG_HandleTypeDef xHrng;


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
