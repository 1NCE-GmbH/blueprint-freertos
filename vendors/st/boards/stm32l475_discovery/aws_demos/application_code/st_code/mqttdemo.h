/**
  ******************************************************************************
  * @file    mqttdemo.h
  * @author  MCD Application Team
  * @brief   Header for mqttdemo.c module
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
#ifndef MQTT_DEMO_H
#define MQTT_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_ATCMD, DBL_LVL_P0, format "\n\r", ## args)
//#define PRINT_FORCE( ... ) PRINT_FORCE(format, args...)
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_MQTTDEMO, DBL_LVL_P0, "MqttDemo: " format "\n\r", ## args)
//#define PRINT_INFO( ... )     PRINT_INFO(format, args...)


#define PRINT_DBG(format, args...) TRACE_PRINT(DBG_CHAN_MQTTDEMO, DBL_LVL_P0, "MqttDemo: " format "\n\r", ## args)
//#define PRINT_DBG( ... )    PRINT_DBG(format, args...)
#define PRINT_ERR(format, args...)  \
  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "MqttDemo ERROR: " format "\n\r", ## args)
//#define PRINT_ERR( ... ) PRINT_ERR(format, args...)
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/**
  * @brief  Initialization
  * @note   mqttdemo initialization - first function to call
  * @param  -
  * @retval -
  */
void mqttdemo_init(void);

/**
  * @brief  Start
  * @note   mqttdemo start - must be called after mqttdemo_init
  * @param  -
  * @retval -
  */
void mqttdemo_start(void);


#ifdef __cplusplus
}
#endif

#endif /* MQTT_DEMO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
