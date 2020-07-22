/**
  ******************************************************************************
  * @file    trace_interface.c
  * @author  MCD Application Team
  * @brief   This file contains trace define utilities
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
#include "trace_interface.h"
#include "cmsis_os_misrac2012.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define PRINT_FORCE(format, args...)  TRACE_PRINT_FORCE(DBG_CHAN_UTILITIES, DBL_LVL_P0, format, ## args)
#define TRACE_IF_CHAN_MAX_VALUE 14U
/* Private defines -----------------------------------------------------------*/
#define MAX_HEX_PRINT_SIZE     210U

/* Private variables ---------------------------------------------------------*/
static bool traceIF_traceEnable = true;
static uint32_t traceIF_Level = TRACE_IF_MASK;
static osMutexId traceIF_uart_mutex = NULL;

static uint8_t traceIF_traceComponent[TRACE_IF_CHAN_MAX_VALUE] =
{
  1U,   /*  DBG_CHAN_GENERIC           */
  1U,   /*  DBG_CHAN_MAIN              */
  1U,   /*  DBG_CHAN_ATCMD             */
  1U,   /*  DBG_CHAN_COM               */
  1U,   /*  DBG_CHAN_MQTTDEMO        */
  1U,   /*  DBG_CHAN_HTTP              */
  1U,   /*  DBG_CHAN_PING              */
  1U,   /*  DBG_CHAN_IPC               */
  1U,   /*  DBG_CHAN_PPPOSIF           */
  1U,   /*  DBG_CHAN_CELLULAR_SERVICE  */
  1U,   /*  DBG_CHAN_NIFMAN            */
  1U,   /*  DBG_CHAN_DATA_CACHE        */
  1U,   /*  DBG_CHAN_UTILITIES         */
  1U,   /*  DBG_CHAN_ERROR_LOGGER      */
};

/* Private function prototypes -----------------------------------------------*/
static void ITM_Out(uint32_t port, uint32_t ch);


/* Global variables ----------------------------------------------------------*/
uint8_t dbgIF_buf[DBG_CHAN_MAX_VALUE][DBG_IF_MAX_BUFFER_SIZE];
uint8_t *traceIF_UartBusyFlag = NULL;

/* Functions Definition ------------------------------------------------------*/
/* static functions */



static void ITM_Out(uint32_t port, uint32_t ch)
{
  /* check port validity (0-31)*/
  if (port <= 31U)
  {
    uint32_t tmp_mask;
    tmp_mask = (ITM->TER & (1UL << port));
    if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&   /* ITM enabled ? */
        (tmp_mask != 0UL))   /* ITM selected Port enabled ? */
    {

      /* Wait until ITM port is ready */
      while (ITM->PORT[port].u32 == 0UL)
      {
        __NOP();
      }

      /* then send data, one byte at a time */
      ITM->PORT[port].u8 = (uint8_t) ch;
    }
  }
}


static void traceIF_uartTransmit(uint8_t *ptr, uint16_t len)
{
  (void)osMutexWait(traceIF_uart_mutex, RTOS_WAIT_FOREVER);
  (void)HAL_UART_Transmit(&TRACE_INTERFACE_UART_HANDLE, (uint8_t *)ptr, len, 0xFFFFU);
  (void)osMutexRelease(traceIF_uart_mutex);
  if (traceIF_UartBusyFlag != NULL)
  {
    while (HAL_UART_Receive_IT(&TRACE_INTERFACE_UART_HANDLE, traceIF_UartBusyFlag, 1U) != HAL_OK)
    {
    }
    traceIF_UartBusyFlag = NULL;
  }
}

/* exported functions */
void traceIF_itmPrint(uint8_t port, uint8_t lvl, uint8_t *pptr, uint16_t len)
{
  uint32_t i;
  uint8_t *ptr;
  ptr = pptr;

  if (traceIF_traceEnable == true)
  {
    if ((traceIF_Level & lvl) != 0U)
    {
      if (traceIF_traceComponent[port] != 0U)
      {
        for (i = 0U; i < len; i++)
        {
          ITM_Out((uint32_t) port, (uint32_t) *ptr);
          ptr++;
        }
      }
    }
  }
}

void traceIF_trace_off(void)
{
  traceIF_traceEnable = false;
}

void traceIF_trace_on(void)
{
  traceIF_traceEnable = true;
}

