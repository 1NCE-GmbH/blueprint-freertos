/**
 ******************************************************************************
 * File Name          : USART.c
 * Description        : This file provides code for the configuration
 *                      of the USART instances.
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
#include "usart.h"
#include "cellular_config_defaults.h"
#include "cellular_comm_interface.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef xConsoleUart;

/* LPUART1 init function */

void MX_LPUART1_UART_Init( void )
{
    hlpuart1.Instance = LPUART1;
    hlpuart1.Init.BaudRate = 115200;
    hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
    hlpuart1.Init.StopBits = UART_STOPBITS_1;
    hlpuart1.Init.Parity = UART_PARITY_NONE;
    hlpuart1.Init.Mode = UART_MODE_TX_RX;
    hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if( HAL_UART_Init( &hlpuart1 ) != HAL_OK )
    {
        Error_Handler();
    }
}
/* USART1 init function */

void MX_USART1_UART_Init( void )
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if( HAL_UART_Init( &huart1 ) != HAL_OK )
    {
        Error_Handler();
    }
}
/* USART2 init function */

void MX_USART2_UART_Init( void )
{
    xConsoleUart.Instance = USART2;
    xConsoleUart.Init.BaudRate = 115200;
    xConsoleUart.Init.WordLength = UART_WORDLENGTH_8B;
    xConsoleUart.Init.StopBits = UART_STOPBITS_1;
    xConsoleUart.Init.Parity = UART_PARITY_NONE;
    xConsoleUart.Init.Mode = UART_MODE_TX_RX;
    xConsoleUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    xConsoleUart.Init.OverSampling = UART_OVERSAMPLING_16;
    xConsoleUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    xConsoleUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if( HAL_UART_Init( &xConsoleUart ) != HAL_OK )
    {
        Error_Handler();
    }
}

/* get a char from the console uart  */
int32_t menu_utils_get_uart_char( uint8_t * car,
                                  uint8_t amount )
{
    int32_t ret;
    HAL_StatusTypeDef ret_hal;

    ret_hal = HAL_UART_Receive( &xConsoleUart, car, amount, 0xFFFFFFU );

    while( ret_hal == HAL_BUSY )
    {
        ( void ) osDelay( 10U );
        ret_hal = HAL_UART_Receive( &xConsoleUart, car, amount, 0xFFFFFFU );
    }

    if( ret_hal == HAL_OK )
    {
        ret = 1;
    }
    else
    {
        *car = 0U;
        ret = 0;
    }

    return ret;
}


void HAL_UART_MspInit( UART_HandleTypeDef * uartHandle )
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    if( uartHandle->Instance == LPUART1 )
    {
        /* USER CODE BEGIN LPUART1_MspInit 0 */

        /* USER CODE END LPUART1_MspInit 0 */
        /* LPUART1 clock enable */
        __HAL_RCC_LPUART1_CLK_ENABLE();


        /* __HAL_RCC_GPIOG_CLK_ENABLE(); */
        /* HAL_PWREx_EnableVddIO2(); */

        /**LPUART1 GPIO Configuration
         * PG8     ------> LPUART1_RX
         * PG7     ------> LPUART1_TX
         */
        GPIO_InitStruct.Pin = ARD_D0_Pin | ARD_D1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
    }
    else if( uartHandle->Instance == USART1 )
    {
        /* Enable UART Clock */
        __HAL_RCC_USART1_CLK_ENABLE();

        /* UART interrupt init */
        HAL_NVIC_SetPriority( USART1_IRQn, 6, 0 );
        HAL_NVIC_EnableIRQ( USART1_IRQn );
    }
    else if( uartHandle->Instance == USART2 )
    {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        /* __HAL_RCC_GPIOD_CLK_ENABLE(); */
        /* __HAL_RCC_GPIOA_CLK_ENABLE(); */

        /**USART2 GPIO Configuration
         * PD6     ------> USART2_RX
         * PA2     ------> USART2_TX
         */
        GPIO_InitStruct.Pin = DEBUG_TRACE_CMD_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init( DEBUG_TRACE_CMD_GPIO_Port, &GPIO_InitStruct );

        GPIO_InitStruct.Pin = DEBUG_TRACE_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init( DEBUG_TRACE_TX_GPIO_Port, &GPIO_InitStruct );

/*    / * USART2 interrupt Init * / */
        HAL_NVIC_SetPriority( USART2_IRQn, 7, 0 );
        HAL_NVIC_EnableIRQ( USART2_IRQn );/*dom */
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit( UART_HandleTypeDef * uartHandle )
{
    if( uartHandle->Instance == LPUART1 )
    {
        /* USER CODE BEGIN LPUART1_MspDeInit 0 */

        /* USER CODE END LPUART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPUART1_CLK_DISABLE();

        /**LPUART1 GPIO Configuration
         * PG8     ------> LPUART1_RX
         * PG7     ------> LPUART1_TX
         */
        HAL_GPIO_DeInit( GPIOG, ARD_D0_Pin | ARD_D1_Pin );

        /* LPUART1 interrupt Deinit */
        HAL_NVIC_DisableIRQ( LPUART1_IRQn );
        /* USER CODE BEGIN LPUART1_MspDeInit 1 */

        /* USER CODE END LPUART1_MspDeInit 1 */
    }
    else if( uartHandle->Instance == USART1 )
    {
        /* UART interrupt deinit */
        HAL_NVIC_DisableIRQ( USART1_IRQn );

        /* Disable UART Clock */
        __HAL_RCC_USART1_CLK_DISABLE();

        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
         * PB6     ------> USART1_TX
         * PG10     ------> USART1_RX
         * PG11     ------> USART1_CTS
         * PG12     ------> USART1_RTS
         */

        /* USART1 interrupt Deinit */
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
    else if( uartHandle->Instance == USART2 )
    {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
         * PD6     ------> USART2_RX
         * PA2     ------> USART2_TX
         */
        HAL_GPIO_DeInit( DEBUG_TRACE_CMD_GPIO_Port, DEBUG_TRACE_CMD_Pin );

        HAL_GPIO_DeInit( DEBUG_TRACE_TX_GPIO_Port, DEBUG_TRACE_TX_Pin );

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ( USART2_IRQn );
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */


void vMainUARTPrintString( char * pcString )
{
    const uint32_t ulTimeout = 3000UL;

    HAL_UART_Transmit( &xConsoleUart,
                       ( uint8_t * ) pcString,
                       strlen( pcString ),
                       ulTimeout );
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
