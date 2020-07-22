/**
  ******************************************************************************
  * @file    sysctrl.c
  * @author  MCD Application Team
  * @brief   This file provides code for generic System Control
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
#include "sysctrl.h"
#include "at_custom_modem_api.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_SYSCTRL == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "SysCtrl:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "SysCtrl ERROR:" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("SysCtrl ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_SYSCTRL */

/* Private variables ---------------------------------------------------------*/
/* AT custom functions ptrs table */
static sysctrl_funcPtrs_t sysctrl_custom_func[DEVTYPE_INVALID] = {0U};

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
sysctrl_status_t SysCtrl_getDeviceDescriptor(sysctrl_device_type_t device_type, sysctrl_info_t *p_devices_list)
{
  sysctrl_status_t retval;

  /* check input parameters validity */
  if (p_devices_list == NULL)
  {
    retval = SCSTATUS_ERROR;
  }
  else
  {
    /* check if device is already initialized */
    if (sysctrl_custom_func[device_type].initialized == 0U)
    {
      /* Init SysCtrl functions pointers */
      (void) atcma_init_sysctrl_func_ptrs(&sysctrl_custom_func[device_type]);

      /* device is initialized now */
      sysctrl_custom_func[device_type].initialized = 1U;
    }
    retval = (*sysctrl_custom_func[device_type].f_getDeviceDescriptor)(device_type, p_devices_list);
  }

  return (retval);
}

sysctrl_status_t SysCtrl_open_channel(sysctrl_device_type_t device_type)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_open_channel)(device_type);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

sysctrl_status_t SysCtrl_close_channel(sysctrl_device_type_t device_type)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_close_channel)(device_type);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

sysctrl_status_t SysCtrl_power_on(sysctrl_device_type_t device_type)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_power_on)(device_type);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

sysctrl_status_t SysCtrl_power_off(sysctrl_device_type_t device_type)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_power_off)(device_type);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

sysctrl_status_t SysCtrl_reset_device(sysctrl_device_type_t device_type)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_reset_device)(device_type);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

sysctrl_status_t SysCtrl_sim_select(sysctrl_device_type_t device_type, sysctrl_sim_slot_t sim_slot)
{
  sysctrl_status_t retval = SCSTATUS_ERROR;

  /* check if device is initialized */
  if (sysctrl_custom_func[device_type].initialized == 1U)
  {
    retval = (*sysctrl_custom_func[device_type].f_sim_select)(device_type, sim_slot);
  }
  else
  {
    PRINT_ERR("Device type %d is not initialized", device_type)
  }

  return (retval);
}

void SysCtrl_delay(uint32_t timeMs)
{
#if (RTOS_USED == 1)
  (void) osDelay(timeMs);
#else
  HAL_Delay(timeMs);
#endif /* RTOS_USED */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
