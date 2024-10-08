/**
 ******************************************************************************
 * File Name          : gpio.c
 * Description        : This file provides code for the configuration
 *                      of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
 *   PE0   ------> FMC_NBL0
 *   PB4 (NJTRST)   ------> SPI1_MISO
 *   PA15 (JTDI)   ------> SPI1_NSS
 *   PH14   ------> DCMI_D4
 *   PI7   ------> DCMI_D7
 *   PE1   ------> FMC_NBL1
 *   PB5   ------> SPI1_MOSI
 *   PG9   ------> FMC_NE2
 *   PD0   ------> FMC_D2_DA2
 *   PI6   ------> S_TIM8_CH2
 *   PI2   ------> SPI2_MISO
 *   PI1   ------> SPI2_SCK
 *   PH15   ------> TIM8_CH3N
 *   PH12   ------> DCMI_D3
 *   PD4   ------> FMC_NOE
 *   PD1   ------> FMC_D3_DA3
 *   PH13   ------> TIM8_CH1N
 *   PE4   ------> SAI1_FS_A
 *   PE3   ------> SAI1_SD_B
 *   PE2   ------> SAI1_MCLK_A
 *   PB9   ------> S_TIM4_CH4
 *   PB7   ------> I2C1_SDA
 *   PD5   ------> FMC_NWE
 *   PD2   ------> SDMMC1_CMD
 *   PC10   ------> SDMMC1_D2
 *   PI4   ------> DCMI_D5
 *   PH9   ------> DCMI_D0
 *   PA12   ------> USB_OTG_FS_DP
 *   PE6   ------> SAI1_SD_A
 *   PE5   ------> DCMI_D6
 *   PC11   ------> SDMMC1_D3
 *   PI5   ------> DCMI_VSYNC
 *   PA11   ------> USB_OTG_FS_DM
 *   PF2   ------> FMC_A2
 *   PF1   ------> FMC_A1
 *   PF0   ------> FMC_A0
 *   PD7   ------> FMC_NE1
 *   PC12   ------> SDMMC1_CK
 *   PA10   ------> USB_OTG_FS_ID
 *   PF3   ------> FMC_A3
 *   PF4   ------> FMC_A4
 *   PF5   ------> FMC_A5
 *   PA8   ------> LPTIM2_OUT
 *   PC9   ------> SDMMC1_D1
 *   PC8   ------> SDMMC1_D0
 *   PF10   ------> ADC3_IN13
 *   PC4   ------> ADCx_IN13
 *   PG1   ------> FMC_A11
 *   PE10   ------> FMC_D7_DA7
 *   PB11   ------> QUADSPI_BK1_NCS
 *   PD15   ------> FMC_D1_DA1
 *   PC0   ------> ADCx_IN1
 *   PC1   ------> ADCx_IN2
 *   PG0   ------> FMC_A10
 *   PE9   ------> FMC_D6_DA6
 *   PE15   ------> FMC_D12_DA12
 *   PG5   ------> FMC_A15
 *   PG4   ------> FMC_A14
 *   PG3   ------> FMC_A13
 *   PG2   ------> FMC_A12
 *   PD10   ------> FMC_D15_DA15
 *   PC3   ------> ADCx_IN4
 *   PA5   ------> SPI1_SCK
 *   PB0   ------> QUADSPI_BK1_IO1
 *   PF15   ------> FMC_A9
 *   PE8   ------> FMC_D5_DA5
 *   PE14   ------> FMC_D11_DA11
 *   PH4   ------> I2C2_SCL
 *   PD14   ------> FMC_D0_DA0
 *   PD12   ------> FMC_A17_ALE
 *   PD11   ------> FMC_A16_CLE
 *   PD13   ------> FMC_A18
 *   PA7   ------> QUADSPI_BK1_IO2
 *   PB1   ------> QUADSPI_BK1_IO0
 *   PF14   ------> FMC_A8
 *   PE7   ------> FMC_D4_DA4
 *   PE13   ------> FMC_D10_DA10
 *   PH5   ------> DCMI_PIXCLK
 *   PD9   ------> FMC_D14_DA14
 *   PD8   ------> FMC_D13_DA13
 *   PA3   ------> QUADSPI_CLK
 *   PA6   ------> QUADSPI_BK1_IO3
 *   PF13   ------> FMC_A7
 *   PE12   ------> FMC_D9_DA9
 *   PH10   ------> DCMI_D1
 *   PH11   ------> DCMI_D2
 *   PB15   ------> SPI2_MOSI
 *   PB14   ------> I2C2_SDA
 *   PA1   ------> ADCx_IN6
 *   PF12   ------> FMC_A6
 *   PE11   ------> FMC_D8_DA8
 *   PB10   ------> SAI1_SCK_A
 *   PH8   ------> DCMI_HSYNC
 */
