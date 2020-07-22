/**
  ******************************************************************************
  * @file    ipc_uart.c
  * @author  MCD Application Team
  * @brief   This file provides code for uart IPC
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
#include <string.h>
#include <stdbool.h>
#include "ipc_uart.h"
#include "ipc_rxfifo.h"
#include "plf_config.h"

#if (RTOS_USED == 1)
#include "cmsis_os_misrac2012.h"
#endif /* RTOS_USED */

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

#if (USE_TRACE_IPC == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_IPC, DBL_LVL_P0, "IPC:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_IPC, DBL_LVL_P1, "IPC:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_IPC, DBL_LVL_ERR, "IPC ERROR:" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_INFO(format, args...)  (void) printf("IPC:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("IPC ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_IPC */

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static uint8_t find_Device(const UART_HandleTypeDef *huart);
static IPC_Status_t change_ipc_channel(IPC_Handle_t *hipc);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  The function initialize the global variables of the IPC.
  * @param  device IPC device identifier.
  * @param  huart Handle to the HAL UART structure.
  * @retval status
  */
IPC_Status_t IPC_init_uart(IPC_Device_t  device, UART_HandleTypeDef  *huart)
{
  IPC_Status_t retval;

  /* check if this device has not been already initialized */
  if (g_IPC_Devices_List[device].state != IPC_STATE_NOT_INITIALIZED)
  {
    retval = IPC_ERROR;
  }
  else if (huart == NULL)
  {
    retval = IPC_ERROR;
  }
  else
  {
    g_IPC_Devices_List[device].state = IPC_STATE_INITIALIZED;
    g_IPC_Devices_List[device].phy_int.interface_type = IPC_INTERFACE_UART;
    g_IPC_Devices_List[device].phy_int.h_uart = huart;
    g_IPC_Devices_List[device].h_current_channel = NULL;
    g_IPC_Devices_List[device].h_inactive_channel = NULL;
    retval = IPC_OK;
  }

  return (retval);
}

/**
  * @brief  Reinitialize the IPC global variables.
  * @param  device IPC device identifier.
  * @retval status
  */
IPC_Status_t IPC_deinit_uart(IPC_Device_t  device)
{
  g_IPC_Devices_List[device].state = IPC_STATE_NOT_INITIALIZED;
  g_IPC_Devices_List[device].phy_int.interface_type = IPC_INTERFACE_UNINITIALIZED;
  g_IPC_Devices_List[device].phy_int.h_uart = NULL;
  g_IPC_Devices_List[device].h_current_channel = NULL;
  g_IPC_Devices_List[device].h_inactive_channel = NULL;

  return (IPC_OK);
}

/**
  * @brief  Open a specific channel.
  * @param  hipc IPC handle to open.
  * @param  device IPC device identifier.
  * @param  mode IPC mode (char or stream).
  * @param  pRxClientCallback Callback ptr called when a message has been received.
  * @param  pTxClientCallback Callback ptr called when a message has been send.
  * @param  pCheckEndOfMsg Callback ptr to the function used to analyze if char received is a termination char
  * @retval status
  */
