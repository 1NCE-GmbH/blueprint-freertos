/**
  ******************************************************************************
  * @file    cellular_service_task.c
  * @author  MCD Application Team
  * @brief   This file defines functions for Cellular Service Task
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
#include "plf_config.h"
#include "cellular_service_task.h"

#include "dc_common.h"
#include "error_handler.h"

#include "at_util.h"
#include "cellular_datacache.h"
#include "cellular_service.h"
#include "cellular_service_os.h"
#include "cellular_service_config.h"
#include "cellular_service_int.h"
#include "cellular_runtime_custom.h"

#if (USE_LOW_POWER == 1)
#include "cellular_service_power.h"
#endif  /* (USE_LOW_POWER == 1) */


/* Private defines -----------------------------------------------------------*/
/*  FOTA Test activation */
#define CST_FOTA_TEST        (0)  /* 0: test not activate - 1: test activated */

/*  NFMC Test activation */
#define CST_TEST_NFMC        (0)  /* 0: test not activate - 1: test activated */
#if (CST_TEST_NFMC == 1)
static uint32_t CST_nfmc_test_count = 0;
#define CST_NFMC_TEST_COUNT_MAX 9
#endif /* (CST_TEST_NFMC == 1) */

/*  RESET Test activation */
#define CST_RESET_DEBUG (0)       /* 0: test not activate - 1: test activated */

/* cellular register Test activation */
#define CST_TEST_REGISTER_FAIL   (0)  /* 0: test not activate - 1: test activated */
#if (CST_TEST_REGISTER_FAIL == 1)
static uint32_t CST_register_fail_test_count = 0;
#define CST_REGISTER_FAIL_TEST_COUNT_MAX 2
#endif /* (CST_TEST_REGISTER_FAIL == 1) */

#define CST_MODEM_POLLING_PERIOD_DEFAULT 5000U

#define CST_BAD_SIG_RSSI 99U

#define CST_COUNT_FAIL_MAX (5U)  /* Max of total restarts allowed after failure*/

/* Max of restarts allowed  after failure for each cause */
#define CST_POWER_ON_RESET_MAX      5U
#define CST_RESET_MAX               5U
#define CST_INIT_MODEM_RESET_MAX    5U
#define CST_CSQ_MODEM_RESET_MAX     5U
#define CST_GNS_MODEM_RESET_MAX     5U
#define CST_ATTACH_RESET_MAX        5U
#define CST_DEFINE_PDN_RESET_MAX    5U
#define CST_ACTIVATE_PDN_RESET_MAX  5U
#define CST_CELLULAR_DATA_RETRY_MAX 5U
#define CST_SIM_RETRY_MAX           (uint16_t)DC_SIM_SLOT_NB
#define CST_GLOBAL_RETRY_MAX        5U
#define CST_RETRY_MAX               5U
#define CST_CMD_RESET_MAX         100U

/* delay for PND activation retry */
#define CST_PDN_ACTIVATE_RETRY_DELAY 30000U

#define CST_FOTA_TIMEOUT      (360000U) /* 6 min (calibrated for cat-M1 network, increase it for cat-NB1) */

/* SIM slot polling period */
#define CST_SIM_POLL_COUNT     200U    /* 20s */

/* Private macros ------------------------------------------------------------*/
#if (USE_PRINTF == 0U)
/* Trace macro definition */
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, "" format "", ## args)

#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)                (void)printf(format , ## args);
#endif  /* (USE_PRINTF == 1) */
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#define PRINT_CELLULAR_SERVICE(format, args...)       \
  TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, format, ## args)
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)   \
  TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_ERR, "ERROR " format, ## args)
#else
#define PRINT_CELLULAR_SERVICE(format, args...)       (void)printf(format , ## args);
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)   (void)printf(format , ## args);
#endif /* (USE_PRINTF == 0U) */
#else
#define PRINT_CELLULAR_SERVICE(...)        __NOP(); /* Nothing to do */
#define PRINT_CELLULAR_SERVICE_ERR(...)    __NOP(); /* Nothing to do */
#endif /* (USE_TRACE_CELLULAR_SERVICE == 1U) */

/* Private typedef -----------------------------------------------------------*/


/* ================================= */
/* List of automation events - END   */
/* ================================= */


/* List of modem failure causes   */
typedef enum
{
  CST_NO_FAIL,
  CST_MODEM_POWER_ON_FAIL,
  CST_MODEM_RESET_FAIL,
  CST_MODEM_CSQ_FAIL,
  CST_MODEM_GNS_FAIL,
  CST_MODEM_REGISTER_FAIL,
  CST_MODEM_ATTACH_FAIL,
  CST_MODEM_PDP_DEFINE_FAIL,
  CST_MODEM_PDP_ACTIVATION_FAIL,
  CST_MODEM_CELLULAR_DATA_FAIL,
  CST_MODEM_SIM_FAIL,
  CST_MODEM_CMD_FAIL
} CST_fail_cause_t;

/* type of messge to send to automaton task */
typedef struct
{
  uint16_t  type ;
  uint16_t  id  ;
} CST_message_t;

/* Cellular context */
typedef struct
{
  CST_state_t          current_state;
  CST_fail_cause_t     fail_cause;
  CS_PDN_event_t       pdn_status;
  CS_SignalQuality_t   signal_quality;
  CS_NetworkRegState_t current_EPS_NetworkRegState;
  CS_NetworkRegState_t current_GPRS_NetworkRegState;
  CS_NetworkRegState_t current_CS_NetworkRegState;
  uint16_t             activate_pdn_nfmc_tempo_count;
  uint16_t             register_retry_tempo_count;
  uint16_t             power_on_reset_count ;
  uint16_t             reset_reset_count ;
  uint16_t             init_modem_reset_count ;
  uint16_t             csq_reset_count ;
  uint16_t             gns_reset_count ;
  uint16_t             attach_reset_count ;
  uint16_t             activate_pdn_reset_count ;
  uint16_t             sim_reset_count ;
  uint16_t             cellular_data_retry_count ;
  uint16_t             cmd_reset_count ;
  uint16_t             reset_count ;
  uint16_t             global_retry_count ;
} CST_context_t;

/* NFMC context */
typedef struct
{
  uint32_t  active;
  uint32_t  tempo[CST_NFMC_TEMPO_NB];
} CST_nfmc_context_t;

/* Private variables ---------------------------------------------------------*/
static osMessageQId      cst_queue_id;
static osTimerId         CST_pdn_activate_retry_timer_handle;
static osTimerId         CST_network_status_timer_handle;
static osTimerId         CST_register_retry_timer_handle;
static osTimerId         CST_fota_timer_handle;
static dc_cellular_info_t      cst_cellular_info;
static dc_sim_info_t           cst_sim_info;
static dc_cellular_data_info_t cst_cellular_data_info;
static dc_cellular_params_t    cst_cellular_params;
static uint8_t cst_sim_slot_index;
static dc_apn_config_t cst_apn_config;
static CST_nfmc_context_t CST_nfmc_context;
static CST_context_t cst_context =
{
  CST_MODEM_INIT_STATE, CST_NO_FAIL, CS_PDN_EVENT_NW_DETACH,    /* Automaton State, FAIL Cause,  */
  { 0, 0},                               /* signal quality */
  CS_NRS_NOT_REGISTERED_NOT_SEARCHING, CS_NRS_NOT_REGISTERED_NOT_SEARCHING, CS_NRS_NOT_REGISTERED_NOT_SEARCHING,
  0,                                     /* activate_pdn_nfmc_tempo_count */
  0,                                     /* register_retry_tempo_count */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     /* Reset counters */
};

static CS_OperatorSelector_t    ctxt_operator =
{
  .mode = CS_NRM_AUTO,
  .format = CS_ONF_NOT_PRESENT,
  .operator_name = "00101",
};

static uint8_t CST_polling_timer_flag = 0U;
static uint8_t CST_csq_count_fail      = 0U;

/* Global variables ----------------------------------------------------------*/
CST_state_t CST_current_state;  /* Current automaton state */
uint8_t CST_polling_active;     /* modem polling acivation flag */


/* Private function prototypes -----------------------------------------------*/
static void CST_reset_fail_count(void);
static void CST_pdn_event_callback(CS_PDN_conf_id_t cid, CS_PDN_event_t pdn_event);
static void CST_network_reg_callback(void);
static void CST_modem_event_callback(CS_ModemEvent_t event);
static void CST_location_info_callback(void);
static void CST_data_cache_cellular_info_set(dc_service_rt_state_t dc_service_state);
static void CST_location_info_callback(void);
static void CST_config_fail_mngt(const uint8_t *msg_fail, CST_fail_cause_t fail_cause, uint16_t *fail_count,
                                 uint16_t fail_max);
static void CST_modem_init(void);
static CS_Status_t CST_set_signal_quality(void);
static void CST_get_device_all_infos(dc_cs_target_state_t  target_state);
static void CST_subscribe_all_net_events(void);
static void CST_subscribe_modem_events(void);
static void CST_notif_cb(dc_com_event_id_t dc_event_id, const void *private_data);
static CST_autom_event_t CST_get_autom_event(osEvent event);
static void CST_power_on_only_modem_mngt(void);
static void CST_init_state_mngt(void);
static CS_Status_t CST_reset_modem(void);
static void CST_reset_modem_mngt(void);
static void CST_modem_powered_on_mngt(void);
static void CST_net_register_mngt(void);
static void CST_signal_quality_test_mngt(void);
static void CST_network_status_test_mngt(void);
static void CST_network_event_mngt(void);
static void CST_network_event_mngt2(void);
static void CST_attach_modem_mngt(void);
static void CST_modem_define_pdn(void);
static CS_Status_t CST_modem_activate_pdn(void);
static void CST_cellular_data_fail_mngt(void);
static void CST_pdn_event_mngt(void);
static void CST_init_state(CST_autom_event_t autom_event);
static void CST_reset_state(CST_autom_event_t autom_event);
static void CST_modem_on_state(CST_autom_event_t autom_event);
static void CST_modem_powered_on_state(CST_autom_event_t autom_event);
static void CST_waiting_for_signal_quality_ok_state(CST_autom_event_t autom_event);
static void CST_waiting_for_network_status_state(CST_autom_event_t autom_event);
static void CST_network_status_ok_state(CST_autom_event_t autom_event);
static void CST_modem_registered_state(CST_autom_event_t autom_event);
static void CST_modem_pdn_activate_state(CST_autom_event_t autom_event);
static void CST_data_ready_state(CST_autom_event_t autom_event);
static void CST_fail_state(CST_autom_event_t autom_event);
static void CST_timer_handler(void);
static void CST_cellular_service_task(void const *argument);
static void CST_polling_timer_callback(void const *argument);
static void CST_pdn_activate_retry_timer_callback(void const *argument);
static void CST_network_status_timer_callback(void const *argument);
static void CST_register_retry_timer_callback(void const *argument);
static void CST_fota_timer_callback(void const *argument);
static uint8_t CST_util_convertStringToInt64(const uint8_t *p_string, uint16_t size, uint32_t *high_part_value,
                                             uint32_t *low_part_value);
static uint32_t CST_calculate_nfmc_tempo_value(uint32_t value, uint32_t imsi_high, uint32_t imsi_low);
static void CST_fill_nfmc_tempo(uint32_t imsi_high, uint32_t imsi_low);
static void CST_modem_start(void);
static CS_SimSlot_t cst_convert_sim_socket_type(dc_cs_sim_slot_type_t sim_slot_value);
static void CST_close_network_interface(void);
#if (USE_LOW_POWER == 1)
static void CST_power_idle_state(CST_autom_event_t autom_event);
#endif /* (USE_LOW_POWER == 1) */

/* Private function Definition -----------------------------------------------*/

/**
  * @brief  convert sim slot DC enum value to CS enum value
  * @param  sim_slot_value    - sim slot DC enum value
  * @retval CS_SimSlot_t  - sim slot CS enum value
  */

static CS_SimSlot_t  cst_convert_sim_socket_type(dc_cs_sim_slot_type_t sim_slot_value)
{
  CS_SimSlot_t enum_value;
  switch (sim_slot_value)
  {
    case DC_SIM_SLOT_MODEM_SOCKET:
    {
      enum_value = CS_MODEM_SIM_SOCKET_0;
      break;
    }
    case DC_SIM_SLOT_MODEM_EMBEDDED_SIM:
    {
      enum_value = CS_MODEM_SIM_ESIM_1;
      break;
    }
    case DC_SIM_SLOT_STM32_EMBEDDED_SIM:
    {
      enum_value = CS_STM32_SIM_2;
      break;
    }
    default:
    {
      enum_value = CS_MODEM_SIM_SOCKET_0;
      break;
    }
  }
  return enum_value;
}

/**
  * @brief  reset failure couters
  * @param  none
  * @retval none
  */
static void CST_reset_fail_count(void)
{
  cst_context.power_on_reset_count       = 0U;
  cst_context.reset_reset_count          = 0U;
  cst_context.init_modem_reset_count     = 0U;
  cst_context.csq_reset_count            = 0U;
  cst_context.gns_reset_count            = 0U;
  cst_context.attach_reset_count         = 0U;
  cst_context.activate_pdn_reset_count   = 0U;
  cst_context.cellular_data_retry_count  = 0U;
  cst_context.global_retry_count         = 0U;
}

/**
  * @brief  PDN event callback
  * @param  cid    - current CID
  * @retval pdn_event  - PDN event
  */
static void CST_pdn_event_callback(CS_PDN_conf_id_t cid, CS_PDN_event_t pdn_event)
{
  UNUSED(cid);
  PRINT_CELLULAR_SERVICE("====================================CST_pdn_event_callback (cid=%d / event=%d)\n\r",
                         cid, pdn_event)
  cst_context.pdn_status = pdn_event;
  (void)CST_send_message(CST_MESSAGE_CS_EVENT, CST_PDN_STATUS_EVENT);

}

/**
  * @brief  URC callback
  * @param  none
  * @retval none
  */
static void CST_network_reg_callback(void)
{
  PRINT_CELLULAR_SERVICE("==================================CST_network_reg_callback\n\r")
  CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
}

/**
  * @brief  location info callback callback
  * @param  none
  * @retval none
  */
static void CST_location_info_callback(void)
{
  PRINT_CELLULAR_SERVICE("CST_location_info_callback\n\r")
}

/**
  * @brief  modem event calback
  * @param  event - modem event
  * @retval none
  */
static void CST_modem_event_callback(CS_ModemEvent_t event)
{
  /* event is a bitmask, we can have more than one evt reported at the same time */
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_BOOT) != 0U)
  {
    /* Modem boot event */
    if (CST_current_state != CST_MODEM_ON_STATE)
    {
      /* If MODEM_ON State don't take account of BOOT event because is the normal case */
      PRINT_CELLULAR_SERVICE("Modem event received: CS_MDMEVENT_BOOT\n\r")
      CST_current_state = CST_MODEM_INIT_STATE;
      CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_INIT_EVENT);
    }
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_POWER_DOWN) != 0U)
  {
    /* Modem power down event*/
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_POWER_DOWN\n\r")
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_FOTA_START) != 0U)
  {
    /* FOTA event: FOTA start */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_FOTA_START\n\r")
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                      sizeof(cst_cellular_data_info));
    cst_cellular_data_info.rt_state = DC_SERVICE_SHUTTING_DOWN;
    CST_current_state = CST_MODEM_REPROG_STATE;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                       sizeof(cst_cellular_data_info));
    (void)osTimerStart(CST_fota_timer_handle, CST_FOTA_TIMEOUT);
  }
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_FOTA_END) != 0U)
  {
    /* FOTA event: FOTA end */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_FOTA_END\n\r")

    /* TRIGGER PLATFORM RESET after a delay  */
    PRINT_CELLULAR_SERVICE("TRIGGER PLATFORM REBOOT AFTER FOTA UPDATE ...\n\r")
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 4, ERROR_FATAL);
  }

