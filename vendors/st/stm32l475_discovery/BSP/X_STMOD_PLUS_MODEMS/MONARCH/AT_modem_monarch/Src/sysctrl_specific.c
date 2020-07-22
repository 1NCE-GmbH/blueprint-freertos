/**
  ******************************************************************************
  * @file    sysctrl_specific.c
  * @author  MCD Application Team
  * @brief   This file provides all the specific code for System control of
  *          Sequans MONARCH modem
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
/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_SYSCTRL == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) TRACE_PRINT_FORCE(DBG_CHAN_ATCMD, DBL_LVL_P0, format "\n\r", ## args)
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "SysCtrl_MONARCH:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR,\
                                                "SysCtrl_MONARCH ERROR:" format "\n\r", ## args)
#elif (USE_PRINTF == 1)
#include <stdio.h>
#define PRINT_FORCE(format, args...) (void) printf(format "\n\r", ## args);
#define PRINT_INFO(format, args...)  (void) printf("SysCtrl_MONARCH:" format "\n\r", ## args);
#define PRINT_ERR(format, args...)   (void) printf("SysCtrl_MONARCH ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...) (void) printf(format "\n\r", ## args);
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)    __NOP(); /* Nothing to do */
#endif /* USE_TRACE_SYSCTRL */

/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static sysctrl_status_t SysCtrl_MONARCH_setup(void);

static void monarch_LP_disable_modem_uart(void);
static void monarch_LP_enable_modem_uart(void);

/* Functions Definition ------------------------------------------------------*/
sysctrl_status_t SysCtrl_MONARCH_getDeviceDescriptor(sysctrl_device_type_t type, sysctrl_info_t *p_devices_list)
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
#if defined(USE_MODEM_GM01Q)
      p_devices_list->type          = DEVTYPE_MODEM_CELLULAR;;
      p_devices_list->ipc_device    = USER_DEFINED_IPC_DEVICE_MODEM;
      p_devices_list->ipc_interface = IPC_INTERFACE_UART;

      (void) IPC_init(p_devices_list->ipc_device, p_devices_list->ipc_interface, &MODEM_UART_HANDLE);
      retval = SCSTATUS_OK;
#else
      retval = SCSTATUS_ERROR;
#endif /* USE_MODEM_GM01Q */
    }
    else
    {
      retval = SCSTATUS_ERROR;
    }
  }

  if (retval != SCSTATUS_ERROR)
  {
    (void) SysCtrl_MONARCH_setup();
  }

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_open_channel(sysctrl_device_type_t type)
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
  /* do not activate autobaud (not compatible with current implementation) */
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

sysctrl_status_t SysCtrl_MONARCH_close_channel(sysctrl_device_type_t type)
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

sysctrl_status_t SysCtrl_MONARCH_power_on(sysctrl_device_type_t type)
{
  (void)type;
  sysctrl_status_t retval = SCSTATUS_OK;

  /* Release Modem Reset: set xRST to LOW */
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_RESET);

  /* Waits for modem to complete its booting procedure */
  PRINT_INFO("Waiting 4000 msec for modem running...")
  SysCtrl_delay(4000U);
  PRINT_INFO("...done")

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_power_off(sysctrl_device_type_t type)
{
  (void)type;
  sysctrl_status_t retval = SCSTATUS_OK;

  /* set xRST to HIGH  */
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_SET);

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_reset(sysctrl_device_type_t type)
{
  (void)type;
  sysctrl_status_t retval = SCSTATUS_OK;

  /* set xRST to LOW */
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_SET);
  SysCtrl_delay(1000);
  /* set xRST to HIGH */
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_RESET);

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_sim_select(sysctrl_device_type_t type, sysctrl_sim_slot_t sim_slot)
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

