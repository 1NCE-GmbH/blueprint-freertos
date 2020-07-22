/**
  ******************************************************************************
  * @file    freertos.c
  * @author  MCD Application Team
  * @brief   Default task : System init
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
#include <stdlib.h>

#include "plf_config.h"
#include "cmsis_os_misrac2012.h"

#include "error_handler.h"
#include "cellular_mngt.h"

#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#endif /* (USE_PRINTF == 0U) */

#if (USE_CELPERF == 1)
#include "celperf.h"
#endif /* (USE_CELPERF == 1) */

#if (!USE_DEFAULT_SETUP == 1)
#include "time_date.h"
#include "app_select.h"
#include "setup.h"
#endif /* (!USE_DEFAULT_SETUP == 1) */
#if (USE_MQTT_DEMO == 1)
#include "mqttdemo.h"
#endif /* (USE_MQTT_DEMO == 1) */

#if (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1)
#include "dc_mems.h"
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */

#if (STACK_ANALYSIS_TRACE == 1)
#include "stack_analysis.h"
#endif /* (STACK_ANALYSIS_TRACE == 1) */

#if (USE_DC_GENERIC == 1)
#include "dc_generic.h"
#endif /* (USE_DC_GENERIC == 1)  */


#if (USE_NETWORK_LIBRARY == 1)
#if (USE_NETEX1 == 1)
#include "netex1.h"
#endif /* (USE_NETEX1 == 1) */
#endif /* (USE_NETWORK_LIBRARY == 1) */

/* Private defines -----------------------------------------------------------*/
#define STACK_ANALYSIS_COUNTER  5U  /*  Stack analysed every 5s */

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_TEST == 1U)
#if (USE_PRINTF == 0U)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_P0, "RTOS:" (format) "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_ERR, "RTOS ERROR:" (format) "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_DBG(format, args...)   (void)printf("RTOS:" format "\n\r", ## args);
#define PRINT_ERR(format, args...)   (void)printf("RTOS ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_DBG(...)      __NOP(); /* Nothing to do */
#define PRINT_ERR(...)      __NOP(); /* Nothing to do */
#endif /* USE_TRACE_TEST */

/* Private variables ---------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void applications_init(void);
static void applications_start(void);
static void utilities_init(void);
static void utilities_start(void);

/* Global variables ----------------------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Private function Definition -----------------------------------------------*/
/**
  * @brief  Initialize the applications
  * @param  -
  * @retval -
  */
static void applications_init(void)
{
#if (USE_MQTT_DEMO == 1)
  mqttdemo_init();
#endif /* (USE_MQTT_DEMO == 1) */
}

/**
  * @brief  Initialize the utilities
  * @param  -
  * @retval -
  */
static void utilities_init(void)
{
  /* call to stackAnalysis_init() must be done earlier */

//#if (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1)
//  dc_mems_init();
//#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */
//
//#if (USE_DC_GENERIC == 1)
//  dc_generic_init();
//#endif /* (USE_DC_GENERIC == 1) */
}

/**
  * @brief  Start the applications
  * @param  -
  * @retval -
  */
static void applications_start(void)
{

#if (USE_MQTT_DEMO == 1)
    mqttdemo_start();
#endif /* (USE_MQTT_DEMO == 1) */

#if (USE_NETWORK_LIBRARY == 1)
#if (USE_NETEX1 == 1)
  netex1_start();
#endif /* (USE_NETEX1 == 1) */
#endif /* (USE_NETWORK_LIBRARY == 1) */
}

/**
  * @brief  Start the utilities
  * @param  -
  * @retval -
  */
static void utilities_start(void)
{
#if (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1)
  dc_mems_start();
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */

#if (USE_DC_GENERIC == 1)
  dc_generic_start();
#endif /* (USE_DC_GENERIC == 1) */
}

/* Functions Definition ------------------------------------------------------*/

/* Init FreeRTOS */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  static osThreadId defaultTaskHandle;

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* definition and creation of DebounceTimer */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, CTRL_THREAD_PRIO, 0, DEFAULT_THREAD_STACK_SIZE);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
  if (defaultTaskHandle == NULL)
  {
#if (USE_PRINTF == 0U)
    /* Error Handler may use trace print */
    traceIF_init();
#endif /* (USE_PRINTF == 0U)  */
    ERROR_Handler(DBG_CHAN_MAIN, 0, ERROR_FATAL);
  }
