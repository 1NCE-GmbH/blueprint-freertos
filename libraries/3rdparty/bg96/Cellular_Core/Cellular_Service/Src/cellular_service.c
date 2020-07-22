/**
  ******************************************************************************
  * @file    cellular_service.c
  * @author  MCD Application Team
  * @brief   This file defines functions for Cellular Service
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
#include <string.h>
#include "plf_config.h"
#include "cellular_service.h"
#include "cellular_service_int.h"
#include "at_core.h"
#include "at_datapack.h"
#include "at_util.h"
#include "sysctrl.h"
#include "cellular_runtime_custom.h"

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_P0, "CS:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_P1, "CS:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_P2, "CS API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_MAIN, DBL_LVL_ERR, "CS ERROR:" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_INFO(format, args...)  (void) printf("CS:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("CS ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_CELLULAR_SERVICE */

/* Private variables ---------------------------------------------------------*/
/* Cellular service context variables */
static at_buf_t cmd_buf[ATCMD_MAX_BUF_SIZE];
static at_buf_t rsp_buf[ATCMD_MAX_BUF_SIZE];

/* Permanent variables */
static at_handle_t _Adapter_Handle;
#if (RTOS_USED == 0)
static cellular_event_callback_t register_event_callback; /* to report idle event in Bare mode */
static __IO uint32_t eventReceived = 0U;
#endif /* RTOS_USED */

/* URC callbacks */
#define CS_MAX_NB_PDP_CTXT  ((uint8_t) CS_PDN_CONFIG_MAX + 1U)
static cellular_urc_callback_t urc_eps_network_registration_callback = NULL;
static cellular_urc_callback_t urc_gprs_network_registration_callback = NULL;
static cellular_urc_callback_t urc_cs_network_registration_callback = NULL;
static cellular_urc_callback_t urc_eps_location_info_callback = NULL;
static cellular_urc_callback_t urc_gprs_location_info_callback = NULL;
static cellular_urc_callback_t urc_cs_location_info_callback = NULL;
static cellular_urc_callback_t urc_signal_quality_callback = NULL;
static cellular_ping_response_callback_t urc_ping_rsp_callback = NULL;
static cellular_pdn_event_callback_t urc_packet_domain_event_callback[CS_MAX_NB_PDP_CTXT] = {NULL};
static cellular_modem_event_callback_t urc_modem_event_callback = NULL;

/* Non-permanent variables  */
static csint_urc_subscription_t cs_ctxt_urc_subscription =
{
  .eps_network_registration = CELLULAR_FALSE,
  .gprs_network_registration = CELLULAR_FALSE,
  .cs_network_registration = CELLULAR_FALSE,
  .eps_location_info = CELLULAR_FALSE,
  .gprs_location_info = CELLULAR_FALSE,
  .cs_location_info = CELLULAR_FALSE,
  .signal_quality = CELLULAR_FALSE,
  .packet_domain_event = CELLULAR_FALSE,
  .ping_rsp = CELLULAR_FALSE,
};
static csint_location_info_t cs_ctxt_eps_location_info =
{
  .ci = 0,
  .lac = 0,
  .ci_updated = CELLULAR_FALSE,
  .lac_updated = CELLULAR_FALSE,
};
static csint_location_info_t cs_ctxt_gprs_location_info =
{
  .ci = 0,
  .lac = 0,
  .ci_updated = CELLULAR_FALSE,
  .lac_updated = CELLULAR_FALSE,
};
static csint_location_info_t cs_ctxt_cs_location_info =
{
  .ci = 0,
  .lac = 0,
  .ci_updated = CELLULAR_FALSE,
  .lac_updated = CELLULAR_FALSE,
};
static CS_NetworkRegState_t cs_ctxt_eps_network_reg_state = CS_NRS_UNKNOWN;
static CS_NetworkRegState_t cs_ctxt_gprs_network_reg_state = CS_NRS_UNKNOWN;
static CS_NetworkRegState_t cs_ctxt_cs_network_reg_state = CS_NRS_UNKNOWN;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void CELLULAR_reset_context(void);
static void CELLULAR_reset_socket_context(void);
static CS_Status_t CELLULAR_init(void);
static void CELLULAR_idle_event_notif(void);
static void CELLULAR_urc_notif(at_buf_t *p_rsp_buf);
static CS_Status_t CELLULAR_analyze_error_report(at_buf_t *p_rsp_buf);
static CS_Status_t convert_SIM_error(const csint_error_report_t *p_error_report);

static CS_Status_t perform_HW_reset(void);
static CS_Status_t perform_SW_reset(void);
static CS_Status_t perform_Factory_reset(void);


static CS_PDN_event_t convert_to_PDN_event(csint_PDN_event_desc_t event_desc);
static CS_PDN_conf_id_t convert_index_to_PDN_conf(uint8_t index);

/* Functions Definition ------------------------------------------------------*/
#if (RTOS_USED == 0)
/**
  * @brief  Check if events have been received in idle (bare mode only)
  * @note   This function has to be called peridiocally by the client in bare mode.
  * @param  event_callback client callback to call when an event occured
  */
void CS_check_idle_events(void)
{
  if (eventReceived > 0)
  {
    AT_getevent(_Adapter_Handle, &rsp_buf[0]);
    eventReceived--;
  }
}

/**
  * @brief  Initialization of Cellular Service in bare mode
  * @note   This function is used to init Cellular Service and also set the event
  *         callback which is used in bare mode only.
  * @note   !!! IMPORTANT !!! in Bare mode, event_callback will be call under interrupt
  *         The client implementation should be as short as possible
  *         (for example: no traces !!!)
  * @param  event_callback client callback to call when an event occured
  */
CS_Status_t CS_init_bare(cellular_event_callback_t event_callback)
{
  CS_Status_t retval = CELLULAR_OK;
  PRINT_API("CS_init_bare")

  retval = CELLULAR_init();

  /* save the callback (after the call to CELLULAR_init) */
  register_event_callback = event_callback;

  return (retval);
}

#else /* RTOS_USED */

/**
  * @brief  Initialization of Cellular Service in RTOS mode
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_init(void)
{
  CS_Status_t retval;
  PRINT_API("CS_init")

  retval = CELLULAR_init();

  return (retval);
}
#endif /* RTOS_USED */

/**
  * @brief  Power ON the modem
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_power_on(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_power_on")

  /* 1st step: power on modem by triggering GPIOs */
  if (SysCtrl_power_on(DEVTYPE_MODEM_CELLULAR) == SCSTATUS_OK)
  {
    /* 2nd step: open UART channel */
    if (SysCtrl_open_channel(DEVTYPE_MODEM_CELLULAR) == SCSTATUS_OK)
    {
      /* 3rd step: open AT channel (IPC) */
      if (AT_open_channel(_Adapter_Handle) == ATSTATUS_OK)
      {

        /* 4th step: send AT commands sequence to setup modem after power on */
        if (DATAPACK_writeStruct(&cmd_buf[0],
                                 (uint16_t) CSMT_NONE,
                                 (uint16_t) 0U,
                                 NULL) == DATAPACK_OK)
        {
          at_status_t err;
          err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_POWER_ON, &cmd_buf[0], &rsp_buf[0]);
          if (err == ATSTATUS_OK)
          {
            PRINT_DBG("Cellular started and ready")
            retval = CELLULAR_OK;
          }
        }
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when power on process")
  }
  return (retval);
}

/**
  * @brief  Power OFF the modem
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_power_off(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_power_off")

  /* reset Cellular Service context */
  CELLULAR_reset_context();

  /* update sockets states */
  csint_modem_reset_update_socket_state();

  /* 1st step: send AT commands sequence for power off (if exist fot this modem) */
  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_POWER_OFF, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      /* 2nd step: close AT channel (IPC) */
      if (AT_close_channel(_Adapter_Handle) == ATSTATUS_OK)
      {
        /* 3rd step: close UART channel */
        if (SysCtrl_close_channel(DEVTYPE_MODEM_CELLULAR) == SCSTATUS_OK)
        {
          /* 4th step: power off modem by triggering GPIOs  */
          if (SysCtrl_power_off(DEVTYPE_MODEM_CELLULAR) == SCSTATUS_OK)
          {
            PRINT_DBG("<Cellular_Service> Stopped")
            retval = CELLULAR_OK;
          }
        }
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during power off process")
  }
  return (retval);
}

/**
  * @brief  Check that modem connection is successfully established
  * @note   Usually, the command AT is sent and OK is expected as response
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_check_connection(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_check_connection")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_CHECK_CNX, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Modem connection OK")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error with modem connection")
  }
  return (retval);
}

/**
  * @brief  Select SIM slot to use.
  * @note   Only one SIM slot is active at a time.
  * @param  simSelected Selected SIM slot
  * @retval CS_Status_t
  */
CS_Status_t CS_sim_select(CS_SimSlot_t simSelected)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_sim_select")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_SIM_SELECT,
                           (uint16_t) sizeof(CS_SimSlot_t),
                           (void *)&simSelected) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SIM_SELECT, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> SIM %d selected", simSelected)
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error with sim selection")
  }
  return (retval);
}


/**
  * @brief  Initialize the service and configures the Modem FW functionalities
  * @note   Used to provide PIN code (if any) and modem function level.
  * @param  init Function level (MINI, FULL, SIM only).
  * @param  reset Indicates if modem reset will be applied or not.
  * @param  pin_code PIN code string.
  *
  * @retval CS_Status_t
  */
CS_Status_t CS_init_modem(CS_ModemInit_t init, CS_Bool_t reset, const CS_CHAR_t *pin_code)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_init_modem")

  csint_modemInit_t modemInit_struct;
  (void) memset((void *)&modemInit_struct, 0, sizeof(modemInit_struct));
  modemInit_struct.init = init;
  modemInit_struct.reset = reset;
  (void) memcpy((void *)&modemInit_struct.pincode.pincode[0],
                (const CS_CHAR_t *)pin_code,
                strlen((const CRC_CHAR_t *)pin_code));

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_INITMODEM,
                           (uint16_t) sizeof(csint_modemInit_t),
                           (void *)&modemInit_struct) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_INIT_MODEM, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Init done succesfully")
      retval = CELLULAR_OK;
    }
    else
    {
      retval = CELLULAR_analyze_error_report(&rsp_buf[0]);
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during init")
  }
  return (retval);
}

/**
  * @brief  Return information related to modem status.
  * @param  p_devinfo Handle on modem information structure.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_device_info(CS_DeviceInfo_t *p_devinfo)
{
  /* static structure used to send datas to lower layer */
  static CS_DeviceInfo_t cs_ctxt_device_info = {0};

  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_get_device_info")

  /* reset our local copy */
  (void) memset((void *)&cs_ctxt_device_info, 0, sizeof(cs_ctxt_device_info));
  cs_ctxt_device_info. field_requested = p_devinfo->field_requested;
  if (DATAPACK_writePtr(&cmd_buf[0],
                        (uint16_t) CSMT_DEVICE_INFO,
                        (void *)&cs_ctxt_device_info) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_GET_DEVICE_INFO, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Device infos received")
      /* send info to user */
      (void) memcpy((void *)p_devinfo, (void *)&cs_ctxt_device_info, sizeof(CS_DeviceInfo_t));
      retval = CELLULAR_OK;
    }
    else
    {
      retval = CELLULAR_analyze_error_report(&rsp_buf[0]);
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when getting device infos")
  }
  return (retval);
}

