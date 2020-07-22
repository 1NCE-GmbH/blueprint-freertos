/**
  ******************************************************************************
  * @file    dc_control.c
  * @author  MCD Application Team
  * @brief   This file contains all the functions for dc_control managememnt
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
#include "cmsis_os_misrac2012.h"
#include "plf_config.h"

#include "dc_control.h"
#include "error_handler.h"

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* debounce timer */
#define DEBOUNCE_TIMEOUT  (200U) /* in millisec */
#define DC_CTRL_MSG_QUEUE_SIZE (uint32_t) (8)

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static osMessageQId dc_ctrl_msg_queue;
static dc_control_button_t dc_control_button_up;
static dc_control_button_t dc_control_button_dn;
static dc_control_button_t dc_control_button_right;
static dc_control_button_t dc_control_button_left;
static dc_control_button_t dc_control_button_sel;

static osTimerId DebounceTimerHandle;
static __IO uint8_t debounce_ongoing = 0U;

/* Global variables ----------------------------------------------------------*/
dc_com_res_id_t    DC_COM_BUTTON_UP      = DC_COM_INVALID_ENTRY;
dc_com_res_id_t    DC_COM_BUTTON_DN      = DC_COM_INVALID_ENTRY;
dc_com_res_id_t    DC_COM_BUTTON_RIGHT   = DC_COM_INVALID_ENTRY;
dc_com_res_id_t    DC_COM_BUTTON_LEFT    = DC_COM_INVALID_ENTRY;
dc_com_res_id_t    DC_COM_BUTTON_SEL     = DC_COM_INVALID_ENTRY;

/* Private function prototypes -----------------------------------------------*/
static void dc_CtrlStartTask(void const *argument);
static void dc_ctrl_osTimerDebounceCallback(void const *argument);

/* Functions Definition ------------------------------------------------------*/

/* static functions ------------------------------------------------------- */

/* dc_ctrl task core function */
static void dc_CtrlStartTask(void const *argument)
{
  osEvent event;
  dc_com_event_id_t event_id;

  for (;;)
  {
    event = osMessageGet(dc_ctrl_msg_queue, RTOS_WAIT_FOREVER);
    if (event.status == osEventMessage)
    {
      event_id = (dc_com_event_id_t)event.value.v;
      (void)dc_com_write_event(&dc_com_db, event_id) ;
    }
  }
}

/* dc_ctrl_osTimerDebounceCallback function */
static void dc_ctrl_osTimerDebounceCallback(void const *argument)
{
  UNUSED(argument);
  debounce_ongoing = 0U;
}

/* external functions ------------------------------------------------------- */
/**
  * @brief  post an event
  * @param  event_id            event id
  * @retval dc_ctrl_status_t    return status
  */
void dc_ctrl_post_event_normal(dc_com_event_id_t event_id)
{
  /* always post event to the queue */
  if (osMessagePut(dc_ctrl_msg_queue, (uint32_t)event_id, 0U) != osOK)
  {
    ERROR_Handler(DBG_CHAN_DATA_CACHE, 7, ERROR_WARNING);
  }
}

/**
  * @brief  debounce event management
  * @param  event_id            event id
  * @retval dc_ctrl_status_t    return status
  */
void dc_ctrl_post_event_debounce(dc_com_event_id_t event_id)
{
  /* post event to the queue only if no ongoing debounce
   * limitation: all events with debounce are sharing same timer
   */
  if (debounce_ongoing == 0U)
  {
    if (osMessagePut(dc_ctrl_msg_queue, (uint32_t)event_id, 0U) != osOK)
    {
      ERROR_Handler(DBG_CHAN_DATA_CACHE, 7, ERROR_WARNING);
    }
    else
    {
      (void)osTimerStart(DebounceTimerHandle, DEBOUNCE_TIMEOUT);
      debounce_ongoing = 1U;
    }
  }
}


/**
  * @brief  component initialisation
  * @param  -
  * @retval dc_ctrl_status_t     return status
  */
dc_ctrl_status_t dc_ctrl_init(void)
{
  /* definition and creation of DebounceTimer */
  osTimerDef(DebounceTimer, dc_ctrl_osTimerDebounceCallback);
  DebounceTimerHandle = osTimerCreate(osTimer(DebounceTimer), osTimerOnce, NULL);

  /* queue creation */
  osMessageQDef(DC_CTRL_MSG_QUEUE, DC_CTRL_MSG_QUEUE_SIZE, uint32_t);
  dc_ctrl_msg_queue = osMessageCreate(osMessageQ(DC_CTRL_MSG_QUEUE), NULL);

  DC_COM_BUTTON_UP      = dc_com_register_serv(&dc_com_db, (void *)&dc_control_button_up,
                                               (uint16_t)sizeof(dc_control_button_t));
  DC_COM_BUTTON_DN      = dc_com_register_serv(&dc_com_db, (void *)&dc_control_button_dn,
                                               (uint16_t)sizeof(dc_control_button_t));
  DC_COM_BUTTON_RIGHT   = dc_com_register_serv(&dc_com_db, (void *)&dc_control_button_right,
                                               (uint16_t)sizeof(dc_control_button_t));
  DC_COM_BUTTON_LEFT    = dc_com_register_serv(&dc_com_db, (void *)&dc_control_button_left,
                                               (uint16_t)sizeof(dc_control_button_t));
  DC_COM_BUTTON_SEL     = dc_com_register_serv(&dc_com_db, (void *)&dc_control_button_sel,
                                               (uint16_t)sizeof(dc_control_button_t));

  return DC_CTRL_OK;
}

/**
  * @brief  component start
  * @param  -
  * @retval dc_ctrl_status_t     return status
  */
dc_ctrl_status_t dc_ctrl_start(void)
{
  static osThreadId dc_CtrlTaskId;

  /* definition and creation of dc_CtrlEventTask */
  osThreadDef(dc_CtrlTask, dc_CtrlStartTask, DC_CTRL_THREAD_PRIO, 0, DC_CTRL_THREAD_STACK_SIZE);
  dc_CtrlTaskId = osThreadCreate(osThread(dc_CtrlTask), NULL);
  if (dc_CtrlTaskId == NULL)
  {
    ERROR_Handler(DBG_CHAN_DATA_CACHE, 7, ERROR_WARNING);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(dc_CtrlTaskId, DC_CTRL_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }

  if (DC_COM_BUTTON_UP != DC_COM_INVALID_ENTRY)
  {
    dc_control_button_up.rt_state    = DC_SERVICE_ON;
  }
  if (DC_COM_BUTTON_DN != DC_COM_INVALID_ENTRY)
  {
    dc_control_button_dn.rt_state    = DC_SERVICE_ON;
  }
  if (DC_COM_BUTTON_RIGHT != DC_COM_INVALID_ENTRY)
  {
    dc_control_button_right.rt_state = DC_SERVICE_ON;
  }
  if (DC_COM_BUTTON_LEFT != DC_COM_INVALID_ENTRY)
  {
    dc_control_button_left.rt_state = DC_SERVICE_ON;
  }
  if (DC_COM_BUTTON_SEL != DC_COM_INVALID_ENTRY)
  {
    dc_control_button_sel.rt_state   = DC_SERVICE_ON;
  }


  return DC_CTRL_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