void traceIF_uartPrint(uint8_t port, uint8_t lvl, uint8_t *pptr, uint16_t len)
{
  uint8_t *ptr;
  ptr = pptr;

  if (traceIF_traceEnable == true)
  {
    if ((traceIF_Level & lvl) != 0U)
    {
      if (traceIF_traceComponent[port] != 0U)
      {
        traceIF_uartTransmit(ptr, len);
      }
    }
  }
}

void traceIF_itmPrintForce(uint8_t port, uint8_t *pptr, uint16_t len)
{
  uint32_t i;
  uint8_t *ptr;
  ptr = pptr;

  for (i = 0U; i < len; i++)
  {
    ITM_Out((uint32_t) port, (uint32_t) *ptr);
    ptr++;
  }
}

void traceIF_uartPrintForce(uint8_t port, uint8_t *pptr, uint16_t len)
{
  UNUSED(port);
  uint8_t *ptr;
  ptr = pptr;

  traceIF_uartTransmit(ptr, len);
}


void traceIF_hexPrint(dbg_channels_t chan, dbg_levels_t level, uint8_t *buff, uint16_t len)
{
#if ((TRACE_IF_TRACES_ITM == 1U) || (TRACE_IF_TRACES_UART == 1U))
  static  uint8_t car[MAX_HEX_PRINT_SIZE];

  uint32_t i;
  uint16_t tmp_len;
  tmp_len = len;

  if (((tmp_len * 2U) + 1U) > MAX_HEX_PRINT_SIZE)
  {
    TRACE_PRINT(chan,  level, "TR (%d/%d)\n\r", (MAX_HEX_PRINT_SIZE >> 1) - 2U, tmp_len)
    tmp_len = (MAX_HEX_PRINT_SIZE >> 1) - 2U;
  }

  for (i = 0U; i < tmp_len; i++)
  {
    uint8_t digit = ((buff[i] >> 4U) & 0xfU);
    if (digit <= 9U)
    {
      car[2U * i] =  digit + 0x30U;
    }
    else
    {
      car[2U * i] =  digit + 0x41U - 10U;
    }

    digit = (0xfU & buff[i]);
    if (digit <= 9U)
    {
      car[(2U * i) + 1U] =  digit + 0x30U;
    }
    else
    {
      car[(2U * i) + 1U] =  digit + 0x41U - 10U;
    }
  }
  car[2U * i] =  0U;

  TRACE_PRINT(chan,  level, "%s ", (CRC_CHAR_t *)car)
#endif  /* ((TRACE_IF_TRACES_ITM == 1U) || (TRACE_IF_TRACES_UART == 1U)) */
}

void traceIF_BufCharPrint(dbg_channels_t chan, dbg_levels_t level, const CRC_CHAR_t *buf, uint16_t size)
{
  for (uint32_t cpt = 0U; cpt < size; cpt++)
  {
    if (buf[cpt] == (CRC_CHAR_t)0)
    {
      TRACE_PRINT(chan, level, "<NULL CHAR>")
    }
    else if (buf[cpt] == '\r')
    {
      TRACE_PRINT(chan, level, "<CR>")
    }
    else if (buf[cpt] == '\n')
    {
      TRACE_PRINT(chan, level, "<LF>")
    }
    else if (buf[cpt] == (CRC_CHAR_t)0x1A)
    {
      TRACE_PRINT(chan, level, "<CTRL-Z>")
    }
    else if ((buf[cpt] >= (CRC_CHAR_t)0x20) && (buf[cpt] <= (CRC_CHAR_t)0x7E))
    {
      /* printable CRC_CHAR_t */
      TRACE_PRINT(chan, level, "%c", buf[cpt])
    }
    else
    {
      /* Special Character - not printable */
      TRACE_PRINT(chan, level, "<SC>")
    }
  }
  TRACE_PRINT(chan, level, "\n\r")
}

void traceIF_BufHexPrint(dbg_channels_t chan, dbg_levels_t level, const CRC_CHAR_t *buf, uint16_t size)
{
  for (uint32_t cpt = 0U; cpt < size; cpt++)
  {
    TRACE_PRINT(chan, level, "0x%02x ", (uint8_t) buf[cpt])
    if ((cpt != 0U) && (((cpt + 1U) % 16U) == 0U))
    {
      TRACE_PRINT(chan, level, "\n\r")
    }
  }
  TRACE_PRINT(chan, level, "\n\r")
}

void traceIF_init(void)
{
  /* Multi call protection */
  if (traceIF_uart_mutex == NULL)
  {
    osMutexDef(osTraceUartMutex);
    traceIF_uart_mutex = osMutexCreate(osMutex(osTraceUartMutex));
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
