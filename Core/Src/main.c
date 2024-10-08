/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "iot_config.h"

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdint.h"
#include "stdarg.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
/* Demo Includes*/
#include "cellular_app.h"

#ifdef IOT_LOG_LEVEL_MAIN
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_DEBUG
    #endif
#endif
/*#include "cmsis_os.h" */
/*#include "i2c.h" */
#include "usart.h"
#include "rng.h"
#include "rtc.h"
#include "gpio.h"


#define MAX_RETRY_ATTEMPTS    5      /* Maximum number of retries */
#define RETRY_DELAY_MS        10000  /* Delay between retry attempts in milliseconds */


/* The SPI driver polls at a high priority. The logging task's priority must also
 * be high to be not be starved of CPU time. */
#define mainLOGGING_TASK_PRIORITY           ( configMAX_PRIORITIES )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 4 )
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )
#define main_RUNNER_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 8 )


/* Heap 2 size for malloc. */
#define HEAP2_SIZE    ( 27 * 1024 )
void vApplicationDaemonTaskStartupHook( void );
extern void RunDemoTask( void );
extern int setupCellular( void );

/**********************
* Global Variables
**********************/
RTC_HandleTypeDef xHrtc;
RNG_HandleTypeDef xHrng;
uint8_t payload_selector;

int32_t delay_publish = 60 * 1000;

/* Private define ------------------------------------------------------------*/
static void SystemClock_Config( void );

/**
 * @brief Initializes the STM32L475 IoT node board.
 *
 * Initialization of clock, LEDs, RNG, RTC, and Cellular module.
 */
static void prvMiscInitialization( void );

/**
 * @brief Initializes the FreeRTOS heap.
 *
 * Heap_5 is being used because the RAM is not contiguous, therefore the heap
 * needs to be initialized.  See http://www.freertos.org/a00111.html
 */
static void prvInitializeHeap( void );

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( void );
void MX_FREERTOS_Init( void );

/* Private user code ---------------------------------------------------------*/
static void CellularDemoTask()
{
    bool retCellular = true;

    /* Setup cellular. */
    retCellular = setupCellular();

    if( !retCellular )
    {
        int retries = 0;

        /* Retry cellular setup with a maximum number of attempts */
        while( ( retries < MAX_RETRY_ATTEMPTS ) && ( !retCellular ) )
        {
            /* Set pin PD3 to high, signaling the attempt */
            HAL_GPIO_WritePin( GPIOD, GPIO_PIN_3, GPIO_PIN_SET );

            /* Delay between retries */
            HAL_Delay( RETRY_DELAY_MS );

            /* Attempt to set up the cellular connection */
            retCellular = setupCellular();

            retries++;
        }

        /* If still unsuccessful after retries, prompt user for manual intervention */
        if( !retCellular )
        {
            /* Optionally, reset the system if necessary (uncomment if used) */
            LogInfo( "System will reset to resolve the issue.\r\n" );
            HAL_NVIC_SystemReset();
            return;
        }
    }

/* Stop here if we fail to initialize cellular. */
    configASSERT( retCellular == true );

    LogInfo( "---- START DEMO : ----- .\r\n" );


    RunDemoTask();
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main( void )
{
    /* Perform any hardware initialization that does not require the RTOS to be
     * running.  */
    prvMiscInitialization();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();


    /* Create tasks that are not dependent on the Cellular being initialized. */
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            mainLOGGING_TASK_PRIORITY,
                            mainLOGGING_MESSAGE_QUEUE_LENGTH );

    /* Start the scheduler.  Initialization that requires the OS to be running,
     */
    vTaskStartScheduler();

    return 0;
}


/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    if( SYSTEM_Init() == pdPASS )
    {
        xTaskCreate( CellularDemoTask,            /* Function that implements the task. */
                     "CellularDemo",              /* Text name for the task - only used for debugging. */
                     main_RUNNER_TASK_STACK_SIZE, /* Size of stack (in words, not bytes) to allocate for the task. */
                     NULL,                        /* Task parameter - not used in this case. */
                     configMAX_PRIORITIES - 2,    /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                     NULL );                      /* Used to pass out a handle to the created task - not used in this case. */
    }
    else
    {
        IotLogError( "System failed to initialize.\r\n" );
        return;
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Initializes the board.
 */
static void prvMiscInitialization( void )
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock. */
    SystemClock_Config();

    /* Heap_5 is being used because the RAM is not contiguous in memory, so the
     * heap must be initialized. */
    prvInitializeHeap();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    /* Disable initialization of Modem-UART.
     * It will be enabled only when requested by upper layers.
     * MX_USART1_UART_Init();
     */
    MX_USART1_UART_Init();
    MX_RTC_Init();
    MX_RNG_Init();
}


/*-----------------------------------------------------------*/

/* Psuedo random number generator.  Just used by demos so does not need to be
 * secure.  Do not use the standard C library rand() function as it can cause
 * unexpected behaviour, such as calls to malloc(). */
int iMainRand32( void )
{
    static UBaseType_t uxlNextRand; /*_RB_ Not seeded. */
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    uxlNextRand = ( ulMultiplier * uxlNextRand ) + ulIncrement;

    return( ( int ) ( uxlNextRand >> 16UL ) & 0x7fffUL );
}

static void prvInitializeHeap( void )
{
    static uint8_t ucHeap1[ configTOTAL_HEAP_SIZE ];
    static uint8_t ucHeap2[ HEAP2_SIZE ] __attribute__( ( section( ".freertos_heap2" ) ) );

    HeapRegion_t xHeapRegions[] =
    {
        { ( unsigned char * ) ucHeap2, sizeof( ucHeap2 ) },
        { ( unsigned char * ) ucHeap1, sizeof( ucHeap1 ) },
        { NULL,                        0                 }
    };

    vPortDefineHeapRegions( xHeapRegions );
}

/*-----------------------------------------------------------*/

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK )
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1
                                         | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_LPUART1
                                         | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_RNG;

    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
    PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
    PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
    PeriphClkInit.PLLSAI1.PLLSAI1N = 20;
    PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;

    if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
    {
        Error_Handler();
    }

    /** Configure the main internal regulator output voltage
     */
    if( HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 ) != HAL_OK )
    {
        Error_Handler();
    }

    /**Configure the Systick interrupt time
     */
    HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq() / 1000 );

    /**Configure the Systick
     */
    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority( SysTick_IRQn, 15, 0 );
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    static TickType_t xLastPrint = 0;
    TickType_t xTimeNow;
    const TickType_t xPrintFrequency = pdMS_TO_TICKS( 30000 );

    xTimeNow = xTaskGetTickCount();

    if( ( xTimeNow - xLastPrint ) > xPrintFrequency )
    {
        IotLogInfo( "." );
        xLastPrint = xTimeNow;
    }
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM3 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef * htim )
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if( htim->Instance == TIM3 )
    {
        HAL_IncTick();
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler( void )
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while( 1 )
    {
    }

    /* USER CODE END Error_Handler_Debug */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