/**
  * @brief  Request the Modem to register to the Cellular Network.
  * @note   This function is used to select the operator. It returns a detailled
  *         network registration status.
  * @param  p_devinfo Handle on operator information structure.
  * @param  p_reg_status Handle on registration information structure.
  *         This information is valid only if return code is CELLULAR_OK
  * @retval CS_Status_t
  */
CS_Status_t CS_register_net(CS_OperatorSelector_t *p_operator,
                            CS_RegistrationStatus_t *p_reg_status)
{
  /* static structure used to send datas to lower layer */
  static CS_OperatorSelector_t cs_ctxt_operator;

  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_register_net")

  /* init returned fields */
  p_reg_status->optional_fields_presence = CS_RSF_NONE;
  p_reg_status->CS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->GPRS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->EPS_NetworkRegState = CS_NRS_UNKNOWN;

  (void) memcpy((void *)&cs_ctxt_operator, (void *)p_operator, sizeof(CS_OperatorSelector_t));
  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_OPERATORSELECT,
                           (uint16_t) sizeof(CS_OperatorSelector_t),
                           (void *)&cs_ctxt_operator) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_REGISTER_NET, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      if (DATAPACK_readStruct(&rsp_buf[0],
                              (uint16_t) CSMT_REGISTRATIONSTATUS,
                              (uint16_t) sizeof(CS_RegistrationStatus_t),
                              p_reg_status) == DATAPACK_OK)
      {
        PRINT_DBG("<Cellular_Service> Network registration done")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during network registration ")
  }
  return (retval);
}

/**
  * @brief  Register to an event change notification related to Network status.
  * @note   This function should be called for each event the user wants to start monitoring.
  * @param  event Event that will be registered to change notification.
  * @param  urc_callback Handle on user callback that will be used to notify a
  *                      change on requested event.
  * @retval CS_Status_t
  */
CS_Status_t CS_subscribe_net_event(CS_UrcEvent_t event, cellular_urc_callback_t urc_callback)
{
  CS_Status_t retval = CELLULAR_OK;
  PRINT_API("CS_subscribe_net_event")

  /* URC registration */
  if (event == CS_URCEVENT_EPS_NETWORK_REG_STAT)
  {
    urc_eps_network_registration_callback = urc_callback;
    cs_ctxt_urc_subscription.eps_network_registration = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_GPRS_NETWORK_REG_STAT)
  {
    urc_gprs_network_registration_callback = urc_callback;
    cs_ctxt_urc_subscription.gprs_network_registration = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_CS_NETWORK_REG_STAT)
  {
    urc_cs_network_registration_callback = urc_callback;
    cs_ctxt_urc_subscription.cs_network_registration = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_EPS_LOCATION_INFO)
  {
    urc_eps_location_info_callback = urc_callback;
    cs_ctxt_urc_subscription.eps_location_info = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_GPRS_LOCATION_INFO)
  {
    urc_gprs_location_info_callback = urc_callback;
    cs_ctxt_urc_subscription.gprs_location_info = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_CS_LOCATION_INFO)
  {
    urc_cs_location_info_callback = urc_callback;
    cs_ctxt_urc_subscription.cs_location_info = CELLULAR_TRUE;
  }
  else if (event == CS_URCEVENT_SIGNAL_QUALITY)
  {
    urc_signal_quality_callback = urc_callback;
    cs_ctxt_urc_subscription.signal_quality = CELLULAR_TRUE;
  }
  else
  {
    PRINT_ERR("<Cellular_Service> invalid event")
    retval = CELLULAR_ERROR;
  }

  if (retval != CELLULAR_ERROR)
  {
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_URC_EVENT,
                             (uint16_t) sizeof(CS_UrcEvent_t),
                             (void *)&event) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SUSBCRIBE_NET_EVENT, &cmd_buf[0], &rsp_buf[0]);
      if (err != ATSTATUS_OK)
      {
        retval = CELLULAR_ERROR;
      }
    }
    else
    {
      retval = CELLULAR_ERROR;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when subscribing event")
  }
  return (retval);
}

/**
  * @brief  Deregister to an event change notification related to Network status.
  * @note   This function should be called for each event the user wants to stop monitoring.
  * @param  event Event that will be deregistered to change notification.
  * @retval CS_Status_t
  */
CS_Status_t CS_unsubscribe_net_event(CS_UrcEvent_t event)
{
  CS_Status_t retval = CELLULAR_OK;
  PRINT_API("CS_unsubscribe_net_event")

  if (event == CS_URCEVENT_EPS_NETWORK_REG_STAT)
  {
    urc_eps_network_registration_callback = NULL;
    cs_ctxt_urc_subscription.eps_network_registration = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_GPRS_NETWORK_REG_STAT)
  {
    urc_gprs_network_registration_callback = NULL;
    cs_ctxt_urc_subscription.gprs_network_registration = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_CS_NETWORK_REG_STAT)
  {
    urc_cs_network_registration_callback = NULL;
    cs_ctxt_urc_subscription.cs_network_registration = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_EPS_LOCATION_INFO)
  {
    urc_eps_location_info_callback = NULL;
    cs_ctxt_urc_subscription.eps_location_info = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_GPRS_LOCATION_INFO)
  {
    urc_gprs_location_info_callback = NULL;
    cs_ctxt_urc_subscription.gprs_location_info = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_CS_LOCATION_INFO)
  {
    urc_cs_location_info_callback = NULL;
    cs_ctxt_urc_subscription.cs_location_info = CELLULAR_FALSE;
  }
  else if (event == CS_URCEVENT_SIGNAL_QUALITY)
  {
    urc_signal_quality_callback = NULL;
    cs_ctxt_urc_subscription.signal_quality = CELLULAR_FALSE;
  }
  else
  {
    PRINT_ERR("<Cellular_Service> invalid event")
    retval = CELLULAR_ERROR;
  }

  if (retval != CELLULAR_ERROR)
  {
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_URC_EVENT,
                             (uint16_t) sizeof(CS_UrcEvent_t),
                             (void *)&event) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_UNSUSBCRIBE_NET_EVENT, &cmd_buf[0], &rsp_buf[0]);
      if (err != ATSTATUS_OK)
      {
        retval = CELLULAR_ERROR;
      }
    }
    else
    {
      retval = CELLULAR_ERROR;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when unsubscribing event")
  }
  return (retval);
}

/**
  * @brief  Request attach to packet domain.
  * @param  none.
  * @retval CS_Status_t
  */
CS_Status_t CS_attach_PS_domain(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_attach_PS_domain")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_ATTACH_PS_DOMAIN,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_ATTACH_PS_DOMAIN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> attach PS domain done")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when attaching PS domain")
  }
  return (retval);
}

/**
  * @brief  Request detach from packet domain.
  * @param  none.
  * @retval CS_Status_t
  */
CS_Status_t CS_detach_PS_domain(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_detach_PS_domain")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_DETACH_PS_DOMAIN,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_DETACH_PS_DOMAIN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> detach PS domain done")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when detaching PS domain")
  }
  return (retval);
}

/**
  * @brief  Request for packet attach status.
  * @param  p_attach Handle to PS attach status.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_attach_status(CS_PSattach_t *p_attach)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_get_attachstatus")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_GET_ATTACHSTATUS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      if (DATAPACK_readStruct(&rsp_buf[0],
                              (uint16_t) CSMT_ATTACHSTATUS,
                              (uint16_t) sizeof(CS_PSattach_t),
                              p_attach) == DATAPACK_OK)
      {
        PRINT_DBG("<Cellular_Service> Attachment status received = %d", *p_attach)
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when getting attachment status")
  }
  return (retval);
}

/**
  * @brief  Read the latest registration state to the Cellular Network.
  * @param  p_reg_status Handle to registration status structure.
  *         This information is valid only if return code is CELLULAR_OK
  * @retval CS_Status_t
  */
CS_Status_t CS_get_net_status(CS_RegistrationStatus_t *p_reg_status)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_get_netstatus")

  /* init returned fields */
  p_reg_status->optional_fields_presence = CS_RSF_NONE;
  p_reg_status->CS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->GPRS_NetworkRegState = CS_NRS_UNKNOWN;
  p_reg_status->EPS_NetworkRegState = CS_NRS_UNKNOWN;

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_GET_NETSTATUS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      if (DATAPACK_readStruct(&rsp_buf[0],
                              (uint16_t) CSMT_REGISTRATIONSTATUS,
                              (uint16_t) sizeof(CS_RegistrationStatus_t),
                              p_reg_status) == DATAPACK_OK)
      {
        PRINT_DBG("<Cellular_Service> Net status received")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when getting net status")
  }
  return (retval);
}

/**
  * @brief  Read the actual signal quality seen by Modem .
  * @param  p_sig_qual Handle to signal quality structure.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_signal_quality(CS_SignalQuality_t *p_sig_qual)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_get_signal_quality")

  CS_SignalQuality_t local_sig_qual = {0};
  (void) memset((void *)&local_sig_qual, 0, sizeof(CS_SignalQuality_t));

  if (DATAPACK_writePtr(&cmd_buf[0],
                        (uint16_t) CSMT_SIGNAL_QUALITY,
                        (void *)&local_sig_qual) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_GET_SIGNAL_QUALITY, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Signal quality informations received")
      /* recopy info to user */
      (void) memcpy((void *)p_sig_qual, (void *)&local_sig_qual, sizeof(CS_SignalQuality_t));
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when getting signal quality")
  }
  return (retval);
}

/**
  * @brief  Activates a PDN (Packet Data Network Gateway) allowing communication with internet.
  * @note   This function triggers the allocation of IP public WAN to the device.
  * @note   Only one PDN can be activated at a time.
  * @param  cid Configuration identifier
  *         This parameter can be one of the following values:
  *         CS_PDN_PREDEF_CONFIG To use default PDN configuration.
  *         CS_PDN_USER_CONFIG_1-5 To use a dedicated PDN configuration.
  *         CS_PDN_CONFIG_DEFAULT To use default PDN config set by CS_set_default_pdn().
  * @retval CS_Status_t
  */
CS_Status_t CS_activate_pdn(CS_PDN_conf_id_t cid)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_activate_pdn for cid=%d", cid)

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_ACTIVATE_PDN,
                           (uint16_t) sizeof(CS_PDN_conf_id_t),
                           (void *)&cid) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_ACTIVATE_PDN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> PDN %d connected", cid)
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when PDN %cid activation", cid)
  }
  return (retval);
}

/**
  * @brief  Deactivates a PDN.
  * @note   This function triggers the allocation of IP public WAN to the device.
  * @note  only one PDN can be activated at a time.
  * @param  cid Configuration identifier
  * @retval CS_Status_t
  */