void MX_GPIO_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_PWREx_EnableVddIO2();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();


    /*Configure GPIO pins : PIPin PIPin PIPin */
    GPIO_InitStruct.Pin = JOY_DOWN_Pin | JOY_LEFT_Pin | JOY_UP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = STMOD_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( STMOD_INT_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pins : PEPin PEPin PEPin PEPin
     *                       PEPin PEPin PEPin PEPin
     *                       PEPin PEPin PEPin */
    GPIO_InitStruct.Pin = PSRAM_NBL0_Pin | PSRAM_NBL1_Pin | D7_Pin | D6_Pin
                          | D12_Pin | D5_Pin | D11_Pin | D4_Pin
                          | D10_Pin | D9_Pin | D8_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

    /*Configure GPIO pins : PBPin PBPin */
    GPIO_InitStruct.Pin = ARD_D12_Pin | ARD_D11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    /*Configure GPIO pins : PAPin PAPin */
    GPIO_InitStruct.Pin = ARD_D10_Pin | ARD_D13_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    /*Configure GPIO pin : PI0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

    /*Configure GPIO pins : PHPin PHPin PHPin PHPin
     *                       PHPin PHPin PHPin */
    GPIO_InitStruct.Pin = DCMI_D4_Pin | DCMI_D3_Pin | DCMI_D0_Pin | DCMI_PIXCK_Pin
                          | DCMI_D1_Pin | DCMI_D2_Pin | DCMI_HSYNC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
    HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );

    /*Configure GPIO pins : PIPin PIPin PIPin */
    GPIO_InitStruct.Pin = DCMI_D7_Pin | DCMI_D5_Pin | DCMI_VSYNC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
    HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

    /*Configure GPIO pins : PGPin PGPin PGPin PGPin
     *                       PGPin PGPin PGPin */
    GPIO_InitStruct.Pin = PSRAM_NE_Pin | PSRAM_A11_Pin | PSRAM_A10_Pin | PSRAM_A15_Pin
                          | PSRAM_A14_Pin | PSRAM_A13_Pin | PSRAM_A12_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

    /*Configure GPIO pins : PDPin PDPin PDPin PDPin
     *                       PDPin PDPin PDPin PDPin
     *                       PDPin PDPin PDPin PDPin
     *                       PDPin */
    GPIO_InitStruct.Pin = D2_Pin | OE_Pin | D3_Pin | WE_Pin
                          | LCD_NE_Pin | D1_Pin | D15_Pin | D0_Pin
                          | PSRAM_A17_Pin | PSRAM_A16_Pin | PSRAM_A18_LCD_RS_Pin | D14_Pin
                          | D13_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ARD_D6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init( ARD_D6_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pins : PIPin PIPin */
    GPIO_InitStruct.Pin = SPI2_MISO_Pin | SPI2_CLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

    /*Configure GPIO pins : PHPin PHPin */
    GPIO_InitStruct.Pin = ARD_D3_Pin | ARD_D9_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ARD_D4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( ARD_D4_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PB8 */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    /*Configure GPIO pins : PGPin PGPin PGPin */
    GPIO_InitStruct.Pin = ARD_D8_Pin | ARD_D2_Pin | ARD_D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

    /*Configure GPIO pins : PEPin PEPin PEPin PEPin */
    GPIO_InitStruct.Pin = SAI1_FSA_Pin | SAI1_SDB_Pin | SAI1_MCKA_Pin | SAI1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
    HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ARD_D5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init( ARD_D5_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = I2C1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init( I2C1_SDA_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = uSD_CMD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init( uSD_CMD_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pins : PCPin PCPin PCPin PCPin
     *                       PCPin */
    GPIO_InitStruct.Pin = uSD_D2_Pin | uSD_D3_Pin | uSD_CLK_Pin | uSD_D1_Pin
                          | uSD_D0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    /*Configure GPIO pins : PAPin PAPin PAPin */
    GPIO_InitStruct.Pin = USB_OTGFS_DP_Pin | USB_OTGFS_DM_Pin | USB_OTGFS_ID_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = DCMI_D6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
    HAL_GPIO_Init( DCMI_D6_GPIO_Port, &GPIO_InitStruct );

    GPIO_InitStruct.Pin = PSRAM_A2_Pin | PSRAM_A1_Pin | PSRAM_A0_Pin | PSRAM_A3_Pin
                          | PSRAM_A4_Pin | PSRAM_A5_Pin | PSRAM_A9_Pin | PSRAM_A8_Pin
                          | PSRAM_A7_Pin | PSRAM_A6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );

    /*Configure GPIO pins : PC6 PC5 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    /*Configure GPIO pin : PG14 */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = DCMI_CLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LPTIM2;
    HAL_GPIO_Init( DCMI_CLK_GPIO_Port, &GPIO_InitStruct );


    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ARD_A3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( ARD_A3_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pins : PCPin PCPin PC1 PCPin */
    GPIO_InitStruct.Pin = ARD_A0_Pin | ARD_A5_Pin | GPIO_PIN_1 | ARD_A2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = I2C2_SCL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init( I2C2_SCL_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = JOY_RIGHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init( JOY_RIGHT_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = SPI2_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( SPI2_MOSI_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = I2C2_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init( I2C2_SDA_GPIO_Port, &GPIO_InitStruct );

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = ARD_A4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( ARD_A4_GPIO_Port, &GPIO_InitStruct );


    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = SAI1_CKA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
    HAL_GPIO_Init( SAI1_CKA_GPIO_Port, &GPIO_InitStruct );

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority( EXTI2_IRQn, 5, 0 );
    HAL_NVIC_DisableIRQ( EXTI2_IRQn );

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority( EXTI9_5_IRQn, 5, 0 );
    HAL_NVIC_EnableIRQ( EXTI9_5_IRQn );

    HAL_NVIC_SetPriority( EXTI15_10_IRQn, 5, 0 );
    HAL_NVIC_EnableIRQ( EXTI15_10_IRQn );
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