#if (USE_LOW_POWER == 1)
  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_LP_ENTER) != 0U)
  {
    /* Enter Low power  */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_LP_ENTER\n\r")
    CST_send_message(CST_MESSAGE_CMD, CST_POWER_SLEEP_COMPLETE_EVENT);
  }

  if (((uint16_t)event & (uint16_t)CS_MDMEVENT_LP_LEAVE) != 0U)
  {
    /* Modem Leave Low Power state  */
    PRINT_CELLULAR_SERVICE("Modem event received:  CS_MDMEVENT_LP_LEAVE\n\r")
  }
#endif /* (USE_LOW_POWER == 1) */
}

/**
  * @brief  update Cellular Info entry of Data Cache
  * @param  dc_service_state - new entry state to set
  * @retval none
  */
static void  CST_data_cache_cellular_info_set(dc_service_rt_state_t dc_service_state)
{
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                    sizeof(cst_cellular_data_info));
  if (cst_cellular_data_info.rt_state != dc_service_state)
  {
    cst_cellular_data_info.rt_state = dc_service_state;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                       sizeof(cst_cellular_data_info));
  }
}

/**
  * @brief   configuration failure management
  * @param  msg_fail   - failure message (only for trace)
  * @param  fail_cause - failure cause
  * @param  fail_count - count of failures
  * @param  fail_max   - max of allowed failures
  * @retval none
  */
static void CST_config_fail_mngt(const uint8_t *msg_fail, CST_fail_cause_t fail_cause, uint16_t *fail_count,
                                 uint16_t fail_max)
{

#if (SW_DEBUG_VERSION == 0)
  UNUSED(msg_fail);
#endif  /* (SW_DEBUG_VERSION == 0) */

  PRINT_CELLULAR_SERVICE("=== %s Fail !!! === \r\n", msg_fail)
  ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 7, ERROR_WARNING);

  *fail_count = *fail_count + 1U;
  cst_context.global_retry_count++;
  cst_context.reset_count++;

  CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
  if ((*fail_count <= fail_max) && (cst_context.global_retry_count <= CST_GLOBAL_RETRY_MAX))
  {
    CST_current_state = CST_MODEM_RESET_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_INIT_EVENT);
    cst_context.fail_cause    = fail_cause;
  }
  else
  {
    CST_current_state = CST_MODEM_FAIL_STATE;
    cst_context.fail_cause    = CST_MODEM_POWER_ON_FAIL;

    PRINT_CELLULAR_SERVICE_ERR("=== CST_set_fail_state %d - count %d/%d FATAL !!! ===\n\r",
                               fail_cause,
                               cst_context.global_retry_count,
                               *fail_count)
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 8, ERROR_FATAL);
  }
}

/**
  * @brief  init modem processing
  * @param  none
  * @retval none
  */
static void CST_modem_init(void)
{
  (void)CS_init();
}

/**
  * @brief  start modem processing
  * @param  none
  * @retval none
  */
static void CST_modem_start(void)
{
  /* request AT core to start */
  if (atcore_task_start(ATCORE_THREAD_STACK_PRIO, ATCORE_THREAD_STACK_SIZE) != ATSTATUS_OK)
  {
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 9, ERROR_WARNING);
  }
}

/**
  * @brief  init modem processing
  * @param  none
  * @retval error code
  */
static CS_Status_t CST_set_signal_quality(void)
{
  CS_Status_t cs_status = CELLULAR_ERROR;
  CS_SignalQuality_t sig_quality;
  if (osCS_get_signal_quality(&sig_quality) == CELLULAR_OK)
  {
    CST_csq_count_fail = 0U;
    if ((sig_quality.rssi != cst_context.signal_quality.rssi) || (sig_quality.ber != cst_context.signal_quality.ber))
    {
      cst_context.signal_quality.rssi = sig_quality.rssi;
      cst_context.signal_quality.ber  = sig_quality.ber;

      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));


      /* if((sig_quality.rssi == 0) || (sig_quality.rssi == CST_BAD_SIG_RSSI)) */
      if (sig_quality.rssi == CST_BAD_SIG_RSSI)
      {
        cst_cellular_info.cs_signal_level    = DC_NO_ATTACHED;
        cst_cellular_info.cs_signal_level_db = (int32_t)DC_NO_ATTACHED;
      }
      else
      {
        cs_status = CELLULAR_OK;
        cst_cellular_info.cs_signal_level     = sig_quality.rssi;             /*  range 0..99 */
        cst_cellular_info.cs_signal_level_db  = (-113 + (2 * (int32_t)sig_quality.rssi)); /* dBm */
      }
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
    }

    PRINT_CELLULAR_SERVICE(" -Sig quality rssi : %d\n\r", sig_quality.rssi)
    PRINT_CELLULAR_SERVICE(" -Sig quality ber  : %d\n\r", sig_quality.ber)
  }
  else
  {
    CST_csq_count_fail++;
    PRINT_CELLULAR_SERVICE("Modem signal quality error\n\r")
    if (CST_csq_count_fail >= CST_COUNT_FAIL_MAX)
    {
      PRINT_CELLULAR_SERVICE("Modem signal quality error max\n\r")
      CST_csq_count_fail = 0U;
      CST_config_fail_mngt(((uint8_t *)"CS_get_signal_quality"),
                           CST_MODEM_CSQ_FAIL,
                           &cst_context.csq_reset_count,
                           CST_CSQ_MODEM_RESET_MAX);
    }
  }
  return cs_status;
}

/**
  * @brief  send message to cellular service task
  * @param  type   - message type
  * @param  event  - event
  * @retval none
  */
void  CST_send_message(CST_message_type_t  type, CST_autom_event_t event)
{
  CST_message_t cmd_message;
  cmd_message.type = (uint16_t)type;
  cmd_message.id   = event;

  uint32_t *cmd_message_p = (uint32_t *)(&cmd_message);
  (void)osMessagePut((osMessageQId)cst_queue_id, *cmd_message_p, 0U);
}


/**
  * @brief  64bits modulo calculation
  * @param  div   - divisor
  * @param  val_m  - high 32bits value
  * @param  val_l  - low  32bits value
  * @retval result of modulo calculation
  */
static uint32_t cst_modulo64(uint32_t div, uint32_t val_m, uint32_t val_l)
{
  uint32_t div_m;
  uint32_t div_l;
  uint32_t tmp_m;
  uint32_t tmp_l;

  div_m = div;
  div_l = 0;
  tmp_m = val_m % div_m;

  tmp_l = val_l;

  while (tmp_m > 0U)
  {
    if (
      (div_m > tmp_m)
      ||
      ((div_m == tmp_m) && (div_l > tmp_l))
    )
    {
      /* Nothing to do */
    }
    else if (div_l > tmp_l)
    {
      tmp_l = tmp_l - div_l;
      tmp_m--;
      tmp_m = tmp_m - div_m;
    }
    else
    {
      tmp_m = tmp_m - div_m;
      tmp_l = tmp_l - div_l;
    }

    div_l = div_l >> 1;
    if ((div_m & 1U) == 1U)
    {
      div_l = div_l | 0x80000000U;
    }
    div_m = div_m >> 1;
  }
  tmp_l = tmp_l % div;
  return tmp_l;
}

/**
  * @brief  convert string to int64
  * @param  p_string   - string
  * @param  size  - size of string
  * @param  high_part_value  - high  32bits value calulated
  * @param  low_part_value   - low   32bits value calulated
  * @retval error code
  */