CS_Status_t CS_deactivate_pdn(CS_PDN_conf_id_t cid)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_deactivate_pdn for cid=%d", cid)

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_DEACTIVATE_PDN,
                           (uint16_t) sizeof(CS_PDN_conf_id_t),
                           (void *)&cid) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DEACTIVATE_PDN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> PDN %d deactivated", cid)
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when PDN %cid deactivation", cid)
  }
  return (retval);
}

/**
  * @brief  Define internet data profile for a configuration identifier
  * @param  cid Configuration identifier
  * @param  apn A string of the access point name (must be non NULL)
  * @param  pdn_conf Structure which contains additional configurations parameters (if non NULL)
  * @retval CS_Status_t
  */
CS_Status_t CS_define_pdn(CS_PDN_conf_id_t cid, const CS_CHAR_t *apn, CS_PDN_configuration_t *pdn_conf)
{
  CS_Status_t retval = CELLULAR_ERROR;
  csint_pdn_infos_t pdn_infos;

  PRINT_API("CS_define_pdn for cid=%d", cid)

  /* check parameters validity */
  if ((cid < CS_PDN_USER_CONFIG_1) || (cid > CS_PDN_USER_CONFIG_5))
  {
    PRINT_ERR("<Cellular_Service> selected configuration id %d can not be set by user", cid)
  }
  else if (apn == NULL)
  {
    PRINT_ERR("<Cellular_Service> apn must be non NULL")
  }
  else
  {
    /* prepare and send PDN infos */
    (void) memset((void *)&pdn_infos, 0, sizeof(csint_pdn_infos_t));
    pdn_infos.conf_id = cid;
    (void) memcpy((void *)&pdn_infos.apn[0],
                  (const CS_CHAR_t *)apn,
                  strlen((const CRC_CHAR_t *)apn));
    (void) memcpy((void *)&pdn_infos.pdn_conf, (void *)pdn_conf, sizeof(CS_PDN_configuration_t));

    if (DATAPACK_writePtr(&cmd_buf[0],
                          (uint16_t) CSMT_DEFINE_PDN,
                          (void *)&pdn_infos) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DEFINE_PDN, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when defining PDN %d", cid)
  }
  return (retval);
}

/**
  * @brief  Select a PDN among of defined configuration identifier(s) as the default.
  * @note   By default, PDN_PREDEF_CONFIG is considered as the default PDN.
  * @param  cid Configuration identifier.
  * @retval CS_Status_t
  */
CS_Status_t CS_set_default_pdn(CS_PDN_conf_id_t cid)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_set_default_pdn (conf_id=%d)", cid)

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_SET_DEFAULT_PDN,
                           (uint16_t) sizeof(CS_PDN_conf_id_t),
                           (void *)&cid) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SET_DEFAULT_PDN, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> PDN %d set as default", cid)
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when setting default PDN %d", cid)
  }
  return (retval);
}

/**
  * @brief  Get the IP address allocated to the device for a given PDN.
  * @param  cid Configuration identifier number.
  * @param  ip_addr_type IP address type and format.
  * @param  p_ip_addr_value Specifies the IP address of the given PDN.
  * @retval CS_Status_t
  */
CS_Status_t CS_get_dev_IP_address(CS_PDN_conf_id_t cid, CS_IPaddrType_t *ip_addr_type, CS_CHAR_t *p_ip_addr_value)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_get_dev_IP_address (for conf_id=%d)", cid)

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_GET_IP_ADDRESS,
                           (uint16_t) sizeof(CS_PDN_conf_id_t),
                           (void *)&cid) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_GET_IP_ADDRESS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      csint_ip_addr_info_t ip_addr_info;
      if (DATAPACK_readStruct(&rsp_buf[0],
                              (uint16_t) CSMT_GET_IP_ADDRESS,
                              (uint16_t) sizeof(csint_ip_addr_info_t),
                              &ip_addr_info) == DATAPACK_OK)
      {
        PRINT_DBG("<Cellular_Service> IP address informations received")
        /* recopy info to user */
        *ip_addr_type = ip_addr_info.ip_addr_type;
        (void) memcpy((void *)p_ip_addr_value,
                      (void *)&ip_addr_info.ip_addr_value,
                      strlen((CRC_CHAR_t *)ip_addr_info.ip_addr_value));
        PRINT_DBG("<Cellular_Service> IP address = %s (type = %d)",
                  (CS_CHAR_t *)ip_addr_info.ip_addr_value, ip_addr_info.ip_addr_type)
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when getting IP address informations")
  }
  return (retval);
}

/**
  * @brief  Register to specified modem events.
  * @note   This function should be called once with all requested events.
  * @param  events_mask Events that will be registered (bitmask)
  * @param  urc_callback Handle on user callback that will be used to notify a
  *         change on requested event.
  * @retval CS_Status_t
  */
CS_Status_t CS_subscribe_modem_event(CS_ModemEvent_t events_mask, cellular_modem_event_callback_t modem_evt_cb)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_subscribe_modem_event")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_MODEM_EVENT,
                           (uint16_t) sizeof(CS_ModemEvent_t),
                           (void *)&events_mask) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SUSBCRIBE_MODEM_EVENT, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      urc_modem_event_callback = modem_evt_cb;
      PRINT_DBG("<Cellular_Service> modem events suscribed")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when subscribing modem event")
  }
  return (retval);
}

/**
  * @brief  Register to event notifications related to internet connection.
  * @note   This function is used to register to an event related to a PDN
  *         Only explicit config id (CS_PDN_USER_CONFIG_1 to CS_PDN_USER_CONFIG_5) are
  *         suppported and CS_PDN_PREDEF_CONFIG
  * @param  cid Configuration identifier number.
  * @param  pdn_event_callback client callback to call when an event occured.
  * @retval CS_Status_t
  */
CS_Status_t  CS_register_pdn_event(CS_PDN_conf_id_t cid, cellular_pdn_event_callback_t pdn_event_callback)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_register_pdn_event")

  /* check parameters validity */
  if (cid > CS_PDN_USER_CONFIG_5)
  {
    PRINT_ERR("<Cellular_Service> only explicit PDN user config is supported (cid=%d)", cid)
  }
  else
  {
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_NONE,
                             (uint16_t) 0U,
                             NULL) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_REGISTER_PDN_EVENT, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> PDN events registered successfully")
        /* register callback */
        urc_packet_domain_event_callback[cid] = pdn_event_callback;
        cs_ctxt_urc_subscription.packet_domain_event = CELLULAR_TRUE;
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service>error when registering PDN events")
  }
  return (retval);
}

/**
  * @brief  Deregister the internet event notifications.
  * @param  cid Configuration identifier number.
  * @retval CS_Status_t
  */
CS_Status_t CS_deregister_pdn_event(CS_PDN_conf_id_t cid)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_deregister_pdn_event")

  /* check parameters validity */
  if (cid > CS_PDN_USER_CONFIG_5)
  {
    PRINT_ERR("<Cellular_Service> only explicit PDN user config is supported (cid=%d)", cid)
  }
  else
  {
    /* register callback */
    urc_packet_domain_event_callback[cid] = NULL;
    cs_ctxt_urc_subscription.packet_domain_event = CELLULAR_FALSE;
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_NONE,
                             (uint16_t) 0U,
                             NULL) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DEREGISTER_PDN_EVENT, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> PDN events deregistered successfully")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when deregistering PDN events")
  }
  return (retval);
}

/**
  * @brief  Allocate a socket among of the free sockets (maximum 6 sockets)
  * @param  addr_type Specifies a communication domain.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4 (default)
  *         IPAT_IPV6  for IPV6
  * @param  protocol Specified the transport protocol to be used with the socket.
  *         TCP_PROTOCOL
  *         UDP_PROTOCOL
  * @param  cid Specifies the identity of PDN configuration to be used.
  *         CS_PDN_PREDEF_CONFIG To use default PDN configuration.
  *         CS_PDN_USER_CONFIG_1-5 To use a dedicated PDN configuration.
  * @retval Socket handle which references allocated socket
  */
socket_handle_t CDS_socket_create(CS_IPaddrType_t addr_type,
                                  CS_TransportProtocol_t protocol,
                                  CS_PDN_conf_id_t cid)
{
  PRINT_API("CDS_socket_create")

  socket_handle_t sockhandle = csint_socket_allocateHandle();
  if (sockhandle == CS_INVALID_SOCKET_HANDLE)
  {
    PRINT_ERR("no free socket handle")
  }
  else if (csint_socket_create(sockhandle, addr_type, protocol, /* default local_port = 0 */ 0U, cid) != CELLULAR_OK)
  {
    /* socket creation error, deallocate handle */
    PRINT_ERR("socket creation failed")
    csint_socket_deallocateHandle(sockhandle);
    sockhandle = CS_INVALID_SOCKET_HANDLE;
  }
  else
  {
    /* socket created */
    PRINT_INFO("allocated socket handle=%ld (local)", sockhandle)
  }

  return (sockhandle);
}

/**
  * @brief  Bind the socket to a local port.
  * @note   If this function is not called, default local port value = 0 will be used.
  * @param  sockHandle Handle of the socket
  * @param  local_port Local port number.
  *         This parameter must be a value between 0 and 65535.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_bind(socket_handle_t sockHandle,
                            uint16_t local_port)
{
  CS_Status_t res;

  PRINT_API("CDS_socket_bind")

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CREATED)
  {
    PRINT_ERR("<Cellular_Service> socket bind allowed only after create/before connect %ld ", sockHandle)
    res = CELLULAR_ERROR;
  }
  else if (csint_socket_bind(sockHandle, local_port) != CELLULAR_OK)
  {
    PRINT_ERR("Socket Bind error")
    res = CELLULAR_ERROR;
  }
  else
  {
    res = CELLULAR_OK;
  }

  return res;
}

/**
  * @brief  Set the callbacks to use when datas are received or sent.
  * @note   This function has to be called before to use a socket.
  * @param  sockHandle Handle of the socket
  * @param  data_ready_cb Pointer to the callback function to call when datas are received
  * @param  data_sent_cb Pointer to the callback function to call when datas has been sent
  *         This parameter is only used for asynchronous behavior (NOT IMPLEMENTED)
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_set_callbacks(socket_handle_t sockHandle,
                                     cellular_socket_data_ready_callback_t data_ready_cb,
                                     cellular_socket_data_sent_callback_t data_sent_cb,
                                     cellular_socket_closed_callback_t remote_close_cb)
{
  CS_Status_t retval;
  PRINT_API("CDS_socket_set_callbacks")

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockHandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    PRINT_ERR("<Cellular_Service> invalid socket handle %ld (set cb)", sockHandle)
    retval = CELLULAR_ERROR;
  }
  /*PRINT_DBG("DBG: socket data ready callback=%p", cs_ctxt_sockets_info[sockHandle].socket_data_ready_callback)*/
  else if (data_ready_cb == NULL)
  {
    PRINT_ERR("data_ready_cb is mandatory")
    retval = CELLULAR_ERROR;
  }
  /*PRINT_DBG("DBG: socket remote closed callback=%p", cs_ctxt_sockets_info[sockHandle].socket_remote_close_callback)*/
  else if (remote_close_cb == NULL)
  {
    PRINT_ERR("remote_close_cb is mandatory")
    retval = CELLULAR_ERROR;
  }
  else
  {
    cs_ctxt_sockets_info[sockHandle].socket_data_ready_callback = data_ready_cb;
    cs_ctxt_sockets_info[sockHandle].socket_remote_close_callback = remote_close_cb;
    retval = CELLULAR_OK;
  }

  if (data_sent_cb != NULL)
  {
    /* NOT SUPPORTED - but does not return an error */
    PRINT_ERR("DATA sent callback not supported (only synch mode)")
  }

  return (retval);
}

