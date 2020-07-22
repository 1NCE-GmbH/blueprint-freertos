/**
  ******************************************************************************
  * @file    ppposif_ipc.h
  * @author  MCD Application Team
  * @brief   Header for ppposif_ipc.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PPOSIF_IPC_H
#define PPOSIF_IPC_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ppposif.h"
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#include "main.h"

/* LwIP is a Third Party so MISRAC messages linked to it are ignored */
/*cstat -MISRAC2012-* */
#include "cc.h"
/*cstat +MISRAC2012-* */

#include "ipc_uart.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  component init
  * @param  pDevice: device to init.
  * @retval return code
  */
extern void  ppposif_ipc_init(IPC_Device_t pDevice);

/**
  * @brief  Tx Send data
  * @param  pDevice: device .
  * @param  data: buffer data to send.
  * @param  len: data size to send.
  * @retval data sent byte number
  */
extern int16_t ppposif_ipc_write(IPC_Device_t pDevice, u8_t *data, int16_t len);

/**
  * @brief  Rcv data
  * @param  pDevice: serial device.
  * @param  data: buffer data to read.
  * @param  len: data size to read.
  * @retval data rcv byte number
  */
extern int16_t   ppposif_ipc_read(IPC_Device_t pDevice, u8_t *buff, int16_t size);

/**
  * @brief  component de init
  * @param  pDevice: device to de init.
  * @retval return code
  */
extern void  ppposif_ipc_deinit(IPC_Device_t pDevice);

/**
  * @brief  data IPC channel select
  * @param  pDevice: device to init.
  * @retval return code
  */
extern void  ppposif_ipc_select(IPC_Device_t pDevice);
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */


#ifdef __cplusplus
}
#endif


#endif /* PPOSIF_IPC_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
