/**
  ******************************************************************************
  * @file    sysctrl_specific.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code for System control of
  *          UG96 Quectel modem (3G)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) YYYY STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sysctrl.h"
#include "sysctrl_specific.h"
#include "ipc_common.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_SYSCTRL == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) TRACE_PRINT_FORCE(DBG_CHAN_ATCMD, DBL_LVL_P0, format "\n\r", ## args)
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "SysCtrl_UG96:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR,\
                                                "SysCtrl_UG96 ERROR:" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...) (void) printf(format "\n\r", ## args);
#define PRINT_INFO(format, args...)  (void) printf("SysCtrl_UG96:" format "\n\r", ## args);
#define PRINT_ERR(format, args...)   (void) printf("SysCtrl_UG96 ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) TRACE_PRINT_FORCE(DBG_CHAN_ATCMD, DBL_LVL_P0, format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...) (void) printf(format "\n\r", ## args);
#endif /* USE_PRINTF */
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)    __NOP(); /* Nothing to do */
#endif /* USE_TRACE_SYSCTRL */

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static sysctrl_status_t SysCtrl_UG96_setup(void);

/* Functions Definition ------------------------------------------------------*/
sysctrl_status_t SysCtrl_UG96_getDeviceDescriptor(sysctrl_device_type_t type, sysctrl_info_t *p_devices_list)
{
  sysctrl_status_t retval;

  if (p_devices_list == NULL)
  {
    retval = SCSTATUS_ERROR;
  }
  else
  {

    /* check type */
    if (type == DEVTYPE_MODEM_CELLULAR)
    {
#if defined(USE_MODEM_UG96)
      p_devices_list->type          = DEVTYPE_MODEM_CELLULAR;
      p_devices_list->ipc_device    = USER_DEFINED_IPC_DEVICE_MODEM;
      p_devices_list->ipc_interface = IPC_INTERFACE_UART;

      (void) IPC_init(p_devices_list->ipc_device, p_devices_list->ipc_interface, &MODEM_UART_HANDLE);
      retval = SCSTATUS_OK;
#else
      retval = SCSTATUS_ERROR;
#endif /* USE_MODEM_UG96 */
    }
    else
    {
      retval = SCSTATUS_ERROR;
    }
  }
  if (retval != SCSTATUS_ERROR)
  {
    (void) SysCtrl_UG96_setup();
  }

  return (retval);
}

sysctrl_status_t SysCtrl_UG96_open_channel(sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  /* UART configuration */
  MODEM_UART_HANDLE.Instance = MODEM_UART_INSTANCE;
  MODEM_UART_HANDLE.Init.BaudRate = MODEM_UART_BAUDRATE;
  MODEM_UART_HANDLE.Init.WordLength = MODEM_UART_WORDLENGTH;
  MODEM_UART_HANDLE.Init.StopBits = MODEM_UART_STOPBITS;
  MODEM_UART_HANDLE.Init.Parity = MODEM_UART_PARITY;
  MODEM_UART_HANDLE.Init.Mode = MODEM_UART_MODE;
  MODEM_UART_HANDLE.Init.HwFlowCtl = MODEM_UART_HWFLOWCTRL;
  MODEM_UART_HANDLE.Init.OverSampling = UART_OVERSAMPLING_16;
  MODEM_UART_HANDLE.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  MODEM_UART_HANDLE.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  /* UART initialization */
  if (HAL_UART_Init(&MODEM_UART_HANDLE) != HAL_OK)
  {
    retval = SCSTATUS_ERROR;
  }

  /* Enable the UART IRQn */
  HAL_NVIC_EnableIRQ(MODEM_UART_IRQN);

  return (retval);
}

sysctrl_status_t SysCtrl_UG96_close_channel(sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  /* Disable the UART IRQn */
  HAL_NVIC_DisableIRQ(MODEM_UART_IRQN);

  /* UART deinitialization */
  if (HAL_UART_DeInit(&MODEM_UART_HANDLE) != HAL_OK)
  {
    PRINT_ERR("HAL_UART_DeInit error")
    retval = SCSTATUS_ERROR;
  }

  return (retval);
}

sysctrl_status_t SysCtrl_UG96_power_on(sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  /* Reference: Quectel BG96&EG95&UG96&M95 R2.0_Compatible_Design_V1.0
  *  PWRKEY   connected to MODEM_PWR_EN (inverse pulse)
  *  RESET_N  connected to MODEM_RST    (inverse)
  *
  * Turn ON module sequence (cf paragraph 4.2)
  *
  *          PWRKEY  RESET_N  modem_state
  * init       0       0        OFF
  * T=0        1       1        OFF
  * T1=100     0       1        BOOTING
  * T1+100     1       1        BOOTING
  * T1+3500    1       1        RUNNING
  */

  /* Set PWR_EN to 1 during at least 100 ms */
  HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_PORT, MODEM_PWR_EN_PIN, GPIO_PIN_SET);
  SysCtrl_delay(150U);
  /* Set PWR_EN to 0 during at least 100ms (200ms) then PWR_EN = 0 */
  HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_PORT, MODEM_PWR_EN_PIN, GPIO_PIN_RESET);
  SysCtrl_delay(200U);
  HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_PORT, MODEM_PWR_EN_PIN, GPIO_PIN_SET);

  /* Waits for Modem to complete its booting procedure */