/**
  * @brief  Define configurable options for a created socket.
  * @note   This function is called to configure one parameter at a time.
  *         If a parameter is not configured with this function, a default value will be applied.
  * @param  sockHandle Handle of the socket
  * @param  opt_level The level of TCP/IP stack component to be configured.
  *         This parameter can be one of the following values:
  *         SOL_IP
  *         SOL_TRANSPORT
  * @param  opt_name
  *         SON_IP_MAX_PACKET_SIZE Maximum packet size for data transfer (0 to 1500 bytes).
  *         SON_TRP_MAX_TIMEOUT Inactivity timeout (0 to 65535, in second, 0 means infinite).
  *         SON_TRP_CONNECT_SETUP_TIMEOUT Maximum timeout to setup connection with remote server
  *              (10 to 1200, in 100 of ms, 0 means infinite).
  *         SON_TRP_TRANSFER_TIMEOUT Maximum timeout to transfer data to remote server
  *              (1 to 255, in ms, 0 means infinite).
  *         SON_TRP_CONNECT_MODE To indicate if connection with modem will be dedicated (CM_ONLINE_MODE)
  *             or stay in command mode (CM_COMMAND_MODE) or stay in online mode until a
  *             suspend timeout expires (CM_ONLINE_AUTOMATIC_SUSPEND).
  *             Only CM_COMMAND_MODE is supported for the moment.
  *         SON_TRP_SUSPEND_TIMEOUT To define inactivty timeout to suspend online mode
  *             (0 to 2000, in ms , 0 means infinite).
  *         SON_TRP_RX_TIMEOUT Maximum timeout to receive data from remoteserver
  *             (0 to 255, in ms, 0 means infinite).
  * @param  p_opt_val Pointer to parameter to update (max size = uint16_t)
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_set_option(socket_handle_t sockHandle,
                                  CS_SocketOptionLevel_t opt_level,
                                  CS_SocketOptionName_t opt_name,
                                  void *p_opt_val)
{
  CS_Status_t retval;
  PRINT_API("CDS_socket_set_option")

  retval = csint_socket_configure(sockHandle, opt_level, opt_name, p_opt_val);
  return (retval);
}

/**
  * @brief  Retrieve configurable options for a created socket.
  * @note   This function is called for one parameter at a time.
  * @note   Function not implemented yet
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_get_option(void)
{
  return (CELLULAR_NOT_IMPLEMENTED);
}

/**
  * @brief  Connect to a remote server (for socket client mode).
  * @note   This function is blocking until the connection is setup or when the timeout to wait
  *         for socket connection expires.
  * @param  sockHandle Handle of the socket.
  * @param  ip_addr_type Specifies the type of IP address of the remote server.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4 (default)
  *         IPAT_IPV6  for IPV6
  * @param  p_ip_addr_value Specifies the IP address of the remote server.
  * @param  remote_port Specifies the port of the remote server.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_connect(socket_handle_t sockHandle,
                               CS_IPaddrType_t ip_addr_type,
                               CS_CHAR_t *p_ip_addr_value,
                               uint16_t remote_port)
{
  at_status_t err;
  CS_Status_t retval;

  PRINT_API("CDS_socket_connect")

  retval = csint_socket_configure_remote(sockHandle, ip_addr_type, p_ip_addr_value, remote_port);
  if (retval == CELLULAR_OK)
  {
    /* Send socket informations to ATcustom
    * no need to test sockHandle validity, it has been tested in csint_socket_configure_remote()
    */
    csint_socket_infos_t *socket_infos = &cs_ctxt_sockets_info[sockHandle];
    if (DATAPACK_writePtr(&cmd_buf[0],
                          (uint16_t) CSMT_SOCKET_INFO,
                          (void *)socket_infos) == DATAPACK_OK)
    {
      if (socket_infos->trp_connect_mode == CS_CM_COMMAND_MODE)
      {
        err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DIAL_COMMAND, &cmd_buf[0], &rsp_buf[0]);
      }
      else
      {
        /* NOT SUPPORTED YET */
        err = ATSTATUS_ERROR;
        /* err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DIAL_ONLINE, &cmd_buf[0], &rsp_buf[0]);*/
      }

      if (err == ATSTATUS_OK)
      {
        /* update socket state */
        cs_ctxt_sockets_info[sockHandle].state = SOCKETSTATE_CONNECTED;
        retval = CELLULAR_OK;
      }
      else
      {
        PRINT_ERR("<Cellular_Service> error when socket connection")
        retval = CELLULAR_ERROR;
      }
    }
    else
    {
      retval = CELLULAR_ERROR;
    }
  }

  return (retval);
}

/**
  * @brief  Listen to clients (for socket server mode).
  * @note   Function not implemeted yet
  * @param  sockHandle Handle of the socket
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_listen(socket_handle_t sockHandle)
{
  UNUSED(sockHandle);

  /* for socket server mode */
  return (CELLULAR_NOT_IMPLEMENTED);
}

/**
  * @brief  Send data over a socket to a remote server.
  * @note   This function is blocking until the data is transfered or when the
  *         timeout to wait for transmission expires.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to transfer.
  * @param  length Length of the data buffer.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_send(socket_handle_t sockHandle,
                            const CS_CHAR_t *p_buf,
                            uint32_t length)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CDS_socket_send (buf@=%p - buflength = %ld)", p_buf, length)

  /* check that size does not exceed maximum buffers size */
  if (length > DEFAULT_IP_MAX_PACKET_SIZE)
  {
    PRINT_ERR("<Cellular_Service> buffer size %ld exceed maximum value %d",
              length,
              DEFAULT_IP_MAX_PACKET_SIZE)
  }
  /* check that socket has been allocated */
  else if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
  {
    PRINT_ERR("<Cellular_Service> socket not connected (state=%d) for handle %ld (send)",
              cs_ctxt_sockets_info[sockHandle].state,
              sockHandle)
  }
  else
  {
    csint_socket_data_buffer_t send_data_struct;
    (void) memset((void *)&send_data_struct, 0, sizeof(csint_socket_data_buffer_t));
    send_data_struct.socket_handle = sockHandle;
    send_data_struct.p_buffer_addr_send = p_buf;
    send_data_struct.p_buffer_addr_rcv = NULL;
    send_data_struct.buffer_size = length;
    send_data_struct.max_buffer_size = length;
    /* following parameters are not used (only in sendto) */
    send_data_struct.ip_addr_type = CS_IPAT_INVALID;
    /* send_data_struct.ip_addr_value already reset */
    /* send_data_struct.remote_port already reset */
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_SOCKET_DATA_BUFFER,
                             (uint16_t) sizeof(csint_socket_data_buffer_t),
                             (void *)&send_data_struct) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SEND_DATA, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> socket data sent")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when sending data to socket")
  }
  return (retval);
}

/**
  * @brief  Send data over a socket to a remote server.
  * @note   This function is blocking until the data is transfered or when the
  *         timeout to wait for transmission expires.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to transfer.
  * @param  length Length of the data buffer.
  * @param  ip_addr_type Specifies the type of the remote IP address.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4
  *         IPAT_IPV6 for IPV6
  * @param  p_ip_addr_value Specifies the remote IP address.
  * @param  remote_port Specifies the remote port.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_sendto(socket_handle_t sockHandle,
                              const CS_CHAR_t *p_buf,
                              uint32_t length,
                              CS_IPaddrType_t ip_addr_type,
                              CS_CHAR_t *p_ip_addr_value,
                              uint16_t remote_port)
{
  CS_Status_t retval = CELLULAR_ERROR;
  at_status_t err;
  size_t ip_addr_length;

  PRINT_API("CDS_socket_send (buf@=%p - buflength = %ld)", p_buf, length)

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
  {
    PRINT_ERR("<Cellular_Service> socket not connected (state=%d) for handle %ld (send)",
              cs_ctxt_sockets_info[sockHandle].state,
              sockHandle)
  }
  /* check that size does not exceed maximum buffers size */
  else if (length > DEFAULT_IP_MAX_PACKET_SIZE)
  {
    PRINT_ERR("<Cellular_Service> buffer size %ld exceed maximum value %d",
              length,
              DEFAULT_IP_MAX_PACKET_SIZE)
  }
  /* check p_ip_addr_value ptr */
  else if (p_ip_addr_value == NULL)
  {
    PRINT_ERR("<Cellular_Service> NULL ptr")
  }
  /* check p_ip_addr_value size */
  else
  {
    ip_addr_length = strlen((const CRC_CHAR_t *)p_ip_addr_value);
    if (ip_addr_length > MAX_IP_ADDR_SIZE)
    {
      PRINT_ERR("<Cellular_Service> IP address too long")
    }
    else
    {
      csint_socket_data_buffer_t send_data_struct;
      (void) memset((void *)&send_data_struct, 0, sizeof(csint_socket_data_buffer_t));
      send_data_struct.socket_handle = sockHandle;
      send_data_struct.p_buffer_addr_send = p_buf;
      send_data_struct.p_buffer_addr_rcv = NULL;
      send_data_struct.buffer_size = length;
      send_data_struct.max_buffer_size = length;
      /* sendto parameters specific */
      send_data_struct.ip_addr_type = ip_addr_type;
      (void) memcpy((void *)send_data_struct.ip_addr_value,
                    (const CS_CHAR_t *)p_ip_addr_value,
                    ip_addr_length);
      send_data_struct.remote_port = remote_port;
      if (DATAPACK_writeStruct(&cmd_buf[0],
                               (uint16_t) CSMT_SOCKET_DATA_BUFFER,
                               (uint16_t) sizeof(csint_socket_data_buffer_t),
                               (void *)&send_data_struct) == DATAPACK_OK)
      {
        err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SEND_DATA, &cmd_buf[0], &rsp_buf[0]);
        if (err == ATSTATUS_OK)
        {
          PRINT_DBG("<Cellular_Service> socket data sent (sendto)")
          retval = CELLULAR_OK;
        }
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when sending data to socket (sendto)")
  }
  return (retval);
}


/**
  * @brief  Receive data from the connected remote server.
  * @note   This function is blocking until expected data length is received or a receive timeout has expired.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to received data.
  * @param  max_buf_length Maximum size of receive data buffer.
  * @retval Size of received data (in bytes).
  */