static uint8_t CST_util_convertStringToInt64(const uint8_t *p_string, uint16_t size, uint32_t *high_part_value,
                                             uint32_t *low_part_value)
{
  return ATutil_convertHexaStringToInt64(p_string, size, high_part_value, low_part_value);
}

/**
  * @brief  calculation of NFMC tempo
  * @param  value   - input value
  * @param  imsi_high  - high IMSI 32bits value
  * @param  imsi_low   - low IMSI  32bits value
  * @retval NFMC tempo calculated
  */
static uint32_t CST_calculate_nfmc_tempo_value(uint32_t value, uint32_t imsi_high, uint32_t imsi_low)
{
  uint32_t temp_value32;
  if (value != 0U)
  {
    temp_value32 = cst_modulo64(value, imsi_high, imsi_low);
  }
  else
  {
    temp_value32 = imsi_low;
  }
  temp_value32 = temp_value32 + value;
  return (0xffffffffU & temp_value32);
}

/**
  * @brief  calculate NFMC tempos and set them in DataCache
  * @param  imsi_high  - high IMSI 32bits value
  * @param  imsi_low   - low IMSI  32bits value
  * @retval none
  */
static void CST_fill_nfmc_tempo(uint32_t imsi_high, uint32_t imsi_low)
{
  uint32_t i;
  dc_nfmc_info_t nfmc_info;

  if (cst_cellular_params.nfmc_active != 0U)
  {
    /* NFMC active */
    nfmc_info.activate = 1U;
    for (i = 0U ; i < CST_NFMC_TEMPO_NB; i++)
    {
      CST_nfmc_context.tempo[i] = CST_calculate_nfmc_tempo_value(cst_cellular_params.nfmc_value[i],
                                                                 imsi_high,
                                                                 imsi_low);
      nfmc_info.tempo[i] = CST_nfmc_context.tempo[i];
      PRINT_CELLULAR_SERVICE("VALUE/TEMPO %ld/%ld\n\r",  cst_cellular_params.nfmc_value[i], CST_nfmc_context.tempo[i])
    }
    nfmc_info.rt_state = DC_SERVICE_ON;
  }
  else
  {
    /* NFMC not active */
    nfmc_info.activate = 0U;
    nfmc_info.rt_state = DC_SERVICE_OFF;
  }
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_NFMC_INFO, (void *)&nfmc_info, sizeof(nfmc_info));
}

/**
  * @brief  update device info in data cache
  * @param  target_state  - modem target state
  * @retval none
  */
static void CST_get_device_all_infos(dc_cs_target_state_t  target_state)
{
  static CS_DeviceInfo_t cst_device_info;
  CS_Status_t            cs_status;
  uint16_t               sim_poll_count;
  uint16_t               end_of_loop;
  uint32_t               CST_IMSI_high;
  uint32_t               CST_IMSI_low;

  sim_poll_count = 0U;

  (void)memset((void *)&cst_device_info, 0, sizeof(CS_DeviceInfo_t));

  /* read current device info in Data Cache */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));


  /* get IMEI */
  cst_device_info.field_requested = CS_DIF_IMEI_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy(cst_cellular_info.imei, cst_device_info.u.imei, DC_MAX_SIZE_IMEI);
    PRINT_CELLULAR_SERVICE(" -IMEI: %s\n\r", cst_device_info.u.imei)
  }
  else
  {
    cst_cellular_info.imei[0] = 0U;
    PRINT_CELLULAR_SERVICE("IMEI error\n\r")
  }


  /* get Manufacturer Name  of modem*/
  cst_device_info.field_requested = CS_DIF_MANUF_NAME_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.manufacturer_name,
                 (CRC_CHAR_t *)cst_device_info.u.manufacturer_name,
                 DC_MAX_SIZE_MANUFACT_NAME);
    PRINT_CELLULAR_SERVICE(" -MANUFACTURER: %s\n\r", cst_device_info.u.manufacturer_name)
  }
  else
  {
    cst_cellular_info.manufacturer_name[0] = 0U;
    PRINT_CELLULAR_SERVICE("Manufacturer Name error\n\r")
  }

  /* get Model modem  */
  cst_device_info.field_requested = CS_DIF_MODEL_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.model,
                 (CRC_CHAR_t *)cst_device_info.u.model,
                 DC_MAX_SIZE_MODEL);
    PRINT_CELLULAR_SERVICE(" -MODEL: %s\n\r", cst_device_info.u.model)
  }
  else
  {
    cst_cellular_info.model[0] = 0U;
    PRINT_CELLULAR_SERVICE("Model error\n\r")
  }

  /* get revision of modem  */
  cst_device_info.field_requested = CS_DIF_REV_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.revision,
                 (CRC_CHAR_t *)cst_device_info.u.revision,
                 DC_MAX_SIZE_REV);
    PRINT_CELLULAR_SERVICE(" -REVISION: %s\n\r", cst_device_info.u.revision)
  }
  else
  {
    cst_cellular_info.revision[0] = 0U;
    PRINT_CELLULAR_SERVICE("Revision error\n\r")
  }

  /* get serial number of modem  */
  cst_device_info.field_requested = CS_DIF_SN_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.serial_number,
                 (CRC_CHAR_t *)cst_device_info.u.serial_number,
                 DC_MAX_SIZE_SN);
    PRINT_CELLULAR_SERVICE(" -SERIAL NBR: %s\n\r", cst_device_info.u.serial_number)
  }
  else
  {
    cst_cellular_info.serial_number[0] = 0U;
    PRINT_CELLULAR_SERVICE("Serial Number error\n\r")
  }

  /* get CCCID  */
  cst_device_info.field_requested = CS_DIF_ICCID_PRESENT;
  if (osCDS_get_device_info(&cst_device_info) == CELLULAR_OK)
  {
    (void)memcpy((CRC_CHAR_t *)cst_cellular_info.iccid,
                 (CRC_CHAR_t *)cst_device_info.u.iccid,
                 DC_MAX_SIZE_ICCID);
    PRINT_CELLULAR_SERVICE(" -ICCID: %s\n\r", cst_device_info.u.iccid)
  }
  else
  {
    cst_cellular_info.serial_number[0] = 0U;
    PRINT_CELLULAR_SERVICE("Serial Number error\n\r")
  }

  /* write updated cellular info in Data Cache */
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

  end_of_loop = 1U;
  if (target_state == DC_TARGET_STATE_FULL)
  {
    /* modem target state: FULL */

    /* SIM info set to 'on going' in Data Cache during SIM connection */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
    cst_sim_info.rt_state   = DC_SERVICE_ON;
    cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_CONNECTION_ON_GOING;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

    /* loop: waiting for SIM status */
    while (end_of_loop != 0U)
    {
      /* try to get IMSI to determine if SIM is present on this slot */
      cst_device_info.field_requested = CS_DIF_IMSI_PRESENT;
      cs_status = osCDS_get_device_info(&cst_device_info);
      if (cs_status == CELLULAR_OK)
      {
        /* IMSI available => SIM is present on this slot */

        /* NFMC tempo calculation */
        (void)CST_util_convertStringToInt64(cst_device_info.u.imsi, 15U, &CST_IMSI_high, &CST_IMSI_low);
        PRINT_CELLULAR_SERVICE(" -IMSI: %lx%lx\n\r", CST_IMSI_high, CST_IMSI_low)
        CST_fill_nfmc_tempo(CST_IMSI_high, CST_IMSI_low);

        (void)memcpy((CRC_CHAR_t *)cst_sim_info.imsi,
                     (CRC_CHAR_t *)cst_device_info.u.imsi,
                     DC_MAX_SIZE_IMSI);
        cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_OK;
        end_of_loop = 0U;
      }
      else if ((cs_status == CELLULAR_SIM_BUSY)
               || (cs_status == CELLULAR_SIM_ERROR))
      {
        /* SIM presently not available: poll it untill available or polling time exceed */
        (void)osDelay(100U);
        sim_poll_count++;
        if (sim_poll_count > CST_SIM_POLL_COUNT)
        {
          /* polling time exceed: SIM not available on this slot */
          sim_poll_count = 0;
          dc_cs_sim_status_t sim_error;
          if (cs_status == CELLULAR_SIM_BUSY)
          {
            sim_error = DC_SIM_BUSY;
          }
          else
          {
            sim_error = DC_SIM_ERROR;
          }

          cst_sim_info.sim_status[cst_sim_slot_index] = sim_error;

          end_of_loop = 0U;
        }
      }
      else
      {
        /* error returned => SIM not available. Getting SIM error cause */
        if (cs_status == CELLULAR_SIM_NOT_INSERTED)
        {
          cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_NOT_INSERTED;
        }
        else if (cs_status == CELLULAR_SIM_PIN_OR_PUK_LOCKED)
        {
          cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_PIN_OR_PUK_LOCKED;
        }
        else if (cs_status == CELLULAR_SIM_INCORRECT_PASSWORD)
        {
          cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_INCORRECT_PASSWORD;
        }
        else
        {
          cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_ERROR;
        }
        end_of_loop = 0U;
      }
    }
    /* Set SIM state in Data Cache */
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
  }
}

/**
  * @brief  subscribe to network event
  * @param  none
  * @retval none
  */
static void CST_subscribe_all_net_events(void)
{
  PRINT_CELLULAR_SERVICE("Subscribe URC events: Network registration\n\r")
  (void)osCDS_subscribe_net_event(CS_URCEVENT_CS_NETWORK_REG_STAT, CST_network_reg_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_GPRS_NETWORK_REG_STAT, CST_network_reg_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_EPS_NETWORK_REG_STAT, CST_network_reg_callback);
  PRINT_CELLULAR_SERVICE("Subscribe URC events: Location info\n\r")
  (void)osCDS_subscribe_net_event(CS_URCEVENT_EPS_LOCATION_INFO, CST_location_info_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_GPRS_LOCATION_INFO, CST_location_info_callback);
  (void)osCDS_subscribe_net_event(CS_URCEVENT_CS_LOCATION_INFO, CST_location_info_callback);
}

/**
  * @brief  subscribe to modem event
  * @param  none
  * @retval none
  */
static void CST_subscribe_modem_events(void)
{
  PRINT_CELLULAR_SERVICE("Subscribe modems events\n\r")
  CS_ModemEvent_t events_mask = (CS_ModemEvent_t)((uint16_t)CS_MDMEVENT_BOOT       |
                                                  (uint16_t)CS_MDMEVENT_POWER_DOWN |
                                                  (uint16_t)CS_MDMEVENT_FOTA_START |
                                                  (uint16_t)CS_MDMEVENT_LP_ENTER   |
                                                  (uint16_t)CS_MDMEVENT_FOTA_END);
  (void)osCDS_subscribe_modem_event(events_mask, CST_modem_event_callback);
}


/**
  * @brief  new apn config requested: set it in Data Cache
  * @param  none
  * @retval none
  */
static void CST_apn_set_new_config(void)
{
  /* read new requested apn config in Data Cache */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_APN_CONFIG, (void *)&cst_apn_config, sizeof(dc_apn_config_t));

  /* read current cellular config in Data Cache */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_CONFIG, (void *)&cst_cellular_params, sizeof(cst_cellular_params));

  /* update new cellular config in Data Cache*/
  cst_cellular_params.sim_slot[cst_sim_info.index_slot].cid  = cst_apn_config.cid;
  (void)memcpy(cst_cellular_params.sim_slot[cst_sim_info.index_slot].apn,
               cst_apn_config.apn, crs_strlen(cst_apn_config.apn) + 1U);
  (void)memcpy(cst_cellular_params.sim_slot[cst_sim_info.index_slot].username,
               cst_apn_config.username, crs_strlen(cst_apn_config.username) + 1U);
  (void)memcpy(cst_cellular_params.sim_slot[cst_sim_info.index_slot].password,
               cst_apn_config.password, crs_strlen(cst_apn_config.password) + 1U);

  cst_cellular_params.rt_state = DC_SERVICE_ON;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_CONFIG, (void *)&cst_cellular_params, sizeof(cst_cellular_params));
}