IPC_Status_t IPC_open_uart(IPC_Handle_t *hipc,
                           IPC_Device_t  device,
                           IPC_Mode_t    mode,
                           IPC_RxCallbackTypeDef pRxClientCallback,
                           IPC_TxCallbackTypeDef pTxClientCallback,
                           IPC_CheckEndOfMsgCallbackTypeDef pCheckEndOfMsg)
{
  IPC_Status_t retval;
  HAL_StatusTypeDef uart_status;

  /* check the handles */
  if (hipc == NULL)
  {
    retval = IPC_ERROR;
  }
  else if (pRxClientCallback == NULL)
  {
    retval = IPC_ERROR;
  }
  else if (pTxClientCallback == NULL)
  {
    retval = IPC_ERROR;
  }
  else if ((mode != IPC_MODE_UART_CHARACTER) && (mode != IPC_MODE_UART_STREAM))
  {
    retval = IPC_ERROR;
  }
  else if ((mode == IPC_MODE_UART_CHARACTER) && (pCheckEndOfMsg == NULL))
  {
    retval = IPC_ERROR;
  }
  else
  {
    /* Register this channel into the IPC devices list */
    if (g_IPC_Devices_List[device].h_current_channel == NULL)
    {
      /* register this channel as default channel (first call) */
      g_IPC_Devices_List[device].h_current_channel = hipc;
      retval = IPC_OK;
    }
    else if (g_IPC_Devices_List[device].h_inactive_channel == NULL)
    {
      /* another channel already registered and active, register this channel as inactive */
      g_IPC_Devices_List[device].h_inactive_channel = hipc;
      retval = IPC_OK;
    }
    else
    {
      /* supports only 2 channels for same IPC device */
      retval = IPC_ERROR;
    }
  }

  if (retval != IPC_ERROR)
  {
    /* initialize common RX buffer */
    g_IPC_Devices_List[device].RxChar[0] = (IPC_CHAR_t)('\0');

    PRINT_DBG("IPC channel %p registered", g_IPC_Devices_List[device].h_current_channel)
    PRINT_DBG("state 0x%x", g_IPC_Devices_List[device].state)
    PRINT_DBG("active channel handle: %p", g_IPC_Devices_List[device].h_current_channel)
    PRINT_DBG("inactive channel handle: %p", g_IPC_Devices_List[device].h_inactive_channel)

    /* select default queue (character or stream) */
    if (mode == IPC_MODE_UART_CHARACTER)
    {
      hipc->RxFifoWrite = RXFIFO_writeCharacter;
    }
#if (IPC_USE_STREAM_MODE == 1U)
    else
    {
      hipc->RxFifoWrite = RXFIFO_writeStream;
    }
#endif /* IPC_USE_STREAM_MODE */

    /* initialize IPC channel parameters */
    hipc->Device_ID = device;
    hipc->Interface.interface_type = g_IPC_Devices_List[device].phy_int.interface_type;
    hipc->Interface.h_uart = g_IPC_Devices_List[device].phy_int.h_uart;
    hipc->State = IPC_STATE_INITIALIZED;

    /* register client callback */
    hipc->RxClientCallback = pRxClientCallback;
    hipc->TxClientCallback = pTxClientCallback;
    hipc->CheckEndOfMsgCallback = pCheckEndOfMsg;
    hipc->Mode = mode;
    hipc->UartBusyFlag = 0U;

    /* init RXFIFO */
    RXFIFO_init(hipc);
#if (IPC_USE_STREAM_MODE == 1U)
    RXFIFO_stream_init(hipc);
#endif /* IPC_USE_STREAM_MODE */

    /* start RX IT */
    uart_status = HAL_UART_Receive_IT(hipc->Interface.h_uart, (uint8_t *)g_IPC_Devices_List[device].RxChar, 1U);
    if (HAL_OK != uart_status)
    {
      PRINT_ERR("HAL_UART_Receive_IT error")
      retval = IPC_ERROR;
    }
    else
    {
      hipc->State = IPC_STATE_ACTIVE;
      retval = IPC_OK;
    }
  }
  return (retval);
}

/**
  * @brief  Close a specific channel.
  * @param  hipc IPC handle to close.
  * @retval status
  */
