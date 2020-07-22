/**
  ******************************************************************************
  * @file    cellular_service_power.c
  * @author  MCD Application Team
  * @brief   This file defines functions for Cellular Service Power
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
#if (USE_LOW_POWER == 1)
#include "cellular_service.h"
#include "cellular_datacache.h"
#include "cellular_service_task.h"
#include "cellular_service_power.h"
#include "cmsis_os_misrac2012.h"
#include "cellular_service_os.h"
#include "plf_power_config.h"

#include "dc_common.h"
#include "error_handler.h"


/* Private macros ------------------------------------------------------------*/

#if (USE_PRINTF == 0U)
/* Trace macro definition */
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, "" format "", ## args)

#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)                (void)printf(format , ## args);
#endif  /* (USE_PRINTF == 1) */
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#define PRINT_CELLULAR_SERVICE(format, args...)       \
  TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, format, ## args)
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)   \
  TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_ERR, "ERROR " format, ## args)
#else
#define PRINT_CELLULAR_SERVICE(format, args...)       (void)printf(format , ## args);
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)   (void)printf(format , ## args);
#endif /* (USE_PRINTF == 0U) */
#else
#define PRINT_CELLULAR_SERVICE(...)        __NOP(); /* Nothing to do */
#define PRINT_CELLULAR_SERVICE_ERR(...)    __NOP(); /* Nothing to do */
#endif /* (USE_TRACE_CELLULAR_SERVICE == 1U) */

/* Private defines -----------------------------------------------------------*/



#define CSP_CMD_PARAM_MAX        5U     /* number max of cmd param        */

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  CSP_LOW_POWER_DISABLED        = 0,     /*!< Low power not enabled          */
  CSP_LOW_POWER_INACTIVE        = 1,     /*!< Low power not active           */
  CSP_LOW_POWER_ON_GOING        = 2,     /*!< Low power activation requested */
  CSP_LOW_POWER_ACTIVE          = 3     /*!< Low power active                */
} CSP_PowerState_t;

typedef struct
{
  CSP_PowerState_t power_state;
}  CSP_Context_t;

/* Private variables ---------------------------------------------------------*/
static osTimerId         CSP_timeout_timer_handle;
static dc_cellular_power_config_t csp_dc_power_config;
static CSP_Context_t     CSP_Context;
/*  mutual exclusion */
/* static osMutexId         CSP_mutex = NULL; */

/* Global variables ----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void CSP_TimeoutTimerCallback(void const *argument);
static void CSP_ArmTimeout(uint32_t timeout);
static void CSP_SleepRequest(uint32_t timeout);



/* Private function Definition -----------------------------------------------*/


/* ============================================================ */
/* ==== STM32 STUB BEGIN ======= */
/* ============================================================ */
static void STM32_SleepRequest(void);
static void STM32_Wakeup(void);

static void STM32_SleepRequest(void)
{
  PRINT_CELLULAR_SERVICE("STM32_SleepRequest\n\r")
}
static void STM32_Wakeup(void)
{
  PRINT_CELLULAR_SERVICE("STM32_Wakeup\n\r")
}
/* ============================================================ */
/* ==== STM32 STUB END ======= */
/* ============================================================ */
/**
  * @brief  timer callback to check sleep activation
  * @param  argument - argument (not used)
  * @retval none
  */
static void CSP_TimeoutTimerCallback(void const *argument)
{
  UNUSED(argument);
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_TimeoutTimerCallback\n\r")
  CSP_Context.power_state = CSP_LOW_POWER_INACTIVE;
  (void)CS_SleepCancel();
  CST_send_message(CST_MESSAGE_CS_EVENT, CST_POWER_SLEEP_TIMEOUT_EVENT);
}

/**
  * @brief  low power leaved
  * @param  none
  * @retval none
  */
void CSP_WakeupComplete(void)
{
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_WakeupComplete\n\r")
  CSP_Context.power_state = CSP_LOW_POWER_INACTIVE;
}

/**
  * @brief  CSP call power mngt
  * @param  none
  * @retval error code
  */
static void CSP_ArmTimeout(uint32_t timeout)
{
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_ArmTimeout\n\r")
  (void)osTimerStart(CSP_timeout_timer_handle, timeout);
}

/**
  * @brief  enter in low power mode
  * @note  called by cellular service task automaton
  * @param  none
  * @retval error code
  */
