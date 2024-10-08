/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler( void );

/* Private defines -----------------------------------------------------------*/
#define JOY_DOWN_Pin                  GPIO_PIN_10
#define JOY_DOWN_GPIO_Port            GPIOI
#define JOY_DOWN_EXTI_IRQn            EXTI15_10_IRQn
#define STMOD_INT_Pin                 GPIO_PIN_2
#define STMOD_INT_GPIO_Port           GPIOH
#define PSRAM_NBL0_Pin                GPIO_PIN_0
#define PSRAM_NBL0_GPIO_Port          GPIOE
#define ARD_D12_Pin                   GPIO_PIN_4
#define ARD_D12_GPIO_Port             GPIOB
#define SWO_Pin                       GPIO_PIN_3
#define SWO_GPIO_Port                 GPIOB
#define ARD_D10_Pin                   GPIO_PIN_15
#define ARD_D10_GPIO_Port             GPIOA
#define SWCLK_Pin                     GPIO_PIN_14
#define SWCLK_GPIO_Port               GPIOA
#define SWDIO_Pin                     GPIO_PIN_13
#define SWDIO_GPIO_Port               GPIOA
#define DCMI_D4_Pin                   GPIO_PIN_14
#define DCMI_D4_GPIO_Port             GPIOH
#define JOY_LEFT_Pin                  GPIO_PIN_9
#define JOY_LEFT_GPIO_Port            GPIOI
#define JOY_LEFT_EXTI_IRQn            EXTI9_5_IRQn
#define DCMI_D7_Pin                   GPIO_PIN_7
#define DCMI_D7_GPIO_Port             GPIOI
#define PSRAM_NBL1_Pin                GPIO_PIN_1
#define PSRAM_NBL1_GPIO_Port          GPIOE
#define ARD_D11_Pin                   GPIO_PIN_5
#define ARD_D11_GPIO_Port             GPIOB
#define PSRAM_NE_Pin                  GPIO_PIN_9
#define PSRAM_NE_GPIO_Port            GPIOG
#define D2_Pin                        GPIO_PIN_0
#define D2_GPIO_Port                  GPIOD
#define ARD_D6_Pin                    GPIO_PIN_6
#define ARD_D6_GPIO_Port              GPIOI
#define SPI2_MISO_Pin                 GPIO_PIN_2
#define SPI2_MISO_GPIO_Port           GPIOI
#define SPI2_CLK_Pin                  GPIO_PIN_1
#define SPI2_CLK_GPIO_Port            GPIOI
#define ARD_D3_Pin                    GPIO_PIN_15
#define ARD_D3_GPIO_Port              GPIOH
#define DCMI_D3_Pin                   GPIO_PIN_12
#define DCMI_D3_GPIO_Port             GPIOH
#define ARD_D4_Pin                    GPIO_PIN_11
#define ARD_D4_GPIO_Port              GPIOI
#define USART1_TX_Pin                 GPIO_PIN_6
#define USART1_TX_GPIO_Port           GPIOB
#define ARD_D8_Pin                    GPIO_PIN_15
#define ARD_D8_GPIO_Port              GPIOG
#define OE_Pin                        GPIO_PIN_4
#define OE_GPIO_Port                  GPIOD
#define D3_Pin                        GPIO_PIN_1
#define D3_GPIO_Port                  GPIOD
#define ARD_D9_Pin                    GPIO_PIN_13
#define ARD_D9_GPIO_Port              GPIOH
#define MDM_SIM_SELECT_1_Pin          GPIO_PIN_3
#define MDM_SIM_SELECT_1_GPIO_Port    GPIOI
#define JOY_UP_Pin                    GPIO_PIN_8
#define JOY_UP_GPIO_Port              GPIOI
#define JOY_UP_EXTI_IRQn              EXTI9_5_IRQn
#define SAI1_FSA_Pin                  GPIO_PIN_4
#define SAI1_FSA_GPIO_Port            GPIOE
#define SAI1_SDB_Pin                  GPIO_PIN_3
#define SAI1_SDB_GPIO_Port            GPIOE
#define SAI1_MCKA_Pin                 GPIO_PIN_2
#define SAI1_MCKA_GPIO_Port           GPIOE
#define ARD_D5_Pin                    GPIO_PIN_9
#define ARD_D5_GPIO_Port              GPIOB
#define I2C1_SDA_Pin                  GPIO_PIN_7
#define I2C1_SDA_GPIO_Port            GPIOB
#define UART1_RX_Pin                  GPIO_PIN_10
#define UART1_RX_GPIO_Port            GPIOG
#define WE_Pin                        GPIO_PIN_5
#define WE_GPIO_Port                  GPIOD
#define uSD_CMD_Pin                   GPIO_PIN_2
#define uSD_CMD_GPIO_Port             GPIOD
#define uSD_D2_Pin                    GPIO_PIN_10
#define uSD_D2_GPIO_Port              GPIOC
#define DCMI_D5_Pin                   GPIO_PIN_4
#define DCMI_D5_GPIO_Port             GPIOI
#define DCMI_D0_Pin                   GPIO_PIN_9
#define DCMI_D0_GPIO_Port             GPIOH
#define USB_OTGFS_DP_Pin              GPIO_PIN_12
#define USB_OTGFS_DP_GPIO_Port        GPIOA
#define JOY_SEL_Pin                   GPIO_PIN_13
#define JOY_SEL_GPIO_Port             GPIOC
#define SAI1_SDA_Pin                  GPIO_PIN_6
#define SAI1_SDA_GPIO_Port            GPIOE
#define DCMI_D6_Pin                   GPIO_PIN_5
#define DCMI_D6_GPIO_Port             GPIOE
#define UART1_CTS_Pin                 GPIO_PIN_11
#define UART1_CTS_GPIO_Port           GPIOG
#define DEBUG_TRACE_CMD_Pin           GPIO_PIN_6
#define DEBUG_TRACE_CMD_GPIO_Port     GPIOD
#define MDM_PWR_EN_Pin                GPIO_PIN_3
#define MDM_PWR_EN_GPIO_Port          GPIOD
#define uSD_D3_Pin                    GPIO_PIN_11
#define uSD_D3_GPIO_Port              GPIOC
#define DCMI_VSYNC_Pin                GPIO_PIN_5
#define DCMI_VSYNC_GPIO_Port          GPIOI
#define USB_OTGFS_DM_Pin              GPIO_PIN_11
#define USB_OTGFS_DM_GPIO_Port        GPIOA
#define PSRAM_A2_Pin                  GPIO_PIN_2
#define PSRAM_A2_GPIO_Port            GPIOF
#define PSRAM_A1_Pin                  GPIO_PIN_1
#define PSRAM_A1_GPIO_Port            GPIOF
#define PSRAM_A0_Pin                  GPIO_PIN_0
#define PSRAM_A0_GPIO_Port            GPIOF
#define UART1_RTS_Pin                 GPIO_PIN_12
#define UART1_RTS_GPIO_Port           GPIOG
#define LCD_NE_Pin                    GPIO_PIN_7
#define LCD_NE_GPIO_Port              GPIOD
#define uSD_CLK_Pin                   GPIO_PIN_12
#define uSD_CLK_GPIO_Port             GPIOC
#define USB_OTGFS_ID_Pin              GPIO_PIN_10
#define USB_OTGFS_ID_GPIO_Port        GPIOA
#define USB_OTGFS_VBUS_Pin            GPIO_PIN_9
#define USB_OTGFS_VBUS_GPIO_Port      GPIOA
#define PSRAM_A3_Pin                  GPIO_PIN_3
#define PSRAM_A3_GPIO_Port            GPIOF
#define PSRAM_A4_Pin                  GPIO_PIN_4
#define PSRAM_A4_GPIO_Port            GPIOF
#define PSRAM_A5_Pin                  GPIO_PIN_5
#define PSRAM_A5_GPIO_Port            GPIOF
#define ARD_D2_Pin                    GPIO_PIN_13
#define ARD_D2_GPIO_Port              GPIOG
#define DCMI_CLK_Pin                  GPIO_PIN_8
#define DCMI_CLK_GPIO_Port            GPIOA
#define uSD_D1_Pin                    GPIO_PIN_9
#define uSD_D1_GPIO_Port              GPIOC
#define uSD_D0_Pin                    GPIO_PIN_8
#define uSD_D0_GPIO_Port              GPIOC
#define ARD_D7_Pin                    GPIO_PIN_6
#define ARD_D7_GPIO_Port              GPIOG
#define MDM_SIM_RST_Pin               GPIO_PIN_7
#define MDM_SIM_RST_GPIO_Port         GPIOC
#define ARD_A3_Pin                    GPIO_PIN_10
#define ARD_A3_GPIO_Port              GPIOF
#define ARD_A0_Pin                    GPIO_PIN_4
#define ARD_A0_GPIO_Port              GPIOC
#define PSRAM_A11_Pin                 GPIO_PIN_1
#define PSRAM_A11_GPIO_Port           GPIOG
#define D7_Pin                        GPIO_PIN_10
#define D7_GPIO_Port                  GPIOE
#define QSPI_BK1_NCS_Pin              GPIO_PIN_11
#define QSPI_BK1_NCS_GPIO_Port        GPIOB
#define ARD_D0_Pin                    GPIO_PIN_8
#define ARD_D0_GPIO_Port              GPIOG
#define ARD_D1_Pin                    GPIO_PIN_7
#define ARD_D1_GPIO_Port              GPIOG
#define D1_Pin                        GPIO_PIN_15
#define D1_GPIO_Port                  GPIOD
#define ARD_A5_Pin                    GPIO_PIN_0
#define ARD_A5_GPIO_Port              GPIOC
#define MDM_SIM_SELECT_0_Pin          GPIO_PIN_2
#define MDM_SIM_SELECT_0_GPIO_Port    GPIOC
#define PSRAM_A10_Pin                 GPIO_PIN_0
#define PSRAM_A10_GPIO_Port           GPIOG
#define D6_Pin                        GPIO_PIN_9
#define D6_GPIO_Port                  GPIOE
#define D12_Pin                       GPIO_PIN_15
#define D12_GPIO_Port                 GPIOE
#define PSRAM_A15_Pin                 GPIO_PIN_5
#define PSRAM_A15_GPIO_Port           GPIOG
#define PSRAM_A14_Pin                 GPIO_PIN_4
#define PSRAM_A14_GPIO_Port           GPIOG
#define PSRAM_A13_Pin                 GPIO_PIN_3
#define PSRAM_A13_GPIO_Port           GPIOG
#define PSRAM_A12_Pin                 GPIO_PIN_2
#define PSRAM_A12_GPIO_Port           GPIOG
#define D15_Pin                       GPIO_PIN_10
#define D15_GPIO_Port                 GPIOD
#define ARD_A2_Pin                    GPIO_PIN_3
#define ARD_A2_GPIO_Port              GPIOC
#define MDM_DTR_Pin                   GPIO_PIN_0
#define MDM_DTR_GPIO_Port             GPIOA
#define ARD_D13_Pin                   GPIO_PIN_5
#define ARD_D13_GPIO_Port             GPIOA
#define QSPI_BK1_IO1_Pin              GPIO_PIN_0
#define QSPI_BK1_IO1_GPIO_Port        GPIOB
#define PSRAM_A9_Pin                  GPIO_PIN_15
#define PSRAM_A9_GPIO_Port            GPIOF
#define D5_Pin                        GPIO_PIN_8
#define D5_GPIO_Port                  GPIOE
#define D11_Pin                       GPIO_PIN_14
#define D11_GPIO_Port                 GPIOE
#define I2C2_SCL_Pin                  GPIO_PIN_4
#define I2C2_SCL_GPIO_Port            GPIOH
#define D0_Pin                        GPIO_PIN_14
#define D0_GPIO_Port                  GPIOD
#define PSRAM_A17_Pin                 GPIO_PIN_12
#define PSRAM_A17_GPIO_Port           GPIOD
#define PSRAM_A16_Pin                 GPIO_PIN_11
#define PSRAM_A16_GPIO_Port           GPIOD
#define PSRAM_A18_LCD_RS_Pin          GPIO_PIN_13
#define PSRAM_A18_LCD_RS_GPIO_Port    GPIOD
#define MDM_SIM_CLK_Pin               GPIO_PIN_4
#define MDM_SIM_CLK_GPIO_Port         GPIOA
#define QSPI_BK1_IO2_Pin              GPIO_PIN_7
#define QSPI_BK1_IO2_GPIO_Port        GPIOA
#define QSPI_BK1_IO0_Pin              GPIO_PIN_1
#define QSPI_BK1_IO0_GPIO_Port        GPIOB
#define PSRAM_A8_Pin                  GPIO_PIN_14
#define PSRAM_A8_GPIO_Port            GPIOF
#define D4_Pin                        GPIO_PIN_7
#define D4_GPIO_Port                  GPIOE
#define D10_Pin                       GPIO_PIN_13
#define D10_GPIO_Port                 GPIOE
#define DCMI_PIXCK_Pin                GPIO_PIN_5
#define DCMI_PIXCK_GPIO_Port          GPIOH
#define D14_Pin                       GPIO_PIN_9
#define D14_GPIO_Port                 GPIOD
#define D13_Pin                       GPIO_PIN_8
#define D13_GPIO_Port                 GPIOD
#define QSPI_CLK_Pin                  GPIO_PIN_3
#define QSPI_CLK_GPIO_Port            GPIOA
#define QSPI_BK1_IO3_Pin              GPIO_PIN_6
#define QSPI_BK1_IO3_GPIO_Port        GPIOA
#define JOY_RIGHT_Pin                 GPIO_PIN_11
#define JOY_RIGHT_GPIO_Port           GPIOF
#define JOY_RIGHT_EXTI_IRQn           EXTI15_10_IRQn
#define PSRAM_A7_Pin                  GPIO_PIN_13
#define PSRAM_A7_GPIO_Port            GPIOF
#define D9_Pin                        GPIO_PIN_12
#define D9_GPIO_Port                  GPIOE
#define DCMI_D1_Pin                   GPIO_PIN_10
#define DCMI_D1_GPIO_Port             GPIOH
#define DCMI_D2_Pin                   GPIO_PIN_11
#define DCMI_D2_GPIO_Port             GPIOH
#define SPI2_MOSI_Pin                 GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port           GPIOB
#define I2C2_SDA_Pin                  GPIO_PIN_14
#define I2C2_SDA_GPIO_Port            GPIOB
#define DEBUG_TRACE_TX_Pin            GPIO_PIN_2
#define DEBUG_TRACE_TX_GPIO_Port      GPIOA
#define ARD_A4_Pin                    GPIO_PIN_1
#define ARD_A4_GPIO_Port              GPIOA
#define MDM_RST_Pin                   GPIO_PIN_2
#define MDM_RST_GPIO_Port             GPIOB
#define PSRAM_A6_Pin                  GPIO_PIN_12
#define PSRAM_A6_GPIO_Port            GPIOF
#define D8_Pin                        GPIO_PIN_11
#define D8_GPIO_Port                  GPIOE
#define SAI1_CKA_Pin                  GPIO_PIN_10
#define SAI1_CKA_GPIO_Port            GPIOB
#define DCMI_HSYNC_Pin                GPIO_PIN_8
#define DCMI_HSYNC_GPIO_Port          GPIOH
#define MDM_SIM_DATA_Pin              GPIO_PIN_12
#define MDM_SIM_DATA_GPIO_Port        GPIOB
#define LED1_Pin                      GPIO_PIN_13
#define LED1_GPIO_Port                GPIOB
/* USER CODE BEGIN Private defines */

extern RTC_HandleTypeDef xHrtc;
extern RNG_HandleTypeDef xHrng;
extern uint8_t payload_selector;
extern uint8_t format_selector;
extern uint8_t packets_selector;
extern uint8_t delay_publish_sel[ 4 ];
extern uint8_t delay_handshake_sel[ 4 ];
extern uint8_t delay_disconnect_sel[ 4 ];
extern int32_t delay_publish;
extern int32_t delay_handshake;
extern int32_t delay_disconnect;
extern int32_t packets_to_send;


/* USER CODE END Private defines */

#ifdef __cplusplus
    }
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