#if (UG96_FASTEST_POWER_ON == 0U)
  PRINT_INFO("Waiting 4000 msec for modem running...")
  SysCtrl_delay(4000U);
  PRINT_INFO("...done")
#else
  PRINT_INFO("start without delay")
#endif /* (UG96_FASTEST_POWER_ON == 0U) */

  return (retval);
}

sysctrl_status_t SysCtrl_UG96_power_off(sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  /* Reference: Quectel BG96&EG95&UG96&M95 R2.0_Compatible_Design_V1.0
  *  PWRKEY   connected to MODEM_PWR_EN (inverse pulse)
  *  RESET_N  connected to MODEM_RST    (inverse)
  *
  * Turn OFF module sequence (cf paragraph 4.3)
  *
  * Need to use AT+QPOWD command
  * reset GPIO pins to initial state only after completion of previous command (between 2s and 40s)
  */
  HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_PORT, MODEM_PWR_EN_PIN, GPIO_PIN_RESET);
  SysCtrl_delay(5000U);

  return (retval);
}

sysctrl_status_t SysCtrl_UG96_reset(sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  /* Reference: Quectel BG96&EG95&UG96&M95 R2.0_Compatible_Design_V1.0
  *  PWRKEY   connected to MODEM_PWR_EN (inverse pulse)
  *  RESET_N  connected to MODEM_RST    (inverse)
  *
  * Reset module sequence (cf paragraph 4.4)
  *
  * Can be done using RESET_N pin to low voltage for 100ms minimum
  *
  *          RESET_N  modem_state
  * init       1        RUNNING
  * T=0        0        OFF
  * T=100      1        BOOTING
  * T>=5000    1        RUNNING
  */
  PRINT_INFO("!!! Hardware Reset triggered !!!")

  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_SET);
  SysCtrl_delay(100U);
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_RESET);

  return (retval);
}


sysctrl_status_t SysCtrl_UG96_sim_select(sysctrl_device_type_t type, sysctrl_sim_slot_t sim_slot)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;

  switch (sim_slot)
  {
    case SC_MODEM_SIM_SOCKET_0:
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_0_GPIO_PORT, MODEM_SIM_SELECT_0_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_1_GPIO_PORT, MODEM_SIM_SELECT_1_PIN, GPIO_PIN_RESET);
      PRINT_INFO("MODEM SIM SOCKET SELECTED")
      break;

    case SC_MODEM_SIM_ESIM_1:
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_0_GPIO_PORT, MODEM_SIM_SELECT_0_PIN, GPIO_PIN_SET);
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_1_GPIO_PORT, MODEM_SIM_SELECT_1_PIN, GPIO_PIN_RESET);
      PRINT_INFO("MODEM SIM ESIM SELECTED")
      break;

    case SC_STM32_SIM_2:
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_0_GPIO_PORT, MODEM_SIM_SELECT_0_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(MODEM_SIM_SELECT_1_GPIO_PORT, MODEM_SIM_SELECT_1_PIN, GPIO_PIN_SET);
      PRINT_INFO("STM32 SIM SELECTED")
      break;

    default:
      PRINT_ERR("Invalid SIM %d selected", sim_slot)
      retval = SCSTATUS_ERROR;
      break;
  }

  return (retval);
}
/* Private function Definition -----------------------------------------------*/
static sysctrl_status_t SysCtrl_UG96_setup(void)
{
  sysctrl_status_t retval = SCSTATUS_OK;

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO config
   * Initial pins state
   */

  /* GPIO config
   * Initial pins state:
   *  PWR_EN initial state = 1 : used to power-on/power-off the board
   *  RST initial state = 0 : used to reset the board
   *  DTR initial state = 0 ; not used
   */
  HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_PORT, MODEM_PWR_EN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MODEM_DTR_GPIO_PORT, MODEM_DTR_PIN, GPIO_PIN_RESET);


  /* Init GPIOs - common parameters */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  /* Init GPIOs - RESET pin */
  GPIO_InitStruct.Pin = MODEM_RST_PIN;
  HAL_GPIO_Init(MODEM_RST_GPIO_PORT, &GPIO_InitStruct);

  /* Init GPIOs - DTR pin */
  GPIO_InitStruct.Pin = MODEM_DTR_PIN;
  HAL_GPIO_Init(MODEM_DTR_GPIO_PORT, &GPIO_InitStruct);

  /* Init GPIOs - PWR_EN pin */
  GPIO_InitStruct.Pin = MODEM_PWR_EN_PIN;
  HAL_GPIO_Init(MODEM_PWR_EN_GPIO_PORT, &GPIO_InitStruct);

  PRINT_FORCE("UG96 UART config: BaudRate=%d / HW flow ctrl=%d",
              MODEM_UART_BAUDRATE, ((MODEM_UART_HWFLOWCTRL == UART_HWCONTROL_NONE) ? 0 : 1))

  return (retval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