void CSP_SleepRequest(uint32_t timeout)
{
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  CS_Status_t cs_status ;
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_SleepRequest\n\r")

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  cs_status = osCDS_suspend_data();
  if (cs_status == CELLULAR_OK)
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
  {
    CSP_ArmTimeout(timeout);
    (void)CS_SleepRequest();
    STM32_SleepRequest();
  }
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  else
  {
    CST_send_message(CST_MESSAGE_CMD, CST_POWER_SLEEP_ABORT_EVENT);
  }
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
}

/**
  * @brief  enter in low power mode
  * @note  called by cellular service task automaton
  * @param  none
  * @retval error code
  */
void CSP_DataIdleManagment(void)
{

  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_DataIdleManagment\n\r")

  switch (csp_dc_power_config.power_mode)
  {
    case DC_POWER_RUN_REAL_TIME:
    {
      break;
    }
    case DC_POWER_RUN_INTERACTIVE_0:
    case DC_POWER_RUN_INTERACTIVE_1:
    case DC_POWER_RUN_INTERACTIVE_2:
    case DC_POWER_RUN_INTERACTIVE_3:
    {
      CSP_SleepRequest(csp_dc_power_config.sleep_request_timeout);
      break;
    }
    case DC_POWER_IDLE:
    {
      CSP_SleepRequest(csp_dc_power_config.sleep_request_timeout);
      break;
    }
    case DC_POWER_IDLE_LP:
    {
      CSP_SleepRequest(csp_dc_power_config.sleep_request_timeout);
      break;
    }
    case DC_POWER_LP:
    {
      CSP_SleepRequest(csp_dc_power_config.sleep_request_timeout);
      break;
    }
    case DC_POWER_ULP:
    {
      CSP_SleepRequest(csp_dc_power_config.sleep_request_timeout);
      break;
    }
    default:
    {
      /* Nothing to do */
      __NOP();
      break;
    }
  }

}


/**
  * @brief  enter in low power mode request
  * @param  none
  * @retval error code
  */
CS_Status_t CSP_DataIdle(void)
{
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_DataIdle\n\r")
  CS_Status_t status;
  status = CELLULAR_OK;
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_POWER_CONFIG, (void *)&csp_dc_power_config,
                    sizeof(dc_cellular_power_config_t));
  if (csp_dc_power_config.rt_state == DC_SERVICE_ON)
  {
    if ((csp_dc_power_config.power_mode == DC_POWER_RUN_INTERACTIVE_0)
        || (csp_dc_power_config.power_mode == DC_POWER_RUN_INTERACTIVE_1)
        || (csp_dc_power_config.power_mode == DC_POWER_RUN_INTERACTIVE_2)
        || (csp_dc_power_config.power_mode == DC_POWER_RUN_INTERACTIVE_3)
        || (csp_dc_power_config.power_mode == DC_POWER_IDLE)
        || (csp_dc_power_config.power_mode == DC_POWER_IDLE_LP)
        || (csp_dc_power_config.power_mode == DC_POWER_LP)
        || (csp_dc_power_config.power_mode == DC_POWER_ULP))
    {
      /*      mutual exclusion  */
      /*      (void)osMutexWait(CSP_mutex, RTOS_WAIT_FOREVER);  */
      if (CSP_Context.power_state == CSP_LOW_POWER_INACTIVE)
      {
        CSP_Context.power_state = CSP_LOW_POWER_ON_GOING;
        CST_send_message(CST_MESSAGE_CMD, CST_POWER_SLEEP_REQUEST_EVENT);
      }
      else
      {
        status  = CELLULAR_ERROR;
      }
    }
  }
  else
  {
    status  = CELLULAR_ERROR;
  }

  return status;
}

/**
  * @brief  enter in low power mode request
  * @param  none
  * @retval error code
  */
void CSP_SleepComplete(void)
{
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_SleepComplete\n\r")
  if (CSP_Context.power_state == CSP_LOW_POWER_ON_GOING)
  {
    (void)osTimerStop(CSP_timeout_timer_handle);
    (void)CS_SleepComplete();
    CSP_Context.power_state = CSP_LOW_POWER_ACTIVE;
  }
}

/**
  * @brief  exit from low power mode request
  * @param  none
  * @retval error code
  */

CS_Status_t CSP_DataWakeup(void)
{
  CS_Status_t status;
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_DataWakeup\n\r")
  /*      mutual exclusion  */
  /*  (void)osMutexRelease(CSP_mutex); */
  if (CSP_Context.power_state == CSP_LOW_POWER_ACTIVE)
  {
    STM32_Wakeup();
    (void)CS_PowerWakeup();

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    status = osCDS_resume_data();
#else
    status = CELLULAR_OK;
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

    CST_send_message(CST_MESSAGE_CMD, CST_POWER_WAKEUP_EVENT);
  }
  else
  {
    status  = CELLULAR_ERROR;
  }
  return status;
}