IPC_Status_t IPC_close_uart(IPC_Handle_t *hipc)
{
  IPC_Status_t retval;

  /* check the handles */
  if (hipc != NULL)
  {
    /* reset IPC status */
    hipc->State = IPC_STATE_NOT_INITIALIZED;
    hipc->RxClientCallback = NULL;
    hipc->CheckEndOfMsgCallback = NULL;

    /* init RXFIFO */
    RXFIFO_init(hipc);
#if (IPC_USE_STREAM_MODE == 1U)
    RXFIFO_stream_init(hipc);
#endif /* IPC_USE_STREAM_MODE */

    /* update g_IPC_Devices_List state */
    uint8_t device_id = hipc->Device_ID;
    if (device_id != IPC_DEVICE_NOT_FOUND)
    {
      if (g_IPC_Devices_List[device_id].h_current_channel == hipc)
      {
        /* if closed IPC was active, remove it and inactive IPC becomes active (or NULL) */
        g_IPC_Devices_List[device_id].h_current_channel = g_IPC_Devices_List[device_id].h_inactive_channel;
        g_IPC_Devices_List[device_id].h_inactive_channel = NULL;
      }
      else if (g_IPC_Devices_List[device_id].h_inactive_channel == hipc)
      {
        /* if closed IPC was inactive, just remove it */
        g_IPC_Devices_List[device_id].h_inactive_channel = NULL;
      }
      else
      {
        /* should not happen */
      }

      retval = IPC_OK;

      /* if no more registered channel, abort RX transfer */
      if ((g_IPC_Devices_List[device_id].h_current_channel == NULL) &&
          (g_IPC_Devices_List[device_id].h_inactive_channel == NULL))
      {
        (void)HAL_UART_AbortTransmit_IT(hipc->Interface.h_uart);
      }

      PRINT_DBG("IPC channel %p closed", hipc)
      PRINT_DBG("state 0x%x", g_IPC_Devices_List[hipc->Device_ID].state)
      PRINT_DBG("active channel handle: %p", g_IPC_Devices_List[hipc->Device_ID].h_current_channel)
      PRINT_DBG("inactive channel handle: %p", g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel)
    }
    else
    {
      /* no device found */
      retval = IPC_ERROR;
    }
  }
  else
  {
    /* no valid handle */
    retval = IPC_ERROR;
  }

  return (retval);
}

/**
  * @brief  Select current channel.
  * @param  hipc IPC handle to select.
  * @retval status
  */
IPC_Status_t IPC_select_uart(IPC_Handle_t *hipc)
{
  IPC_Status_t retval = IPC_ERROR;
  PRINT_DBG("IPC select %p", hipc)

  /* check the handle */
  if (hipc != NULL)
  {
    if (hipc != g_IPC_Devices_List[hipc->Device_ID].h_current_channel)
    {
      (void) change_ipc_channel(hipc);
    }
    retval = IPC_OK;
  }

  return (retval);
}

/**
  * @brief  Reset IPC handle to select.
  * @retval status
  */
IPC_Status_t IPC_reset_uart(IPC_Handle_t *hipc)
{
  IPC_Status_t retval = IPC_ERROR;
  PRINT_DBG("IPC reset %p", hipc)

  /* check the handle */
  if (hipc != NULL)
  {
    /* initialize common RX buffer */
    uint8_t device_id = hipc->Device_ID;
    if (device_id != IPC_DEVICE_NOT_FOUND)
    {
      g_IPC_Devices_List[device_id].RxChar[0] = (IPC_CHAR_t)('\0');

      /* init RXFIFO */
      RXFIFO_init(hipc);
#if (IPC_USE_STREAM_MODE == 1U)
      RXFIFO_stream_init(hipc);
#endif /* IPC_USE_STREAM_MODE */

      /* -- Deinit UART  and Init UART --
      if (HAL_UART_DeInit(hipc->Interface.h_uart) != HAL_OK)
      {
          retval = IPC_ERROR;
      }

      if (HAL_UART_Init(hipc->Interface.h_uart) != HAL_OK)
      {
          retval = IPC_ERROR;
      }
        */

      /* rearm IT */
      (void) HAL_UART_Receive_IT(hipc->Interface.h_uart, (uint8_t *)g_IPC_Devices_List[device_id].RxChar, 1U);
      hipc->State = IPC_STATE_ACTIVE;
      retval = IPC_OK;
    }
  }

  return (retval);
}

/**
  * @brief  Get other channel handle if exists.
  * @param  hipc IPC handle.
  * @retval IPC_Handle_t*
  */
IPC_Handle_t *IPC_get_other_channel_uart(const IPC_Handle_t *hipc)
{
  IPC_Handle_t *handle = NULL;

  if ((g_IPC_Devices_List[hipc->Device_ID].h_current_channel == hipc) &&
      (g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel != NULL))
  {
    handle = g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel;
  }

  if ((g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel == hipc) &&
      (g_IPC_Devices_List[hipc->Device_ID].h_current_channel != NULL))
  {
    handle = g_IPC_Devices_List[hipc->Device_ID].h_current_channel;
  }

  /* no other valid channel available */
  return (handle);
}