/**
  * @brief  Data cache callback: send a message to cellular service automaton task
  * @param  dc_event_id  - data cache event
  * @param  private_gui_data  - private Data Cache context (not used)
  * @retval none
  */
static void CST_notif_cb(dc_com_event_id_t dc_event_id, const void *private_data)
{
  UNUSED(private_data);
  uint32_t old_apn_len;
  uint32_t new_apn_len;


  if ((dc_event_id == DC_CELLULAR_DATA_INFO) || (dc_event_id == DC_CELLULAR_NIFMAN_INFO))
  {
    /* Cellular Data Info entry updated */
    CST_message_t cmd_message;
    uint32_t *cmd_message_p = (uint32_t *)(&cmd_message);
    cmd_message.type = (uint16_t)CST_MESSAGE_DC_EVENT;
    if (dc_event_id != DC_COM_INVALID_ENTRY)
    {
      cmd_message.id   = (uint16_t)dc_event_id;
      (void)osMessagePut(cst_queue_id, *cmd_message_p, 0U);
    }
  }
  else if (dc_event_id == DC_CELLULAR_TARGET_STATE_CMD)
  {
    /* new modem target state required  */
    CST_message_t cmd_message;
    uint32_t *cmd_message_p = (uint32_t *)(&cmd_message);
    cmd_message.type = (uint16_t)CST_MESSAGE_DC_EVENT;
    cmd_message.id   = (uint16_t)dc_event_id;
    (void)osMessagePut(cst_queue_id, *cmd_message_p, 0U);
  }
  else if (dc_event_id == DC_CELLULAR_APN_CONFIG)
  {
    /* new apn config required  */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_APN_CONFIG, (void *)&cst_apn_config,
                      sizeof(dc_apn_config_t));

    if (cst_apn_config.rt_state == DC_SERVICE_ON)
    {
      old_apn_len = crs_strlen(cst_cellular_params.sim_slot[cst_sim_slot_index].apn);
      new_apn_len = crs_strlen(cst_apn_config.apn);
      if ((old_apn_len != new_apn_len)
          ||
          (memcmp(cst_apn_config.apn, cst_cellular_params.sim_slot[cst_sim_slot_index].apn, new_apn_len) != 0))
      {
#if (!USE_DEFAULT_SETUP == 1)
        uint32_t ret;
        ret = CST_update_config_setup_handler(&cst_apn_config, cst_sim_info.active_slot);
        if (ret != 0U)
#endif /* (!USE_DEFAULT_SETUP == 1) */
        {
          PRINT_CELLULAR_SERVICE("*********** CST New APN config %s. Reset modem ********\n\r", cst_apn_config.apn)
          CST_current_state = CST_MODEM_RESET_STATE;
          CST_send_message(CST_MESSAGE_CS_EVENT, CST_APN_CONFIG_EVENT);
        }
      }
    }
  }
  else if (dc_event_id == DC_CELLULAR_POWER_CONFIG)
  {
    /* set power config  */
#if (USE_LOW_POWER == 1)
    CSP_SetPowerConfig();
#endif  /* (USE_LOW_POWER == 1) */
  }
  else
  {
    /* Nothing to do */
  }

}

/**
  * @brief  get automaton event from message event
  * @param  event  - message event
  * @retval automaton event
  */
static CST_autom_event_t CST_get_autom_event(osEvent event)
{
  static dc_cellular_target_state_t cst_target_state;
  CST_autom_event_t autom_event;
  CST_message_t  message;
  CST_message_t *message_p;
  autom_event = CST_NO_EVENT;
  message_p = (CST_message_t *) & (event.value.v);
  message   = *message_p;

  /* 4 types of messages: */
  /*
       -> CS automaton event
       -> CS CMD    (ON/OFF)
       -> DC EVENT  (DC_CELLULAR_DATA_INFO: / FAIL)
       -> DC TIMER  (Polling handle)
  */
  if (message.type == (uint16_t)CST_MESSAGE_TIMER_EVENT)
  {
    autom_event = CST_POLLING_TIMER;
  }
  else if (message.type == (uint16_t)CST_MESSAGE_CS_EVENT)
  {
    autom_event = (CST_autom_event_t)message.id;
  }
  else if (message.type == (uint16_t)CST_MESSAGE_URC_EVENT)
  {
    autom_event = CST_MODEM_URC;
  }
  else if (message.type == (uint16_t)CST_MESSAGE_CMD)
  {
    switch (message.id)
    {
      case CST_INIT_EVENT:
      {
        autom_event = CST_INIT_EVENT;
        break;
      }
      case CST_MODEM_POWER_ON_EVENT:
      {
        autom_event = CST_MODEM_POWER_ON_EVENT;
        break;
      }
#if (USE_LOW_POWER == 1)
      case CST_POWER_SLEEP_REQUEST_EVENT:
      {
        autom_event = CST_POWER_SLEEP_REQUEST_EVENT;
        break;
      }
      case CST_POWER_SLEEP_COMPLETE_EVENT:
      {
        autom_event = CST_POWER_SLEEP_COMPLETE_EVENT;
        break;
      }
      case CST_POWER_WAKEUP_EVENT:
      {
        autom_event = CST_POWER_WAKEUP_EVENT;
        break;
      }
      case CST_POWER_SLEEP_TIMEOUT_EVENT:
      {
        autom_event = CST_POWER_SLEEP_TIMEOUT_EVENT;
        break;
      }
#endif /* (USE_LOW_POWER == 1) */

      default:
      {
        autom_event = CST_CMD_UNKWONW_EVENT;
        break;
      }
    }
  }
  else if (message.type == (uint16_t)CST_MESSAGE_DC_EVENT)
  {
    if (message.id == (uint16_t)DC_CELLULAR_DATA_INFO)
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                        sizeof(cst_cellular_data_info));
      if (DC_SERVICE_FAIL == cst_cellular_data_info.rt_state)
      {
        autom_event = CST_CELLULAR_DATA_FAIL_EVENT;
      }
    }
    if (message.id == (uint16_t)DC_CELLULAR_TARGET_STATE_CMD)
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_TARGET_STATE_CMD, (void *)&cst_target_state, sizeof(cst_target_state));
      if (DC_SERVICE_ON == cst_target_state.rt_state)
      {
        cst_cellular_params.target_state = cst_target_state.target_state;
        autom_event = CST_CELLULAR_STATE_EVENT;
      }
    }
    if (message.id == (uint16_t) DC_CELLULAR_NIFMAN_INFO)
    {
      autom_event = CST_NIFMAN_EVENT;
    }
  }
  else
  {
    PRINT_CELLULAR_SERVICE("CST_get_autom_event : No type matching\n\r")
  }

  return autom_event;
}

/* ===================================================================
   Automaton functions  BEGIN
   =================================================================== */

/**
  * @brief  The modem is powered on alone without network attachement
  * @param  none
  * @retval none
  */
static void CST_power_on_only_modem_mngt(void)
{
  PRINT_CELLULAR_SERVICE("*********** CST_power_on_only_modem_mngt ********\n\r")
  (void)osCDS_power_on();
  PRINT_CELLULAR_SERVICE("*********** MODEM ON ********\n\r")
}

/**
  * @brief  power on modem processing
  * @param  none
  * @retval none
  */
static void CST_init_state_mngt(void)
{
  CS_Status_t cs_status;
  dc_cellular_info_t dc_cellular_info;

  PRINT_CELLULAR_SERVICE("*********** CST_init_state_mngt ********\n\r")

  if (cst_cellular_params.target_state == DC_TARGET_STATE_OFF)
  {
    CST_current_state = CST_MODEM_OFF_STATE;
    /* Data Cache -> Radio ON */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
    dc_cellular_info.modem_state = DC_MODEM_STATE_OFF;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
  }
  else
  {
    CST_current_state = CST_MODEM_ON_STATE;
    cs_status = osCDS_power_on();

#if (CST_RESET_DEBUG == 1)
    if (cst_context.power_on_reset_count == 0)
    {
      cs_status = CELLULAR_ERROR;
    }
#endif /* (CST_RESET_DEBUG == 1) */

    if (cs_status != CELLULAR_OK)
    {
      CST_config_fail_mngt(((uint8_t *)"CST_cmd"),
                           CST_MODEM_POWER_ON_FAIL,
                           &cst_context.power_on_reset_count,
                           CST_POWER_ON_RESET_MAX);

    }
    else
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
      dc_cellular_info.rt_state = DC_SERVICE_RUN;
      dc_cellular_info.modem_state = DC_MODEM_STATE_POWERED_ON;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_POWERED_ON_EVENT);
    }
  }
}

/**
  * @brief  modem reset processing
  * @param  none
  * @retval none
  */
static CS_Status_t CST_reset_modem(void)
{
  return osCDS_reset(CS_RESET_AUTO);
}

/**
  * @brief  power on modem processing
  * @param  none
  * @retval none
  */
static void CST_reset_modem_mngt(void)
{
  CS_Status_t cs_status;
  dc_cellular_info_t dc_cellular_info;

  PRINT_CELLULAR_SERVICE("*********** CST_init_state_mngt ********\n\r")
  cs_status = CST_reset_modem();
#if (CST_RESET_DEBUG == 1)
  if (cst_context.reset_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif /* (CST_RESET_DEBUG == 1) */

  if (cs_status != CELLULAR_OK)
  {
    CST_config_fail_mngt(((uint8_t *)"CST_reset_modem_mngt"),
                         CST_MODEM_RESET_FAIL,
                         &cst_context.reset_reset_count,
                         CST_RESET_MAX);

  }
  else
  {
    /* Data Cache -> Data transfer off */
    CST_data_cache_cellular_info_set(DC_SERVICE_OFF);

    /* Data Cache -> Radio ON */
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info_t));
    dc_cellular_info.rt_state = DC_SERVICE_ON;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info_t));
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_POWERED_ON_EVENT);
  }
}

/**
  * @brief  init modem management
  * @param  none
  * @retval none
  */
