/**
  ******************************************************************************
  * @file    dc_control.h
  * @author  MCD Application Team
  * @brief   Header for dc_control.c module
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
#ifndef DC_CONTROL_H
#define DC_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "dc_common.h"

/** @addtogroup DC
  * @{
  */

/** @defgroup DC_CONTROL Data Cache Control services
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DC_CONTROL_Constants Constants
  * @{
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup DC_CONTROL_Types Types
  * @{
  */

typedef enum
{
  DC_CTRL_OK = 0x00,
  DC_CTRL_ERROR
} dc_ctrl_status_t;


/* Data Cache structure for button entries */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t  rt_state;
} dc_control_button_t;

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup DC_CONTROL_Variables Variables
  * @{
  */

/* Button event Data Cache entries */
/** @brief Control Button UP Data Cache entries         */
extern dc_com_res_id_t    DC_COM_BUTTON_UP;

/** @brief Control Button DOWN Data Cache entries       */
extern dc_com_res_id_t    DC_COM_BUTTON_DN;

/** @brief Control Button RIGHT Data Cache entries      */
extern dc_com_res_id_t    DC_COM_BUTTON_RIGHT;

/** @brief Control Button LEFT Data Cache entries       */
extern dc_com_res_id_t    DC_COM_BUTTON_LEFT;

/** @brief Control Button SELECTION Data Cache entries  */
extern dc_com_res_id_t    DC_COM_BUTTON_SEL;

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup DC_CONTROL_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup DC_CONTROL_Functions Functions
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/*** Component Initialization/Start *******************************************/
/*** Used by Cellular-Service - Not an User Interface *************************/
/**
  * @brief  Component initialisation
  * @param  -
  * @retval dc_ctrl_status_t     return status
  */
dc_ctrl_status_t dc_ctrl_init(void);


/**
  * @brief  Component start
  * @param  -
  * @retval dc_ctrl_status_t     return status
  */
dc_ctrl_status_t dc_ctrl_start(void);


/**
  * @brief  Post an event
  * @param  event_id            event id
  * @retval dc_ctrl_status_t    return status
  */
void dc_ctrl_post_event_normal(dc_com_event_id_t event_id);

/**
  * @brief  De-bounce event management
  * @param  event_id            event id
  * @retval dc_ctrl_status_t    return status
  */
void dc_ctrl_post_event_debounce(dc_com_event_id_t event_id);

#ifdef __cplusplus
}
#endif

#endif /* __DC_CONTROL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