/**
  * @brief  Send data over an UART channel.
  * @param  hipc IPC handle.
  * @param  p_TxBuffer Pointer to the data buffer to transfer.
  * @param  bufsize Length of the data buffer.
  * @retval status
  */
IPC_Status_t IPC_send_uart(IPC_Handle_t *hipc, uint8_t *p_TxBuffer, uint16_t bufsize)
{
  IPC_Status_t retval;
  /* PRINT_DBG(">IPC_send: buf@=%p size=%d", aTxBuffer, bufsize) */

  /* check the handles */
  if (hipc == NULL)
  {
    retval = IPC_ERROR;
  }
#if (RTOS_USED == 1)
  /* Test if current hipc */
  else if (hipc != g_IPC_Devices_List[hipc->Device_ID].h_current_channel)
  {
    retval = IPC_ERROR;
  }
#endif /* RTOS_USED */
  else
  {
    /* send string in one block */
    while (true)
    {
      HAL_StatusTypeDef err;
      err = HAL_UART_Transmit_IT(hipc->Interface.h_uart, (uint8_t *)p_TxBuffer, bufsize);
      if (err !=  HAL_BUSY)
      {
        if ((hipc->UartBusyFlag == 1U) && (hipc->Interface.interface_type == IPC_INTERFACE_UART))
        {
          hipc->UartBusyFlag = 0U;
          while (HAL_UART_Receive_IT(hipc->Interface.h_uart, (uint8_t *)g_IPC_Devices_List[hipc->Device_ID].RxChar, 1U)
                 != HAL_OK)
          {
          }
          PRINT_INFO("Receive rearmed...")
        }
        break;
      }


#if (RTOS_USED == 1)
      (void) osDelay(10U);
#endif /* RTOS_USED */
    }
    retval = IPC_OK;
  }
  return (retval);
}

/**
  * @brief  Receive a message from an UART channel.
  * @param  hipc IPC handle.
  * @param  p_msg Pointer to the IPC message structure to fill with received message.
  * @retval status
  */
IPC_Status_t IPC_receive_uart(IPC_Handle_t *hipc, IPC_RxMessage_t *p_msg)
{
  IPC_Status_t retval;

  int16_t unread_msg;

#if (DBG_IPC_RX_FIFO != 0U)
  int16_t free_bytes;
#endif /* DBG_IPC_RX_FIFO */

  /* check the handle */
  if (hipc == NULL)
  {
    PRINT_ERR("IPC_receive err - hipc NULL")
    retval = IPC_ERROR;
  }
  else if (hipc->Mode == IPC_MODE_UART_CHARACTER)
  {
    if (p_msg == NULL)
    {
      PRINT_ERR("IPC_receive err - p_msg NULL")
      retval = IPC_ERROR;
    }
    else
    {
#if (DBG_IPC_RX_FIFO != 0U)
      free_bytes = RXFIFO_getFreeBytes(hipc);
      PRINT_DBG("free_bytes before msg read=%d", free_bytes)
#endif /* DBG_IPC_RX_FIFO */

      /* read the first unread message */
      unread_msg = RXFIFO_read(hipc, p_msg);
      if (unread_msg == -1)
      {
        PRINT_ERR("IPC_receive err - no unread msg")
        retval = IPC_ERROR;
      }
      else
      {
#if (DBG_IPC_RX_FIFO != 0U)
        free_bytes = RXFIFO_getFreeBytes(hipc);
        PRINT_DBG("free bytes after msg read=%d", free_bytes)
#endif /* DBG_IPC_RX_FIFO */

        if (hipc->State == IPC_STATE_PAUSED)
        {
#if (DBG_IPC_RX_FIFO != 0U)
          /* dump_RX_dbg_infos(hipc, 1, 1); */
          PRINT_INFO("Resume IPC (paused %d times) %d unread msg", hipc->dbgRxQueue.cpt_RXPause, unread_msg)
#endif /* DBG_IPC_RX_FIFO */

          hipc->State = IPC_STATE_ACTIVE;
          (void) HAL_UART_Receive_IT(hipc->Interface.h_uart, (uint8_t *)g_IPC_Devices_List[hipc->Device_ID].RxChar, 1U);
        }

        if (unread_msg == 0)
        {
          retval = IPC_RXQUEUE_EMPTY;
        }
        else
        {
          retval = IPC_RXQUEUE_MSG_AVAIL;
        }
      }
    }
  }
  else
  {
    PRINT_ERR("IPC_receive err - IPC mode not matching")
    retval = IPC_ERROR;
  }

  return (retval);
}