static void  CST_modem_powered_on_mngt(void)
{
  CS_Status_t cs_status;
  dc_cellular_info_t dc_cellular_info;

  PRINT_CELLULAR_SERVICE("*********** CST_modem_powered_on_mngt ********\n\r")
  (void)osCS_sim_select(cst_convert_sim_socket_type(cst_sim_info.active_slot));

  (void)osDelay(10);
  if (cst_cellular_params.set_pdn_mode != 0U)
  {
    /* we must first define Cellular context before activating the RF because
     * modem will immediately attach to network once RF is enabled
     */
    PRINT_CELLULAR_SERVICE("CST_modem_powered_on_mngt : CST_modem_define_pdn\n\r")
    CST_modem_define_pdn();
  }

  if (cst_cellular_params.target_state == DC_TARGET_STATE_SIM_ONLY)
  {
    cs_status = osCDS_init_modem(CS_CMI_SIM_ONLY, CELLULAR_FALSE, CST_SIM_PINCODE);
  }
  else
  {
    cs_status = osCDS_init_modem(CS_CMI_FULL, CELLULAR_FALSE, CST_SIM_PINCODE);
  }

  if (cs_status == CELLULAR_SIM_INCORRECT_PASSWORD)
  {
    PRINT_FORCE("==================================\n\r")
    PRINT_FORCE(" WARNING: WRONG PIN CODE !!!\n\r")
    PRINT_FORCE(" DO NOT RESTART THE BOARD WITHOUT SETTING A CORRECT PIN CODE\n\r")
    PRINT_FORCE(" TO AVOID LOCKING THE SIM ! \n\r")
    PRINT_FORCE("==================================\n\r")
    for (;;)
    {
      /* Infinite loop to avoid to restart the board */
      __NOP(); /* Nothing to do */
    }
  }
  else if (cs_status == CELLULAR_SIM_PIN_OR_PUK_LOCKED)
  {
    PRINT_FORCE("==================================\n\r")
    PRINT_FORCE(" WARNING: PIN OK PUK LOCKED !!!  \n\r")
    PRINT_FORCE(" PROCESSING STOPPED\n\r")
    PRINT_FORCE("==================================\n\r")
    for (;;)
    {
      /* Infinite loop to avoid to restart the board */
      __NOP(); /* Nothing to do */
    }
  }
  else
  {
    /* Nothing to do */
  }

#if (CST_RESET_DEBUG == 1)
  if (cst_context.init_modem_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif /* (CST_RESET_DEBUG == 1) */
  if ((cs_status == CELLULAR_SIM_NOT_INSERTED) || (cs_status == CELLULAR_ERROR) || (cs_status == CELLULAR_SIM_ERROR))
  {
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
    cst_sim_info.sim_status[cst_sim_slot_index] = DC_SIM_NOT_INSERTED;
    cst_sim_info.rt_state   = DC_SERVICE_ON;
    cst_sim_slot_index++;

    if ((uint16_t)cst_sim_slot_index  >= (uint16_t)cst_cellular_params.sim_slot_nb)
    {
      cst_sim_slot_index = 0 ;
      PRINT_CELLULAR_SERVICE("CST_modem_on_state : No SIM found\n\r")
    }

    cst_sim_info.active_slot = cst_cellular_params.sim_slot[cst_sim_slot_index].sim_slot_type;
    cst_sim_info.index_slot  = cst_sim_slot_index;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

    CST_config_fail_mngt(((uint8_t *)"CST_modem_powered_on_mngt"),
                         CST_MODEM_SIM_FAIL,
                         &cst_context.sim_reset_count,
                         CST_SIM_RETRY_MAX);

  }
  else
  {


    /* Init Power config after Modem Power On and before subsribe modem evnets  */
#if (USE_LOW_POWER == 1)
    CSP_InitPowerConfig();
#endif  /* (USE_LOW_POWER == 1) */

    CST_subscribe_all_net_events();

    /* overwrite operator parameters */
    ctxt_operator.mode   = CS_NRM_AUTO;
    ctxt_operator.format = CS_ONF_NOT_PRESENT;
    CST_get_device_all_infos(cst_cellular_params.target_state);
    if (cst_cellular_params.target_state != DC_TARGET_STATE_SIM_ONLY)
    {
      CST_current_state = CST_MODEM_POWERED_ON_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_INITIALIZED_EVENT);
    }
    else
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info_t));
      dc_cellular_info.rt_state    = DC_SERVICE_ON;
      dc_cellular_info.modem_state = DC_MODEM_STATE_SIM_CONNECTED;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info_t));
      CST_current_state = CST_MODEM_SIM_ONLY_STATE;
    }
  }
}

/**
  * @brief  network registration management
  * @param  none
  * @retval none
  */
static void  CST_net_register_mngt(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t  cst_ctxt_reg_status;

  PRINT_CELLULAR_SERVICE("=== CST_net_register_mngt ===\n\r")
  cs_status = osCDS_register_net(&ctxt_operator, &cst_ctxt_reg_status);
  if (cs_status == CELLULAR_OK)
  {
    cst_context.current_EPS_NetworkRegState  = cst_ctxt_reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = cst_ctxt_reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = cst_ctxt_reg_status.CS_NetworkRegState;
    /*   to force to attach to PS domain by default (in case the Modem does not perform automatic PS attach.) */
    /*   need to check target state in future. */
    (void)osCDS_attach_PS_domain();

    CST_send_message(CST_MESSAGE_CS_EVENT, CST_SIGNAL_QUALITY_EVENT);
  }
  else
  {
    PRINT_CELLULAR_SERVICE("===CST_net_register_mngt - FAIL !!! ===\n\r")
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 18, ERROR_WARNING);
  }
}

/**
  * @brief  test if modem catch right signal
  * @param  none
  * @retval none
  */
static void  CST_signal_quality_test_mngt(void)
{
  PRINT_CELLULAR_SERVICE("*********** CST_signal_quality_test_mngt ********\n\r")

  if ((cst_context.signal_quality.rssi != 0U) && (cst_context.signal_quality.rssi != CST_BAD_SIG_RSSI))
  {
    (void)osTimerStart(CST_network_status_timer_handle, cst_cellular_params.attachment_timeout);
    PRINT_CELLULAR_SERVICE("-----> Start NW REG TIMEOUT TIMER   : %ld\n\r", cst_cellular_params.attachment_timeout)

    CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_STATUS_EVENT);
  }

}

/**
  * @brief  test if network status is OK
  * @param  none
  * @retval none
  */
static void  CST_network_status_test_mngt(void)
{
  PRINT_CELLULAR_SERVICE("*********** CST_network_status_test_mngt ********\n\r")

#if (CST_TEST_REGISTER_FAIL == 1)
  if (CST_register_fail_test_count >= CST_REGISTER_FAIL_TEST_COUNT_MAX)
#endif /* (CST_TEST_REGISTER_FAIL == 1) */
  {
    if ((cst_context.current_EPS_NetworkRegState  == CS_NRS_REGISTERED_HOME_NETWORK)
        || (cst_context.current_EPS_NetworkRegState  == CS_NRS_REGISTERED_ROAMING)
        || (cst_context.current_GPRS_NetworkRegState == CS_NRS_REGISTERED_HOME_NETWORK)
        || (cst_context.current_GPRS_NetworkRegState == CS_NRS_REGISTERED_ROAMING))
    {
      /* When registered then stop the NW REG TIMER */
      (void)osTimerStop(CST_network_status_timer_handle);
      PRINT_CELLULAR_SERVICE("-----> Stop NW REG TIMEOUT TIMER\n\r")
      CST_current_state = CST_NETWORK_STATUS_OK_STATE;
      cst_context.register_retry_tempo_count = 0U;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_STATUS_OK_EVENT);
    }
    else
    {
      /* check signal quality if OK then does nothing, wait that Modem completes the registration to the network */
      /* if signal level is KO we lost the signal then enter to the wait for signal state*/
      (void)CST_set_signal_quality();
      if ((cst_context.signal_quality.rssi == 0U) || (cst_context.signal_quality.rssi == CST_BAD_SIG_RSSI))
      {
        /* When registered then stop the NW REG TIMER */
        (void)osTimerStop(CST_network_status_timer_handle);
        PRINT_CELLULAR_SERVICE("-----> BAD SIGNAL : Stop NW REG TIMEOUT TIMER\n\r")
        CST_current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
        cst_context.register_retry_tempo_count = 0U;
        CST_send_message(CST_MESSAGE_CS_EVENT, CST_SIGNAL_QUALITY_EVENT);
      }
    }
  }
}


/**
  * @brief  close network interface
  * @param  none
  * @retval none
  */
static void CST_close_network_interface(void)
{
  dc_nifman_info_t nifman_info;

  PRINT_CELLULAR_SERVICE("*********** CST_close_network_interface ********\n\r")
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
  CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
  if (nifman_info.rt_state   ==  DC_SERVICE_ON)
  {
    for (uint8_t i = 0; i < 10U; i++)
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
      if (nifman_info.rt_state !=  DC_SERVICE_OFF)
      {
        PRINT_CELLULAR_SERVICE("*********** wait for closing Network Interface ********\n\r")
        (void)osDelay(1000);
      }
      else
      {
        PRINT_CELLULAR_SERVICE("nifman_info.rt_state: DC_SERVICE_OFF: NW IF CLOSED DOWN\n\r")
        break;
      }
    }
  }
}

/**
  * @brief  network event processing
  * @param  none
  * @retval none
  */
