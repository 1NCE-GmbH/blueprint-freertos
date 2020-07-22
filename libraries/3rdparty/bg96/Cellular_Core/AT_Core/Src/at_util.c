/**
  ******************************************************************************
  * @file    at_util.c
  * @author  MCD Application Team
  * @brief   This file provides code for atcore utilities
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
#include "at_util.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define MAX_32BITS_STRING_SIZE (8U)  /* = max string size for a 32bits value (FFFF.FFFF) */
#define MAX_64BITS_STRING_SIZE (16U) /* = max string size for a 64bits value (FFFF.FFFF.FFFF.FFFF) */
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
uint32_t ATutil_ipow(uint32_t base, uint16_t exp)
{
  uint16_t local_exp = exp;
  uint32_t local_base = base;

  /* implementation of power function */
  uint32_t result = 1U;
  while (local_exp != 0U)
  {
    if ((local_exp & 1U) != 0U)
    {
      result *= local_base;
    }
    local_exp >>= 1;
    local_base *= local_base;
  }

  return result;
}

uint32_t ATutil_convertStringToInt(const uint8_t *p_string, uint16_t size)
{
  uint32_t conv_nbr = 0U;

  /* auto-detect if this is an hexa value (format: 0x....) */
  if ((size > 2U) && (p_string[1] == 120U)) /* ASCII value 120 = 'x' */
  {
    conv_nbr = ATutil_convertHexaStringToInt32(p_string, size);
  }
  else
  {
    uint16_t idx;
    uint16_t nb_digit_ignored = 0U;
    uint16_t loop = 0U;

    /* decimal value */
    for (idx = 0U; idx < size; idx++)
    {
      /* consider only the numbers */
      if ((p_string[idx] >= 48U) && (p_string[idx] <= 57U))
      {
        loop++;
        conv_nbr = conv_nbr +
                   (((uint32_t) p_string[idx] - 48U) * ATutil_ipow(10U, (size - loop - nb_digit_ignored)));
      }
      else
      {
        nb_digit_ignored++;
      }
    }
  }

  return (conv_nbr);
}

