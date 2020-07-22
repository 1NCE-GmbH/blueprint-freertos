/**
  ******************************************************************************
  * @file    com_sockets_net_compat.h
  * @author  MCD Application Team
  * @brief   Com sockets net definition for compatibility LwIP / Modem
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
#ifndef COM_SOCKETS_NET_COMPAT_H
#define COM_SOCKETS_NET_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"

/* Common Exported constants -------------------------------------------------*/
/** @addtogroup COM_SOCKETS_Constants
  * @{
  */
#define COM_SOCKET_INVALID_ID ((int32_t)-1) /*!< Socket invalid Id */

/**
  * @}
  */

#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)

/* Exported constants --------------------------------------------------------*/
/** @addtogroup COM_SOCKETS_Constants
  * @{
  */

#define COM_INADDR_ANY          ((uint32_t)0x00000000UL) /*!< All IP adresses accepted */

/* Socket Family */
#define COM_AF_UNSPEC       0 /*!< Socket Family Unspecified */
#define COM_AF_INET         2 /*!< Socket Family Internet Address Family */
#define COM_AF_INET6        COM_AF_UNSPEC /*!< Socket Family Internet Address Family version 6
                                               When USE_SOCKETS_TYPE = USE_SOCKETS_MODEM - NOT Supported */

/* Socket Type TCP/UDP/RAW */
#define COM_SOCK_STREAM     1 /*!< Socket Type Stream(TCP) - Supported */
#define COM_SOCK_DGRAM      2 /*!< Socket Type Datagram(UDP) - Supported */
#define COM_SOCK_RAW        3 /*!< Socket Type Raw
                                   When USE_SOCKETS_TYPE = USE_SOCKETS_MODEM - NOT Supported */

/* Socket Protocol */
#define COM_IPPROTO_IP      0  /*!< Socket Protocol IPv4 Level   */
#define COM_IPPROTO_ICMP    1  /*!< Socket Protocol ICMP
                                    When USE_SOCKETS_TYPE = USE_SOCKETS_MODEM - NOT Supported */
#define COM_IPPROTO_UDP     2  /*!< Socket Protocol UDP Protocol */
#define COM_IPPROTO_IPV6    3  /*!< Socket Protocol IPv6 Level
                                    When USE_SOCKETS_TYPE = USE_SOCKETS_MODEM - NOT Supported */
#define COM_IPPROTO_TCP     6  /*!< Socket Protocol TCP Protocol */

/*
 * Socket Options
 */
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define COM_SOL_SOCKET     0xfff  /*!< options for socket level - used for (get/set)sockopt() */

#define COM_SO_SNDTIMEO    0x1005 /*!< Socket Options send timeout - used for (get/set)sockopt() */
#define COM_SO_RCVTIMEO    0x1006 /*!< Socket Options receive timeout - used for (get/set)sockopt() */
#define COM_SO_ERROR       0x1007 /*!< Socket Options get error status and clear - used for (get/set)sockopt() */

/* Flags used with recv. */
#define COM_MSG_WAIT       0x00    /*!< Blocking     */
#define COM_MSG_DONTWAIT   0x01    /*!< Non blocking */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @addtogroup COM_SOCKETS_Types
  * @{
  */

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @addtogroup COM_SOCKETS_Variables
  * @{
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @addtogroup COM_SOCKETS_Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @addtogroup COM_SOCKETS_Functions
  * @{
  */

/**
  * @}
  */

#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM) */

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)

/* Includes ------------------------------------------------------------------*/
/* only when USE_SOCKETS_TYPE == USE_SOCKETS_LWIP */
/* LwIP is a Third Party so MISRAC messages linked to it are ignored */
/*cstat -MISRAC2012-* */
#include "lwip/sockets.h"
/*cstat +MISRAC2012-* */

/* Exported constants --------------------------------------------------------*/
#define COM_INADDR_ANY      INADDR_ANY

/* Socket Family */
#define COM_AF_UNSPEC       AF_UNSPEC
#define COM_AF_INET         AF_INET
#define COM_AF_INET6        AF_INET6

/* Socket Type TCP/UDP/RAW */
#define COM_SOCK_STREAM     SOCK_STREAM
#define COM_SOCK_DGRAM      SOCK_DGRAM
#define COM_SOCK_RAW        SOCK_RAW

/* Socket Protocol */
#define COM_IPPROTO_IP      IPPROTO_IP
#define COM_IPPROTO_ICMP    IPPROTO_ICMP
#define COM_IPPROTO_UDP     IPPROTO_UDP
#define COM_IPPROTO_IPV6    IPPROTO_IPV6
#define COM_IPPROTO_TCP     IPPROTO_TCP

/*
 * Socket Options
 */
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define COM_SOL_SOCKET     SOL_SOCKET

#define COM_SO_SNDTIMEO    SO_SNDTIMEO
#define COM_SO_RCVTIMEO    SO_RCVTIMEO
#define COM_SO_ERROR       SO_ERROR

/* Flags used with recv. */
#define COM_MSG_WAIT       0x00
#define COM_MSG_DONTWAIT   MSG_DONTWAIT

/* Exported types ------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */


#ifdef __cplusplus
}
#endif

#endif /* COM_SOCKETS_NET_COMPAT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