static void  CST_network_event_mngt2(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t reg_status;

  PRINT_CELLULAR_SERVICE("*********** CST_network_event_mngt2 ********\n\r")
  (void)memset((void *)&reg_status, 0, sizeof(reg_status));
  if (CST_current_state == CST_MODEM_DATA_READY_STATE)
  {
    CST_close_network_interface();
  }

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  osCCS_get_wait_cs_resource();
  (void)osCDS_suspend_data();
  cs_status = osCDS_get_net_status(&reg_status);
#else
  cs_status = osCDS_get_net_status(&reg_status);
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  osCCS_get_release_cs_resource();
#endif      /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
  /* we informs NIFMAN to release the PPP link (we should keep the same for socket mode as well... */
  if (cs_status == CELLULAR_OK)
  {
    cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;

    if ((cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_ROAMING)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_ROAMING))
    {
      CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
      CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    }
    else /* device registered to network */
    {
      if (((uint16_t)reg_status.optional_fields_presence & (uint16_t)CS_RSF_FORMAT_PRESENT) != 0U)
      {
        (void)dc_com_read(&dc_com_db,  DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        (void)memcpy(cst_cellular_info.mno_name, reg_status.operator_name, DC_MAX_SIZE_MNO_NAME);
        cst_cellular_info.rt_state              = DC_SERVICE_ON;
        (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        PRINT_CELLULAR_SERVICE(" ->operator_name = %s", reg_status.operator_name)
      }
      CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    }
  }
  else
  {
    cst_context.current_EPS_NetworkRegState  = CS_NRS_NOT_REGISTERED_SEARCHING;
    cst_context.current_GPRS_NetworkRegState = CS_NRS_NOT_REGISTERED_SEARCHING;
    cst_context.current_GPRS_NetworkRegState = CS_NRS_NOT_REGISTERED_SEARCHING;
    CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
    CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    PRINT_CELLULAR_SERVICE("******** CST_network_event_mngt2: osCDS_get_net_status FAIL ****\n\r")
  }
}

/**
  * @brief  network event processing
  * @param  none
  * @retval none
  */
static void  CST_network_event_mngt(void)
{
  CS_Status_t cs_status;
  CS_RegistrationStatus_t reg_status;

  PRINT_CELLULAR_SERVICE("*********** CST_network_event_mngt ********\n\r")
  (void)memset((void *)&reg_status, 0, sizeof(reg_status));
  cs_status = osCDS_get_net_status(&reg_status);
  if (cs_status == CELLULAR_OK)
  {
    /*  once network registration status is received then stop timer for network registration. */
    /*  Should we first check if Timer is started before stoping it?  */
    (void)osTimerStop(CST_network_status_timer_handle);
    PRINT_CELLULAR_SERVICE("-----> Stop NW REG TIMEOUT TIMER\n\r")

    cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
    cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
    cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;

    if ((cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_EPS_NetworkRegState  != CS_NRS_REGISTERED_ROAMING)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_HOME_NETWORK)
        && (cst_context.current_GPRS_NetworkRegState != CS_NRS_REGISTERED_ROAMING))
    {
      CST_data_cache_cellular_info_set(DC_SERVICE_OFF);
      CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    }
    else /* device registered to network */
    {
      if (((uint16_t)reg_status.optional_fields_presence & (uint16_t)CS_RSF_FORMAT_PRESENT) != 0U)
      {
        (void)dc_com_read(&dc_com_db,  DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        (void)memcpy(cst_cellular_info.mno_name, reg_status.operator_name, DC_MAX_SIZE_MNO_NAME);
        cst_cellular_info.rt_state              = DC_SERVICE_ON;
        (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        PRINT_CELLULAR_SERVICE(" ->operator_name = %s", reg_status.operator_name)
      }
      CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
    }
  }
  else
  {
    PRINT_CELLULAR_SERVICE("******** CST_network_event_mngt: osCDS_get_net_status FAIL ****\n\r")
  }

}

/**
  * @brief  network attachment management
  * @param  none
  * @retval none
  */
static void  CST_attach_modem_mngt(void)
{
  CS_Status_t              cs_status;
  CS_PSattach_t            cst_ctxt_attach_status;
  CS_RegistrationStatus_t  reg_status;

  PRINT_CELLULAR_SERVICE("*********** CST_attach_modem_mngt ********\n\r")

  (void)memset((void *)&reg_status, 0, sizeof(CS_RegistrationStatus_t));
  cs_status = osCDS_get_net_status(&reg_status);

  if (cs_status == CELLULAR_OK)
  {
    if (((uint16_t)reg_status.optional_fields_presence & (uint16_t)CS_RSF_FORMAT_PRESENT) != 0U)
    {
      (void)dc_com_read(&dc_com_db,  DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
      (void)memcpy(cst_cellular_info.mno_name, reg_status.operator_name, DC_MAX_SIZE_MNO_NAME);
      cst_cellular_info.rt_state              = DC_SERVICE_ON;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

      PRINT_CELLULAR_SERVICE(" ->operator_name = %s\n\r", reg_status.operator_name)
    }
  }

  cs_status = osCDS_get_attach_status(&cst_ctxt_attach_status);
#if (CST_RESET_DEBUG == 1)
  if (cst_context.attach_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif  /* (CST_RESET_DEBUG == 1) */
  if (cs_status != CELLULAR_OK)
  {
    PRINT_CELLULAR_SERVICE("*********** CST_attach_modem_mngt fail ********\n\r")
    CST_config_fail_mngt(((uint8_t *)"CS_get_attach_status FAIL"),
                         CST_MODEM_ATTACH_FAIL,
                         &cst_context.attach_reset_count,
                         CST_ATTACH_RESET_MAX);
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_FAIL_EVENT);
  }
  else
  {
    if (cst_ctxt_attach_status == CS_PS_ATTACHED)
    {
      PRINT_CELLULAR_SERVICE("*********** CST_attach_modem_mngt OK ********\n\r")
      CST_current_state = CST_MODEM_REGISTERED_STATE;
      CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_ATTACHED_EVENT);
    }

    else
    {
      PRINT_CELLULAR_SERVICE("===CST_attach_modem_mngt - NOT ATTACHED !!! ===\n\r")
      /* We propose to simply wait network event at "waiting for signal quality OK" state */
      PRINT_CELLULAR_SERVICE("===>CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE <===\n\r")
      CST_current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
    }
  }
}

/**
  * @brief  PDN definition management
  * @param  none
  * @retval none
  */
static void CST_modem_define_pdn(void)
{
  CS_PDN_configuration_t pdn_conf;

  CS_Status_t cs_status;
  /* define user PDN configurations */

  /* common user name and password */
  (void)memset((void *)&pdn_conf, 0, sizeof(CS_PDN_configuration_t));
  (void)memcpy((CRC_CHAR_t *)pdn_conf.username,
               (CRC_CHAR_t *)cst_cellular_params.sim_slot[cst_sim_slot_index].username,
               crs_strlen(cst_cellular_params.sim_slot[cst_sim_slot_index].username) + 1U);

  (void)memcpy((CRC_CHAR_t *)pdn_conf.password,
               (CRC_CHAR_t *)cst_cellular_params.sim_slot[cst_sim_slot_index].password,
               crs_strlen(cst_cellular_params.sim_slot[cst_sim_slot_index].password) + 1U);

  /* example for CS_PDN_USER_CONFIG_1 with access point name =  "PDN CONFIG 1" */
  cs_status = osCDS_define_pdn(cst_get_cid_value(cst_cellular_params.sim_slot[cst_sim_slot_index].cid),
                               (const uint8_t *)cst_cellular_params.sim_slot[cst_sim_slot_index].apn,
                               &pdn_conf);


#if (CST_RESET_DEBUG == 1)
  if (cst_context.activate_pdn_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif /* (CST_RESET_DEBUG == 1) */

  if (cs_status != CELLULAR_OK)
  {
    CST_config_fail_mngt(((uint8_t *)"CST_modem_define_pdn"),
                         CST_MODEM_PDP_DEFINE_FAIL,
                         &cst_context.activate_pdn_reset_count,
                         CST_DEFINE_PDN_RESET_MAX);
  }
  /*
      else
      {
          CST_send_message(CST_MESSAGE_CS_EVENT, CST_PDP_ACTIVATED_EVENT);
      }
  */
}

/**
  * @brief  PDN activation management
  * @param  none
  * @retval none
  */
static CS_Status_t CST_modem_activate_pdn(void)
{
  CS_Status_t cs_status;
  (void)osCDS_set_default_pdn(cst_get_cid_value(cst_cellular_params.sim_slot[cst_sim_slot_index].cid));

  /* register to PDN events for this CID*/
  (void)osCDS_register_pdn_event(cst_get_cid_value(cst_cellular_params.sim_slot[cst_sim_slot_index].cid),
                                 CST_pdn_event_callback);

  cs_status = osCDS_activate_pdn(CS_PDN_CONFIG_DEFAULT);
#if (CST_TEST_NFMC == 1)
  CST_nfmc_test_count++;
  if (CST_nfmc_test_count < CST_NFMC_TEST_COUNT_MAX)
  {
    cs_status = (CS_Status_t)1;
  }
#endif  /*  (CST_TEST_NFMC == 1)  */

#if (CST_RESET_DEBUG == 1)
  if (cst_context.activate_pdn_reset_count == 0)
  {
    cs_status = CELLULAR_ERROR;
  }
#endif  /*  (CST_RESET_DEBUG == 1)  */

  if (cs_status != CELLULAR_OK)
  {
    if (CST_nfmc_context.active == 0U)
    {
      (void)osTimerStart(CST_pdn_activate_retry_timer_handle, CST_PDN_ACTIVATE_RETRY_DELAY);
      PRINT_CELLULAR_SERVICE("-----> CST_modem_activate_pdn NOK - retry tempo  : %d\n\r", CST_PDN_ACTIVATE_RETRY_DELAY)
    }
    else
    {
      (void)osTimerStart(CST_pdn_activate_retry_timer_handle,
                         CST_nfmc_context.tempo[cst_context.activate_pdn_nfmc_tempo_count]);
      PRINT_CELLULAR_SERVICE("-----> CST_modem_activate_pdn NOK - retry tempo %d : %ld\n\r",
                             cst_context.activate_pdn_nfmc_tempo_count + 1U,
                             CST_nfmc_context.tempo[cst_context.activate_pdn_nfmc_tempo_count])
    }

    cst_context.activate_pdn_nfmc_tempo_count++;
    if (cst_context.activate_pdn_nfmc_tempo_count >= CST_NFMC_TEMPO_NB)
    {
      cst_context.activate_pdn_nfmc_tempo_count = 0U;
    }
  }
  else
  {
    cst_context.activate_pdn_nfmc_tempo_count = 0U;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_PDP_ACTIVATED_EVENT);
  }
  return cs_status;
}

/**
  * @brief  cellular data failure management
  * @param  none
  * @retval none
  */
static void CST_cellular_data_fail_mngt(void)
{
  if (CST_current_state == CST_MODEM_DATA_READY_STATE)
  {
    CST_close_network_interface();
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    osCCS_get_wait_cs_resource();
    (void)osCDS_suspend_data(); /* osCDS_stop_data should be the called */
    osCCS_get_release_cs_resource();
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
  }
  /* when DATA plane fails then we just wait event in signal state */
  CST_current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
}

/**
  * @brief  PDN event management
  * @param  none
  * @retval none
  */
static void CST_pdn_event_mngt(void)
{
  if (cst_context.pdn_status == CS_PDN_EVENT_NW_DETACH)
  {
    /* Workaround waiting for Modem behaviour clarification */
    CST_network_event_mngt2();
  }
  else if (
    (cst_context.pdn_status == CS_PDN_EVENT_NW_DEACT)
    || (cst_context.pdn_status == CS_PDN_EVENT_NW_PDN_DEACT))
  {
    PRINT_CELLULAR_SERVICE("=========CST_pdn_event_mngt CS_PDN_EVENT_NW_DEACT\n\r")
    CST_current_state = CST_MODEM_REGISTERED_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_ATTACHED_EVENT);
  }
  else
  {
    CST_current_state = CST_WAITING_FOR_NETWORK_STATUS_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_CALLBACK_EVENT);
  }
}

/**
  * @brief  cellular state event management
  * @param  none
  * @retval none
  */
static void CST_cellular_state_event_mngt(void)
{
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
  cst_cellular_info.rt_state = DC_SERVICE_UNAVAIL;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

  (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
  cst_sim_info.rt_state = DC_SERVICE_UNAVAIL;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

  (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                    sizeof(cst_cellular_data_info));
  /* informs NIFMAN that data plane is shutting down  */
  cst_cellular_data_info.rt_state = DC_SERVICE_SHUTTING_DOWN;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cst_cellular_data_info,
                     sizeof(cst_cellular_data_info));

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  if (CST_current_state == CST_MODEM_DATA_READY_STATE)
  {
    (void)osCDS_suspend_data();
  }
#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP */

  if (cst_cellular_params.target_state == DC_TARGET_STATE_SIM_ONLY)
  {
    PRINT_CELLULAR_SERVICE("****** Transition to CST_MODEM_SIM_ONLY_STATE Ongoing *****\n\r")
    (void) osCDS_init_modem(CS_CMI_SIM_ONLY, CELLULAR_FALSE, CST_SIM_PINCODE);
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));
    cst_cellular_info.modem_state = DC_MODEM_STATE_SIM_CONNECTED;
    (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(dc_cellular_info_t));
    CST_current_state = CST_MODEM_SIM_ONLY_STATE;
    PRINT_CELLULAR_SERVICE("****** CST_MODEM_SIM_ONLY_STATE *****\n\r")
  }
  else
  {
    (void) osCDS_init_modem(CS_CMI_MINI, CELLULAR_FALSE, CST_SIM_PINCODE);
    (void)CS_power_off();
    CST_current_state = CST_MODEM_OFF_STATE;
    CST_send_message(CST_MESSAGE_CMD, CST_INIT_EVENT);
  }
}


/* =================================================================
   Management automaton functions according current automation state
   ================================================================= */

/**
  * @brief  init state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_init_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_INIT_EVENT:
    {
      CST_init_state_mngt();
      break;
    }
    case CST_MODEM_POWER_ON_EVENT:
    {
      CST_current_state = CST_MODEM_ON_ONLY_STATE;
      CST_power_on_only_modem_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 23, ERROR_WARNING);
      break;
    }
  }
  /* subscribe modem events after power ON */
  CST_subscribe_modem_events();
}

/**
  * @brief  reset state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_reset_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_INIT_EVENT:
    {
      CST_current_state = CST_MODEM_ON_STATE;
      CST_reset_modem_mngt();
      break;
    }
    case CST_APN_CONFIG_EVENT:
    {
      CST_current_state = CST_MODEM_ON_STATE;
      CST_apn_set_new_config();

      CST_reset_modem_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 24, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  modem on state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_on_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_MODEM_POWERED_ON_EVENT:
    {
      CST_modem_powered_on_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 25, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  modem off state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_off_state(CST_autom_event_t autom_event)
{
  dc_cellular_info_t dc_cellular_info;
  switch (autom_event)
  {
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_init_state_mngt();
      break;
    }
    case CST_INIT_EVENT:
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
      dc_cellular_info.modem_state = DC_MODEM_STATE_OFF;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
      break;
    }
    default:
    {
      /* Nothing to do */
      break;
    }
  }
}

