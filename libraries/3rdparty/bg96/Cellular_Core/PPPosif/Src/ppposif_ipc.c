/**
  ******************************************************************************
  * @file    ppposif_ipc.c
  * @author  MCD Application Team
  * @brief   Interface between ppposif and IPC
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
#include "ppposif_ipc.h"
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)

#include "cmsis_os_misrac2012.h"
#include "ipc_uart.h"
#include "main.h"
#include "error_handler.h"
#include "plf_config.h"

/* Private defines -----------------------------------------------------------*/
#define SND_MAX 800


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  IPC_Handle_t *ipcHandle;
  osSemaphoreId     rcvSemaphore;
  osSemaphoreId     sndSemaphore;
  __IO uint32_t TransmitChar;
  __IO uint32_t rcvSemaphoreFlag;
  __IO uint32_t sndSemaphoreFlag;
  __IO uint32_t TransmitOnGoing;
  u8_t              snd_buff[SND_MAX];
} ppposif_ipc_ctx_t;

/* Private macros ------------------------------------------------------------*/
#define PPPOSIF_IPC_SEMAPHORE_DEF(name,index)  \
  const osSemaphoreDef_t os_semaphore_def_##name##index = { 0 }
#define PPPOSIF_IPC_SEMAPHORE(name,index)  \
  &os_semaphore_def_##name##index

/* Private variables ---------------------------------------------------------*/
static ppposif_ipc_ctx_t ppposif_ipc_ctx[2];
static IPC_Handle_t   IPC_Handle[2] ;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void IPC_MessageSentCallback(IPC_Handle_t *ipcHandle);
static void IPC_MessageReceivedCallback(IPC_Handle_t *ipcHandle);


/* Functions Definition ------------------------------------------------------*/
/* Private function Definition -----------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
static void IPC_MessageSentCallback(IPC_Handle_t *ipcHandle)
{
  ppposif_ipc_ctx[ipcHandle->Device_ID].TransmitOnGoing = 0U;
  (void)osSemaphoreRelease(ppposif_ipc_ctx[ipcHandle->Device_ID].sndSemaphore);
}

/**
  * @brief  Rx  IPC completed callback
  * @param  clienthandle: IPC handle
  * @retval None
  */
static void IPC_MessageReceivedCallback(IPC_Handle_t *ipcHandle)
{
  /* Warning ! this function is called under IT */
  (void)osSemaphoreRelease(ppposif_ipc_ctx[ipcHandle->Device_ID].rcvSemaphore);
  ppposif_ipc_ctx[ipcHandle->Device_ID].rcvSemaphoreFlag = 1U;
}

/**
  * @brief  component init
  * @param  pDevice: device to init.
  * @retval return code
  */

/* Exported function Definition -----------------------------------------------*/
void ppposif_ipc_init(IPC_Device_t pDevice)
{
  ppposif_ipc_ctx[pDevice].ipcHandle         = &IPC_Handle[pDevice];
  ppposif_ipc_ctx[pDevice].TransmitChar      = 0U;
  ppposif_ipc_ctx[pDevice].rcvSemaphoreFlag  = 0U;
  ppposif_ipc_ctx[pDevice].sndSemaphoreFlag  = 0U;
  ppposif_ipc_ctx[pDevice].TransmitOnGoing   = 0U;

  PPPOSIF_IPC_SEMAPHORE_DEF(SEM_UART_RCV, pDevice);
  ppposif_ipc_ctx[pDevice].rcvSemaphore = osSemaphoreCreate(PPPOSIF_IPC_SEMAPHORE(SEM_UART_RCV, pDevice), 10000);
  if (ppposif_ipc_ctx[pDevice].rcvSemaphore == NULL)
  {
    ERROR_Handler(DBG_CHAN_PPPOSIF, 11, ERROR_FATAL);
  }

  PPPOSIF_IPC_SEMAPHORE_DEF(SEM_UART_SND, pDevice);
  ppposif_ipc_ctx[pDevice].sndSemaphore = osSemaphoreCreate(PPPOSIF_IPC_SEMAPHORE(SEM_UART_SND, pDevice), 1);
  (void)osSemaphoreWait(ppposif_ipc_ctx[pDevice].sndSemaphore, RTOS_WAIT_FOREVER);

#if defined(AT_TEST)
  if (pDevice == IPC_DEVICE_0)
  {
    IPC_init(pDevice, IPC_INTERFACE_UART, &MODEM_UART_HANDLE);
  }
#endif  /* defined(AT_TEST) */

  (void)IPC_open(&IPC_Handle[pDevice],  pDevice, IPC_MODE_UART_STREAM, IPC_MessageReceivedCallback,
                 IPC_MessageSentCallback, NULL);
}

/**
  * @brief  data IPC channel select
  * @param  pDevice: device to init.
  * @retval return code
  */

void ppposif_ipc_select(IPC_Device_t pDevice)
{
  (void)IPC_select(&IPC_Handle[pDevice]);
}

/**
  * @brief  component de init
  * @param  pDevice: device to de init.
  * @retval return code
  */

void ppposif_ipc_deinit(IPC_Device_t pDevice)
{
  UNUSED(pDevice);
}

/**
  * @brief  Rcv data
  * @param  pDevice: serial device.
  * @param  data: buffer data to read.
  * @param  len: data size to read.
  * @retval data rcv byte number
  */

int16_t ppposif_ipc_read(IPC_Device_t pDevice, u8_t *buff, int16_t size)
{

  ppposif_ipc_ctx[pDevice].rcvSemaphoreFlag = 2U;
  (void)osSemaphoreWait(ppposif_ipc_ctx[pDevice].rcvSemaphore, RTOS_WAIT_FOREVER);
  ppposif_ipc_ctx[pDevice].rcvSemaphoreFlag = 0U;
  __disable_irq();
  (void)IPC_streamReceive(ppposif_ipc_ctx[pDevice].ipcHandle, buff, &size);
  __enable_irq();

  return size;
}

/**
  * @brief  Tx Send data
  * @param  pDevice: device .
  * @param  data: buffer data to send.
  * @param  len: data size to send.
  * @retval data sent byte number
  */
int16_t ppposif_ipc_write(IPC_Device_t pDevice, u8_t *data, int16_t len)
{
  int16_t temp_len;
  temp_len = len;
  IPC_Status_t status;
  ppposif_ipc_ctx[pDevice].TransmitOnGoing = 1U;

  /*   for(int i=0; i<temp_len; i++) ppposif_ipc_ctx[pDevice].snd_buff[i] = data[i];*/

  status = IPC_send(ppposif_ipc_ctx[pDevice].ipcHandle, (uint8_t *) data, (uint16_t)temp_len);
  if (status != IPC_OK)
  {
    temp_len = 0;
  }
  else
  {
    ppposif_ipc_ctx[pDevice].sndSemaphoreFlag = 1U;
    (void)osSemaphoreWait(ppposif_ipc_ctx[pDevice].sndSemaphore, RTOS_WAIT_FOREVER);

    ppposif_ipc_ctx[pDevice].TransmitChar += (uint16_t)temp_len;
    ppposif_ipc_ctx[pDevice].sndSemaphoreFlag = 0U;
  }
  return temp_len;
}
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



