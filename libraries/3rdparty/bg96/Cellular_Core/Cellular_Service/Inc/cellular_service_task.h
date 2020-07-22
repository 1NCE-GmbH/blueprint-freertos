/**
  ******************************************************************************
  * @file    cellular_service_task.h
  * @author  MCD Application Team
  * @brief   Header for cellular_service_task.c module
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
#ifndef CELLULAR_SERVICE_TASK_H
#define CELLULAR_SERVICE_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os_misrac2012.h"
#include "cellular_service.h"

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* automaton event type */
typedef uint16_t CST_autom_event_t;

/* Cellular service automaton states */
typedef enum
{
  CST_MODEM_INIT_STATE,
  CST_MODEM_RESET_STATE,
  CST_MODEM_OFF_STATE,
  CST_MODEM_ON_STATE,
  CST_MODEM_ON_ONLY_STATE,
  CST_MODEM_POWERED_ON_STATE,
  CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE,
  CST_WAITING_FOR_NETWORK_STATUS_STATE,
  CST_NETWORK_STATUS_OK_STATE,
  CST_MODEM_REGISTERED_STATE,
  CST_MODEM_PDN_ACTIVATE_STATE,
  CST_MODEM_PDN_ACTIVATED_STATE,
  CST_MODEM_DATA_READY_STATE,
  CST_MODEM_REPROG_STATE,
  CST_MODEM_FAIL_STATE,
  CST_MODEM_NETWORK_STATUS_FAIL_STATE,
  CST_MODEM_SIM_ONLY_STATE,
  CST_MODEM_POWER_DATA_IDLE_STATE
} CST_state_t;

/* ================================= */
/* List of automation events - BEGIN */
/* ================================= */
#define CST_INIT_EVENT                             1U  /* init */
#define CST_MODEM_POWER_ON_EVENT                   2U  /* modem power on request */
#define CST_MODEM_POWERED_ON_EVENT                 3U  /* modem powered on */
#define CST_MODEM_INITIALIZED_EVENT                4U  /* modem initialized */
#define CST_NETWORK_CALLBACK_EVENT                 5U  /* cellular callback  */
#define CST_SIGNAL_QUALITY_EVENT                   6U  /* signal quality updated */
#define CST_NW_REG_TIMEOUT_TIMER_EVENT             7U  /* register timeout reached */
#define CST_NETWORK_STATUS_EVENT                   8U  /* network status to check */
#define CST_NETWORK_STATUS_OK_EVENT                9U  /* network status ok */
#define CST_MODEM_ATTACHED_EVENT                  10U  /* modem atteched */
#define CST_PDP_ACTIVATED_EVENT                   11U  /* PDN activated */
#define CST_MODEM_PDN_ACTIVATE_RETRY_TIMER_EVENT  12U  /* timer to retry PDN activation reached */
#define CST_PDN_STATUS_EVENT                      13U  /* PDN status to check */
#define CST_CELLULAR_DATA_FAIL_EVENT              14U  /* cellular data fail */
#define CST_FAIL_EVENT                            15U  /* modem fail */
#define CST_POLLING_TIMER                         16U  /* time to poll modem */
#define CST_MODEM_URC                             17U  /* modem URC received */
#define CST_NO_EVENT                              18U  /* no event to process */
#define CST_CMD_UNKWONW_EVENT                     19U  /* unknown cmd reveived */
#define CST_CELLULAR_STATE_EVENT                  20U  /* modem target state request */
#define CST_APN_CONFIG_EVENT                      21U  /* new apn config rerquest */
#define CST_NIFMAN_EVENT                          22U  /* NIFMAN NETWORK INTERFACE event */
#if (USE_LOW_POWER == 1)
#define CST_POWER_SLEEP_TIMEOUT_EVENT             23U  /* low power entry timeout */
#define CST_POWER_SLEEP_REQUEST_EVENT             24U  /* low power request */
#define CST_POWER_SLEEP_COMPLETE_EVENT            25U  /* low power completed */
#define CST_POWER_WAKEUP_EVENT                    26U  /* exit from low power */
#define CST_POWER_SLEEP_ABORT_EVENT               27U  /* low power request abort */
#endif /* (USE_LOW_POWER == 1) */


/* list of event messages to send to cellular automaton task */
typedef enum
{
  CST_MESSAGE_CS_EVENT    = 0,
  CST_MESSAGE_DC_EVENT    = 1,
  CST_MESSAGE_URC_EVENT   = 2,
  CST_MESSAGE_TIMER_EVENT = 3,
  CST_MESSAGE_CMD         = 4
} CST_message_type_t;

/* External variables --------------------------------------------------------*/
extern CST_state_t CST_current_state;
extern uint8_t    *CST_SimSlotName_p[3];
extern uint8_t    CST_polling_active;

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/**
  * @brief  starts cellular service command
  * @note   cellular service task automaton and tempos are started
  * @param  none
  * @retval return code
  */
#if (USE_CMD_CONSOLE == 1)
extern CS_Status_t CST_cmd_cellular_service_start(void);
#endif  /*  (USE_CMD_CONSOLE == 1) */

/**
  * @brief  allows to get cellular service automaton current state
  * @param  none
  * @retval automaton state
  */
extern CST_state_t CST_get_state(void);

/**
  * @brief  allows to set radio on: start cellular automaton
  * @param  none
  * @retval return code
  */
extern CS_Status_t CST_radio_on(void);

/**
  * @brief  allows to boot modem only without network register
  * @note   the application is not started
  * @param  none
  * @retval return code
  */
extern CS_Status_t CST_modem_power_on(void);

/**
  * @brief  initializes cellular service component
  * @param  none
  * @retval return code
  */
extern CS_Status_t CST_cellular_service_init(void);

/**
  * @brief  starts cellular service component
  * @note   cellular service task automaton and tempos are started
  * @param  none
  * @retval return code
  */
extern CS_Status_t CST_cellular_service_start(void);

/**
  * @brief  allows to get the modem IP addr
  * @note   to be able to return an IP address the modem must be in data tranfer state
  * @note   else an error is returned
  * @param  ip_addr_type - type of IP address
  * @param  p_ip_addr_value - IP address value returned by the function
  * @retval return code
  */
extern CS_Status_t CST_get_dev_IP_address(CS_IPaddrType_t *ip_addr_type, CS_CHAR_t *p_ip_addr_value);


/**
  * @brief  sends a message to cellular service task automaton
  * @param  type - type of massage
  * @param  event - event
  * @retval none
  */
extern void  CST_send_message(CST_message_type_t  type, CST_autom_event_t event);

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_SERVICE_TASK_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