/**
  * @brief  modem sim only state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_sim_only_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_CELLULAR_STATE_EVENT:
    {
      if ((cst_cellular_params.target_state != DC_TARGET_STATE_OFF)
          && (cst_cellular_params.target_state != DC_TARGET_STATE_UNKNOWN))
      {
        CST_modem_powered_on_mngt();
      }
      else if (cst_cellular_params.target_state == DC_TARGET_STATE_OFF)
      {
        CST_cellular_state_event_mngt();
      }
      else
      {
        /* Nothing to do */
        __NOP();
      }
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 25, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  modem powered on state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_powered_on_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_MODEM_INITIALIZED_EVENT:
    {
      CST_current_state = CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE;
      CST_net_register_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 27, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  waiting for signal quality state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_waiting_for_signal_quality_ok_state(CST_autom_event_t autom_event)
{
  PRINT_CELLULAR_SERVICE("\n\r ====> CST_waiting_for_signal_quality_ok_state <===== \n\r")

  switch (autom_event)
  {
    case CST_NETWORK_CALLBACK_EVENT:
    {
      CST_network_event_mngt();
      break;
    }
    case CST_SIGNAL_QUALITY_EVENT:
    {
      CST_signal_quality_test_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 28, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  waiting for network status state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_waiting_for_network_status_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_NETWORK_CALLBACK_EVENT:
    case CST_NETWORK_STATUS_EVENT:
    {
      CST_network_status_test_mngt();
      break;
    }
    case CST_NW_REG_TIMEOUT_TIMER_EVENT:
    {
      PRINT_CELLULAR_SERVICE("-----> NW REG TIMEOUT TIMER EXPIRY WE PWDN THE MODEM \n\r")
      CST_current_state = CST_MODEM_NETWORK_STATUS_FAIL_STATE;
      (void)CS_power_off();

      (void)osTimerStart(CST_register_retry_timer_handle,
                         CST_nfmc_context.tempo[cst_context.register_retry_tempo_count]);
      PRINT_CELLULAR_SERVICE("-----> CST_waiting_for_network_status NOK - retry tempo %d : %ld\n\r",
                             cst_context.register_retry_tempo_count + 1U,
                             CST_nfmc_context.tempo[cst_context.register_retry_tempo_count])
#if (CST_TEST_REGISTER_FAIL == 1)
      CST_register_fail_test_count++;
#endif /* (CST_TEST_REGISTER_FAIL == 1) */
      cst_context.register_retry_tempo_count++;
      if (cst_context.register_retry_tempo_count >= CST_NFMC_TEMPO_NB)
      {
        cst_context.register_retry_tempo_count = 0U;
      }
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 29, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  network status ok state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_network_status_ok_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_NETWORK_STATUS_OK_EVENT:
    {
      CST_attach_modem_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 30, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  modem registered state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_registered_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_MODEM_ATTACHED_EVENT:
    {
      CST_current_state = CST_MODEM_PDN_ACTIVATE_STATE;
      (void)CST_modem_activate_pdn();
      break;
    }
    case CST_NETWORK_CALLBACK_EVENT:
    {
      CST_network_event_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 31, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @brief  modem PDN activate state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_modem_pdn_activate_state(CST_autom_event_t autom_event)
{
  dc_cellular_info_t dc_cellular_info;
  switch (autom_event)
  {
    case CST_PDP_ACTIVATED_EVENT:
    {
      CST_reset_fail_count();
      CST_current_state       = CST_MODEM_DATA_READY_STATE;
      CST_data_cache_cellular_info_set(DC_SERVICE_ON);
      /* Data Cache -> Radio ON */
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));
      dc_cellular_info.modem_state = DC_MODEM_STATE_DATA_OK;
      (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&dc_cellular_info, sizeof(dc_cellular_info));

      break;
    }
    case CST_MODEM_PDN_ACTIVATE_RETRY_TIMER_EVENT:
    {
      (void)CST_modem_activate_pdn();
      break;
    }
    case CST_NETWORK_CALLBACK_EVENT:
    {
      CST_network_event_mngt();
      break;
    }
    case CST_PDN_STATUS_EVENT:
    {
      CST_pdn_event_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      CST_cellular_state_event_mngt();
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 32, ERROR_WARNING);
      break;
    }
  }
}

/**
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_data_ready_state(CST_autom_event_t autom_event)
{
  dc_nifman_info_t nifman_info;
  dc_cellular_target_state_t cst_target_state;

  switch (autom_event)
  {
    case CST_NETWORK_CALLBACK_EVENT:
    {
      CST_network_event_mngt();
      break;
    }
    case CST_NIFMAN_EVENT:
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_TARGET_STATE_CMD, (void *)&cst_target_state, sizeof(cst_target_state));
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
      if (DC_SERVICE_ON == cst_target_state.rt_state)
      {
        /*  we check target state to process any target state change while Network IF is being estabhlished. */
        if (cst_target_state.target_state != DC_TARGET_STATE_FULL)
        {
          if (nifman_info.rt_state == DC_SERVICE_ON)
          {
            CST_close_network_interface();
            CST_cellular_state_event_mngt();
          }
        }
      }
      break;
    }

    case CST_CELLULAR_DATA_FAIL_EVENT:
    {
      CST_cellular_data_fail_mngt();
      break;
    }
    case CST_PDN_STATUS_EVENT:
    {
      CST_pdn_event_mngt();
      break;
    }
    case CST_CELLULAR_STATE_EVENT:
    {
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
      if (nifman_info.rt_state == DC_SERVICE_ON)
      {
        CST_close_network_interface();
        CST_cellular_state_event_mngt();
      }
      break;
    }

#if (USE_LOW_POWER == 1)
    case CST_POWER_SLEEP_REQUEST_EVENT:
    {
      CST_current_state = CST_MODEM_POWER_DATA_IDLE_STATE;
      CSP_DataIdleManagment();
      break;
    }
#endif  /* (USE_LOW_POWER == 1) */

    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 33, ERROR_WARNING);
      break;
    }
  }
}



/**
  * @brief  power idle state processing
  * @param  autom_event - automaton event
  * @retval none
  */
#if (USE_LOW_POWER == 1)
static void CST_power_idle_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_POWER_SLEEP_TIMEOUT_EVENT:
    {
      CSP_WakeupComplete();
      CST_current_state = CST_MODEM_DATA_READY_STATE;
      break;
    }
    case CST_POWER_SLEEP_ABORT_EVENT:
    {
      CSP_WakeupComplete();
      CST_current_state = CST_MODEM_DATA_READY_STATE;
      break;
    }
    case CST_POWER_SLEEP_COMPLETE_EVENT:
    {
      CSP_SleepComplete();
      break;
    }
    case CST_POWER_WAKEUP_EVENT:
    {
      CSP_WakeupComplete();
      CST_current_state = CST_MODEM_DATA_READY_STATE;
      break;
    }
    case CST_FAIL_EVENT:
    {
      CSP_WakeupComplete();
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, __LINE__, ERROR_WARNING);
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 26, ERROR_WARNING);
      break;
    }
  }
}
#endif /* (USE_LOW_POWER == 1) */

/**
  * @brief  failure state processing
  * @param  autom_event - automaton event
  * @retval none
  */
static void CST_fail_state(CST_autom_event_t autom_event)
{
  switch (autom_event)
  {
    case CST_FAIL_EVENT:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, __LINE__, ERROR_WARNING);
      break;
    }
    default:
    {
      ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 26, ERROR_WARNING);
      break;
    }
  }
}

/* ===================================================================
   Automaton functions  END
   =================================================================== */

/**
  * @brief  Timer handler
  * @note   During configuration :  Signal quality  and network status Polling
  * @note   After configuration  :  Modem monitoring (Signal quality checking)
  * @param  none
  * @retval none
  */
static void CST_timer_handler(void)
{
  CS_Status_t cs_status ;
  CS_RegistrationStatus_t reg_status;

  /*    PRINT_CELLULAR_SERVICE("-----> CST_timer_handler  <-----\n\r") */
  CST_polling_timer_flag = 0U;
  if (CST_current_state == CST_WAITING_FOR_NETWORK_STATUS_STATE)
  {
    (void)memset((void *)&reg_status, 0, sizeof(CS_RegistrationStatus_t));
    cs_status = osCDS_get_net_status(&reg_status);
    if (cs_status == CELLULAR_OK)
    {
      cst_context.current_EPS_NetworkRegState  = reg_status.EPS_NetworkRegState;
      cst_context.current_GPRS_NetworkRegState = reg_status.GPRS_NetworkRegState;
      cst_context.current_CS_NetworkRegState   = reg_status.CS_NetworkRegState;
      PRINT_CELLULAR_SERVICE("-----> CST_timer_handler - CST_NETWORK_STATUS_EVENT <-----\n\r")

      CST_send_message(CST_MESSAGE_CS_EVENT, CST_NETWORK_STATUS_EVENT);

      if (((uint16_t)reg_status.optional_fields_presence & (uint16_t)CS_RSF_FORMAT_PRESENT) != 0U)
      {
        (void)dc_com_read(&dc_com_db,  DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));
        (void)memcpy(cst_cellular_info.mno_name, reg_status.operator_name, DC_MAX_SIZE_MNO_NAME);
        cst_cellular_info.rt_state              = DC_SERVICE_ON;
        (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

        PRINT_CELLULAR_SERVICE(" ->operator_name = %s\n\r", reg_status.operator_name)
      }
    }
    else
    {
      CST_config_fail_mngt(((uint8_t *)"osCDS_get_net_status"),
                           CST_MODEM_GNS_FAIL,
                           &cst_context.gns_reset_count,
                           CST_GNS_MODEM_RESET_MAX);
    }
  }

  if (CST_current_state == CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE)
  {
    (void)CST_set_signal_quality();
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_SIGNAL_QUALITY_EVENT);
  }

#if (CST_FOTA_TEST == 1)
  if (CST_current_state == CST_MODEM_DATA_READY_STATE)
  {
    CST_modem_event_callback(CS_MDMEVENT_FOTA_START);
  }
#endif /* (CST_FOTA_TEST == 1) */

#if (CST_MODEM_POLLING_PERIOD != 0)
  if (CST_polling_active == 1U)
  {
    if (CST_current_state == CST_MODEM_DATA_READY_STATE)
    {
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
      osCCS_get_wait_cs_resource();
      dc_nifman_info_t nifman_info;
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
      if (nifman_info.rt_state   ==  DC_SERVICE_ON)
      {
        /* we should read the status if connection lost while changing to at command mode */
        cs_status = osCDS_suspend_data();

        /* For instance disable the signal polling to test suspend resume  */
        (void)CST_set_signal_quality();
        /* we should read the status if connection lost while resuming data */
        cs_status = osCDS_resume_data();
      }
      osCCS_get_release_cs_resource();
      if (cs_status != CELLULAR_OK)
      {
        /* to add resume_data failure */
        CST_cellular_data_fail_mngt();
      }
#else
      /* For instance disable the signal polling to test suspend resume  */
      (void)CST_set_signal_quality();
#endif /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
    }
  }
#endif /* CST_MODEM_POLLING_PERIOD != 0) */
}

/**
  * @brief  Cellular Service Task : automaton management
  * @param  argument - Task argument (not used)
  * @retval none
  */