#if (IPC_USE_STREAM_MODE == 1U)
/**
  * @brief  Receive a data buffer from an UART channel.
  * @param  hipc IPC handle.
  * @param  p_buffer Pointer to the data buffer to transfer.
  * @param  p_len Length of the data buffer.
  * @note   p_len value received from user indicates the maximum user buffer length
  *         and p_len is updated with received buffer size.
  * @retval status
  */
IPC_Status_t IPC_streamReceive_uart(IPC_Handle_t *hipc,  uint8_t *p_buffer, int16_t *p_len)
{
  IPC_Status_t retval;
  uint16_t rx_size = 0;

  if (*p_len > 0)
  {
    uint16_t maximum_buffer_size = (uint16_t) * p_len;

    if (hipc->Mode == IPC_MODE_UART_STREAM)
    {
      /* receive */
      while ((hipc->RxBuffer.available_char != 0U) &&
             (rx_size < maximum_buffer_size))
      {
        p_buffer[rx_size] = hipc->RxBuffer.data[hipc->RxBuffer.index_read];
        hipc->RxBuffer.index_read++;
        if (hipc->RxBuffer.index_read >= IPC_RXBUF_STREAM_MAXSIZE)
        {
          hipc->RxBuffer.index_read = 0;
        }
        rx_size++;
        hipc->RxBuffer.available_char--;
      }

      /* update buffer size */
      *p_len = (int16_t) rx_size;
      retval = IPC_OK;
    }
    else
    {
      PRINT_ERR("IPC_receive err - IPC mode not matching")
      retval = IPC_ERROR;
    }
  }
  else
  {
    PRINT_ERR("IPC_receive err - Invalid buffer size")
    retval = IPC_ERROR;
  }

  return (retval);
}
#endif  /* IPC_USE_STREAM_MODE */


/**
  * @brief  Dump content of IPC Rx queue (for debug purpose).
  * @param  hipc IPC handle.
  * @param  readable If equal 1, print special characters explicitly (<CR>, <LF>, <NULL>).
  * @retval none
  */
void IPC_dump_RX_queue_uart(const IPC_Handle_t *hipc, uint8_t readable)
{
#if (USE_TRACE_IPC == 1U)
  IPC_RxHeader_t header;
  uint16_t dump_index;
  uint16_t first_uncomplete_header;
  uint16_t first_uncomplete_size;
  uint16_t last_index;

  /* Take a picture of the RX queue at the time of entry in this function
    *  new char could be received but we do not take them into account
    * => set variables now
  */
  first_uncomplete_header = hipc->RxQueue.current_msg_index;
  first_uncomplete_size = hipc->RxQueue.current_msg_size;
  dump_index = hipc->RxQueue.index_read;
  last_index = hipc->RxQueue.index_write;

  PRINT_INFO(" *** IPC state =%d ", hipc->State)
  PRINT_INFO(" *** First header position =%d ", dump_index)
  PRINT_INFO(" *** Last incomplete header position=%d ", first_uncomplete_header)
  PRINT_INFO(" *** Current write pos=%d ", last_index)

  /* read message header */
  RXFIFO_readMsgHeader_at_pos(hipc, &header, dump_index);

  while ((header.complete == 1U) && (dump_index != first_uncomplete_header))
  {
    uint16_t core_msg_index = (dump_index + IPC_RXMSG_HEADER_SIZE) % IPC_RXBUF_MAXSIZE;
    PRINT_INFO(" ### Complete msg, size=%d, data pos=%d:", header.size, core_msg_index)
    RXFIFO_print_data(hipc, core_msg_index, header.size, readable);

    /* read next message header */
    dump_index = (dump_index + IPC_RXMSG_HEADER_SIZE + header.size) % IPC_RXBUF_MAXSIZE;
    RXFIFO_readMsgHeader_at_pos(hipc, &header, dump_index);
  }

  /* debug info: last queue message */
  if (header.complete == 1U)
  {
    /* should not happen... */
    uint16_t core_msg_index = (dump_index + IPC_RXMSG_HEADER_SIZE) % IPC_RXBUF_MAXSIZE;
    PRINT_INFO(" ### Last msg is complete, size=%d, data pos=%d: ", header.size, core_msg_index)
    RXFIFO_print_data(hipc, (dump_index + IPC_RXMSG_HEADER_SIZE), first_uncomplete_size, readable);
  }
  else
  {
    uint16_t core_msg_index = (dump_index + IPC_RXMSG_HEADER_SIZE) % IPC_RXBUF_MAXSIZE;
    PRINT_INFO(" ### Last msg is not complete, size=%d, pos=%d: ", first_uncomplete_size, core_msg_index)
    RXFIFO_print_data(hipc, core_msg_index, first_uncomplete_size, readable);
  }
#else
  UNUSED(hipc);
  UNUSED(readable);
#endif /* USE_TRACE_IPC */
}

