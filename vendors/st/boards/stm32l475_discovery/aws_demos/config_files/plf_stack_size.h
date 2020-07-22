/**
  ******************************************************************************
  * @file    stack_size.h
  * @author  MCD Application Team
  * @brief   This file contains the size of all the stacks
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
#ifndef PLF_STACK_SIZE_H
#define PLF_STACK_SIZE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "plf_features.h"

/* Exported constants --------------------------------------------------------*/

#define TCPIP_THREAD_STACK_SIZE             (512U)
#define DEFAULT_THREAD_STACK_SIZE           (384U)
#define FREERTOS_TIMER_THREAD_STACK_SIZE    (256U)
#define FREERTOS_IDLE_THREAD_STACK_SIZE     (128U)

#define ATCORE_THREAD_STACK_SIZE            (320U)
#define CELLULAR_SERVICE_THREAD_STACK_SIZE  (512U)
#define NIFMAN_THREAD_STACK_SIZE            (384U)

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#define PPPOSIF_CLIENT_THREAD_STACK_SIZE    (640U)
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#define DC_CTRL_THREAD_STACK_SIZE           (256U)
#if (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1)
#define DC_MEMS_THREAD_STACK_SIZE           (256U)
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */

#if (USE_HTTP_CLIENT == 1)
#define HTTPCLIENT_THREAD_STACK_SIZE        (448U)
#endif /* (USE_HTTP_CLIENT == 1) */
#if (USE_PING_CLIENT == 1)
#define PINGCLIENT_THREAD_STACK_SIZE        (384U)
#endif /* (USE_PING_CLIENT == 1) */
#if (USE_MQTT_DEMO == 1)
//#define mqttdemo_THREAD_STACK_SIZE        (384U)
#define mqttdemo_THREAD_STACK_SIZE        (7000U)

#endif /* (USE_MQTT_DEMO == 1) */

#if (USE_CMD_CONSOLE == 1)
#define CMD_THREAD_STACK_SIZE               (600U)
#endif /* (USE_CMD_CONSOLE == 1) */

#if (USE_NETWORK_LIBRARY == 1)
#define NET_CELLULAR_BASE_THREAD_STACK_SIZE      DEFAULT_THREAD_STACK_SIZE
#endif /* (USE_NETWORK_LIBRARY == 1) */

#if (USE_NETEX1 == 1)
#define NETEX1_THREAD_STACK_SIZE            (600U)
#endif /* (USE_NETEX1 == 1) */

#define USED_DC_CTRL_THREAD_STACK_SIZE           DC_CTRL_THREAD_STACK_SIZE
#define USED_ATCORE_THREAD_STACK_SIZE            ATCORE_THREAD_STACK_SIZE
#define USED_CELLULAR_SERVICE_THREAD_STACK_SIZE  CELLULAR_SERVICE_THREAD_STACK_SIZE
#define USED_NIFMAN_THREAD_STACK_SIZE            NIFMAN_THREAD_STACK_SIZE
#define USED_DEFAULT_THREAD_STACK_SIZE           DEFAULT_THREAD_STACK_SIZE
#define USED_FREERTOS_TIMER_THREAD_STACK_SIZE    FREERTOS_TIMER_THREAD_STACK_SIZE
#define USED_FREERTOS_IDLE_THREAD_STACK_SIZE     FREERTOS_IDLE_THREAD_STACK_SIZE

#define USED_DC_CTRL_THREAD           1
#define USED_ATCORE_THREAD            1
#define USED_NIFMAN_THREAD            1
#define USED_CELLULAR_SERVICE_THREAD  1
#define USED_DEFAULT_THREAD           1
#define USED_FREERTOS_TIMER_THREAD    1
#define USED_FREERTOS_IDLE_THREAD     1


#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#define USED_TCPIP_THREAD_STACK_SIZE           TCPIP_THREAD_STACK_SIZE
#define USED_TCPIP_THREAD                      1
#else
#define USED_TCPIP_THREAD_STACK_SIZE           0U
#define USED_TCPIP_THREAD                      0
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#define USED_PPPOSIF_CLIENT_THREAD_STACK_SIZE  PPPOSIF_CLIENT_THREAD_STACK_SIZE
#define USED_PPPOSIF_CLIENT_THREAD             1
#else
#define USED_PPPOSIF_CLIENT_THREAD_STACK_SIZE  0U
#define USED_PPPOSIF_CLIENT_THREAD             0
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#if (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1)
#define USED_DC_MEMS_THREAD_STACK_SIZE           DC_MEMS_THREAD_STACK_SIZE
#define USED_DC_MEMS_THREAD                        1
#else
#define USED_DC_MEMS_THREAD_STACK_SIZE             0U
#define USED_DC_MEMS_THREAD                        0
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */

