/**
  ******************************************************************************
  * @file    at_datapack.c
  * @author  MCD Application Team
  * @brief   This file provides code for packing/unpacking datas
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
#include "at_datapack.h"
#include "at_util.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define  DATASTRUCT_POINTER_TYPE   ((uint8_t) 1U)
#define  DATASTRUCT_CONTENT_TYPE   ((uint8_t) 2U)
#define  DATAPACK_HEADER_BYTE_SIZE ((uint8_t) 4U)

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATDATAPACK == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "Datapack:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "Datapack:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "Datapack ERROR:" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_INFO(format, args...)  (void) printf("Datapack:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("Datapack ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATDATAPACK */

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static DataPack_Status_t check_structure_to_read(uint8_t *p_buf, uint16_t msgtype,
                                                 uint16_t expect_size, uint8_t expect_type);

/* Functions Definition ------------------------------------------------------*/
/*
*
* Header description:
*  - byte0,byte1: message type (provided by user)
*  - byte2,byte3: data size (header not included)
*  - byte5: content type (is data a complete struct data or pointer on struct)
* Data
*
*/

/**
  * @brief  This function pack a data buffer. It contains the address
  *         of a user data structure.
  *
  * @param  p_buf Handle on data buffer to pack
  * @param  msgtype Type of message
  * @param  p_data Pointer to user data structure
  * @retval DataPack_Status_t
  */
DataPack_Status_t DATAPACK_writePtr(uint8_t *p_buf, uint16_t msgtype, void *p_data)
{
  DataPack_Status_t retvalue;
  uint16_t size = (uint16_t) sizeof(datapack_structptr_t);
  datapack_structptr_t sptr;
  sptr.structptr = p_data;

  /* check pointers */
  if ((p_data == NULL) || (p_buf == NULL))
  {
    PRINT_ERR("DATAPACK_writePtr error (%p, %p)", p_data, p_buf)
    retvalue = DATAPACK_ERROR;
  }
  else
  {
    /* write header: message type */
    p_buf[0] = (uint8_t)(msgtype >> 8);
    p_buf[1] = (uint8_t)(msgtype & 0x00FFU);

    /* write header: size */
    p_buf[2] = (uint8_t)(size >> 8);
    p_buf[3] = (uint8_t)(size & 0x00FFU);

    /* write header: content type (pointer of datas) */
    p_buf[4] = DATASTRUCT_POINTER_TYPE;

    /* write pointer on user data structure */
    (void) memcpy((void *)&p_buf[DATAPACK_HEADER_BYTE_SIZE + 1U], (void *)&sptr, sizeof(datapack_structptr_t));

    retvalue = DATAPACK_OK;
  }
  return (retvalue);
}

/**
  * @brief  This function pack a data buffer. It contains a user data structure.
  *         The size of this data structure shall not exceed the maximum size.
  * @param  p_buf Handle on data buffer to pack
  * @param  msgtype Type of message
  * @param  size Size of the structure
  * @param  p_data Pointer to user data structure
  * @retval DataPack_Status_t
  */
DataPack_Status_t DATAPACK_writeStruct(uint8_t *p_buf, uint16_t msgtype, uint16_t size, void *p_data)
{

  DataPack_Status_t retvalue;

#if (USE_TRACE_ATDATAPACK == 1U)
  static uint32_t datapack_biggest_size = 0U; /* for debug only, used to track maximum struct size */
  if (size > datapack_biggest_size)
  {
    datapack_biggest_size = size;
  }
  PRINT_DBG("<MAX SIZE INFO> msgtype=%d size=%d (biggest =%ld)", msgtype, size, datapack_biggest_size)
#endif /* USE_TRACE_ATDATAPACK */

  /* check maximum size and pointer */
  if (((size - DATAPACK_HEADER_BYTE_SIZE) > DATAPACK_MAX_BUF_SIZE) || (p_buf == NULL))
  {
    PRINT_ERR("DATAPACK_writeStruct error")
    retvalue = DATAPACK_ERROR;
  }
  else
  {
    /* write header: message type */
    p_buf[0] = (uint8_t)(msgtype >> 8);
    p_buf[1] = (uint8_t)(msgtype & 0x00FFU);

    /* write header: size */
    p_buf[2] = (uint8_t)(size >> 8);
    p_buf[3] = (uint8_t)(size & 0x00FFU);

    /* write header: content type (pointer of datas) */
    p_buf[4] = DATASTRUCT_CONTENT_TYPE;

    /* transmit structure content */
    (void) memcpy((void *)&p_buf[DATAPACK_HEADER_BYTE_SIZE + 1U],
                  (void *)p_data,
                  (size_t) size);

    retvalue = DATAPACK_OK;
  }

  return (retvalue);
}