/**
  * @brief  set power config
  * @param  none
  * @retval error code
  */

void CSP_SetPowerConfig(void)
{
  CS_set_power_config_t cs_power_config;
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_SetPowerConfig\n\r")
  if (CSP_Context.power_state != CSP_LOW_POWER_DISABLED)
  {
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_POWER_CONFIG, (void *)&csp_dc_power_config,
                      sizeof(dc_cellular_power_config_t));
    if ((csp_dc_power_config.rt_state == DC_SERVICE_ON) && (csp_dc_power_config.power_cmd == DC_POWER_CMD_SETTING))
    {
      if (csp_dc_power_config.psm_present == true)
      {
        cs_power_config.psm_present              = CELLULAR_TRUE;
        cs_power_config.psm.req_periodic_RAU     = csp_dc_power_config.psm.req_periodic_RAU;
        cs_power_config.psm.req_GPRS_READY_timer = csp_dc_power_config.psm.req_GPRS_READY_timer;
        cs_power_config.psm.req_periodic_TAU     = csp_dc_power_config.psm.req_periodic_TAU;
        cs_power_config.psm.req_active_time      = csp_dc_power_config.psm.req_active_time;
      }
      else
      {
        cs_power_config.psm_present = CELLULAR_FALSE;
      }

      if (csp_dc_power_config.edrx_present == true)
      {
        cs_power_config.edrx_present      = CELLULAR_TRUE;
        cs_power_config.edrx.act_type     = csp_dc_power_config.edrx.act_type;
        cs_power_config.edrx.req_value    = csp_dc_power_config.edrx.req_value;
      }
      else
      {
        cs_power_config.edrx_present = CELLULAR_FALSE;
      }

      switch (csp_dc_power_config.power_mode)
      {
        case DC_POWER_RUN_REAL_TIME:
        {
          /*  eDRX disable */
          cs_power_config.edrx_mode = EDRX_MODE_DISABLE;
          /*  PSM disable */
          cs_power_config.psm_mode  = PSM_MODE_DISABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_RUN_INTERACTIVE_0:
        {
          /*  eDRX disable */
          cs_power_config.edrx_mode = EDRX_MODE_DISABLE;
          /*  PSM disable */
          cs_power_config.psm_mode  = PSM_MODE_DISABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_RUN_INTERACTIVE_1:
        {
          /*  eDRX disable */
          cs_power_config.edrx_mode = EDRX_MODE_DISABLE;
          /*  PSM enable */
          cs_power_config.psm_mode  = PSM_MODE_ENABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_RUN_INTERACTIVE_2:
        {
          /*  eDRX enable */
          cs_power_config.edrx_mode = PSM_MODE_ENABLE;
          /*  PSM enable */
          cs_power_config.psm_mode  = PSM_MODE_ENABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_RUN_INTERACTIVE_3:
        {
          /*  eDRX enable */
          cs_power_config.edrx_mode = PSM_MODE_ENABLE;
          /*  PSM disable */
          cs_power_config.psm_mode  = EDRX_MODE_DISABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_IDLE:
        {
          /*  eDRX disable */
          cs_power_config.edrx_mode = EDRX_MODE_DISABLE;
          /*  PSM disable */
          cs_power_config.psm_mode  = PSM_MODE_DISABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_IDLE_LP:
        {
          /*  eDRX enable */
          cs_power_config.edrx_mode = EDRX_MODE_ENABLE;
          /* set eDRX parameters*/

          /*  PSM disable */
          cs_power_config.psm_mode  = PSM_MODE_DISABLE;
          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_LP:
        {
          /*  eDRX enable */
          cs_power_config.edrx_mode = EDRX_MODE_ENABLE;
          /* set eDRX parameters*/

          /*  PSM enable */
          cs_power_config.psm_mode  = PSM_MODE_ENABLE;
          /* set PSM parameters */

          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        case DC_POWER_ULP:
        {
          /*  eDRX enable */
          cs_power_config.edrx_mode = EDRX_MODE_ENABLE;
          /* set eDRX parameters*/

          /*  PSM enable */
          cs_power_config.psm_mode  = PSM_MODE_ENABLE;
          /* set PSM parameters */

          (void)CS_SetPowerConfig(&cs_power_config);
          break;
        }
        default:
        {
          /* Nothing to do */
          __NOP();
          break;
        }
      }
    }
  }
}

/**
  * @brief  CS power feature init
  * @param  none
  * @retval error code
  */

void CSP_Init(void)
{
  /* register all cellular entries of Data Cache */
  CSP_Context.power_state = CSP_LOW_POWER_DISABLED;

  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_Init\n\r")

  /* register default values in data cache */
  /* Note: these values can be overloaded by application between cellular_init()
        and cellula_start() calls */
  csp_dc_power_config.rt_state                  = DC_SERVICE_ON;
  csp_dc_power_config.power_cmd                 = DC_POWER_CMD_INIT;
  csp_dc_power_config.power_mode                = DC_POWER_MODE_DEFAULT;
  csp_dc_power_config.sleep_request_timeout     = DC_POWER_SLEEP_REQUEST_TIMEOUT_DEFAULT;

  csp_dc_power_config.psm_present               = true;
  csp_dc_power_config.psm.req_periodic_RAU      = DC_POWER_PSM_REQ_PERIODIC_RAU_DEFAULT;
  csp_dc_power_config.psm.req_GPRS_READY_timer  = DC_POWER_PSM_REQ_GPRS_READY_TIMER_DEFAULT;
  csp_dc_power_config.psm.req_periodic_TAU      = DC_POWER_PSM_REQ_PERIODIC_TAU_DEFAULT ;
  csp_dc_power_config.psm.req_active_time       = DC_POWER_PSM_REQ_ACTIVE_TIMER_DEFAULT ;

  csp_dc_power_config.edrx_present              = true;
  csp_dc_power_config.edrx.act_type             = DC_POWER_EDRX_ACT_TYPE_DEFAULT;
  csp_dc_power_config.edrx.req_value            = DC_POWER_EDRX_REQ_VALUE_DEFAULT;

  (void)dc_com_write(&dc_com_db, DC_CELLULAR_POWER_CONFIG, (void *)&csp_dc_power_config,
                     sizeof(dc_cellular_power_config_t));
}


/**
  * @brief  initializes power config (set default values)
  * @param  none
  * @retval error code
  */

void CSP_InitPowerConfig(void)
{
  CS_init_power_config_t cs_power_config;
  CS_Status_t status;

  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_InitPowerConfig\n\r")
  if (CSP_Context.power_state == CSP_LOW_POWER_DISABLED)
  {
    /*      mutual exclusion  */
    /*    osMutexDef(CSP_mutex_def);  */
    /*    CSP_mutex = osMutexCreate(osMutex(CSP_mutex_def));  */

    (void)dc_com_read(&dc_com_db, DC_CELLULAR_POWER_CONFIG, (void *)&csp_dc_power_config,
                      sizeof(dc_cellular_power_config_t));

    if ((csp_dc_power_config.rt_state != DC_SERVICE_ON)
        || (csp_dc_power_config.psm_present  != true)
        || (csp_dc_power_config.edrx_present != true))

    {
      CSP_Context.power_state = CSP_LOW_POWER_DISABLED;
    }
    else
    {
      cs_power_config.low_power_enable         = CELLULAR_TRUE;
      cs_power_config.psm.req_periodic_RAU     = csp_dc_power_config.psm.req_periodic_RAU;
      cs_power_config.psm.req_GPRS_READY_timer = csp_dc_power_config.psm.req_GPRS_READY_timer;
      cs_power_config.psm.req_periodic_TAU     = csp_dc_power_config.psm.req_periodic_TAU;
      cs_power_config.psm.req_active_time      = csp_dc_power_config.psm.req_active_time;

      cs_power_config.edrx.act_type            = csp_dc_power_config.edrx.act_type;
      cs_power_config.edrx.req_value           = csp_dc_power_config.edrx.req_value;
      status = CS_InitPowerConfig(&cs_power_config);

      if (status == CELLULAR_OK)
      {
        CSP_Context.power_state = CSP_LOW_POWER_INACTIVE;
      }
    }
  }
}

/**
  * @brief  CS power feature start
  * @param  none
  * @retval error code
  */

void CSP_Start(void)
{
  PRINT_CELLULAR_SERVICE("++++++++++++++++ CSP_Start\n\r")
  /* init timer for timeout management */
  /* creates timer */
  osTimerDef(CSP_timeout_timer, CSP_TimeoutTimerCallback);
  CSP_timeout_timer_handle = osTimerCreate(osTimer(CSP_timeout_timer), osTimerOnce, NULL);
}
#endif  /* (USE_LOW_POWER == 1) */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