static void monarch_LP_disable_modem_uart(void)
{
  PRINT_INFO("monarch_LP_disable_modem_uart");
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Un-configure modem UART GPIO pins */
  GPIO_InitStruct.Pin = MODEM_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MODEM_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_RX_PIN;
  HAL_GPIO_Init(MODEM_RX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_CTS_PIN;
  HAL_GPIO_Init(MODEM_CTS_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_RTS_PIN;
  HAL_GPIO_Init(MODEM_RTS_GPIO_PORT, &GPIO_InitStruct);

  /* Re-configure RTS pin to allow to trigger Low Power */
  GPIO_InitStruct.Pin = MODEM_RTS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MODEM_RTS_GPIO_PORT, &GPIO_InitStruct);

}

static void monarch_LP_enable_modem_uart(void)
{
  PRINT_INFO("monarch_LP_enable_modem_uart");
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Re-configure modem UART GPIO pins : */
  GPIO_InitStruct.Pin = MODEM_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = MODEM_UART_ALTERNATE;
  HAL_GPIO_Init(MODEM_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_RX_PIN;
  HAL_GPIO_Init(MODEM_RX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_CTS_PIN;
  HAL_GPIO_Init(MODEM_CTS_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MODEM_RTS_PIN;
  HAL_GPIO_Init(MODEM_RTS_GPIO_PORT, &GPIO_InitStruct);
}

sysctrl_status_t SysCtrl_MONARCH_suspend_channel_request(IPC_Handle_t *ipc_handle, sysctrl_device_type_t type)
{
  UNUSED(ipc_handle);
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;
  PRINT_INFO("modem channel suspend request")

  /* NOTE
   *   for Monarch, we are using the RTS pin to request modem enter in Low Power
   *   To do this, we reconfigure the UART GPIOs (Note: could be avoided if HW flow control not used...)
   *   As a consequence, we can not use UART anymore !
   *   This is why we call SysCtrl_MONARCH_close_channel() immediatly and do not wait for
   *   SysCtrl_MONARCH_suspend_channel_complete as we are doing for other modems.
   */
  (void)SysCtrl_MONARCH_close_channel(type);

  /* Reconfigure GPIOs:
   * disable modem UART GPIO and enable STM32-RTS as GPIO output to control Low Power
   */
  monarch_LP_disable_modem_uart();

  PRINT_INFO(">>> Request modem to enter Low Power (set STM32-RTS pin to HIGH)");
  HAL_GPIO_WritePin(MODEM_RTS_GPIO_PORT, MODEM_RTS_PIN, GPIO_PIN_SET);

  /* now wait for RING pin transition from HIGH to LOW before to close UART channel
   */

#if defined(SIMULATE_MODEM_LP)
  /* simulate wait for RING instead of waiting real GPIO trigger */
  PRINT_INFO("<<< SIMULATE MODEM LP ENTRY >>> wait 5 sec but should wait for RING instead...");
  SysCtrl_delay(5000U);
#endif /* SIMULATE_MODEM_LP */

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_suspend_channel_complete(IPC_Handle_t *ipc_handle, sysctrl_device_type_t type)
{
  UNUSED(ipc_handle);
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;
  PRINT_INFO("modem channel suspend complete")

  /* RING pin transition from HIGH to LOW has been detected */

  /* Note: for other modem, we close UART channel at this point but for Monarch it has been done before
   *       due to UART reconfiguration
   */

  PRINT_INFO(">>> Complete modem enter Low Power");

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_resume_channel(IPC_Handle_t *ipc_handle, sysctrl_device_type_t type)
{
  UNUSED(type);
  sysctrl_status_t retval = SCSTATUS_OK;
  PRINT_INFO("modem channel resume")

  PRINT_INFO(">>> Request modem to leave Low Power (set STM32-RTS pin to LOW)");
  HAL_GPIO_WritePin(MODEM_RTS_GPIO_PORT, MODEM_RTS_PIN, GPIO_PIN_RESET);

  /* Reconfigure GPIOs:
   * re- enable modem UART
   */
  monarch_LP_enable_modem_uart();
  SysCtrl_delay(15U);

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
  /* do not activate autobaud (not compatible with current implementation) */
  MODEM_UART_HANDLE.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  /* UART initialization */

  if (HAL_UART_Init(&MODEM_UART_HANDLE) != HAL_OK)
  {
    PRINT_ERR("error in HAL_UART_Init")
    retval = SCSTATUS_ERROR;
  }
  else
  {
    (void) IPC_reset(ipc_handle);
    HAL_NVIC_EnableIRQ(MODEM_UART_IRQN);
    SysCtrl_delay(100U);
    PRINT_INFO("resume channel OK ");

    /* for debug:
    PRINT_INFO("MODEM RING pin status = %d", HAL_GPIO_ReadPin(MODEM_RING_GPIO_PORT, MODEM_RING_PIN));
    PRINT_INFO("MODEM DTR pin status = %d", HAL_GPIO_ReadPin(MODEM_DTR_GPIO_PORT, MODEM_DTR_PIN));
    PRINT_INFO("MODEM UART_TX status = %d", HAL_GPIO_ReadPin(MODEM_TX_GPIO_PORT, MODEM_TX_PIN));
    PRINT_INFO("MODEM UART_RX status = %d", HAL_GPIO_ReadPin(MODEM_RX_GPIO_PORT, MODEM_RX_PIN));
    */
  }

  return (retval);
}

sysctrl_status_t SysCtrl_MONARCH_setup_LowPower_Int(uint8_t set)
{
  sysctrl_status_t retval = SCSTATUS_OK;

  if (set == 1U)
  {
    /* activate RING interrupt to detect modem enters in Low Power mode
     * falling edge
     */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MODEM_RING_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MODEM_RING_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(MODEM_RING_GPIO_PORT, MODEM_RING_PIN, GPIO_PIN_SET);

    HAL_NVIC_EnableIRQ(MODEM_RING_IRQN);
  }
  else
  {
    /* deactivate RING interrupt
     */
    HAL_NVIC_DisableIRQ(MODEM_RING_IRQN);
  }

  return (retval);
}

/* Private function Definition -----------------------------------------------*/
static sysctrl_status_t SysCtrl_MONARCH_setup(void)
{
  sysctrl_status_t retval = SCSTATUS_OK;
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO config
   * Initial pins state
   */
  HAL_GPIO_WritePin(MODEM_RST_GPIO_PORT, MODEM_RST_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = MODEM_RST_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MODEM_RST_GPIO_PORT, &GPIO_InitStruct);

  PRINT_FORCE("MONARCH UART config: BaudRate=%d / HW flow ctrl=%d", MODEM_UART_BAUDRATE,
              ((MODEM_UART_HWFLOWCTRL == UART_HWCONTROL_NONE) ? 0 : 1));

  return (retval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