static void CST_cellular_service_task(void const *argument)
{
  osEvent event;
  CST_autom_event_t autom_event;

  for (;;)
  {
    event = osMessageGet(cst_queue_id, RTOS_WAIT_FOREVER);
    autom_event = CST_get_autom_event(event);
    /*    PRINT_CELLULAR_SERVICE("======== CST_cellular_service_task autom_event %d\n\r", autom_event) */

    if (autom_event == CST_POLLING_TIMER)
    {
      CST_timer_handler();
    }
    else if (autom_event != CST_NO_EVENT)
    {
      switch (CST_current_state)
      {
        case CST_MODEM_INIT_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_INIT_STATE <-----\n\r")

          CST_init_state(autom_event);
          break;
        }
        case CST_MODEM_RESET_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_RESET_STATE <-----\n\r")
          CST_reset_state(autom_event);
          break;
        }
        case CST_MODEM_ON_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_ON_STATE <-----\n\r")
          CST_modem_on_state(autom_event);
          break;
        }
        case CST_MODEM_OFF_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_OFF_STATE <-----\n\r")
          CST_modem_off_state(autom_event);
          break;
        }
        case CST_MODEM_SIM_ONLY_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_SIM_ONLY_STATE <-----\n\r")
          CST_modem_sim_only_state(autom_event);
          break;
        }
        case CST_MODEM_POWERED_ON_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_POWERED_ON_STATE <-----\n\r")
          CST_modem_powered_on_state(autom_event);
          break;
        }
        case CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_WAITING_FOR_SIGNAL_QUALITY_OK_STATE <-----\n\r")
          CST_waiting_for_signal_quality_ok_state(autom_event);
          break;
        }
        case CST_WAITING_FOR_NETWORK_STATUS_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_WAITING_FOR_NETWORK_STATUS_STATE <-----\n\r")
          CST_waiting_for_network_status_state(autom_event);
          break;
        }

        case CST_NETWORK_STATUS_OK_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_NETWORK_STATUS_OK_STATE <-----\n\r")
          CST_network_status_ok_state(autom_event);
          break;
        }

        case CST_MODEM_REGISTERED_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_REGISTERED_STATE <-----\n\r")
          CST_modem_registered_state(autom_event);
          break;
        }
        case CST_MODEM_PDN_ACTIVATE_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_PDN_ACTIVATE_STATE <-----\n\r")
          CST_modem_pdn_activate_state(autom_event);
          break;
        }

        case CST_MODEM_DATA_READY_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_DATA_READY_STATE <-----\n\r")
          CST_data_ready_state(autom_event);
          break;
        }
#if (USE_LOW_POWER == 1)
        case CST_MODEM_POWER_DATA_IDLE_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_POWER_DATA_IDLE_STATE <-----\n\r")
          CST_power_idle_state(autom_event);
          break;
        }
#endif /* (USE_LOW_POWER == 1) */

        case CST_MODEM_FAIL_STATE:
        {
          PRINT_CELLULAR_SERVICE("-----> State : CST_MODEM_FAIL_STATE <-----\n\r")
          CST_fail_state(autom_event);
          break;
        }
        default:
        {
          PRINT_CELLULAR_SERVICE("-----> State : Not State <-----\n\r")
          break;
        }
      }
    }
    else
    {
      PRINT_CELLULAR_SERVICE("============ CST_cellular_service_task : autom_event = no event \n\r")
    }
  }
}

/**
  * @brief  CST_polling_timer_callback function
  * @param  argument - argument (not used)
  * @retval none
  */
static void CST_polling_timer_callback(void const *argument)
{
  UNUSED(argument);
  if (CST_current_state != CST_MODEM_INIT_STATE)
  {
    CST_message_t cmd_message;
    uint32_t *cmd_message_p = (uint32_t *)(&cmd_message);

    cmd_message.type = (uint16_t)CST_MESSAGE_TIMER_EVENT;
    cmd_message.id   = CST_NO_EVENT;

    if (CST_polling_timer_flag == 0U)
    {
      CST_polling_timer_flag = 1U;
      (void)osMessagePut(cst_queue_id, *cmd_message_p, 0U);
    }
  }
}

/**
  * @brief  timer callback to retry PDN activation
  * @param  argument - argument (not used)
  * @retval none
  */
static void CST_pdn_activate_retry_timer_callback(void const *argument)
{
  UNUSED(argument);
  if (CST_current_state == CST_MODEM_PDN_ACTIVATE_STATE)
  {
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_MODEM_PDN_ACTIVATE_RETRY_TIMER_EVENT);
  }
}

/**
  * @brief  timer callback to retry network register
  * @param  argument - argument (not used)
  * @retval none
  */
static void CST_register_retry_timer_callback(void const *argument)
{
  UNUSED(argument);
  if (CST_current_state == CST_MODEM_NETWORK_STATUS_FAIL_STATE)
  {
    CST_current_state = CST_MODEM_INIT_STATE;
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_INIT_EVENT);
  }
}

/**
  * @brief  timer callback to check network status
  * @param  argument - argument (not used)
  * @retval none
  */
static void CST_network_status_timer_callback(void const *argument)
{
  UNUSED(argument);
  if (CST_current_state == CST_WAITING_FOR_NETWORK_STATUS_STATE)
  {
    CST_send_message(CST_MESSAGE_CS_EVENT, CST_NW_REG_TIMEOUT_TIMER_EVENT);
  }
}

/**
  * @brief  FOTA timeout: FOTA failure => board restart
  * @param  argument - argument (not used)
  * @retval none
  */
static void CST_fota_timer_callback(void const *argument)
{
  UNUSED(argument);
  PRINT_CELLULAR_SERVICE("CST FOTA FAIL : Timeout expired RESTART\n\r")

  ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 20, ERROR_FATAL);
}


/* PublicFunctions Definition ------------------------------------------------------*/
/**
  * @brief  allows to set radio on: start cellular automaton
  * @param  none
  * @retval return code
  */
CS_Status_t  CST_radio_on(void)
{
  CST_send_message(CST_MESSAGE_CMD, CST_INIT_EVENT);
  return CELLULAR_OK;
}

/**
  * @brief  allows to boot modem only without network register
  * @note   the application is not started
  * @param  none
  * @retval return code
  */
CS_Status_t  CST_modem_power_on(void)
{
  CST_send_message(CST_MESSAGE_CMD, CST_MODEM_POWER_ON_EVENT);
  return CELLULAR_OK;
}

/**
  * @brief  allows to get the modem IP addr
  * @note   to be able to return an IP address the modem must be in data tranfer state
  * @note   else an error is returned
  * @param  ip_addr_type - type of IP address
  * @param  p_ip_addr_value - IP address value returned by the function
  * @retval return code
  */
CS_Status_t CST_get_dev_IP_address(CS_IPaddrType_t *ip_addr_type, CS_CHAR_t *p_ip_addr_value)
{
  return CS_get_dev_IP_address(cst_get_cid_value(cst_cellular_params.sim_slot[cst_sim_slot_index].cid), ip_addr_type,
                               p_ip_addr_value);
}

/**
  * @brief  initializes cellular service component
  * @param  none
  * @retval return code
  */
CS_Status_t CST_cellular_service_init(void)
{
  CST_current_state = CST_MODEM_INIT_STATE;
  CST_modem_init();

  (void)osCDS_cellular_service_init();
  CST_csq_count_fail = 0U;
  CST_polling_active = 0U;

  (void)CST_config_init();
#if (USE_LOW_POWER == 1)
  CSP_Init();
#endif /* (USE_LOW_POWER == 1) */

  osMessageQDef(cst_queue_id, 10, sizeof(CST_message_t));
  cst_queue_id = osMessageCreate(osMessageQ(cst_queue_id), NULL);
  if (cst_queue_id == NULL)
  {
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 18, ERROR_FATAL);
  }
  return CELLULAR_OK;
}

/**
  * @brief  allows to get cellular service automaton current state
  * @param  none
  * @retval automaton state
  */
CST_state_t CST_get_state(void)
{
  return CST_current_state;
}

/**
  * @brief  starts cellular service component
  * @note   cellular service task automaton and tempos are started
  * @param  none
  * @retval return code
  */
CS_Status_t CST_cellular_service_start(void)
{
  static osThreadId CST_cellularServiceThreadId = NULL;
  dc_nfmc_info_t nfmc_info;
  uint32_t cst_polling_period;

  osTimerId         cst_polling_timer_handle;
  /* reads cellular configuration in Data Cache */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_CONFIG, (void *)&cst_cellular_params, sizeof(cst_cellular_params));

  /* initializes Data Cache SIM slot entry  */
  (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));
  cst_sim_info.sim_status[DC_SIM_SLOT_MODEM_SOCKET]       = DC_SIM_NOT_USED;
  cst_sim_info.sim_status[DC_SIM_SLOT_MODEM_EMBEDDED_SIM] = DC_SIM_NOT_USED;
  cst_sim_info.sim_status[DC_SIM_SLOT_STM32_EMBEDDED_SIM] = DC_SIM_NOT_USED;
  cst_sim_slot_index = 0;
  cst_sim_info.active_slot = cst_cellular_params.sim_slot[cst_sim_slot_index].sim_slot_type;
  cst_sim_info.index_slot  = cst_sim_slot_index;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&cst_sim_info, sizeof(cst_sim_info));

  /* modem start */
  CST_modem_start();

  /* register component to Data Cache  */
  (void)dc_com_register_gen_event_cb(&dc_com_db, CST_notif_cb, (const void *)NULL);
  cst_cellular_info.mno_name[0]           = 0U;
  cst_cellular_info.rt_state              = DC_SERVICE_UNAVAIL;


  /* registers component callback to Data Cache  */
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_INFO, (void *)&cst_cellular_info, sizeof(cst_cellular_info));

  /* initializes Data Cache NFMC entry  */
  nfmc_info.rt_state = DC_SERVICE_UNAVAIL;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_NFMC_INFO, (void *)&nfmc_info, sizeof(nfmc_info));

  /* creates and start modem polling timer */
  osTimerDef(cs_polling_timer, CST_polling_timer_callback);
  cst_polling_timer_handle = osTimerCreate(osTimer(cs_polling_timer), osTimerPeriodic, NULL);
#if (CST_MODEM_POLLING_PERIOD == 0)
  cst_polling_period = CST_MODEM_POLLING_PERIOD_DEFAULT;
#else
  cst_polling_period = CST_MODEM_POLLING_PERIOD;
#endif /* (CST_MODEM_POLLING_PERIOD == 1) */
  (void)osTimerStart(cst_polling_timer_handle, cst_polling_period);

  /* creates timers */
  osTimerDef(CST_pdn_activate_retry_timer, CST_pdn_activate_retry_timer_callback);
  CST_pdn_activate_retry_timer_handle = osTimerCreate(osTimer(CST_pdn_activate_retry_timer), osTimerOnce, NULL);

  osTimerDef(CST_waiting_for_network_status_timer, CST_network_status_timer_callback);
  CST_network_status_timer_handle = osTimerCreate(osTimer(CST_waiting_for_network_status_timer), osTimerOnce, NULL);

  osTimerDef(CST_register_retry_timer, CST_register_retry_timer_callback);
  CST_register_retry_timer_handle = osTimerCreate(osTimer(CST_register_retry_timer), osTimerOnce, NULL);

  osTimerDef(CST_fota_timer, CST_fota_timer_callback);
  CST_fota_timer_handle = osTimerCreate(osTimer(CST_fota_timer), osTimerOnce, NULL);

  /* creates and starts cellar service task automaton */
  osThreadDef(cellularServiceTask, CST_cellular_service_task, CELLULAR_SERVICE_THREAD_PRIO, 0,
              CELLULAR_SERVICE_THREAD_STACK_SIZE);
  CST_cellularServiceThreadId = osThreadCreate(osThread(cellularServiceTask), NULL);

  if (CST_cellularServiceThreadId == NULL)
  {
    ERROR_Handler(DBG_CHAN_CELLULAR_SERVICE, 19, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(CST_cellularServiceThreadId, CELLULAR_SERVICE_THREAD_STACK_SIZE);
#endif /* (STACK_ANALYSIS_TRACE==1) */
  }

  return CELLULAR_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