int32_t CDS_socket_receive(socket_handle_t sockHandle,
                           CS_CHAR_t *p_buf,
                           uint32_t max_buf_length)
{
  int32_t returned_data_size;
  CS_Status_t status = CELLULAR_ERROR;
  uint32_t bytes_received = 0U;

  PRINT_API("CDS_socket_receive")

  /* check that size does not exceed maximum buffers size */
  if (max_buf_length > DEFAULT_IP_MAX_PACKET_SIZE)
  {
    PRINT_ERR("<Cellular_Service> buffer size %ld exceed maximum value %d",
              max_buf_length,
              DEFAULT_IP_MAX_PACKET_SIZE)
  }
  /* check that socket has been allocated */
  else if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
  {
    PRINT_ERR("<Cellular_Service> socket not connected (state=%d) for handle %ld (rcv)",
              cs_ctxt_sockets_info[sockHandle].state,
              sockHandle)
  }
  else
  {
    csint_socket_data_buffer_t receive_data_struct = {0};
    (void) memset((void *)&receive_data_struct, 0, sizeof(csint_socket_data_buffer_t));
    receive_data_struct.socket_handle = sockHandle;
    receive_data_struct.p_buffer_addr_send = NULL;
    receive_data_struct.p_buffer_addr_rcv = p_buf;
    receive_data_struct.buffer_size = 0U;
    receive_data_struct.max_buffer_size = max_buf_length;
    /* following parameters are not used (only in receivefrom) */
    receive_data_struct.ip_addr_type = CS_IPAT_INVALID;
    /* receive_data_struct.ip_addr_value already reset */
    /* receive_data_struct.remote_port already reset */
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_SOCKET_DATA_BUFFER,
                             (uint16_t) sizeof(csint_socket_data_buffer_t),
                             (void *)&receive_data_struct) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_RECEIVE_DATA, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        if (DATAPACK_readStruct(&rsp_buf[0],
                                (uint16_t) CSMT_SOCKET_RXDATA,
                                (uint16_t) sizeof(uint32_t),
                                &bytes_received) == DATAPACK_OK)
        {
          status = CELLULAR_OK;
        }
      }
    }
  }

  if (status == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when receiving data from socket")
    returned_data_size = -1;
  }
  else
  {
    PRINT_INFO("Size of data received on the socket= %ld bytes", bytes_received)
    returned_data_size = ((int32_t)bytes_received);
  }

  return (returned_data_size);
}

/**
  * @brief  Receive data from the connected remote server.
  * @note   This function is blocking until expected data length is received or a receive timeout has expired.
  * @param  sockHandle Handle of the socket
  * @param  p_buf Pointer to the data buffer to received data.
  * @param  max_buf_length Maximum size of receive data buffer.
  * @param  ip_addr_type Specifies the type of the remote IP address.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4
  *         IPAT_IPV6 for IPV6
  * @param  p_ip_addr_value Specifies the remote IP address.
  * @param  remote_port Specifies the remote port.
  * @retval Size of received data (in bytes).
  */
int32_t CDS_socket_receivefrom(socket_handle_t sockHandle,
                               CS_CHAR_t *p_buf,
                               uint32_t max_buf_length,
                               CS_IPaddrType_t *p_ip_addr_type,
                               CS_CHAR_t *p_ip_addr_value,
                               uint16_t *p_remote_port)
{
  int32_t returned_data_size;
  CS_Status_t status = CELLULAR_ERROR;
  uint32_t bytes_received = 0U;

  PRINT_API("CDS_socket_receive")

  /* check that size does not exceed maximum buffers size */
  if (max_buf_length > DEFAULT_IP_MAX_PACKET_SIZE)
  {
    PRINT_ERR("<Cellular_Service> buffer size %ld exceed maximum value %d",
              max_buf_length,
              DEFAULT_IP_MAX_PACKET_SIZE)
  }
  /* check that socket has been allocated */
  else if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
  {
    PRINT_ERR("<Cellular_Service> socket not connected (state=%d) for handle %ld (rcv)",
              cs_ctxt_sockets_info[sockHandle].state,
              sockHandle)
  }
  else
  {
    csint_socket_data_buffer_t receive_data_struct = {0};
    (void) memset((void *)&receive_data_struct, 0, sizeof(csint_socket_data_buffer_t));
    receive_data_struct.socket_handle = sockHandle;
    receive_data_struct.p_buffer_addr_send = NULL;
    receive_data_struct.p_buffer_addr_rcv = p_buf;
    receive_data_struct.buffer_size = 0U;
    receive_data_struct.max_buffer_size = max_buf_length;
    /* following parameters are returned parameters */
    receive_data_struct.ip_addr_type = CS_IPAT_INVALID;
    /* receive_data_struct.ip_addr_value already reset */
    /* receive_data_struct.remote_port already reset */
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_SOCKET_DATA_BUFFER,
                             (uint16_t) sizeof(csint_socket_data_buffer_t),
                             (void *)&receive_data_struct) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_RECEIVE_DATA_FROM, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        csint_socket_rxdata_from_t  rx_data_from;
        if (DATAPACK_readStruct(&rsp_buf[0],
                                (uint16_t) CSMT_SOCKET_RXDATA_FROM,
                                (uint16_t) sizeof(csint_socket_rxdata_from_t),
                                &rx_data_from) == DATAPACK_OK)
        {
          bytes_received = rx_data_from.bytes_received;
          /* recopy info to user */
          *p_ip_addr_type = rx_data_from.ip_addr_type;
          (void) memcpy((void *)p_ip_addr_value,
                        (void *)&rx_data_from.ip_addr_value,
                        strlen((CRC_CHAR_t *)rx_data_from.ip_addr_value));
          *p_remote_port = rx_data_from.remote_port;
          status = CELLULAR_OK;
        }
      }
    }

  }

  if (status == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when receiving data from socket")
    returned_data_size = -1;
  }
  else
  {
    PRINT_INFO("Size of data received on the socket= %ld bytes", bytes_received)
    returned_data_size = ((int32_t)bytes_received);
  }

  return (returned_data_size);
}

/**
  * @brief  Free a socket handle.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @param  sockHandle Handle of the socket
  * @param  force Force to free the socket.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_close(socket_handle_t sockHandle, uint8_t force)
{
  UNUSED(force);
  CS_Status_t retval = CELLULAR_ERROR;

  PRINT_API("CDS_socket_close")

  if (cs_ctxt_sockets_info[sockHandle].state == SOCKETSTATE_CONNECTED)
  {
    /* Send socket informations to ATcustom */
    csint_socket_infos_t *socket_infos = &cs_ctxt_sockets_info[sockHandle];
    if (DATAPACK_writePtr(&cmd_buf[0],
                          (uint16_t) CSMT_SOCKET_INFO,
                          (void *)socket_infos) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SOCKET_CLOSE, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        /* deallocate socket handle and reinit socket parameters */
        csint_socket_deallocateHandle(sockHandle);
        retval = CELLULAR_OK;
      }
    }
  }
  else if (cs_ctxt_sockets_info[sockHandle].state == SOCKETSTATE_CREATED)
  {
    PRINT_INFO("<Cellular_Service> socket was not connected ")
    /* deallocate socket handle and reinit socket parameters */
    csint_socket_deallocateHandle(sockHandle);
    retval = CELLULAR_OK;
  }
  else if (cs_ctxt_sockets_info[sockHandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    PRINT_ERR("<Cellular_Service> invalid socket handle %ld (close)", sockHandle)
    retval = CELLULAR_ERROR;
  }
  else if (cs_ctxt_sockets_info[sockHandle].state == SOCKETSTATE_ALLOC_BUT_INVALID)
  {
    PRINT_INFO("<Cellular_Service> invalid socket state (after modem reboot) ")
    /* deallocate socket handle and reinit socket parameters */
    csint_socket_deallocateHandle(sockHandle);
    retval = CELLULAR_OK;
  }
  else
  {
    PRINT_ERR("<Cellular_Service> invalid socket state %d (close)", cs_ctxt_sockets_info[sockHandle].state)
    retval = CELLULAR_ERROR;
  }


  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when closing socket")
  }
  return (retval);
}

/**
  * @brief  Get connection status for a given socket.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @param  sockHandle Handle of the socket
  * @param  p_infos Pointer of infos structure.
  * @retval CS_Status_t
  */
CS_Status_t CDS_socket_cnx_status(socket_handle_t sockHandle,
                                  CS_SocketCnxInfos_t *p_infos)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CDS_socket_cnx_status")

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockHandle].state != SOCKETSTATE_CONNECTED)
  {
    PRINT_ERR("<Cellular_Service> socket not connected (state=%d) for handle %ld (status)",
              cs_ctxt_sockets_info[sockHandle].state,
              sockHandle)
  }
  else
  {
    /* Send socket informations to ATcustom */
    csint_socket_cnx_infos_t socket_cnx_infos;
    socket_cnx_infos.socket_handle = sockHandle;
    socket_cnx_infos.infos = p_infos;
    (void) memset((void *)p_infos, 0, sizeof(CS_SocketCnxInfos_t));
    if (DATAPACK_writePtr(&cmd_buf[0],
                          (uint16_t) CSMT_SOCKET_CNX_STATUS,
                          (void *)&socket_cnx_infos) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SOCKET_CNX_STATUS, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> socket cnx status received")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when requesting socket cnx status")
  }
  return (retval);
}

/**
  * @brief  Request to suspend DATA mode.
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_suspend_data(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_suspend_data")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DATA_SUSPEND, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> DATA suspended")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when suspending DATA")
  }
  return (retval);
}

/**
  * @brief  Request to resume DATA mode.
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_resume_data(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_resume_data")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DATA_RESUME, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> DATA resumed")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when resuming DATA")
  }
  return (retval);
}

/**
  * @brief  Request to reset the device.
  * @param  rst_type Type of reset requested (SW, HW, ...)
  * @retval CS_Status_t
  */
CS_Status_t CS_reset(CS_Reset_t rst_type)
{
  CS_Status_t retval = CELLULAR_OK;
  PRINT_API("CS_reset")

  /* reset Cellular Service context */
  CELLULAR_reset_context();

  /* update sockets states */
  csint_modem_reset_update_socket_state();

  /* reset AT context */
  if (AT_reset_context(_Adapter_Handle) != ATSTATUS_OK)
  {
    PRINT_ERR("<Cellular_Service> Reset context error")
    retval = CELLULAR_ERROR;
  }

  switch (rst_type)
  {
    case CS_RESET_HW:
      /* perform hardware reset */
      if (perform_HW_reset() != CELLULAR_OK)
      {
        retval = CELLULAR_ERROR;
      }
      break;

    case CS_RESET_SW:
      /* perform software reset */
      if (perform_SW_reset() != CELLULAR_OK)
      {
        retval = CELLULAR_ERROR;
      }
      break;

    case CS_RESET_AUTO:
      /* perform software reset first */
      if (perform_SW_reset() != CELLULAR_OK)
      {
        /* if software reset failed, perform hardware reset */
        if (perform_HW_reset() != CELLULAR_OK)
        {
          retval = CELLULAR_ERROR;
        }
      }
      break;

    case CS_RESET_FACTORY_RESET:
      /* perform factory reset  */
      if (perform_Factory_reset() != CELLULAR_OK)
      {
        retval = CELLULAR_ERROR;
      }
      break;

    default:
      PRINT_ERR("Invalid reset type")
      retval = CELLULAR_ERROR;
      break;
  }

  return (retval);
}