#if (USE_MQTT_DEMO == 1)
#define USED_mqttdemo_THREAD_STACK_SIZE        mqttdemo_THREAD_STACK_SIZE
#define USED_mqttdemo_THREAD                   1
#else
#define USED_mqttdemo_THREAD_STACK_SIZE        0U
#define USED_mqttdemo_THREAD                   0
#endif /* (USE_MQTT_DEMO == 1) */

#if (USE_HTTP_CLIENT == 1)
#define USED_HTTPCLIENT_THREAD_STACK_SIZE        HTTPCLIENT_THREAD_STACK_SIZE
#define USED_HTTPCLIENT_THREAD                   1
#else
#define USED_HTTPCLIENT_THREAD_STACK_SIZE        0U
#define USED_HTTPCLIENT_THREAD                   0
#endif /* (USE_HTTP_CLIENT == 1) */

#if (USE_PING_CLIENT == 1)
#define USED_PINGCLIENT_THREAD_STACK_SIZE        PINGCLIENT_THREAD_STACK_SIZE
#define USED_PINGCLIENT_THREAD                   1
#else
#define USED_PINGCLIENT_THREAD_STACK_SIZE        0U
#define USED_PINGCLIENT_THREAD                   0
#endif /* (USE_PING_CLIENT == 1) */

#if (USE_CMD_CONSOLE == 1)
#define USED_CMD_THREAD_STACK_SIZE           CMD_THREAD_STACK_SIZE
#define USED_CMD_THREAD                      1
#else
#define USED_CMD_THREAD_STACK_SIZE           0U
#define USED_CMD_THREAD                      0
#endif /* (USE_CMD_CONSOLE == 1) */

#if (USE_NETWORK_LIBRARY == 1)
#define NET_CELLULAR_THREAD_STACK_SIZE  NET_CELLULAR_BASE_THREAD_STACK_SIZE
#define USED_NET_CELLULAR_THREAD             1
#else
#define NET_CELLULAR_THREAD_STACK_SIZE  0U
#define USED_NET_CELLULAR_THREAD             0
#endif /* (USE_NETWORK_LIBRARY == 1) */

#if (USE_NETEX1 == 1)
#define USED_NETEX1_THREAD_STACK_SIZE        NETEX1_THREAD_STACK_SIZE
#define USED_NETEX1_THREAD                   1
#else
#define USED_NETEX1_THREAD_STACK_SIZE        0U
#define USED_NETEX1_THREAD                   0
#endif /* (USE_NETEX1 == 1) */

#define TOTAL_THREAD_STACK_SIZE                \
  USED_TCPIP_THREAD_STACK_SIZE                 \
  +USED_DEFAULT_THREAD_STACK_SIZE              \
  +USED_PPPOSIF_CLIENT_THREAD_STACK_SIZE       \
  +USED_DC_CTRL_THREAD_STACK_SIZE              \
  +USED_ATCORE_THREAD_STACK_SIZE               \
  +USED_NIFMAN_THREAD_STACK_SIZE               \
  +USED_DC_MEMS_THREAD_STACK_SIZE              \
  +USED_mqttdemo_THREAD_STACK_SIZE           \
  +USED_HTTPCLIENT_THREAD_STACK_SIZE           \
  +USED_PINGCLIENT_THREAD_STACK_SIZE           \
  +USED_FREERTOS_TIMER_THREAD_STACK_SIZE       \
  +USED_FREERTOS_IDLE_THREAD_STACK_SIZE        \
  +USED_CMD_THREAD_STACK_SIZE                  \
  +NET_CELLULAR_THREAD_STACK_SIZE              \
  +USED_NETEX1_THREAD_STACK_SIZE               \
  +USED_CELLULAR_SERVICE_THREAD_STACK_SIZE



#define THREAD_NUMBER                \
  USED_TCPIP_THREAD                  \
  +USED_DEFAULT_THREAD               \
  +USED_PPPOSIF_CLIENT_THREAD        \
  +USED_DC_CTRL_THREAD               \
  +USED_ATCORE_THREAD                \
  +USED_NIFMAN_THREAD                \
  +USED_DC_MEMS_THREAD               \
  +USED_mqttdemo_THREAD            \
  +USED_HTTPCLIENT_THREAD            \
  +USED_PINGCLIENT_THREAD            \
  +USED_FREERTOS_TIMER_THREAD        \
  +USED_FREERTOS_IDLE_THREAD         \
  +USED_CMD_THREAD                   \
  +USED_NETEX1_THREAD                \
  +USED_NET_CELLULAR_THREAD          \
  +USED_CELLULAR_SERVICE_THREAD

#ifndef APPLICATION_HEAP_SIZE
#define APPLICATION_HEAP_SIZE       (0U)
#endif  /* APPLICATION_HEAP_SIZE */

#define PARTIAL_HEAP_SIZE       (8196U)
#define TOTAL_HEAP_SIZE         (((TOTAL_THREAD_STACK_SIZE)*4U)+(size_t)PARTIAL_HEAP_SIZE+(size_t)APPLICATION_HEAP_SIZE)

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* PLF_STACK_SIZE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