/**
  * @brief  This function unpack a data buffer which contains the address
  *         of a user data structure. Then it updates the pointer p_data to
  *         point on this user data structure.
  *
  * @param  p_buf Handle on packed data buffer
  * @param  msgtype
  * @param  p_data Adress of pointer to user data strucutre
  * @retval DataPack_Status_t
  */
DataPack_Status_t DATAPACK_readPtr(uint8_t *p_buf, uint16_t msgtype, void **p_data)
{
  DataPack_Status_t retval;
  /* local structure used to retrieve pointer value */
  datapack_structptr_t sptr;

  /* check pointer */
  if (p_buf == NULL)
  {
    PRINT_ERR("DATAPACK_readPtr pointer error (%p)", p_buf)
    retval = DATAPACK_ERROR;
  }
  else
  {
    /* check validity of data structure to read */
    uint16_t size = (uint16_t) sizeof(datapack_structptr_t);
    retval = check_structure_to_read(p_buf, msgtype, size, DATASTRUCT_POINTER_TYPE);
    if (retval == DATAPACK_OK)
    {
      /* retrieve pointer value in the message */
      (void) memcpy((void *)&sptr,
                    (void *)&p_buf[DATAPACK_HEADER_BYTE_SIZE + 1U],
                    (size_t) size);

      /* update user pointer to point on the data structure pointer received in the message */
      *p_data = sptr.structptr;
    }
  }

  return (retval);
}

/**
  * @brief  This function unpack a data buffer. It contains a user data structure.
  *         The size of this data structure shall not exceed the maximum size.
  * @param  p_buf Handle on packed data buffer
  * @param  msgtype Type of message
  * @param  size Size of the structure
  * @param  p_data Pointer to user data structure
  * @retval DataPack_Status_t
  */
DataPack_Status_t DATAPACK_readStruct(uint8_t *p_buf, uint16_t msgtype, uint16_t size, void *p_data)
{
  DataPack_Status_t retval;

  /* check pointers */
  if ((p_data == NULL) || (p_buf == NULL))
  {
    PRINT_ERR("DATAPACK_readStruct pointer error( %p, %p)", p_data, p_buf)
    retval = DATAPACK_ERROR;
  }
  else
  {
    /* check validity of data structure to read */
    retval = check_structure_to_read(p_buf, msgtype, size, DATASTRUCT_CONTENT_TYPE);
    if (retval == DATAPACK_OK)
    {
      /* retrieve data */
      (void) memcpy((void *)p_data,
                    (void *)&p_buf[DATAPACK_HEADER_BYTE_SIZE + 1U],
                    (size_t) size);
    }
  }

  return (retval);
}

uint16_t DATAPACK_readMsgType(uint8_t *p_buf)
{
  uint16_t msgtype;
  /* read header: message type */
  msgtype = ((uint16_t)p_buf[0] << 8) + (uint16_t)p_buf[1];
  return ((uint16_t)msgtype);
}

uint16_t DATAPACK_readSize(uint8_t *p_buf)
{
  uint16_t size;
  /* read header: size */
  size = ((uint16_t)p_buf[2] << 8) + (uint16_t)p_buf[3];
  return (size);
}

/* static functions ---------------------------------------------------------------------- */
static DataPack_Status_t check_structure_to_read(uint8_t *p_buf, uint16_t msgtype,
                                                 uint16_t expect_size, uint8_t expect_type)
{
  DataPack_Status_t retval;
  uint16_t rx_msgtype;

  /* check that indicated msg type and received msg type are matching */
  rx_msgtype = DATAPACK_readMsgType(p_buf);
  if (rx_msgtype != msgtype)
  {
    PRINT_INFO("DATAPACK read msgtype not matching (%d vs %d)", rx_msgtype, msgtype)
    retval = DATAPACK_ERROR;
  }
  else
  {
    /* check that expect_size and received size are matching */
    uint16_t rx_size;
    rx_size = DATAPACK_readSize(p_buf);
    if (rx_size != expect_size)
    {
      PRINT_ERR("DATAPACK read size error (%d, %d)", rx_size, expect_size)
      retval = DATAPACK_ERROR;
    }
    else
    {
      /* check that expect_type corresponds to the received content type */
      uint8_t rx_contenttype;
      rx_contenttype = p_buf[4];
      if (rx_contenttype != expect_type)
      {
        PRINT_ERR("DATAPACK read content type not matching")
        retval = DATAPACK_ERROR;
      }
      else
      {
        retval = DATAPACK_OK;
      }
    }
  }

  return (retval);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