/* Callback functions ----------------------------------------------------------*/
/**
  * @brief  IPC uart RX callback (called under IT !).
  * @param  UartHandle Ptr to the HAL UART handle.
  * @retval none
  */
void IPC_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Warning ! this function is called under IT */
  uint8_t device_id = find_Device(UartHandle);
  if (device_id < IPC_MAX_DEVICES)
  {
    if (g_IPC_Devices_List[device_id].h_current_channel != NULL)
    {
      g_IPC_Devices_List[device_id].h_current_channel->RxFifoWrite(g_IPC_Devices_List[device_id].h_current_channel,
                                                                   g_IPC_Devices_List[device_id].RxChar[0]);
    }
  }
}

/**
  * @brief  IPC uart TX callback (called under IT !).
  * @param  UartHandle Ptr to the HAL UART handle.
  * @retval none
  */
void IPC_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Warning ! this function is called under IT */
  uint8_t device_id = find_Device(UartHandle);
  if (device_id < IPC_MAX_DEVICES)
  {
    if (g_IPC_Devices_List[device_id].h_current_channel != NULL)
    {
      /* Set transmission flag: transfer complete */
      g_IPC_Devices_List[device_id].h_current_channel->TxClientCallback(
        g_IPC_Devices_List[device_id].h_current_channel
      );
    }
  }
}

/**
  * @brief  IPC uart error callback (called under IT !).
  * @param  UartHandle Ptr to the HAL UART handle.
  * @retval none
  */
void IPC_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  UNUSED(UartHandle);
  /* Warning ! this function is called under IT */
}

/* Private function Definition -----------------------------------------------*/
static uint8_t find_Device(const UART_HandleTypeDef *huart)
{
  /* search device corresponding to this uart in the devices list */
  uint8_t device_id = IPC_DEVICE_NOT_FOUND;
  uint8_t i = 0U;
  bool leave_loop = false;
  do
  {
    /* search the device corresponding to this instance */
    if (huart->Instance == g_IPC_Devices_List[i].phy_int.h_uart->Instance)
    {
      /* force loop exit */
      device_id = i;
      leave_loop = true;
    }
    i++;
  } while ((leave_loop == false) && (i < IPC_MAX_DEVICES));

  return (device_id);
}

static IPC_Status_t change_ipc_channel(IPC_Handle_t *hipc)
{
  IPC_Handle_t *tmp_handle;

  tmp_handle = g_IPC_Devices_List[hipc->Device_ID].h_current_channel;

  /* swap channels */
  g_IPC_Devices_List[hipc->Device_ID].h_current_channel = hipc;
  g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel = tmp_handle;

  PRINT_DBG("Change IPC channels")
  PRINT_DBG("state 0x%x", g_IPC_Devices_List[hipc->Device_ID].state)
  PRINT_DBG("active channel handle: %p", g_IPC_Devices_List[hipc->Device_ID].h_current_channel)
  PRINT_DBG("inactive channel handle: %p", g_IPC_Devices_List[hipc->Device_ID].h_inactive_channel)

  return (IPC_OK);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