/**
  * @brief  DNS request
  * @note   Get IP address of the specifed hostname
  * @param  cid Configuration identifier number.
  * @param  *dns_req Handle to DNS request structure.
  * @param  *dns_resp Handle to DNS response structure.
  * @retval CS_Status_t
  */
CS_Status_t CS_dns_request(CS_PDN_conf_id_t cid, CS_DnsReq_t *dns_req, CS_DnsResp_t *dns_resp)
{
  /* static strucure. Use google as default primary DNS service */
  static const CS_DnsConf_t cs_default_primary_dns_addr = { .primary_dns_addr = "8.8.8.8" };

  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_dns_request")

  /* build internal structure to send */
  csint_dns_request_t loc_dns_req;
  (void) memset((void *)&loc_dns_req, 0, sizeof(csint_dns_request_t));
  /* set CID */
  loc_dns_req.conf_id = cid;
  /* recopy dns_req */
  (void) memcpy((void *)&loc_dns_req.dns_req, dns_req, sizeof(CS_DnsReq_t));
  /* set DNS primary address to use - for the moment, it is hard-coded */
  (void) memcpy((void *)&loc_dns_req.dns_conf,
                (const CS_DnsConf_t *)&cs_default_primary_dns_addr,
                sizeof(CS_DnsConf_t));

  if (DATAPACK_writePtr(&cmd_buf[0],
                        (uint16_t) CSMT_DNS_REQ,
                        (void *)&loc_dns_req) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DNS_REQ, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      if (DATAPACK_readStruct(&rsp_buf[0],
                              (uint16_t) CSMT_DNS_REQ,
                              (uint16_t) sizeof(dns_resp->host_addr),
                              dns_resp->host_addr) == DATAPACK_OK)
      {
        PRINT_DBG("<Cellular_Service> DNS configuration done")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during DNS request")
  }
  return (retval);
}


/**
  * @brief  Ping an IP address on the network
  * @note   Usually, the command AT is sent and OK is expected as response
  * @param  address or full paramaters set
  * @param  ping_callback Handle on user callback that will be used to analyze
  *                              the ping answer from the network address.
  * @retval CS_Status_t
  */
CS_Status_t CDS_ping(CS_PDN_conf_id_t cid, CS_Ping_params_t *ping_params,
                     cellular_ping_response_callback_t cs_ping_rsp_cb)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_ping_address")
  PRINT_API("CS_ping_address")

  /* build internal structure to send */
  csint_ping_params_t loc_ping_params;
  (void) memset((void *)&loc_ping_params, 0, sizeof(csint_ping_params_t));
  loc_ping_params.conf_id = cid;
  (void) memcpy((void *)&loc_ping_params.ping_params, ping_params, sizeof(CS_Ping_params_t));

  /* save the callback */
  urc_ping_rsp_callback = cs_ping_rsp_cb;

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_PING_ADDRESS,
                           (uint16_t) sizeof(csint_ping_params_t),
                           (void *)&loc_ping_params) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_PING_IP_ADDRESS, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Ping address OK")
      /* Check if ping response received from lower layer
       * Two cases are possible:
       *  1) ping reports are received as URC (will be received later)
       *  2) ping reports received inside ping transaction: final report should be received now
       */
      CS_Ping_response_t ping_rsp;
      (void) memset((void *)&ping_rsp, 0, sizeof(CS_Ping_response_t));
      if (DATAPACK_readStruct(rsp_buf,
                              (uint16_t) CSMT_URC_PING_RSP,
                              (uint16_t) sizeof(CS_Ping_response_t),
                              (void *)&ping_rsp) == DATAPACK_OK)
      {
        if (ping_rsp.index != PING_INVALID_INDEX)
        {
          PRINT_INFO("<Cellular_Service> Ping transaction finished")
          if (urc_ping_rsp_callback != NULL)
          {
            (* urc_ping_rsp_callback)(ping_rsp);
          }
        }
        else
        {
          PRINT_INFO("<Cellular_Service> Waiting for ping reports")
        }
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during ping request")
  }
  return (retval);
}

/**
  * @brief  Send a string will which be sended as it is to the modem (termination char will be added automatically)
  * @note   The termination char will be automatically added by the lower layer
  * @param  direct_cmd_tx The structure describing the command to send to the modem
  * @param  direct_cmd_callback Callback to send back to user the content of response received from the modem
  *                             paramater NOT IMPLEMENTED YET
  * @retval CS_Status_t
  */
CS_Status_t CS_direct_cmd(CS_direct_cmd_tx_t *direct_cmd_tx, cellular_direct_cmd_callback_t direct_cmd_callback)
{
  UNUSED(direct_cmd_callback); /* direct_cmd_callback not used for the moment */

  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("CS_direct_cmd")

  if (direct_cmd_tx->cmd_size <= MAX_DIREXT_CMD_SIZE)
  {
    if (DATAPACK_writePtr(&cmd_buf[0],
                          (uint16_t) CSMT_DIRECT_CMD,
                          (void *)direct_cmd_tx) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_DIRECT_CMD, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> Direct command infos received")
        retval = CELLULAR_OK;
      }
      else
      {
        retval = CELLULAR_analyze_error_report(&rsp_buf[0]);
      }
    }
  }
  else
  {
    PRINT_INFO("<Cellular_Service> Direct command command size to big (limit=%d)", MAX_DIREXT_CMD_SIZE)
    retval = CELLULAR_ERROR;
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when sending direct cmd")
  }
  return (retval);
}

/* Low Power API ----------------------------------------------------------------------------------------------- */

/**
  * @brief  Initialise power configuration. Called systematically first.
  * @note
  * @param  power_config Pointer to the structure describing the power parameters
  * @retval CS_Status_t
  */
CS_Status_t CS_InitPowerConfig(CS_init_power_config_t *p_power_config)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_InitPowerConfig")

  static CS_init_power_config_t cs_ctxt_init_power_config;
  (void) memset((void *)&cs_ctxt_init_power_config, 0, sizeof(CS_init_power_config_t));
  (void) memcpy((void *)&cs_ctxt_init_power_config, (void *)p_power_config, sizeof(CS_init_power_config_t));

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_INIT_POWER_CONFIG,
                           (uint16_t) sizeof(CS_init_power_config_t),
                           (void *)&cs_ctxt_init_power_config) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_INIT_POWER_CONFIG, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Power configuration set")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when setting power configuration")
  }

  return (retval);
}

/**
  * @brief  Send power configuration (PSM & DRX) to apply to the modem
  * @note
  * @param  power_config Pointer to the structure describing the power parameters
  * @retval CS_Status_t
  */
CS_Status_t CS_SetPowerConfig(CS_set_power_config_t *p_power_config)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_SetPowerConfig")

  static CS_set_power_config_t cs_ctxt_set_power_config;
  (void) memset((void *)&cs_ctxt_set_power_config, 0, sizeof(CS_set_power_config_t));
  (void) memcpy((void *)&cs_ctxt_set_power_config, (void *)p_power_config, sizeof(CS_set_power_config_t));

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_SET_POWER_CONFIG,
                           (uint16_t) sizeof(CS_set_power_config_t),
                           (void *)&cs_ctxt_set_power_config) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SET_POWER_CONFIG, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Power configuration set")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error when setting power configuration")
  }

  return (retval);
}

/**
  * @brief  Request sleep procedure
  * @note
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_SleepRequest(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_SleepRequest")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SLEEP_REQUEST, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Sleep Request done")
      retval = CELLULAR_OK;
    }
  }

  return (retval);
}

/**
  * @brief  Complete sleep procedure
  * @note
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_SleepComplete(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_SleepComplete")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SLEEP_COMPLETE, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Sleep Complete done")
      retval = CELLULAR_OK;
    }
  }

  return (retval);
}

/**
  * @brief  Cancel sleep procedure
  * @note   if something goes wrong: for example, the URC indicating that modem entered
  *         in Low Power has not been received.
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_SleepCancel(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_SleepCancel")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_SLEEP_CANCEL, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Sleep Cancel done")
      retval = CELLULAR_OK;
    }
  }

  return (retval);
}

/**
  * @brief  Request Power WakeUp
  * @note
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t CS_PowerWakeup(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_INFO("CS_PowerWakeup")

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_NONE,
                           (uint16_t) 0U,
                           NULL) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_WAKEUP, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Power WakeUp done")
      retval = CELLULAR_OK;
    }
  }

  return (retval);
}

/* Private function Definition -----------------------------------------------*/
static CS_Status_t perform_HW_reset(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("perform_HW_reset")

  CS_Reset_t rst_type = CS_RESET_HW;

  if (SysCtrl_reset_device(DEVTYPE_MODEM_CELLULAR) == SCSTATUS_OK)
  {
    if (DATAPACK_writeStruct(&cmd_buf[0],
                             (uint16_t) CSMT_RESET,
                             (uint16_t) sizeof(CS_Reset_t),
                             (void *)&rst_type) == DATAPACK_OK)
    {
      at_status_t err;
      err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_RESET, &cmd_buf[0], &rsp_buf[0]);
      if (err == ATSTATUS_OK)
      {
        PRINT_DBG("<Cellular_Service> HW device reset done")
        retval = CELLULAR_OK;
      }
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_DBG("<Cellular_Service> error during HW device reset")
  }
  return (retval);
}

static CS_Status_t perform_SW_reset(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("perform_SW_reset")

  CS_Reset_t rst_type = CS_RESET_SW;

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_RESET,
                           (uint16_t) sizeof(CS_Reset_t),
                           (void *)&rst_type) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_RESET, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> SW device reset done")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_DBG("<Cellular_Service> error during SW device reset")
  }
  return (retval);
}

static CS_Status_t perform_Factory_reset(void)
{
  CS_Status_t retval = CELLULAR_ERROR;
  PRINT_API("perform_SW_reset")

  CS_Reset_t rst_type = CS_RESET_FACTORY_RESET;

  if (DATAPACK_writeStruct(&cmd_buf[0],
                           (uint16_t) CSMT_RESET,
                           (uint16_t) sizeof(CS_Reset_t),
                           (void *)&rst_type) == DATAPACK_OK)
  {
    at_status_t err;
    err = AT_sendcmd(_Adapter_Handle, (at_msg_t) SID_CS_RESET, &cmd_buf[0], &rsp_buf[0]);
    if (err == ATSTATUS_OK)
    {
      PRINT_DBG("<Cellular_Service> Factory device reset done")
      retval = CELLULAR_OK;
    }
  }

  if (retval == CELLULAR_ERROR)
  {
    PRINT_ERR("<Cellular_Service> error during Factory device reset")
  }
  return (retval);
}

