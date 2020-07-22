/**
  ******************************************************************************
  * @file    httpclient_config.h
  * @author  MCD Application Team
  * @brief   default configuration parameters for http client
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
#ifndef HTTPCLIENT_CONFIG_H
#define HTTPCLIENT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define HTTPCLIENT_CONFIG_UNITARY_TEST (0) /* 0: unitary tests not activated,
                                              1: unitary tests activated */

#define HTTPCLIENT_DEFAULT_ENABLED ((uint8_t *)"0")

#define IOT_SERVER_NAME        ((uint8_t *)"grovestreams.com")
#define IOT_SERVER_IP          ((uint32_t)0xA30CECADU)  /* 173.236.12.163 */
#define IOT_SERVER_PORT        ((uint16_t)80U)

#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 1)
#define HTTP_SERVER_TEST_NAME  ((uint8_t *)"parishttpserver.st")
#define HTTP_SERVER_TEST_IP    ((uint32_t)0x0102A8C0U)   /* 192.168.2.1 */
#define HTTP_SERVER_TEST_PORT  ((uint16_t)8888U)
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 1 */

#define HTTPCLIENT_DEFAULT_CLOUD_KEY_PUT ((uint8_t*)"467b0b6a-a0ba-3860-94aa-92e5aa9baec6")
#define HTTPCLIENT_DEFAULT_CLOUD_KEY_GET ((uint8_t*)"5087466e-2e51-357f-94c8-e8b935396a42")

#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 0)
#define HTTPCLIENT_DEFAULT_DISTANTNAME  IOT_SERVER_NAME
#define HTTPCLIENT_DEFAULT_DISTANTIP    IOT_SERVER_IP
#define HTTPCLIENT_DEFAULT_DISTANTPORT  IOT_SERVER_PORT
#else
#define HTTPCLIENT_DEFAULT_DISTANTNAME  HTTP_SERVER_TEST_NAME
#define HTTPCLIENT_DEFAULT_DISTANTIP    HTTP_SERVER_TEST_IP
#define HTTPCLIENT_DEFAULT_DISTANTPORT  HTTP_SERVER_TEST_PORT
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 0 */

#define HTTPCLIENT_DEFAULT_COMPONENT_ID        ((uint8_t*)"Sensors_DEF")
#define HTTPCLIENT_DEFAULT_PUT_REQUEST_PERIOD  ((uint8_t*)"25")
#define HTTPCLIENT_DEFAULT_GET_REQUEST_PERIOD  ((uint8_t*)"2")
#define HTTPCLIENT_DEFAULT_SENSOR1_TYPE        ((uint8_t*)"1")
#define HTTPCLIENT_DEFAULT_SENSOR1_ID          ((uint8_t*)"batlevel")
#define HTTPCLIENT_DEFAULT_SENSOR2_TYPE        ((uint8_t*)"2")
#define HTTPCLIENT_DEFAULT_SENSOR2_ID          ((uint8_t*)"siglevel")
#define HTTPCLIENT_DEFAULT_SENSOR3_TYPE        ((uint8_t*)"5")
#define HTTPCLIENT_DEFAULT_SENSOR3_ID          ((uint8_t*)"hum")
#define HTTPCLIENT_DEFAULT_SENSOR4_TYPE        ((uint8_t*)"6")
#define HTTPCLIENT_DEFAULT_SENSOR4_ID          ((uint8_t*)"temp")
#define HTTPCLIENT_DEFAULT_SENSOR5_TYPE        ((uint8_t*)"7")
#define HTTPCLIENT_DEFAULT_SENSOR5_ID          ((uint8_t*)"press")
#define HTTPCLIENT_DEFAULT_SENSOR_END          ((uint8_t*)"0")
#define HTTPCLIENT_DEFAULT_GET_TYPE            ((uint8_t*)"1")
#define HTTPCLIENT_DEFAULT_GET_ID              ((uint8_t*)"ledlight")
#define HTTPCLIENT_DEFAULT_GET_END             ((uint8_t*)"0")

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* HTTPCLIENT_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
