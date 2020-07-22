/**
  ******************************************************************************
  * @file    cellular_runtime.c
  * @author  MCD Application Team
  * @brief   implementation of cellular_runtime functions
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
#include <stdbool.h>

#include "cellular_runtime_custom.h"
#include "cellular_runtime_standard.h"

/* Private defines -----------------------------------------------------------*/
#define CRC_IP_ADDR_DIGIT_SIZE 3U

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* functions ---------------------------------------------------------*/

/**
  * @brief  get ip addr+port from string
  * @note   input format d[dd].d[dd].d[dd].d[dd][:pppp)
  * @param  string    ip addr + port to parse
  * @param  addr      (out) uint8_t[4]   4 bytes of ipp addr
  * @param  port      (out) optionnal ip port
  * @note             if no port is needed put this parameter to NULL
  * @retval validity of conversion: retval==0 conversion OK /   retval!=0 conversion KO
  */
uint32_t crc_get_ip_addr(uint8_t *string, uint8_t *addr, uint16_t *port)
{
  uint8_t i;
  uint8_t j;
  uint32_t ret;
  uint32_t offset;
  bool leave;

  ret    = 0U;
  offset = 0;

  leave = false;

  /* parse the 4 byte of the IP addr */
  for (i = 0U ; (i < 4U) && (leave == false) ; i++)
  {
    /* parse the digits (max 3) of the IP addr */
    for (j = 0U ; j <= CRC_IP_ADDR_DIGIT_SIZE ; j++)
    {
      if ((string[j + offset] < (uint8_t)'0') || (string[j + offset] > (uint8_t)'9'))
      {
        /* not a decimal digit => end of  addr byte */
        break;
      }
    }

    if ((j == (CRC_IP_ADDR_DIGIT_SIZE + 1U)) || (j == 0U))
    {
      /* not a correct addr byte found => return error */
      ret = 1;
      leave = true;
    }
    else
    {
      /* correct addr byte found: convert it ito integer */
      addr[i] = (uint8_t)crs_atoi(&string[offset]);
      if (string[offset + j] != (uint8_t)'.')
      {
        /* not the byte separator => end of parsing */
        leave = true;
      }
      offset = offset + j + 1U;
    }
  }

  if (i != 4U)
  {
    /* number of arrd bytes != 4 => not an ip addr */
    ret = 1;
  }
  else
  {
    if (port != NULL)
    {
      /* port number requested => parse the port number */
      if (string[offset - 1U] == (uint8_t)':')
      {
        /*  port number separation found => convert port number to integer */
        *port = (uint16_t)crs_atoi(&string[offset]);
      }
      else
      {
        /* no port number separation found => no port number */
        *port = 0;
      }
    }
  }

  if (ret == 1U)
  {
    /* conversion fail => set returned ip addr to 0  */
    addr[0] = 0U;
    addr[1] = 0U;
    addr[2] = 0U;
    addr[3] = 0U;
    if (port != NULL)
    {
      /* set returned port number to 0  */
      *port = 0;
    }
  }
  return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