static void CELLULAR_reset_context(void)
{
  /* init cs_ctxt_urc_subscription */
  cs_ctxt_urc_subscription.eps_network_registration = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.gprs_network_registration = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.cs_network_registration = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.eps_location_info = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.gprs_location_info = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.cs_location_info = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.signal_quality = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.packet_domain_event = CELLULAR_FALSE;
  cs_ctxt_urc_subscription.ping_rsp = CELLULAR_FALSE;

  /* init cs_ctxt_eps_location_info */
  cs_ctxt_eps_location_info.ci = 0U;
  cs_ctxt_eps_location_info.lac = 0U;
  cs_ctxt_eps_location_info.ci_updated = CELLULAR_FALSE;
  cs_ctxt_eps_location_info.lac_updated = CELLULAR_FALSE;

  /* init cs_ctxt_gprs_location_info */
  cs_ctxt_gprs_location_info.ci = 0U;
  cs_ctxt_gprs_location_info.lac = 0U;
  cs_ctxt_gprs_location_info.ci_updated = CELLULAR_FALSE;
  cs_ctxt_gprs_location_info.lac_updated = CELLULAR_FALSE;

  /* init cs_ctxt_cs_location_info */
  cs_ctxt_cs_location_info.ci = 0U;
  cs_ctxt_cs_location_info.lac = 0U;
  cs_ctxt_cs_location_info.ci_updated = CELLULAR_FALSE;
  cs_ctxt_cs_location_info.lac_updated = CELLULAR_FALSE;

  /* init network states */
  cs_ctxt_eps_network_reg_state = CS_NRS_UNKNOWN;
  cs_ctxt_gprs_network_reg_state = CS_NRS_UNKNOWN;
  cs_ctxt_cs_network_reg_state = CS_NRS_UNKNOWN;
}

static void CELLULAR_reset_socket_context(void)
{
  PRINT_DBG("CELLULAR_reset_socket_context")
  uint8_t cpt;
  for (cpt = 0U; cpt < CELLULAR_MAX_SOCKETS; cpt ++)
  {
    csint_socket_init((socket_handle_t)cpt);
  }
}

static CS_Status_t CELLULAR_init(void)
{
  CS_Status_t retval = CELLULAR_ERROR;

  /* static variables */
  static sysctrl_info_t modem_device_infos;  /* LTE Modem informations */

  if (SysCtrl_getDeviceDescriptor(DEVTYPE_MODEM_CELLULAR, &modem_device_infos) == SCSTATUS_OK)
  {
    /* init ATCore & IPC layers*/
    (void) AT_init();

    _Adapter_Handle = AT_open(&modem_device_infos, CELLULAR_idle_event_notif, CELLULAR_urc_notif);
    if (_Adapter_Handle != AT_HANDLE_INVALID)
    {
      /* init local context variables */
      CELLULAR_reset_context();

      /* init socket context */
      CELLULAR_reset_socket_context();

      retval = CELLULAR_OK;
    }
  }

  return (retval);
}

static void CELLULAR_idle_event_notif(void)
{
#if (RTOS_USED == 0)
  PRINT_API("<Cellular_Service> idle event notif")
  eventReceived++;

  if (register_event_callback != NULL)
  {
    /* inform client that an event has been received */
    (* register_event_callback)(eventReceived);
  }
#endif /* RTOS_USED */
}

static void CELLULAR_urc_notif(at_buf_t *p_rsp_buf)
{
  uint16_t msgtype;

  PRINT_API("<Cellular_Service> CELLULAR_urc_notif")

  msgtype = DATAPACK_readMsgType(p_rsp_buf);

  /* --- EPS NETWORK REGISTRATION URC --- */
  if ((msgtype == (uint16_t) CSMT_URC_EPS_NETWORK_REGISTRATION_STATUS) &&
      (cs_ctxt_urc_subscription.eps_network_registration == CELLULAR_TRUE))
  {
    CS_NetworkRegState_t rx_state;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_EPS_NETWORK_REGISTRATION_STATUS,
                            (uint16_t) sizeof(CS_NetworkRegState_t),
                            (void *)&rx_state) == DATAPACK_OK)
    {
      /* if network registration status has changed, notify client */
      if (rx_state != cs_ctxt_eps_network_reg_state)
      {
        PRINT_DBG("<Cellular_Service> EPS network registration updated: %d", rx_state)
        cs_ctxt_eps_network_reg_state = rx_state;
        if (urc_eps_network_registration_callback != NULL)
        {
          /* possible evolution: pack datas to client */
          (* urc_eps_network_registration_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> EPS network registration unchanged")
      }
    }
  }
  /* --- GPRS NETWORK REGISTRATION URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_GPRS_NETWORK_REGISTRATION_STATUS) &&
           (cs_ctxt_urc_subscription.gprs_network_registration == CELLULAR_TRUE))
  {
    CS_NetworkRegState_t rx_state;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_GPRS_NETWORK_REGISTRATION_STATUS,
                            (uint16_t) sizeof(CS_NetworkRegState_t),
                            (void *)&rx_state) == DATAPACK_OK)
    {
      /* if network registration status has changed, notify client */
      if (rx_state != cs_ctxt_gprs_network_reg_state)
      {
        PRINT_DBG("<Cellular_Service> GPRS network registration updated: %d", rx_state)
        cs_ctxt_gprs_network_reg_state = rx_state;
        if (urc_gprs_network_registration_callback != NULL)
        {
          /* TODO pack datas to client ?? */
          (* urc_gprs_network_registration_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> GPRS network registration unchanged")
      }
    }
  }
  /* --- CS NETWORK REGISTRATION URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_CS_NETWORK_REGISTRATION_STATUS) &&
           (cs_ctxt_urc_subscription.cs_network_registration == CELLULAR_TRUE))
  {
    CS_NetworkRegState_t rx_state;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_CS_NETWORK_REGISTRATION_STATUS,
                            (uint16_t) sizeof(CS_NetworkRegState_t),
                            (void *)&rx_state) == DATAPACK_OK)
    {
      /* if network registration status has changed, notify client */
      if (rx_state != cs_ctxt_cs_network_reg_state)
      {
        PRINT_DBG("<Cellular_Service> CS network registration updated: %d", rx_state)
        cs_ctxt_cs_network_reg_state = rx_state;
        if (urc_cs_network_registration_callback != NULL)
        {
          /* TODO pack datas to client ?? */
          (* urc_cs_network_registration_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> CS network registration unchanged")
      }
    }
  }
  /* --- EPS LOCATION INFORMATION URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_EPS_LOCATION_INFO) &&
           (cs_ctxt_urc_subscription.eps_location_info == CELLULAR_TRUE))
  {
    CS_Bool_t loc_update = CELLULAR_FALSE;
    csint_location_info_t rx_loc;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_EPS_LOCATION_INFO,
                            (uint16_t) sizeof(csint_location_info_t),
                            (void *)&rx_loc) == DATAPACK_OK)
    {
      /* ci received and changed since last time ? */
      if (rx_loc.ci_updated == CELLULAR_TRUE)
      {
        if (rx_loc.ci != cs_ctxt_eps_location_info.ci)
        {
          /* ci has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_eps_location_info.ci = rx_loc.ci;
        }

        /* if local ci info was not updated */
        if (cs_ctxt_eps_location_info.ci_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_eps_location_info.ci_updated = CELLULAR_TRUE;
        }
      }

      /* lac received and changed since last time ? */
      if (rx_loc.lac_updated == CELLULAR_TRUE)
      {
        if (rx_loc.lac != cs_ctxt_eps_location_info.lac)
        {
          /* lac has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_eps_location_info.lac = rx_loc.lac;
        }

        /* if local lac info was not updated */
        if (cs_ctxt_eps_location_info.lac_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_eps_location_info.lac_updated = CELLULAR_TRUE;
        }
      }

      /* if location has changed, notify client */
      if (loc_update == CELLULAR_TRUE)
      {
        if (urc_eps_location_info_callback != NULL)
        {
          PRINT_DBG("<Cellular_Service> EPS location information info updated: lac=%d, ci=%ld", rx_loc.lac, rx_loc.ci)
          /* TODO pack datas to client ?? */
          (* urc_eps_location_info_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> EPS location information unchanged")
      }
    }
  }
  /* --- GPRS LOCATION INFORMATION URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_GPRS_LOCATION_INFO) &&
           (cs_ctxt_urc_subscription.gprs_location_info == CELLULAR_TRUE))
  {
    CS_Bool_t loc_update = CELLULAR_FALSE;
    csint_location_info_t rx_loc;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_GPRS_LOCATION_INFO,
                            (uint16_t) sizeof(csint_location_info_t),
                            (void *)&rx_loc) == DATAPACK_OK)
    {
      /* ci received and changed since last time ? */
      if (rx_loc.ci_updated == CELLULAR_TRUE)
      {
        if (rx_loc.ci != cs_ctxt_gprs_location_info.ci)
        {
          /* ci has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_gprs_location_info.ci = rx_loc.ci;
        }

        /* if local ci info was not updated */
        if (cs_ctxt_gprs_location_info.ci_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_gprs_location_info.ci_updated = CELLULAR_TRUE;
        }
      }

      /* lac received and changed since last time ? */
      if (rx_loc.lac_updated == CELLULAR_TRUE)
      {
        if (rx_loc.lac != cs_ctxt_gprs_location_info.lac)
        {
          /* lac has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_gprs_location_info.lac = rx_loc.lac;
        }

        /* if local lac info was not updated */
        if (cs_ctxt_gprs_location_info.lac_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_gprs_location_info.lac_updated = CELLULAR_TRUE;
        }
      }

      /* if location has changed, notify client */
      if (loc_update == CELLULAR_TRUE)
      {
        if (urc_gprs_location_info_callback != NULL)
        {
          PRINT_DBG("<Cellular_Service> GPRS location information info updated: lac=%d, ci=%ld", rx_loc.lac, rx_loc.ci)
          /* TODO pack datas to client ?? */
          (* urc_gprs_location_info_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> GPRS location information unchanged")
      }
    }
  }
  /* --- CS LOCATION INFORMATION URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_CS_LOCATION_INFO) &&
           (cs_ctxt_urc_subscription.cs_location_info == CELLULAR_TRUE))
  {
    CS_Bool_t loc_update = CELLULAR_FALSE;
    csint_location_info_t rx_loc;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_CS_LOCATION_INFO,
                            (uint16_t) sizeof(csint_location_info_t),
                            (void *)&rx_loc) == DATAPACK_OK)
    {
      /* ci received and changed since last time ? */
      if (rx_loc.ci_updated == CELLULAR_TRUE)
      {
        if (rx_loc.ci != cs_ctxt_cs_location_info.ci)
        {
          /* ci has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_cs_location_info.ci = rx_loc.ci;
        }

        /* if local ci info was not updated */
        if (cs_ctxt_cs_location_info.ci_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_cs_location_info.ci_updated = CELLULAR_TRUE;
        }
      }

      /* lac received and changed since last time ? */
      if (rx_loc.lac_updated == CELLULAR_TRUE)
      {
        if (rx_loc.lac != cs_ctxt_cs_location_info.lac)
        {
          /* lac has change */
          loc_update = CELLULAR_TRUE;
          cs_ctxt_cs_location_info.lac = rx_loc.lac;
        }

        /* if local lac info was not updated */
        if (cs_ctxt_cs_location_info.lac_updated == CELLULAR_FALSE)
        {
          loc_update = CELLULAR_TRUE;
          cs_ctxt_cs_location_info.lac_updated = CELLULAR_TRUE;
        }
      }

      /* if location has changed, notify client */
      if (loc_update == CELLULAR_TRUE)
      {
        if (urc_cs_location_info_callback != NULL)
        {
          PRINT_DBG("<Cellular_Service> CS location information info updated: lac=%d, ci=%ld", rx_loc.lac, rx_loc.ci)
          /* TODO pack datas to client ?? */
          (* urc_cs_location_info_callback)();
        }
      }
      else
      {
        PRINT_DBG("<Cellular_Service> CS location information unchanged")
      }
    }
  }
  /* --- SIGNAL QUALITY URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_SIGNAL_QUALITY) &&
           (cs_ctxt_urc_subscription.signal_quality == CELLULAR_TRUE))
  {
    CS_SignalQuality_t local_sig_qual;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_SIGNAL_QUALITY,
                            (uint16_t) sizeof(CS_SignalQuality_t),
                            (void *)&local_sig_qual) == DATAPACK_OK)
    {

      if (urc_signal_quality_callback != NULL)
      {
        PRINT_INFO("<Cellular_Service> CS signal quality info updated: rssi=%d, ber=%d", local_sig_qual.rssi,
                   local_sig_qual.ber)
        /* TODO pack datas to client ?? */
        (* urc_signal_quality_callback)();
      }
    }
  }
  /* --- SOCKET DATA PENDING URC --- */
  else if (msgtype == (uint16_t) CSMT_URC_SOCKET_DATA_PENDING)
  {
    /* unpack datas received */
    socket_handle_t sockHandle;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_SOCKET_DATA_PENDING,
                            (uint16_t) sizeof(socket_handle_t),
                            (void *)&sockHandle) == DATAPACK_OK)
    {
      if (sockHandle != CS_INVALID_SOCKET_HANDLE)
      {
        /* inform client that data are pending */
        if (cs_ctxt_sockets_info[sockHandle].socket_data_ready_callback != NULL)
        {
          (* cs_ctxt_sockets_info[sockHandle].socket_data_ready_callback)(sockHandle);
        }
      }
    }
  }
  /* --- SOCKET DATA CLOSED BY REMOTE URC --- */
  else if (msgtype == (uint16_t) CSMT_URC_SOCKET_CLOSED)
  {
    /* unpack datas received */
    socket_handle_t sockHandle;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_SOCKET_CLOSED,
                            (uint16_t) sizeof(socket_handle_t),
                            (void *)&sockHandle) == DATAPACK_OK)
    {
      if (sockHandle != CS_INVALID_SOCKET_HANDLE)
      {
        /* inform client that socket has been closed by remote  */
        if (cs_ctxt_sockets_info[sockHandle].socket_remote_close_callback != NULL)
        {
          (* cs_ctxt_sockets_info[sockHandle].socket_remote_close_callback)(sockHandle);
        }

        /* do not deallocate socket handle and reinit socket parameters
        *   socket_deallocateHandle(sockHandle);
        *   client has to confirm with a call to CDS_socket_close()
        */
      }
    }
  }
  /* --- PACKET DOMAIN EVENT URC --- */
  else if ((msgtype == (uint16_t) CSMT_URC_PACKET_DOMAIN_EVENT) &&
           (cs_ctxt_urc_subscription.packet_domain_event == CELLULAR_TRUE))
  {
    /* unpack datas received */
    csint_PDN_event_desc_t pdn_event;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_PACKET_DOMAIN_EVENT,
                            (uint16_t) sizeof(csint_PDN_event_desc_t),
                            (void *)&pdn_event) == DATAPACK_OK)
    {
      PRINT_DBG("PDN event: origine=%d scope=%d type=%d (user cid=%d) ",
                pdn_event.event_origine, pdn_event.event_scope,
                pdn_event.event_type, pdn_event.conf_id)

      /* is it a valid PDN ? */
      if ((pdn_event.conf_id == CS_PDN_USER_CONFIG_1) ||
          (pdn_event.conf_id == CS_PDN_USER_CONFIG_2) ||
          (pdn_event.conf_id == CS_PDN_USER_CONFIG_3) ||
          (pdn_event.conf_id == CS_PDN_USER_CONFIG_4) ||
          (pdn_event.conf_id == CS_PDN_USER_CONFIG_5) ||
          (pdn_event.conf_id == CS_PDN_PREDEF_CONFIG))
      {
        if (urc_packet_domain_event_callback[pdn_event.conf_id] != NULL)
        {
          CS_PDN_event_t conv_pdn_event = convert_to_PDN_event(pdn_event);
          (* urc_packet_domain_event_callback[pdn_event.conf_id])(pdn_event.conf_id, conv_pdn_event);
        }
      }
      else if (pdn_event.conf_id == CS_PDN_ALL)
      {
        CS_PDN_event_t conv_pdn_event = convert_to_PDN_event(pdn_event);
        /* event reported to all PDN registered */
        for (uint8_t loop = 0U; loop < CS_MAX_NB_PDP_CTXT; loop++)
        {
          if (urc_packet_domain_event_callback[loop] != NULL)
          {
            CS_PDN_conf_id_t pdn_cid = convert_index_to_PDN_conf(loop);
            (* urc_packet_domain_event_callback[loop])(pdn_cid, conv_pdn_event);
          }
        }
      }
      else
      {
        PRINT_INFO("PDN not identified")
      }
    }
  }
  /* --- PING URC --- */
  else if (msgtype == (uint16_t) CSMT_URC_PING_RSP)
  {
    /* unpack datas received */
    CS_Ping_response_t ping_rsp;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_PING_RSP,
                            (uint16_t) sizeof(CS_Ping_response_t),
                            (void *)&ping_rsp) == DATAPACK_OK)
    {
      PRINT_INFO("ping URC received at CS level")
      if (urc_ping_rsp_callback != NULL)
      {
        (* urc_ping_rsp_callback)(ping_rsp);
      }
    }
  }
  /* --- MODEM EVENT URC --- */
  else if (msgtype == (uint16_t) CSMT_URC_MODEM_EVENT)
  {
    /* unpack datas received */
    CS_ModemEvent_t modem_events;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_URC_MODEM_EVENT,
                            (uint16_t) sizeof(CS_ModemEvent_t),
                            (void *)&modem_events) == DATAPACK_OK)
    {
      PRINT_DBG("MODEM events received= 0x%x", modem_events)
      if (urc_modem_event_callback != NULL)
      {
        (* urc_modem_event_callback)(modem_events);
      }
    }
  }
  else
  {
    PRINT_DBG("ignore received URC (type=%d)", msgtype)
  }
}