#if (STACK_ANALYSIS_TRACE == 1)
  stackAnalysis_init();
  (void)stackAnalysis_addStackSizeByHandle(defaultTaskHandle,
                                           (uint16_t)DEFAULT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* definition and creation of testTask */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}


/* StartDefaultTask function */
void StartDefaultTask(void const *argument)
{
#if (!USE_DEFAULT_SETUP == 1)
  uint16_t application_to_start;
#endif /* (!USE_DEFAULT_SETUP == 1) */
#if (STACK_ANALYSIS_TRACE == 1)
#if (STACK_ANALYSIS_PERIODICAL == 1)
  uint16_t stack_analysis_counter = 0U;
#endif /* (STACK_ANALYSIS_PERIODICAL == 1) */
#endif /* (STACK_ANALYSIS_TRACE == 1) */

  /* RandomNumberGenerator */
  srand(osKernelSysTick());

#if (USE_PRINTF == 0U)
  /* Error Handler in the modules below may use trace print */
  /* Recall traceIF_init() in case MX_FREERTOS_Init is not used or is redefined */
  traceIF_init();
#endif /* (USE_PRINTF == 0U)  */

#if (STACK_ANALYSIS_TRACE == 1)
  /* Recall stackAnalysis_init() in case MX_FREERTOS_Init is not used or is redefined */
  stackAnalysis_init();
  /* check values in task.c tskIDLE_STACK_SIZE */
  (void)stackAnalysis_addStackSizeByHandle(xTaskGetIdleTaskHandle(),
                                           (uint16_t)configMINIMAL_STACK_SIZE);
  /* check values in FreeRTOSConfig.h */
  (void)stackAnalysis_addStackSizeByHandle(xTimerGetTimerDaemonTaskHandle(),
                                           (uint16_t)configTIMER_TASK_STACK_DEPTH);
#endif /* (STACK_ANALYSIS_TRACE == 1) */


#if (!USE_DEFAULT_SETUP == 1)
  /* If menu is used, menu variables must be initialized before the rest of software */
  app_select_init();
  setup_init();
#endif /* (!USE_DEFAULT_SETUP == 1) */

  /* Cellular components statical init */
#if (USE_NETWORK_LIBRARY == 1)
  cellular_net_init();
#else
  cellular_init();
#endif /* (USE_NETWORK_LIBRARY == 1) */

  /* Application components statical init  */
  applications_init();

  /* Other optionnal components statical init */
  utilities_init();

#if (!USE_DEFAULT_SETUP == 1)
  /* If menu is used, menu will provide the type of start */
  application_to_start = app_select_start();
  if (application_to_start == (uint16_t)APP_SELECT_CST_MODEM_START)
  {
    cellular_modem_start();
  }
  else if (application_to_start == (uint16_t)APP_SELECT_FAQ)
  {
    cellular_faq_start();
  }
  else
#endif /* (!USE_DEFAULT_SETUP == 1) */
  {
#if (!USE_DEFAULT_SETUP == 1)
    if (application_to_start == (uint16_t)APP_SELECT_SETUP_CODE)
    {
      setup_start();
    }
    setup_apply();
#endif /* (!USE_DEFAULT_SETUP == 1) */

    /* Cellular components start */
#if (USE_NETWORK_LIBRARY == 1)
    cellular_net_start();
#else
    cellular_start();
#endif /* (USE_NETWORK_LIBRARY == 1) */

    /* Application components start */
    applications_start();

    /* Utilities components start */
    utilities_start();
  }
  /* Infinite loop */
  for (;;)
  {
#if (STACK_ANALYSIS_TRACE == 1)
#if (STACK_ANALYSIS_PERIODICAL == 1)
    if (stack_analysis_counter == 0U)
    {
      /* Trace Stacks after all components init */
      (void)stackAnalysis_trace(STACK_ANALYSIS_PRINT);
      stack_analysis_counter = STACK_ANALYSIS_COUNTER;
    }
    else
    {
      stack_analysis_counter--;
    }
#endif /* (STACK_ANALYSIS_PERIODICAL == 1) */
#endif /* (STACK_ANALYSIS_TRACE == 1) */
    (void)osDelay(1000U);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
