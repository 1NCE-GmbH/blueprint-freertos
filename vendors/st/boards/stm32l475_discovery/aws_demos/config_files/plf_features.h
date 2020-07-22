/**
  ******************************************************************************
  * @file    plf_features.h
  * @author  MCD Application Team
  * @brief   Includes feature list to include in firmware
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
#ifndef PLF_FEATURES_H
#define PLF_FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* ================================================= */
/*          USER MODE                                */
/* ================================================= */

/* ===================================== */
/* BEGIN - Cellular data mode            */
/* ===================================== */
/* Possible values for USE_SOCKETS_TYPE */
#define USE_SOCKETS_LWIP   (0)  /* define value affected to LwIP sockets type */
#define USE_SOCKETS_MODEM  (1)  /* define value affected to Modem sockets type */
/* Sockets location */

#if !defined USE_SOCKETS_TYPE
#define USE_SOCKETS_TYPE   (USE_SOCKETS_MODEM)
#endif  /* !defined USE_SOCKETS_TYPE */

/* If activated then com_ping interfaces in com_sockets module are defined
   mandatory when USE_PING_CLIENT is defined */
#if !defined USE_COM_PING
#define USE_COM_PING       (0)  /* 0: not activated, 1: activated */
#endif  /* !defined USE_COM_PING */


/* ===================================== */
/* END - Cellular data mode              */
/* ===================================== */

/* ===================================== */
/* BEGIN - Applications to include       */
/* ===================================== */
#if !defined USE_MQTT_DEMO
#define USE_MQTT_DEMO    (1) /* 0: not activated, 1: activated */
#endif  /* !defined USE_MQTT_DEMO */

#if !defined USE_HTTP_CLIENT
#define USE_HTTP_CLIENT    (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_HTTP_CLIENT */

#if !defined USE_PING_CLIENT
#define USE_PING_CLIENT    (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_PING_CLIENT */

/* MEMS setup */
/* USE_DC_MEMS enables MEMS management */
#if !defined USE_DC_MEMS
#define USE_DC_MEMS        (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_DC_MEMS */

/* USE_SIMU_MEMS enables MEMS simulation management */
#if !defined USE_SIMU_MEMS
#define USE_SIMU_MEMS      (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_SIMU_MEMS */

/* if USE_DC_MEMS and USE_SIMU_MEMS are both defined, the behaviour of availability of MEMS board:
 if  MEMS board is connected, true values are returned
 if  MEMS board is not connected, simulated values are returned
 Note: USE_DC_MEMS and USE_SIMU_MEMS are independent
*/

/* use generic datacache entries */
#if !defined USE_DC_GENERIC
#define USE_DC_GENERIC      (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_DC_GENERIC */

#if (( USE_PING_CLIENT == 1 ) && ( USE_COM_PING == 0 ))
#error USE_COM_PING must be set to 1 when Ping Client is activated.
#endif /* ( USE_PING_CLIENT == 1 ) && ( USE_COM_PING == 0 ) */

/* ===================================== */
/* END   - Applications to include       */
/* ===================================== */

/* ======================================= */
/* BEGIN -  Miscellaneous functionalities  */
/* ======================================= */

/* To activate Network Library */
#if !defined USE_NETWORK_LIBRARY
#define USE_NETWORK_LIBRARY       (0)  /* 0: not activated, 1: activated */
#endif  /* !defined USE_NETWORK_LIBRARY */


/* To include RTC service */
#if !defined USE_RTC
#define USE_RTC       (1) /* 0: not activated, 1: activated */
#endif  /* !defined USE_RTC */

/* To include cellular performance test */
#if !defined USE_CELPERF
#define USE_CELPERF           (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_CELPERF */

#if !defined USE_DEFAULT_SETUP
#define USE_DEFAULT_SETUP     (1) /* 0: Use setup menu,
                                     1: Use default parameters, no setup menu */
#endif  /* !defined USE_DEFAULT_SETUP */

/* use UART Communication between two boards */
#if !defined USE_LINK_UART
#define USE_LINK_UART         (0) /* 0: not activated, 1: activated */
#endif  /* !defined USE_LINK_UART */

/* For internal test. Do not activate ! */
#if (USE_NETWORK_LIBRARY == 1)
#if !defined USE_NETEX1
#define USE_NETEX1      (1) /* 0: not activated, 1: activated */
#endif  /* !defined USE_NETEX1 */
#endif  /* USE_NETWORK_LIBRARY == 1 */

/* ======================================= */
/* END   -  Miscellaneous functionalities  */
/* ======================================= */

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* PLF_FEATURES_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