static CS_Status_t CELLULAR_analyze_error_report(at_buf_t *p_rsp_buf)
{
  CS_Status_t retval;
  uint16_t msgtype;

  PRINT_API("<Cellular_Service> CELLULAR_analyze_error_report")
  retval = CELLULAR_ERROR;
  msgtype = DATAPACK_readMsgType(p_rsp_buf);

  /* check if we have received an error report */
  if (msgtype == (uint16_t) CSMT_ERROR_REPORT)
  {
    csint_error_report_t error_report;
    if (DATAPACK_readStruct(p_rsp_buf,
                            (uint16_t) CSMT_ERROR_REPORT,
                            (uint16_t) sizeof(csint_error_report_t),
                            (void *)&error_report) == DATAPACK_OK)
    {
      switch (error_report.error_type)
      {
        /* SIM error */
        case CSERR_SIM:
          retval = convert_SIM_error(&error_report);
          break;

        /* default error */
        default:
          retval = CELLULAR_ERROR;
          break;
      }
    }
  }

  PRINT_DBG("CS returned modified value after error report analysis = %d", retval)
  return (retval);
}

static CS_Status_t convert_SIM_error(const csint_error_report_t *p_error_report)
{
  CS_Status_t retval;

  /* convert SIM state to Cellular Service error returned to the client */
  switch (p_error_report->sim_state)
  {
    case CS_SIMSTATE_SIM_NOT_INSERTED:
      retval = CELLULAR_SIM_NOT_INSERTED;
      break;
    case CS_SIMSTATE_SIM_BUSY:
      retval = CELLULAR_SIM_BUSY;
      break;
    case CS_SIMSTATE_SIM_WRONG:
    case CS_SIMSTATE_SIM_FAILURE:
      retval = CELLULAR_SIM_ERROR;
      break;
    case CS_SIMSTATE_SIM_PIN_REQUIRED:
    case CS_SIMSTATE_SIM_PIN2_REQUIRED:
    case CS_SIMSTATE_SIM_PUK_REQUIRED:
    case CS_SIMSTATE_SIM_PUK2_REQUIRED:
      retval = CELLULAR_SIM_PIN_OR_PUK_LOCKED;
      break;
    case CS_SIMSTATE_INCORRECT_PASSWORD:
      retval = CELLULAR_SIM_INCORRECT_PASSWORD;
      break;
    default:
      retval = CELLULAR_SIM_ERROR;
      break;
  }
  return (retval);
}

static CS_PDN_event_t convert_to_PDN_event(csint_PDN_event_desc_t event_desc)
{
  CS_PDN_event_t ret = CS_PDN_EVENT_OTHER;

  if ((event_desc.event_origine == CGEV_EVENT_ORIGINE_NW) &&
      (event_desc.event_scope == CGEV_EVENT_SCOPE_GLOBAL) &&
      (event_desc.event_type == CGEV_EVENT_TYPE_DETACH))
  {
    ret = CS_PDN_EVENT_NW_DETACH;
  }
  else if ((event_desc.event_origine == CGEV_EVENT_ORIGINE_NW) &&
           (event_desc.event_scope == CGEV_EVENT_SCOPE_GLOBAL) &&
           (event_desc.event_type == CGEV_EVENT_TYPE_DEACTIVATION))
  {
    ret = CS_PDN_EVENT_NW_DEACT;
  }
  else if ((event_desc.event_origine == CGEV_EVENT_ORIGINE_NW) &&
           (event_desc.event_scope == CGEV_EVENT_SCOPE_PDN) &&
           (event_desc.event_type == CGEV_EVENT_TYPE_DEACTIVATION))
  {
    ret = CS_PDN_EVENT_NW_PDN_DEACT;
  }
  else
  {
    /* ignored */
  }

  return (ret);
}

static CS_PDN_conf_id_t convert_index_to_PDN_conf(uint8_t index)
{
  CS_PDN_conf_id_t PDNconf;
  switch (index)
  {
    case 0:
      PDNconf = CS_PDN_PREDEF_CONFIG;
      break;
    case 1:
      PDNconf = CS_PDN_USER_CONFIG_1;
      break;
    case 2:
      PDNconf = CS_PDN_USER_CONFIG_2;
      break;
    case 3:
      PDNconf = CS_PDN_USER_CONFIG_3;
      break;
    case 4:
      PDNconf = CS_PDN_USER_CONFIG_4;
      break;
    case 5:
      PDNconf = CS_PDN_USER_CONFIG_5;
      break;
    default:
      PDNconf = CS_PDN_NOT_DEFINED;
      break;
  }
  return (PDNconf);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


