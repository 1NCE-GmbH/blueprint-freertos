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
#include <string.h>
#include <math.h>

#include "cellular_runtime_standard.h"

/* Private defines -----------------------------------------------------------*/
#define CRS_STRLEN_MAX 2048U

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* functions ---------------------------------------------------------*/

/**
  * @brief  convert an integer to ascii string
  * @param  num numbre to convert
  * @param  base base number (only 10 or 16)
  * @param  str (out)  result of conversion
  * @retval result of conversion
  */

uint8_t *crs_itoa(int32_t num, uint8_t *str, uint32_t base)
{
  uint32_t i;
  uint32_t j;
  bool isNegative;
  int32_t rem;
  int32_t char32;
  uint8_t temp;
  int32_t num_tmp;

  num_tmp = num;
  isNegative = false;
  i = 0U;

  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num_tmp == 0)
  {
    str[i] = (uint8_t)'0';
    i++;
    str[i] = (uint8_t)'\0';
  }
  else
  {
    /* num != 0: normal algo */
    if ((num_tmp < 0) && (base == 10U))
    {
      /* negative number */
      isNegative = true;
      num_tmp = -num_tmp;
    }

    /* convert integer to string (inverted) */
    while (num_tmp != 0)
    {
      rem = num_tmp % (int32_t)base;
      if ((rem > 9))
      {
        /* hexdecimal digit */
        char32 = ((rem - 10) + (int32_t)'a');
        str[i] = (uint8_t)char32;
      }
      else
      {
        /* decimal digit */
        char32 = (rem + (int32_t)'0');
        str[i] = (uint8_t)char32;
      }
      num_tmp = num_tmp / (int32_t)base;
      i++;
    }

    if (isNegative == true)
    {
      /* negative number => adding '-' at the beginning of the string */

      str[i] = (uint8_t)'-';
      i++;
    }

    /* adding end of string */
    str[i] = (uint8_t)'\0';

    /* invert string order */
    for (j = 0U; j < (i >> 1) ; j++)
    {
      temp     = str[j];
      str[j]   = str[i - j - 1U];
      str[i - j - 1U] = temp;
    }
  }
  return str;
}

/**
  * @brief  convert a ascci number to an integer
  * @param  string  ascii string number to convert
  * @retval result of conversion
  */
int32_t crs_atoi(const uint8_t *string)
{
  int32_t result;
  int32_t digit;

  uint32_t offset;
  int8_t sign;
  uint8_t digit8;
  bool  leave;

  result = 0;
  offset = 0U;

  if (*string == (uint8_t)'-')
  {
    /* negative number */
    sign = 1;
    offset++;
  }
  else
  {
    /* positive number */
    sign = 0;
    if (string[offset] == (uint8_t)'+')
    {
      offset++;
    }
  }

  leave = false;

  /* partsing string while decimal digit are found */
  while (leave == false)
  {
    if ((string[offset] < (uint8_t)'0') || (string[offset] > (uint8_t)'9'))
    {
      /* not a digit => end of parsing */
      leave = true;
    }
    else
    {
      /* digit found => adding it in the integer result */
      digit8 = string[offset] - (uint8_t)'0';
      digit  = (int32_t)digit8;
      result = (10 * result) + digit;
      offset++;
    }
  }

  if (sign != 0)
  {
    /* negative number => set the oposite */
    result = -result;
  }
  return result;
}

/**
  * @brief  convert a ascci hex number to an integer
  * @param  string  ascii string hex number to convert
  * @retval result of conversion
  */
int32_t crs_atoi_hex(const uint8_t *string)
{
  int32_t result;
  uint32_t digit;
  uint8_t digit8;
  uint32_t offset;

  result = 0;
  offset = 0;

  if (string != NULL)
  {
    /* partsing string while hexadecimal digit are found */
    while (true)
    {
      if ((string[offset] >= (uint8_t)'0') && (string[offset] <= (uint8_t)'9'))
      {
        /* decimal digit found */
        digit8 = string[offset] - (uint8_t)'0';
        digit  = (uint32_t)digit8;
      }
      else if ((string[offset] >= (uint8_t)'a') && (string[offset] <= (uint8_t)'f'))
      {
        /* hexa decimal digit found */
        digit8 = string[offset] - (uint8_t)'a' + 10U;
        digit  = (uint32_t)digit8;
      }
      else if ((string[offset] >= (uint8_t)'A') && (string[offset] <= (uint8_t)'F'))
      {
        /* hexa decimal digit found */
        digit8 =  string[offset] - (uint8_t)'A' + 10U;
        digit  = (uint32_t)digit8;
      }
      else
      {
        /* not a digit => end of number */
        break;
      }

      /*  adding the current digit in the integer result */
      result = (16 * result) + (int32_t)digit;
      offset++;
    }
  }
  return result;
}


/**
  * @brief  get lenght of a string
  * @param  string  string to get lenght
  * @retval string lenght (0 is no '\0' found in the string
  */
uint32_t crs_strlen(const uint8_t *string)
{
  uint32_t i;
  uint32_t res;
  res = 0;

  if (string != NULL)
  {
    /* parsing string looking for '0' char */
    for (i = 0U ; i < CRS_STRLEN_MAX ; i++)
    {
      if (string[i] == 0U)
      {
        /* end of string found */
        res = i;
        break;
      }
    }
  }
  return res;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