uint32_t ATutil_convertHexaStringToInt32(const uint8_t *p_string, uint16_t size)
{
  uint32_t conv_nbr = 0U; /* returned value = converted numder (0 if an error occurs) */
  uint16_t idx;
  uint16_t nb_digit_ignored;
  uint16_t loop = 0U;
  uint16_t str_size_to_convert;

  /* This function assumes that the string value is an hexadecimal value with or without Ox prefix
   * It converts a string to its hexadecimal value (32 bits value)
   * example:
   * explicit input string format from "0xW" to "0xWWWWXXXX"
   * implicit input string format from "W" to "WWWWXXXX"
   * where X,Y,W and Z are characters from '0' to 'F'
   */

  /* auto-detect if 0x is present */
  if ((size > 2U) && (p_string[1] == 120U)) /* ASCII value 120 = 'x' */
  {
    /* 0x is present */
    nb_digit_ignored = 2U;
  }
  else
  {
    /* 0x is not present */
    nb_digit_ignored = 0U;
  }

  /* if 0x is present, we can skip it */
  str_size_to_convert = size - nb_digit_ignored;

  /* check maximum string size */
  if (str_size_to_convert <= MAX_32BITS_STRING_SIZE)
  {
    /* convert string to hexa value */
    for (idx = nb_digit_ignored; idx < size; idx++)
    {
      if ((p_string[idx] >= 48U) && (p_string[idx] <= 57U))
      {
        /* 0 to 9 */
        loop++;
        conv_nbr = conv_nbr +
                   (((uint32_t)p_string[idx] - 48U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else if ((p_string[idx] >= 97U) && (p_string[idx] <= 102U))
      {
        /* a to f */
        loop++;
        conv_nbr = conv_nbr +
                   (((uint32_t)p_string[idx] - 97U + 10U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else if ((p_string[idx] >= 65U) && (p_string[idx] <= 70U))
      {
        /* A to F */
        loop++;
        conv_nbr = conv_nbr +
                   (((uint32_t)p_string[idx] - 65U + 10U) * ATutil_ipow(16U, (size - loop - nb_digit_ignored)));
      }
      else
      {
        nb_digit_ignored++;
      }
    }
  }

  return (conv_nbr);
}

uint8_t ATutil_convertHexaStringToInt64(const uint8_t *p_string, uint16_t size, uint32_t *high_part_value,
                                        uint32_t *low_part_value)
{
  uint8_t retval;
  uint16_t nb_digit_ignored;
  uint16_t str_size_to_convert;
  uint16_t high_part_size;
  uint16_t low_part_size;

  /* This function assumes that the string value is an hexadecimal value with or without Ox prefix
   * It converts a string to its hexadecimal value (64 bits made of two 32 bits values)
   * example:
   * explicit input string format from "0xW" to "0xWWWWXXXXYYYYZZZZ"
   * implicit input string format from "W" to "WWWWXXXXYYYYZZZZ"
   * where X,Y,W and Z are characters from '0' to 'F'
   */

  /* init decoded values */
  *high_part_value = 0U;
  *low_part_value = 0U;

  /* auto-detect if 0x is present */
  if ((size > 2U) && (p_string[1] == 120U)) /* ASCII value 120 = 'x' */
  {
    /* 0x is present */
    nb_digit_ignored = 2U;
  }
  else
  {
    /* 0x is not present */
    nb_digit_ignored = 0U;
  }

  /* if 0x is present, we can skip it */
  str_size_to_convert = size - nb_digit_ignored;

  /* check maximum string size */
  if (str_size_to_convert > MAX_64BITS_STRING_SIZE)
  {
    /* conversion error */
    retval = 0U;
  }
  else
  {
    if (str_size_to_convert > 8U)
    {
      high_part_size = str_size_to_convert - 8U;
      /* convert upper part if exists */
      *high_part_value = ATutil_convertHexaStringToInt32((const uint8_t *)(p_string + nb_digit_ignored),
                                                         high_part_size);
    }
    else
    {
      high_part_size = 0U;
    }

    /* convert lower part */
    low_part_size = str_size_to_convert - high_part_size;
    *low_part_value =  ATutil_convertHexaStringToInt32((const uint8_t *)(p_string + nb_digit_ignored + high_part_size),
                                                       low_part_size);

    /* string successfully converted */
    retval = 1U;
  }

  return (retval);
}

uint32_t ATutil_convertBinStringToInt32(const uint8_t *p_string, uint16_t size)
{
  uint32_t conv_nbr = 0U; /* returned value = converted numder (returns 0 if an error occurs) */

  for (uint16_t i = 0; i < size; i++)
  {
    /* convert ASCII character to its value (0x31 for 1, 0x30 for 0) */
    uint32_t bit = (p_string[size - i - 1U] == 0x31U) ? 1U : 0U;
    /* bit weight */
    uint32_t weight = ATutil_ipow(2U, i);
    conv_nbr += bit * weight;
  }

  return (conv_nbr);
}

uint8_t ATutil_isNegative(const uint8_t *p_string, uint16_t size)
{
  /* returns 1 if number in p_string is negative */
  uint8_t isneg = 0U;
  uint16_t idx;

  /* search for "-" until to find a valid number */
  for (idx = 0U; idx < size; idx++)
  {
    /* search for "-" */
    if (p_string[idx] == 45U)
    {
      isneg = 1U;
    }
    /* check for leave-loop condition (negative or valid number found) */
    if ((isneg == 1U) ||
        ((p_string[idx] >= 48U) && (p_string[idx] <= 57U)))
    {
      break;
    }
  }
  return (isneg);
}

void ATutil_convertStringToUpperCase(uint8_t *p_string, uint16_t size)
{
  uint16_t idx = 0U;
  while ((p_string[idx] != 0U) && (idx < size))
  {
    /* if lower case character... */
    if ((p_string[idx] >= 97U) && (p_string[idx] <= 122U))
    {
      /* ...convert it to uppercase character */
      p_string[idx] -= 32U;
    }
    idx++;
  }
}

/**
  * @brief  Converts a value to its binary string
  *         example: the decimal value 54531 (=0xD503) will be converted to the
  *                  string 1101010100000011 with /0 has end character
  * @param  value the decimal value to convert.
  * @param  nbBits number of bits to compute.
  * @param  sizeStr size of result string (should be equal or greater than nbBits + 1).
  * @param  binStr ptr to the result string.
  * @retval 0 if no error, 1 if an error occured.
  */
uint8_t ATutil_convert_uint8_to_binary_string(uint32_t value, uint8_t nbBits, uint8_t sizeStr, uint8_t *binStr)
{
  uint8_t retval;

  /* String need to be at least one character more than the number of bits */
  if (sizeStr > nbBits)
  {
    for (uint8_t i = 0U; i < nbBits; i++)
    {
      /* convert to binary string */
      binStr[nbBits - i - 1U] = (((value >> i) % 2U) == 0U) ? 0x30U : 0x31U;
    }
    /* set end string character */
    binStr[nbBits] = 0U;
    retval = 0U;
  }
  else
  {
    retval = 1U;
  }

  return (retval);
}

/**
  * @brief  Remove double quotes "" from a p_Src buffer,
  *         and recopy the result to p_Dst buffer
  * @param  p_Src ptr to source buffer (string with quotes)
  * @param  srcSize of p_Src buffer
  * @param  p_Dst ptr to Destination Buffer (string without quotes)
  * @param  dstSize of p_Dst buffer
  * @retval size of destination string (util part).
  */
uint16_t ATutil_remove_quotes(const uint8_t *p_Src, uint16_t srcSize, uint8_t *p_Dst, uint16_t dstSize)
{
  uint16_t src_idx;
  uint16_t dest_idx = 0U;

  /* reset p_Dst buffer */
  (void) memset((void *)p_Dst, 0, dstSize);

  /* parse p_Src */
  for (src_idx = 0; ((src_idx < srcSize) && (dest_idx < dstSize)); src_idx++)
  {
    /* remove quotes from the string */
    if (p_Src[src_idx] != 0x22U)
    {
      /* write to p_Dst*/
      p_Dst[dest_idx] = p_Src[src_idx];
      dest_idx++;
    }
  }

  return (dest_idx);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
