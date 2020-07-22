/**
  ******************************************************************************
  * @file    at_custom_signalling.c
  * @author  MCD Application Team
  * @brief   This file provides common code for the modems
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

/* AT commands format
 * AT+<X>=?    : TEST COMMAND
 * AT+<X>?     : READ COMMAND
 * AT+<X>=...  : WRITE COMMAND
 * AT+<X>      : EXECUTION COMMAND
*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "at_core.h"
#include "at_modem_common.h"
#include "at_modem_signalling.h"
#include "at_modem_socket.h"
#include "at_datapack.h"
#include "at_util.h"
#include "sysctrl.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define MAX_CGEV_PARAM_SIZE         (32U)
#define ATC_GET_MINIMUM_SIZE(a,b) (((a)<(b))?(a):(b))

#define UNKNOWN_NETWORK_TYPE (uint8_t)(0x0) /* should match to NETWORK_TYPE_LUT */
#define CS_NETWORK_TYPE      (uint8_t)(0x1) /* should match to NETWORK_TYPE_LUT */
#define GPRS_NETWORK_TYPE    (uint8_t)(0x2) /* should match to NETWORK_TYPE_LUT */
#define EPS_NETWORK_TYPE     (uint8_t)(0x3) /* should match to NETWORK_TYPE_LUT */

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCUSTOM_MODEM == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "ATCModem:" format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "ATCModem:" format "\n\r", ## args)
#define PRINT_API(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P2, "ATCModem API:" format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "ATCModem ERROR:" format "\n\r", ## args)
#define PRINT_BUF(pbuf, size)       TRACE_PRINT_BUF_CHAR(DBG_CHAN_ATCMD, DBL_LVL_P1, (const CRC_CHAR_t *)pbuf, size);
#else
#define PRINT_INFO(format, args...)  (void) printf("ATCModem:" format "\n\r", ## args);
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void) printf("ATCModem ERROR:" format "\n\r", ## args);
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_PRINTF */
#else
#define PRINT_INFO(...)   __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_API(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#define PRINT_BUF(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCUSTOM_MODEM */

/* START_PARAM_LOOP and END_PARAM_LOOP macros are used to loop on all fields
*  received in a message.
*  Only non-null length fields are analysed.
*  End the analyze when the end of the message or an error has been detected.
*/
#define START_PARAM_LOOP()  uint8_t exitcode = 0U;\
  do {\
    if (atcc_extractElement(p_at_ctxt, p_msg_in, element_infos) != ATENDMSG_NO) {exitcode = 1U;}\
    if (element_infos->str_size != 0U)\
    {\

#define END_PARAM_LOOP()  }\
  if (retval == ATACTION_RSP_ERROR) {exitcode = 1U;}\
  } while (exitcode == 0U);

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void display_clear_network_state(CS_NetworkRegState_t state, uint8_t network_type);
static CS_NetworkRegState_t convert_NetworkState(uint32_t state, uint8_t network_type);
static CS_PDN_conf_id_t find_user_cid_with_matching_ip_addr(atcustom_persistent_context_t *p_persistent_ctxt,
                                                            csint_ip_addr_info_t *ip_addr_struct);
static at_action_rsp_t analyze_CmeError(at_context_t *p_at_ctxt,
                                        atcustom_modem_context_t *p_modem_ctxt,
                                        const IPC_RxMessage_t *p_msg_in,
                                        at_element_info_t *element_infos);
static void set_error_report(csint_error_type_t err_type, atcustom_modem_context_t *p_modem_ctxt);
static uint32_t extract_hex_value_from_quotes(const uint8_t *p_str, uint16_t str_size, uint8_t param_size);
static uint32_t extract_bin_value_from_quotes(const uint8_t *p_str, uint16_t str_size, uint8_t param_size);

/* Private function Definition -----------------------------------------------*/

static void display_clear_network_state(CS_NetworkRegState_t state, uint8_t network_type)
{
#if (USE_TRACE_ATCUSTOM_MODEM == 1U) /* to avoid warning when no traces */
  /* Commands Look-up table for AT+QCFG */
  static const AT_CHAR_t NETWORK_TYPE_LUT[][16] =
  {
    {"(unknown)"},
    {"(CS)"},
    {"(GPRS)"},
    {"(EPS)"},
  };

  /* check that network type is valid */
  if (network_type <= EPS_NETWORK_TYPE)
  {
    switch (state)
    {
      case CS_NRS_NOT_REGISTERED_NOT_SEARCHING:
        /* Trace only */
        PRINT_INFO("NetworkState %s = NOT_REGISTERED_NOT_SEARCHING", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_HOME_NETWORK:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_HOME_NETWORK", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_NOT_REGISTERED_SEARCHING:
        /* Trace only */
        PRINT_INFO("NetworkState %s = NOT_REGISTERED_SEARCHING", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTRATION_DENIED:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTRATION_DENIED", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_UNKNOWN:
        /* Trace only */
        PRINT_INFO("NetworkState %s = UNKNOWN", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_ROAMING:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_ROAMING", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_SMS_ONLY_HOME_NETWORK:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_SMS_ONLY_HOME_NETWORK", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_SMS_ONLY_ROAMING:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_SMS_ONLY_ROAMING", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_EMERGENCY_ONLY:
        /* Trace only */
        PRINT_INFO("NetworkState %s = EMERGENCY_ONLY", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_CFSB_NP_HOME_NETWORK:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_CFSB_NP_HOME_NETWORK", NETWORK_TYPE_LUT[network_type])
        break;
      case CS_NRS_REGISTERED_CFSB_NP_ROAMING:
        /* Trace only */
        PRINT_INFO("NetworkState %s = REGISTERED_CFSB_NP_ROAMING", NETWORK_TYPE_LUT[network_type])
        break;
      default:
        /* Trace only */
        PRINT_INFO("unknown state value")
        break;
    }
  }
  else
  {
    /* Trace only */
    PRINT_ERR("Invalid network type %d", network_type)
  }
#else /* USE_TRACE_ATCUSTOM_MODEM == 0U */
  UNUSED(state);
  UNUSED(network_type);
#endif /* USE_TRACE_ATCUSTOM_MODEM == 1U */
}

static CS_NetworkRegState_t convert_NetworkState(uint32_t state, uint8_t network_type)
{
  CS_NetworkRegState_t retval;

  switch (state)
  {
    case 0:
      retval = CS_NRS_NOT_REGISTERED_NOT_SEARCHING;
      break;
    case 1:
      retval = CS_NRS_REGISTERED_HOME_NETWORK;
      break;
    case 2:
      retval = CS_NRS_NOT_REGISTERED_SEARCHING;
      break;
    case 3:
      retval = CS_NRS_REGISTRATION_DENIED;
      break;
    case 4:
      retval = CS_NRS_UNKNOWN;
      break;
    case 5:
      retval = CS_NRS_REGISTERED_ROAMING;
      break;
    case 6:
      retval = CS_NRS_REGISTERED_SMS_ONLY_HOME_NETWORK;
      break;
    case 7:
      retval = CS_NRS_REGISTERED_SMS_ONLY_ROAMING;
      break;
    case 8:
      retval = CS_NRS_EMERGENCY_ONLY;
      break;
    case 9:
      retval = CS_NRS_REGISTERED_CFSB_NP_HOME_NETWORK;
      break;
    case 10:
      retval = CS_NRS_REGISTERED_CFSB_NP_ROAMING;
      break;
    default:
      retval = CS_NRS_UNKNOWN;
      break;
  }

  display_clear_network_state(retval, network_type);

  return (retval);
}

/*
 * Try to find user cid with matching IP address
 */
static CS_PDN_conf_id_t find_user_cid_with_matching_ip_addr(atcustom_persistent_context_t *p_persistent_ctxt,
                                                            csint_ip_addr_info_t *ip_addr_struct)
{
  CS_PDN_conf_id_t user_cid = CS_PDN_NOT_DEFINED;

  /* search user config ID corresponding to this IP address */
  for (uint8_t loop = 0U; loop < MODEM_MAX_NB_PDP_CTXT; loop++)
  {
    atcustom_modem_cid_table_t *p_tmp;
    p_tmp = &p_persistent_ctxt->modem_cid_table[loop];
    PRINT_DBG("[Compare ip addr with user cid=%d]: <%s> vs <%s>",
              loop,
              (CRC_CHAR_t *)&ip_addr_struct->ip_addr_value,
              (CRC_CHAR_t *)&p_tmp->ip_addr_infos.ip_addr_value)

    /* quick and dirty solution
     * should implement a better solution */
    uint8_t size1;
    uint8_t size2;
    uint8_t minsize;
    size1 = (uint8_t) strlen((CRC_CHAR_t *)&ip_addr_struct->ip_addr_value);
    size2 = (uint8_t) strlen((CRC_CHAR_t *)&p_tmp->ip_addr_infos.ip_addr_value);
    minsize = (size1 < size2) ? size1 : size2;
    if ((0 == memcmp((AT_CHAR_t *)&ip_addr_struct->ip_addr_value[0],
                     (AT_CHAR_t *)&p_tmp->ip_addr_infos.ip_addr_value[0],
                     (size_t) minsize)) &&
        (minsize != 0U)
       )
    {
      user_cid = atcm_convert_index_to_PDN_conf(loop);
      PRINT_DBG("Found matching user cid=%d", user_cid)
    }
  }

  return (user_cid);
}

/*
 * Analyze +CME ERROR
*/
static at_action_rsp_t analyze_CmeError(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                        const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter analyze_CmeError_CPIN()")

  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    AT_CHAR_t line[32] = {0U};
    PRINT_DBG("CME ERROR parameter received:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    /* copy element to line for parsing */
    if (element_infos->str_size <= 32U)
    {
      (void) memcpy((void *)&line[0],
                    (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                    (size_t) element_infos->str_size);
    }
    else
    {
      PRINT_ERR("line exceed maximum size, line ignored...")
    }

    /* convert string to test to upper case */
    ATutil_convertStringToUpperCase(&line[0], 32U);

    /* extract value and compare it to expected value */
    if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM NOT INSERTED") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_NOT_INSERTED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PIN NECESSARY") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PIN_REQUIRED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PIN REQUIRED") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PIN_REQUIRED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PUK REQUIRED") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PUK_REQUIRED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM FAILURE") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_FAILURE;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM BUSY") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_BUSY;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM WRONG") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_WRONG;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "INCORRECT PASSWORD") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_INCORRECT_PASSWORD;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PIN2 REQUIRED") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PIN2_REQUIRED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PUK2 REQUIRED") != NULL)
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PUK2_REQUIRED;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
    else
    {
      p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
      p_modem_ctxt->persist.sim_state = CS_SIMSTATE_UNKNOWN;
      set_error_report(CSERR_SIM, p_modem_ctxt);
    }
  }
  END_PARAM_LOOP()

  return (retval);
}

/*
 * Update error report that will be sent to Cellular Service
 */
static void set_error_report(csint_error_type_t err_type, atcustom_modem_context_t *p_modem_ctxt)
{
  p_modem_ctxt->SID_ctxt.error_report.error_type = err_type;

  switch (err_type)
  {
    case CSERR_SIM:
      p_modem_ctxt->SID_ctxt.error_report.sim_state = p_modem_ctxt->persist.sim_state;
      break;

    default:
      /* nothing to do*/
      break;
  }
}

#define LAC_TAC_SIZE   4 /* size = 2 bytes -> 4 half-bytes */
#define CI_SIZE        8 /* size = 4 bytes -> 8 half_bytes */
#define RAC_SIZE       2 /* size = 1 byte  -> 2 half-bytes */
#define MAX_PARAM_SIZE 8 /* max of previous values         */
/*
 * Extract the value of an hexadecimal parameter from a string
 */
static uint32_t extract_hex_value_from_quotes(const uint8_t *p_str, uint16_t str_size, uint8_t param_size)
{
  uint8_t tmp_array[MAX_PARAM_SIZE] = {0};
  uint16_t real_size;
  uint32_t converted_value;
  real_size = ATutil_remove_quotes(p_str, str_size, &tmp_array[0], param_size);
  converted_value = ATutil_convertHexaStringToInt32(&tmp_array[0], real_size);

  return (converted_value);
}

/*
 * Extract the value of an binary parameter from a string
 */
static uint32_t extract_bin_value_from_quotes(const uint8_t *p_str, uint16_t str_size, uint8_t param_size)
{
  uint8_t tmp_array[MAX_PARAM_SIZE] = {0};
  uint16_t real_size;
  uint32_t converted_value;
  real_size = ATutil_remove_quotes(p_str, str_size, &tmp_array[0], param_size);
  converted_value = ATutil_convertBinStringToInt32(&tmp_array[0], real_size);

  return (converted_value);
}
/* Functions Definition ------------------------------------------------------*/

/* ==========================  Build 3GPP TS 27.007 commands ========================== */
at_status_t fCmdBuild_NoParams(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_atp_ctxt);
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  /* Command as no parameters - STUB function */
  PRINT_API("enter fCmdBuild_NoParams()")

  return (retval);
}

at_status_t fCmdBuild_CGSN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGSN()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* AT+CGSN=<n> where <n>:
      * 0: serial number
      * 1: IMEI
      * 2: IMEISV (IMEI and Software version number)
      * 3: SVN (Software version number)
      *
      * <n> parameter is set previously
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d",
                   p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param);
  }
  return (retval);
}

at_status_t fCmdBuild_CMEE(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CMEE()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* AT+CMEE=<n> where <n>:
      * 0: <err> result code disabled and ERROR used
      * 1: <err> result code enabled and numeric <ERR> values used
      * 2: <err> result code enabled and verbose <ERR> values used
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", p_modem_ctxt->persist.cmee_level);
  }
  return (retval);
}

at_status_t fCmdBuild_CPIN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CPIN()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    PRINT_DBG("pin code= %s", p_modem_ctxt->SID_ctxt.modem_init.pincode.pincode)

    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"%s\"",
                   p_modem_ctxt->SID_ctxt.modem_init.pincode.pincode);
  }
  return (retval);
}

at_status_t fCmdBuild_CFUN(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CFUN()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_INIT_MODEM)
    {
      uint8_t fun;
      uint8_t rst;
      /* at modem init, get CFUN mode from parameters */
      const csint_modemInit_t *modemInit_struct = &(p_modem_ctxt->SID_ctxt.modem_init);

      /* AT+CFUN=<fun>,<rst>
        * where <fun>:
        *  0: minimum functionality
        *  1: full functionality
        *  4: disable phone TX & RX
        * where <rst>:
        *  0: do not reset modem before setting <fun> parameter
        *  1: reset modem before setting <fun> parameter
        */

      /* convert cellular service paramaters to Modem format */
      if (modemInit_struct->init == CS_CMI_FULL)
      {
        fun = 1U;
      }
      else if (modemInit_struct->init == CS_CMI_SIM_ONLY)
      {
        fun = 4U;
      }
      else
      {
        fun = 0U; /* default value, if CS_CMI_MINI */
      }

      (modemInit_struct->reset == CELLULAR_TRUE) ? (rst = 1U) : (rst = 0U);
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d", fun, rst);
    }
    else /* user settings */
    {
      /* set parameter defined by user */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,0", p_modem_ctxt->CMD_ctxt.cfun_value);
    }
  }
  return (retval);
}

at_status_t fCmdBuild_COPS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_COPS()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    CS_OperatorSelector_t *operatorSelect = &(p_modem_ctxt->SID_ctxt.write_operator_infos);

    if (operatorSelect->mode == CS_NRM_AUTO)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    else if (operatorSelect->mode == CS_NRM_MANUAL)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1,%d,%s", operatorSelect->format,
                     operatorSelect->operator_name);
    }
    else if (operatorSelect->mode == CS_NRM_DEREGISTER)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "2");
    }
    else if (operatorSelect->mode == CS_NRM_MANUAL_THEN_AUTO)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "4,%d,%s", operatorSelect->format,
                     operatorSelect->operator_name);
    }
    else
    {
      PRINT_ERR("invalid mode value for +COPS")
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
  }
  return (retval);
}

at_status_t fCmdBuild_CGATT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGATT()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* if cgatt set by user or if in ATTACH sequence */
    if ((p_modem_ctxt->CMD_ctxt.cgatt_write_cmd_param == CGATT_ATTACHED) ||
        (p_atp_ctxt->current_SID == (at_msg_t) SID_ATTACH_PS_DOMAIN))
    {
      /* request attach */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1");
    }
    /* if cgatt set by user or if in DETACH sequence */
    else if ((p_modem_ctxt->CMD_ctxt.cgatt_write_cmd_param == CGATT_DETACHED) ||
             (p_atp_ctxt->current_SID == (at_msg_t) SID_DETACH_PS_DOMAIN))
    {
      /* request detach */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    /* not in ATTACH or DETACH sequence and cgatt_write_cmd_param not set by user: error ! */
    else
    {
      PRINT_ERR("CGATT state parameter not set")
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CREG()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SUSBCRIBE_NET_EVENT)
    {
      /* always request all notif with +CREG:2, will be sorted at cellular service level */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", (uint8_t)CXREG_ENABLE_NETWK_REG_LOC_URC);
    }
    else if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_UNSUSBCRIBE_NET_EVENT)
    {
      /* disable notifications */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    else
    {
      /* for other SID, use param value set by user */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d",
                     (uint8_t) p_modem_ctxt->CMD_ctxt.cxreg_write_cmd_param);
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CGREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGREG()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SUSBCRIBE_NET_EVENT)
    {
      atcustom_CxREG_n_t param_value;
      if (p_modem_ctxt->persist.psm_requested == AT_TRUE)
      {
        param_value = CXREG_ENABLE_PSM_NETWK_REG_LOC_URC;
      }
      else
      {
        param_value = CXREG_ENABLE_NETWK_REG_LOC_URC;
      }
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", (uint8_t) param_value);
    }
    else if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_UNSUSBCRIBE_NET_EVENT)
    {
      /* disable notifications */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    else
    {
      /* for other SID, use param value set by user */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d",
                     (uint8_t) p_modem_ctxt->CMD_ctxt.cxreg_write_cmd_param);
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CEREG(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CEREG()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_SUSBCRIBE_NET_EVENT)
    {
      atcustom_CxREG_n_t param_value;
      if (p_modem_ctxt->persist.psm_requested == AT_TRUE)
      {
        param_value = CXREG_ENABLE_PSM_NETWK_REG_LOC_URC;
      }
      else
      {
        param_value = CXREG_ENABLE_NETWK_REG_LOC_URC;
      }
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", (uint8_t)param_value);
    }
    else if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_UNSUSBCRIBE_NET_EVENT)
    {
      /* disable notifications */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    else
    {
      /* for other SID, use param value set by user */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d",
                     (uint8_t) p_modem_ctxt->CMD_ctxt.cxreg_write_cmd_param);
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CGEREP(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGEREP()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* Packet Domain event reporting +CGEREP
      * 3GPP TS 27.007
      * Write command: +CGEREP=[<mode>[,<bfr>]]
      * where:
      *  <mode> = 0: no URC (+CGEV) are reported
      *         = 1: URC are discsarded when link is reserved (data on) and forwared otherwise
      *         = 2: URC are buffered when link is reserved and send when link freed, and forwared otherwise
      *  <bfr>  = 0: MT buffer of URC is cleared when <mode> 1 or 2 is entered
      *         = 1: MT buffer of URC is flushed to TE when <mode> 1 or 2 is entered
      */
    if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_REGISTER_PDN_EVENT)
    {
      /* enable notification (hard-coded value 1,0) */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1,0");
    }
    else if (p_atp_ctxt->current_SID == (at_msg_t) SID_CS_DEREGISTER_PDN_EVENT)
    {
      /* disable notifications */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
    else
    {
      /* nothing to do */
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CGDCONT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGDCONT()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {

    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
    PRINT_INFO("user cid = %d, modem cid = %d", (uint8_t)current_conf_id, modem_cid)
    /* check if this PDP context has been defined */
    if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].conf_id != CS_PDN_NOT_DEFINED)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,\"%s\",\"%s\"",
                     modem_cid,
                     atcm_get_PDPtypeStr(p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.pdp_type),
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].apn);
    }
    else
    {
      PRINT_ERR("PDP context not defined for conf_id %d", current_conf_id)
      retval = ATSTATUS_ERROR;
    }
  }

  return (retval);
}

at_status_t fCmdBuild_CGACT(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGACT()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);

    /* check if this PDP context has been defined */
    if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].conf_id == CS_PDN_NOT_DEFINED)
    {
      /* Trace only */
      PRINT_INFO("PDP context not explicitly defined for conf_id %d (using modem params)", current_conf_id)
    }

    /* PDP context activate or deactivate
      *  3GPP TS 27.007
      *  AT+CGACT=[<state>[,<cid>[,<cid>[,...]]]]
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d",
                   ((p_modem_ctxt->CMD_ctxt.pdn_state == PDN_STATE_ACTIVATE) ? 1 : 0),
                   modem_cid);
  }

  return (retval);
}

at_status_t fCmdBuild_CGAUTH(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGAUTH()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* cf 3GPP TS 27.007
    *  AT+CGAUTH: Define PDP context Authentication Parameters
    *  AT+QICSGP=<cid>[,<auth_pro>[,<userid>[,<password>]]]
    *  - cid: context id (specifies a paritcular PDP context definition)
    *  - auth_pro: 0 for none, 1 for PAP, 2 for CHAP
    *  - username: string
    *  - password: string
    */

    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);
    PRINT_INFO("user cid = %d, modem cid = %d", (uint8_t)current_conf_id, modem_cid)
    uint8_t auth_protocol;

    /*  authentication protocol: 0,1 or 2. 0 for none, 1 for PAP, 2 for CHAP */
    if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.username[0] == 0U)
    {
      /* no username => no authentication */
      auth_protocol = 0U;

      /* build command */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d",
                     modem_cid,
                     auth_protocol);
    }
    else
    {
      /* username => PAP authentication protocol */
      auth_protocol = 1U;

      /* build command */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d,\"%s\",\"%s\"",
                     modem_cid,
                     auth_protocol,
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.username,
                     p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].pdn_conf.password);
    }

  }

  return (retval);
}

at_status_t fCmdBuild_CGDATA(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGDATA()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);

    /* check if this PDP context has been defined */
    if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].conf_id == CS_PDN_NOT_DEFINED)
    {
      /* Trace only */
      PRINT_INFO("PDP context not explicitly defined for conf_id %d (using modem params)", current_conf_id)
    }

    /* Enter data state
      *  3GPP TS 27.007
      *  AT+CGDATA[=<L2P>[,<cid>[,<cid>[,...]]]]
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "\"PPP\",%d",
                   modem_cid);
  }

  return (retval);
}

at_status_t fCmdBuild_CGPADDR(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CGPADDR()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    CS_PDN_conf_id_t current_conf_id = atcm_get_cid_current_SID(p_modem_ctxt);
    uint8_t modem_cid = atcm_get_affected_modem_cid(&p_modem_ctxt->persist, current_conf_id);

    /* check if this PDP context has been defined */
    if (p_modem_ctxt->persist.pdp_ctxt_infos[current_conf_id].conf_id == CS_PDN_NOT_DEFINED)
    {
      /* Trace only */
      PRINT_INFO("PDP context not explicitly defined for conf_id %d (using modem params)", current_conf_id)
    }

    /* Show PDP adress(es)
      *  3GPP TS 27.007
      *  AT+CGPADDR[=<cid>[,<cid>[,...]]]
      *
      *  implemenation: we only request address for 1 cid (if more cid required, call it again)
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", modem_cid);
  }

  return (retval);
}

/* ==========================  Build V.25ter commands ========================== */
at_status_t fCmdBuild_ATD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATD()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* actually implemented specifically for each modem
      *  following example is not guaranteed ! (cid is not specified here)
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "*99#");
  }
  return (retval);
}

at_status_t fCmdBuild_ATE(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATE()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* echo mode ON or OFF */
    if (p_modem_ctxt->CMD_ctxt.command_echo == AT_TRUE)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1");
    }
    else
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
  }
  return (retval);
}

at_status_t fCmdBuild_ATV(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATV()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* echo mode ON or OFF */
    if (p_modem_ctxt->CMD_ctxt.dce_full_resp_format == AT_TRUE)
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "1");
    }
    else
    {
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
    }
  }
  return (retval);
}

at_status_t fCmdBuild_ATX(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATX()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* CONNECT Result code and monitor call progress
      *  for the moment, ATX0 to return result code only, dial tone and busy detection are both disabled
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
  }
  return (retval);
}

at_status_t fCmdBuild_ATZ(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ATZ()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    uint8_t profile_nbr = 0;
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d", profile_nbr);
  }
  return (retval);
}

at_status_t fCmdBuild_IPR(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_IPR()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* set baud rate */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%ld", p_modem_ctxt->CMD_ctxt.baud_rate);
  }
  return (retval);
}

at_status_t fCmdBuild_IFC(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_IPR()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* set flow control */
    if (p_modem_ctxt->CMD_ctxt.flow_control_cts_rts == AT_FALSE)
    {
      /* No flow control */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0,0");
    }
    else
    {
      /* CTS/RTS activated */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "2,2");
    }
  }
  return (retval);
}


at_status_t fCmdBuild_ESCAPE_CMD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_ESCAPE_CMD()")

  /* only for RAW command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD)
  {
    /* set escape sequence (as define in custom modem specific) */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%s", p_atp_ctxt->current_atcmd.name);
    /* set raw command size */
    p_atp_ctxt->current_atcmd.raw_cmd_size = strlen((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params);
  }
  else
  {
    retval = ATSTATUS_ERROR;
  }
  return (retval);
}

at_status_t fCmdBuild_AT_AND_D(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_AT_AND_D()")

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* Set DTR function mode  (cf V.25ter)
      * hard-coded to 0
      */
    (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
  }

  return (retval);
}

at_status_t fCmdBuild_CPSMS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CPSMS()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* 3GPP TS 27.007
     * Power Saving Mode Setting
     *
     * AT+CPSMS=[<mode>[,<Requested_Periodic-RAU>[,<Requested_GPRS-READY-timer>[,<Requested_Periodic-TAU>[
     * ,<Requested_Active-Time>]]]]]
     *
     * mode: 0 disable PSM, 1 enable PSM
     * > parameters for GERAN/UTRAN:
     * Requested_Periodic-RAU: (stringtype, one byte in an 8 bit format) cf Table 10.5.163a from TS 24.008, T3312
     * Requested_GPRS-READY-timer: (string type, one byte in an 8 bit format) cf Table 10.5.172 from TS 24.008, T3314
     * > parameters for E-UTRAN:
     * Requested_Periodic-TAU: (string type, one byte in an 8 bit format) cf Table 10.5.163a from TS 24.008, T3412
     * Requested_Active-Time: (string type, one byte in an 8 bit format) cf Table 10.5.163 from TS 24.008, T3324
     *
     * Note 1:
     * There is a special form of the command: AT+CPSMS= (with all parameters omitted).
     * In this form, PSM is disabled (mode=0) and all parameters set to the manufacturer specific default values.
     *
     * Note 2:
     * Table 10.5.163a from TS 24.008
     * Bits 5 to 1 represent the binary coded timer value
     * Bits 6 to 8 defines the timer value unit as follow (in order : bits 8 7 6)
     *   000: value is incremented in multiples of 10 minutes
     *   001: value is incremented in multiples of 1 hour
     *   010: value is incremented in multiples of 10 hours
     *   011: value is incremented in multiples of 2 seconds
     *   100: value is incremented in multiples of 30 seconds
     *   101: value is incremented in multiples of 1 minute
     *   110: value is incremented in multiples of 320 hours
     *   111: value indicates that the timer is deactivated
     *
     * Note 3:
     * Table 10.5.163 and Table 10.5.172 from TS 24.008
     * Bits 5 to 1 represent the binary coded timer value
     * Bits 6 to 8 defines the timer value unit as follow (in order : bits 8 7 6)
     *   000: value is incremented in multiples of 2 seconds
     *   001: value is incremented in multiples of 1 minute
     *   010: value is incremented in multiples of 10 hours
     *   111: value indicates that the timer is deactivated
     *   other values shall be interpreted as mutliples of 1 minute
     *
     * exple:
     * AT+CPSMS=1,,,”00000100”,”00001111”
     * Set the requested T3412 value to 40 minutes, and set the requested T3324 value to 30 seconds
    */

    /* buffers used to convert values to binary string (size = number of bits + 1 for end string character) */
    CS_PSM_params_t *p_psm_params = &(p_modem_ctxt->SID_ctxt.set_power_config.psm);
    uint8_t mode;
    uint8_t req_periodic_rau[9] = {0U};
    uint8_t req_gprs_ready_time[9] = {0U};
    uint8_t req_periodic_tau[9] = {0U};
    uint8_t req_active_time[9] = {0U};

    (void)ATutil_convert_uint8_to_binary_string((uint32_t)p_psm_params->req_periodic_RAU,
                                                (uint8_t)8U,
                                                (uint8_t)sizeof(req_periodic_rau),
                                                &req_periodic_rau[0]);
    (void)ATutil_convert_uint8_to_binary_string((uint32_t)p_psm_params->req_GPRS_READY_timer,
                                                (uint8_t)8U,
                                                (uint8_t)sizeof(req_gprs_ready_time),
                                                &req_gprs_ready_time[0]);
    (void) ATutil_convert_uint8_to_binary_string((uint32_t)p_psm_params->req_periodic_TAU,
                                                 (uint8_t)8U,
                                                 (uint8_t)sizeof(req_periodic_tau),
                                                 &req_periodic_tau[0]);
    (void) ATutil_convert_uint8_to_binary_string((uint32_t)p_psm_params->req_active_time,
                                                 (uint8_t)8U,
                                                 (uint8_t)sizeof(req_active_time),
                                                 &req_active_time[0]);

    if (p_modem_ctxt->SID_ctxt.set_power_config.psm_present == CELLULAR_TRUE)
    {
      if (p_modem_ctxt->SID_ctxt.set_power_config.psm_mode == PSM_MODE_DISABLE)
      {
        /* PSM disabled */
        mode = 0U;
      }
      else
      {
        /* PSM enabled */
        mode = 1U;
      }

      /* prepare the command
       *  Note: do not send values for 2G/3G networks
       */
      (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,,,\"%s\",\"%s\"",
                     mode,
                     req_periodic_tau,
                     req_active_time);

      /* full command version:
       *
       * (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,\"%s\",\"%s\",\"%s\",\"%s\"",
       *                mode,
       *                req_periodic_rau,
       *                req_gprs_ready_time,
       *                req_periodic_tau,
       *                req_active_time );
       */

    }
    else
    {
      /* no PSM parameters, skip the command */
      PRINT_INFO("No PSM parameters available, command skipped")
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
  }
  else
  {
    PRINT_ERR("invalid pointer to PSM parameters")
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}

at_status_t fCmdBuild_CEDRXS(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  UNUSED(p_modem_ctxt);

  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_CEDRXS()")

  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* 3GPP TS 27.007
     * eDRX Setting
     *
     * AT+CEDRXS=[<mode>,[,<AcT-type>[,<Requested_eDRX_value>]]]
     *
     * mode: 0 disable eDRX, 1 enable eDRX, 2 enable eDRX and enable URC +CEDRXP, 3 disable eDRX and reset default
     * AcT-type: 0 Act not using eDRX, 1 EC-GSM-IoT , 2 GSM, 3 UTRAN,
     *           4 E-UTRAN WB-S1 mode(LTE and LTE cat.M1), 5 E-UTRAN NB-S1 mode(LTE cat.NB1)
     * Requested_eDRX_value: (string type, half a byte in an 4 bit format)
     *                        Bits 4 to 1 of octet 3 of Extended DRX parameters
     *                        cf Table 10.5.5.32 from TS 24.008
     *
     * exple:
     * AT+CEDRX=1,5,”0000”
     * Set the requested e-I-DRX value to 5.12 second
    */

    if (p_modem_ctxt->SID_ctxt.set_power_config.edrx_present == CELLULAR_TRUE)
    {
      uint8_t edrx_req_value[5] = {0};
      (void) ATutil_convert_uint8_to_binary_string((uint32_t) p_modem_ctxt->SID_ctxt.set_power_config.edrx.req_value,
                                                   (uint8_t) 4U,
                                                   (uint8_t) sizeof(edrx_req_value),
                                                   &edrx_req_value[0]);

      if (p_modem_ctxt->SID_ctxt.set_power_config.edrx_mode == EDRX_MODE_DISABLE)
      {
        /* eDRX disabled */
        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "0");
      }
      else if (p_modem_ctxt->SID_ctxt.set_power_config.edrx_mode == EDRX_MODE_DISABLE_AND_RESET)
      {
        /* eDRX disabled */
        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "3");
      }
      else
      {
        /* eDRX enabled */
        uint8_t edrx_mode;
        edrx_mode = (p_modem_ctxt->SID_ctxt.set_power_config.edrx_mode == EDRX_MODE_ENABLE_WITH_URC) ? 2U : 1U;

        (void) sprintf((CRC_CHAR_t *)p_atp_ctxt->current_atcmd.params, "%d,%d,\"%s\"",
                       edrx_mode,
                       p_modem_ctxt->SID_ctxt.set_power_config.edrx.act_type,
                       edrx_req_value);
      }
    }
    else
    {
      /* no eDRX parameters, skip the command */
      PRINT_INFO("No EDRX parameters available, command skipped")
      atcm_program_SKIP_CMD(p_atp_ctxt);
    }
  }
  else
  {
    PRINT_ERR("invalid pointer to EDRX parameters")
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}

at_status_t fCmdBuild_DIRECT_CMD(atparser_context_t *p_atp_ctxt, atcustom_modem_context_t *p_modem_ctxt)
{
  at_status_t retval = ATSTATUS_OK;
  PRINT_API("enter fCmdBuild_DIRECT_CMD()")

  /* only for RAW command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_RAW_CMD)
  {
    if (p_modem_ctxt->SID_ctxt.direct_cmd_tx->cmd_size != 0U)
    {
      uint32_t str_size = p_modem_ctxt->SID_ctxt.direct_cmd_tx->cmd_size;
      (void) memcpy((void *)p_atp_ctxt->current_atcmd.params,
                    (const CS_CHAR_t *)p_modem_ctxt->SID_ctxt.direct_cmd_tx->cmd_str,
                    str_size);

      /* add termination characters */
      uint32_t endstr_size = strlen((CRC_CHAR_t *)&p_atp_ctxt->endstr);
      (void) memcpy((void *)&p_atp_ctxt->current_atcmd.params[str_size],
                    p_atp_ctxt->endstr,
                    endstr_size);

      /* set raw command size */
      p_atp_ctxt->current_atcmd.raw_cmd_size = str_size + endstr_size;
    }
    else
    {
      PRINT_ERR("ERROR, send buffer is empty")
      retval = ATSTATUS_ERROR;
    }
  }
  return (retval);
}

/* ==========================  Analyze 3GPP TS 27.007 commands ========================== */
at_action_rsp_t fRspAnalyze_None(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  UNUSED(p_msg_in);
  UNUSED(element_infos);

  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_None()")

  /* no parameters expected */

  return (retval);
}

at_action_rsp_t fRspAnalyze_Error(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  at_action_rsp_t retval;
  PRINT_API("enter fRspAnalyze_Error()")

  /* analyse parameters for ERROR */
  /* use CmeErr function for the moment */
  retval = fRspAnalyze_CmeErr(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);

  return (retval);
}

at_action_rsp_t fRspAnalyze_CmeErr(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  const atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  /*UNUSED(p_msg_in);*/
  /*UNUSED(element_infos);*/

  at_action_rsp_t retval = ATACTION_RSP_ERROR;
  PRINT_API("enter fRspAnalyze_CmeErr()")

  /* Analyze CME error to report it to upper layers */
  (void) analyze_CmeError(p_at_ctxt, p_modem_ctxt, p_msg_in, element_infos);

  /* specific treatments for +CME ERROR, depending of current command */
  switch (p_atp_ctxt->current_atcmd.id)
  {
    case CMD_AT_CGSN:
    {

      if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_SN)
      {
        PRINT_DBG("Modem Error for CGSN_SN, use unitialized value")
        (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.serial_number), 0, MAX_SIZE_SN);
      }
      else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_IMEI)
      {
        PRINT_DBG("Modem Error for CGSN_IMEI, use unitialized value")
        (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imei), 0, MAX_SIZE_IMEI);
      }
      else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_IMEISV)
      {
        PRINT_DBG("Modem Error for CGSN_IMEISV, use unitialized value, parameter ignored")
      }
      else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_SVN)
      {
        PRINT_DBG("Modem Error for CGSN_SVN, use unitialized value, parameter ignored")
      }
      else
      {
        PRINT_DBG("Modem Error for CGSN, unexpected parameter")
        retval = ATACTION_RSP_ERROR;
      }
      break;
    }

    case CMD_AT_CIMI:
      PRINT_DBG("Modem Error for CIMI, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imsi), 0, MAX_SIZE_IMSI);
      break;

    case CMD_AT_CGMI:
      PRINT_DBG("Modem Error for CGMI, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.manufacturer_name), 0, MAX_SIZE_MANUFACT_NAME);
      break;

    case CMD_AT_CGMM:
      PRINT_DBG("Modem Error for CGMM, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.model), 0, MAX_SIZE_MODEL);
      break;

    case CMD_AT_CGMR:
      PRINT_DBG("Modem Error for CGMR, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.revision), 0, MAX_SIZE_REV);
      break;

    case CMD_AT_CNUM:
      PRINT_DBG("Modem Error for CNUM, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.phone_number), 0, MAX_SIZE_PHONE_NBR);
      break;

    case CMD_AT_GSN:
      PRINT_DBG("Modem Error for GSN, use unitialized value")
      (void) memset((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imei), 0, MAX_SIZE_IMEI);
      break;

    case CMD_AT_CPIN:
      PRINT_DBG("Analyze Modem Error for CPIN")
      break;

    /* consider all other error cases for AT commands
      * case ?:
      * etc...
      */

    default:
      PRINT_DBG("Modem Error for cmd (id=%ld)", p_atp_ctxt->current_atcmd.id)
      retval = ATACTION_RSP_ERROR;
      break;
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CmsErr(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  UNUSED(p_msg_in);
  UNUSED(element_infos);

  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CmsErr()")

  /* analyze parameters for +CMS ERROR */
  /* Not implemented */

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGMI(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGMI()")

  /* analyze parameters for +CGMI */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("Manufacturer name:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.manufacturer_name),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t)element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGMM(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGMM()")

  /* analyze parameters for +CGMM */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("Model:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.model),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t)element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGMR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGMR()")

  /* analyze parameters for +CGMR */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("Revision:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.revision),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t)element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGSN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGSN()")

  /* analyze parameters for +CGSN */
  /* only for write command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_SN)
    {
      /* serial number */
      PRINT_DBG("Serial Number:")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

      (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.serial_number),
                    (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                    (size_t) element_infos->str_size);
    }
    else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_IMEI)
    {
      /* IMEI */
      PRINT_DBG("IMEI:")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

      (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imei),
                    (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                    (size_t) element_infos->str_size);
    }
    else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_IMEISV)
    {
      /* IMEISV */
      PRINT_DBG("IMEISV (NOT USED):")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
    }
    else if (p_modem_ctxt->CMD_ctxt.cgsn_write_cmd_param == CGSN_SVN)
    {
      /* SVN */
      PRINT_DBG("SVN (NOT USED):")
      PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)
    }
    else
    {
      /* nothing to do */
    }
  }

  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("Serial Number:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.serial_number),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t) element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CIMI(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CIMI()")

  /* analyze parameters for +CIMI */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("IMSI:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imsi),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t) element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CEER(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  UNUSED(p_msg_in);
  UNUSED(element_infos);

  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CEER()")

  /* analyze parameters for CEER */

  return (retval);
}

at_action_rsp_t fRspAnalyze_CPIN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CPIN()")

  /* analyze parameters for CPIN */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    AT_CHAR_t line[32] = {0U};
    PRINT_DBG("CPIN parameter received:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    /* copy element to line for parsing */
    if (element_infos->str_size <= 32U)
    {
      (void) memcpy((void *)&line[0],
                    (const void *) & (p_msg_in->buffer[element_infos->str_start_idx]),
                    (size_t) element_infos->str_size);

      /* extract value and compare it to expected value */
      if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PIN") != NULL)
      {
        PRINT_DBG("waiting for SIM PIN")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PIN_REQUIRED;
        set_error_report(CSERR_SIM, p_modem_ctxt);
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PUK") != NULL)
      {
        PRINT_DBG("waiting for SIM PUK")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PUK_REQUIRED;
        set_error_report(CSERR_SIM, p_modem_ctxt);
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PIN2") != NULL)
      {
        PRINT_DBG("waiting for SIM PUK2")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PUK2_REQUIRED;
        set_error_report(CSERR_SIM, p_modem_ctxt);
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "SIM PUK2") != NULL)
      {
        PRINT_DBG("waiting for SIM PUK")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_SIM_PUK_REQUIRED;
        set_error_report(CSERR_SIM, p_modem_ctxt);
      }
      else if ((AT_CHAR_t *) strstr((const CRC_CHAR_t *)&line[0], "READY") != NULL)
      {
        PRINT_DBG("CPIN READY")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_TRUE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_READY;
      }
      else
      {
        PRINT_ERR("UNEXPECTED CPIN STATE")
        p_modem_ctxt->persist.sim_pin_code_ready = AT_FALSE;
        p_modem_ctxt->persist.sim_state = CS_SIMSTATE_UNKNOWN;
        set_error_report(CSERR_SIM, p_modem_ctxt);
      }
    }
    else
    {
      PRINT_ERR("line exceed maximum size, line ignored...")
      retval = ATACTION_RSP_IGNORED;
    }

  }
  END_PARAM_LOOP()
  return (retval);
}

at_action_rsp_t fRspAnalyze_CFUN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CFUN()")

  /* analyze parameters for +CFUN
  *  answer to CFUN read command
  *     +CFUN: <state>
  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
  {
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t cfun_status = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                       element_infos->str_size);
      if (cfun_status == 1U)
      {
        p_modem_ctxt->SID_ctxt.cfun_status = CS_CMI_FULL;
      }
      else if (cfun_status == 4U)
      {
        p_modem_ctxt->SID_ctxt.cfun_status = CS_CMI_SIM_ONLY;
      }
      else
      {
        /* default value ( if equal to O or anything else) */
        p_modem_ctxt->SID_ctxt.cfun_status = CS_CMI_MINI;
      }
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_COPS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_COPS()")

  /* analyze parameters for +COPS
    *  Different cases to consider (as format is different)
    *  1/ answer to COPS read command
    *     +COPS: <mode>[,<format>,<oper>[,<AcT>]]
    *  2/ answer to COPS test command
    *     +COPS: [list of supported (<stat>,long alphanumeric <oper>,
    *            short alphanumeric <oper>,numeric <oper>[,<AcT>])s]
    *            [,,(list ofsupported <mode>s),(list of supported <format>s)]
  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
  {
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      /* mode (mandatory) */
      uint32_t mode = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      switch (mode)
      {
        case 0:
          p_modem_ctxt->SID_ctxt.read_operator_infos.mode = CS_NRM_AUTO;
          break;
        case 1:
          p_modem_ctxt->SID_ctxt.read_operator_infos.mode = CS_NRM_MANUAL;
          break;
        case 2:
          p_modem_ctxt->SID_ctxt.read_operator_infos.mode = CS_NRM_DEREGISTER;
          break;
        case 4:
          p_modem_ctxt->SID_ctxt.read_operator_infos.mode = CS_NRM_MANUAL_THEN_AUTO;
          break;
        default:
          PRINT_ERR("invalid mode value in +COPS")
          p_modem_ctxt->SID_ctxt.read_operator_infos.mode = CS_NRM_AUTO;
          break;
      }

      PRINT_DBG("+COPS: mode = %d", p_modem_ctxt->SID_ctxt.read_operator_infos.mode)
    }
    else if (element_infos->param_rank == 3U)
    {
      /* format (optional) */
      uint32_t format = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
      p_modem_ctxt->SID_ctxt.read_operator_infos.optional_fields_presence |= CS_RSF_FORMAT_PRESENT; /* bitfield */
      switch (format)
      {
        case 0:
          p_modem_ctxt->SID_ctxt.read_operator_infos.format = CS_ONF_LONG;
          break;
        case 1:
          p_modem_ctxt->SID_ctxt.read_operator_infos.format = CS_ONF_SHORT;
          break;
        case 2:
          p_modem_ctxt->SID_ctxt.read_operator_infos.format = CS_ONF_NUMERIC;
          break;
        default:
          PRINT_ERR("invalid format value")
          p_modem_ctxt->SID_ctxt.read_operator_infos.format = CS_ONF_NOT_PRESENT;
          break;
      }
      PRINT_DBG("+COPS: format = %d", p_modem_ctxt->SID_ctxt.read_operator_infos.format)
    }
    else if (element_infos->param_rank == 4U)
    {
      /* operator name (optional) */
      if (element_infos->str_size <= MAX_SIZE_OPERATOR_NAME)
      {
        p_modem_ctxt->SID_ctxt.read_operator_infos.optional_fields_presence |=
          CS_RSF_OPERATOR_NAME_PRESENT; /* bitfield */
        (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.read_operator_infos.operator_name[0]),
                      (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                      (size_t) element_infos->str_size);
        PRINT_DBG("+COPS: operator name = %s", p_modem_ctxt->SID_ctxt.read_operator_infos.operator_name)
      }
      else
      {
        PRINT_ERR("error, operator name too long")
        retval = ATACTION_RSP_ERROR;
      }
    }
    else if (element_infos->param_rank == 5U)
    {
      /* AccessTechno (optional) */
      p_modem_ctxt->SID_ctxt.read_operator_infos.optional_fields_presence |= CS_RSF_ACT_PRESENT;  /* bitfield */
      uint32_t AcT = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                               element_infos->str_size);
      switch (AcT)
      {
        case 0:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_GSM;
          break;
        case 1:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_GSM_COMPACT;
          break;
        case 2:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_UTRAN;
          break;
        case 3:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_GSM_EDGE;
          break;
        case 4:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_UTRAN_HSDPA;
          break;
        case 5:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_UTRAN_HSUPA;
          break;
        case 6:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_UTRAN_HSDPA_HSUPA;
          break;
        case 7:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_E_UTRAN;
          break;
        case 8:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_EC_GSM_IOT;
          PRINT_INFO(">>> Access Technology : LTE Cat.M1 <<<")
          break;
        case 9:
          p_modem_ctxt->SID_ctxt.read_operator_infos.AcT = CS_ACT_E_UTRAN_NBS1;
          PRINT_INFO(">>> Access Technology : LTE Cat.NB1 <<<")
          break;
        default:
          PRINT_ERR("invalid AcT value")
          break;
      }

      PRINT_DBG("+COPS: Access technology = %ld", AcT)

    }
    else
    {
      /* parameters ignored */
    }
    END_PARAM_LOOP()
  }

  if (p_atp_ctxt->current_atcmd.type == ATTYPE_TEST_CMD)
  {
    PRINT_DBG("+COPS for test cmd NOT IMPLEMENTED")
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CNUM(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  UNUSED(p_msg_in);
  UNUSED(element_infos);

  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fCmdBuild_CNUM()")

  PRINT_DBG("+CNUM cmd NOT IMPLEMENTED")

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGATT(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGATT()")

  /* analyze parameters for +CGATT
  *  Different cases to consider (as format is different)
  *  1/ answer to CGATT read command
  *     +CGATT: <state>
  *  2/ answer to CGATT test command
  *     +CGATT: (list of supported <state>s)
  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
  {
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t attach = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
      p_modem_ctxt->SID_ctxt.attach_status = (attach == 1U) ? CS_PS_ATTACHED : CS_PS_DETACHED;
      PRINT_DBG("attach status = %d", p_modem_ctxt->SID_ctxt.attach_status)
    }
    END_PARAM_LOOP()
  }

  if (p_atp_ctxt->current_atcmd.type == ATTYPE_TEST_CMD)
  {
    PRINT_DBG("+CGATT for test cmd NOT IMPLEMENTED")
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CREG()")
  PRINT_DBG("current cmd = %ld", p_atp_ctxt->current_atcmd.id)

  /* analyze parameters for +CREG
  *  Different cases to consider (as format is different)
  *  1/ answer to CREG read command
  *     +CREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
  *  2/ answer to CREG test command
  *     +CREG: (list of supported <n>s)
  *  3/ URC:
  *     +CREG: <stat>[,[<lac>],[<ci>],[<AcT>]]
  */
  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CREG)
  {
    /* analyze parameters for +CREG */
    if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
    {
      START_PARAM_LOOP()
      if (element_infos->param_rank == 2U)
      {
        /* param traced only */
        PRINT_DBG("+CREG: n=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                            element_infos->str_size))
      }
      if (element_infos->param_rank == 3U)
      {
        uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
        p_modem_ctxt->persist.cs_network_state = convert_NetworkState(stat, CS_NETWORK_TYPE);
        PRINT_DBG("+CREG: stat=%ld", stat)
      }
      if (element_infos->param_rank == 4U)
      {
        uint32_t lac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                     element_infos->str_size, LAC_TAC_SIZE);
        p_modem_ctxt->persist.cs_location_info.lac = (uint16_t)lac;
        PRINT_INFO("+CREG: lac=%ld =0x%lx", lac, lac)
      }
      if (element_infos->param_rank == 5U)
      {
        uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                    element_infos->str_size, CI_SIZE);
        p_modem_ctxt->persist.cs_location_info.ci = (uint32_t)ci;
        PRINT_INFO("+CREG: ci=%ld =0x%lx", ci, ci)
      }
      if (element_infos->param_rank == 6U)
      {
        /* param traced only */
        PRINT_DBG("+CREG: act=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      /* other parameters are not supported yet */
      END_PARAM_LOOP()
    }
    /* analyze parameters for +CREG */
    else if (p_atp_ctxt->current_atcmd.type == ATTYPE_TEST_CMD)
    {
      PRINT_DBG("+CREG for test cmd NOT IMPLEMENTED")
    }
    else
    {
      /* nothing to do */
    }

  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      p_modem_ctxt->persist.urc_avail_cs_network_registration = AT_TRUE;
      p_modem_ctxt->persist.cs_network_state = convert_NetworkState(stat, CS_NETWORK_TYPE);
      PRINT_DBG("+CREG URC: stat=%ld", stat)
    }
    if (element_infos->param_rank == 3U)
    {
      uint32_t lac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                   element_infos->str_size, LAC_TAC_SIZE);
      p_modem_ctxt->persist.urc_avail_cs_location_info_lac = AT_TRUE;
      p_modem_ctxt->persist.cs_location_info.lac = (uint16_t)lac;
      PRINT_INFO("+CREG URC: lac=%ld =0x%lx", lac, lac)
    }
    if (element_infos->param_rank == 4U)
    {
      uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size, CI_SIZE);
      p_modem_ctxt->persist.urc_avail_cs_location_info_ci = AT_TRUE;
      p_modem_ctxt->persist.cs_location_info.ci = (uint32_t)ci;
      PRINT_INFO("+CREG URC: ci=%ld =0x%lx", ci, ci)
    }
    if (element_infos->param_rank == 5U)
    {
      /* param traced only */
      PRINT_DBG("+CREG URC: act=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                          element_infos->str_size))
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGREG()")
  PRINT_DBG("current cmd = %ld", p_atp_ctxt->current_atcmd.id)

  /* analyze parameters for +CGREG
  *  Different cases to consider (as format is different)
  *  1/ answer to CGREG read command
  *     +CGREG: <n>,<stat>[,[<lac>],[<ci>],[<AcT>, [<rac>] [,<cause_type>,<reject_cause>]]]
  *  2/ answer to CGREG test command
  *     +CGREG: (list of supported <n>s)
  *  3/ URC:
  *     +CGREG: <stat>[,[<lac>],[<ci>],[<AcT>],[<rac>]]
  */
  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CGREG)
  {
    /* analyze parameters for +CREG */
    if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
    {
      START_PARAM_LOOP()
      if (element_infos->param_rank == 2U)
      {
        /* param traced only */
        PRINT_DBG("+CGREG: n=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      if (element_infos->param_rank == 3U)
      {
        uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
        p_modem_ctxt->persist.gprs_network_state = convert_NetworkState(stat, GPRS_NETWORK_TYPE);
        PRINT_DBG("+CGREG: stat=%ld", stat)
      }
      if (element_infos->param_rank == 4U)
      {
        uint32_t lac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                     element_infos->str_size, LAC_TAC_SIZE);
        p_modem_ctxt->persist.gprs_location_info.lac = (uint16_t)lac;
        PRINT_INFO("+CGREG: lac=%ld =0x%lx", lac, lac)
      }
      if (element_infos->param_rank == 5U)
      {
        uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                    element_infos->str_size, CI_SIZE);
        p_modem_ctxt->persist.gprs_location_info.ci = (uint32_t)ci;
        PRINT_INFO("+CGREG: ci=%ld =0x%lx", ci, ci)
      }
      if (element_infos->param_rank == 6U)
      {
        /* param traced only */
        PRINT_DBG("+CGREG: act=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      if (element_infos->param_rank == 7U)
      {
        /* param traced only */
        PRINT_DBG("+CGREG: rac=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      if (element_infos->param_rank == 8U)
      {
        /* param traced only */
        PRINT_DBG("+CGREG: cause_type=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      if (element_infos->param_rank == 9U)
      {
        /* param traced only */
        PRINT_DBG("+CGREG: reject_cause=%ld",
                  ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      if (element_infos->param_rank == 10U)
      {
        /* parameter present only if n=4 or 5 */
        /* param traced only */
        uint32_t active_time = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                             element_infos->str_size, 8);
        PRINT_INFO("+CGREG: active_time= %ld (=0x%lx)", active_time, active_time)
        /* TODO report to upper layers */
      }
      if (element_infos->param_rank == 11U)
      {
        /* parameter present only if n=4 or 5 */
        /* param traced only */
        uint32_t periodic_rau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                              element_infos->str_size, 8);
        PRINT_INFO("+CGREG: periodic_rau= %ld (=0x%lx)", periodic_rau, periodic_rau)
        /* TODO report to upper layers */
      }
      if (element_infos->param_rank == 12U)
      {
        /* parameter present only if n=4 or 5 */
        /* param traced only */
        uint32_t gprs_ready_timer = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                  element_infos->str_size, 8);
        PRINT_INFO("+CGREG: gprs_ready_timer= %ld (=0x%lx)", gprs_ready_timer, gprs_ready_timer)
        /* TODO report to upper layers */
      }
      END_PARAM_LOOP()
    }
    /* analyze parameters for +CGREG */
    else if (p_atp_ctxt->current_atcmd.type == ATTYPE_TEST_CMD)
    {
      PRINT_DBG("+CGREG for test cmd NOT IMPLEMENTED")
    }
    else
    {
      /* nothing to do */
    }
  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      p_modem_ctxt->persist.urc_avail_gprs_network_registration = AT_TRUE;
      p_modem_ctxt->persist.gprs_network_state = convert_NetworkState(stat, GPRS_NETWORK_TYPE);
      PRINT_DBG("+CGREG URC: stat=%ld", stat)
    }
    if (element_infos->param_rank == 3U)
    {
      uint32_t lac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                   element_infos->str_size, LAC_TAC_SIZE);
      p_modem_ctxt->persist.urc_avail_gprs_location_info_lac = AT_TRUE;
      p_modem_ctxt->persist.gprs_location_info.lac = (uint16_t)lac;
      PRINT_INFO("+CGREG URC: lac=%ld =0x%lx", lac, lac)
    }
    if (element_infos->param_rank == 4U)
    {
      uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size, CI_SIZE);
      p_modem_ctxt->persist.urc_avail_gprs_location_info_ci = AT_TRUE;
      p_modem_ctxt->persist.gprs_location_info.ci = (uint32_t)ci;
      PRINT_INFO("+CGREG URC: ci=%ld =0x%lx", ci, ci)
    }
    if (element_infos->param_rank == 5U)
    {
      /* param traced only */
      PRINT_DBG("+CGREG URC: act=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 6U)
    {
      /* param traced only */
      PRINT_DBG("+CGREG URC: rac=%ld",
                extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                              element_infos->str_size, RAC_SIZE))
    }
    if (element_infos->param_rank == 7U)
    {
      /* param traced only */
      PRINT_DBG("+CGREG URC: cause_type=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 8U)
    {
      /* param traced only */
      PRINT_DBG("+CGREG URC: reject_cause=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 9U)
    {
      /* param traced only */
      uint32_t active_time = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                           element_infos->str_size, 8);
      PRINT_INFO("+CGREG URC: active_time= %ld (=0x%lx)", active_time, active_time)
      /* TODO report to upper layers */
    }
    if (element_infos->param_rank == 10U)
    {
      /* param traced only */
      uint32_t periodic_rau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                            element_infos->str_size, 8);
      PRINT_INFO("+CGREG URC: periodic_rau= %ld (=0x%lx)", periodic_rau, periodic_rau)
      /* TODO report to upper layers */
    }
    if (element_infos->param_rank == 11U)
    {
      /* param traced only */
      uint32_t gprs_ready_timer = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 8);
      PRINT_INFO("+CGREG URC: gprs_ready_timer= %ld (=0x%lx)", gprs_ready_timer, gprs_ready_timer)
      /* TODO report to upper layers */
    }
    END_PARAM_LOOP()
  }

  return (retval);

}

at_action_rsp_t fRspAnalyze_CEREG(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CEREG()")
  PRINT_DBG("current cmd = %ld", p_atp_ctxt->current_atcmd.id)

  /* analyze parameters for +CEREG
  *  Different cases to consider (as format is different)
  *  1/ answer to CEREG read command
  *     +CEREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>[,<cause_type>,<reject_cause>]]]
  *  2/ answer to CEREG test command
  *     +CEREG: (list of supported <n>s)
  *  3/ URC:
  *     +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>]]
  */
  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CEREG)
  {
    /* analyze parameters for +CEREG */
    if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
    {
      uint32_t n_val = 0U;

      START_PARAM_LOOP()
      if (element_infos->param_rank == 2U)
      {
        /* <n> parameter */
        n_val = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size);
        PRINT_DBG("+CEREG: n=%ld", n_val)
      }
      if (element_infos->param_rank == 3U)
      {
        uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
        p_modem_ctxt->persist.eps_network_state = convert_NetworkState(stat, EPS_NETWORK_TYPE);
        PRINT_DBG("+CEREG: stat=%ld", stat)
      }

      if (element_infos->param_rank == 4U)
      {
        uint32_t tac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                     element_infos->str_size, LAC_TAC_SIZE);
        p_modem_ctxt->persist.eps_location_info.lac = (uint16_t)tac;
        PRINT_INFO("+CEREG: tac=%ld =0x%lx", tac, tac)
      }
      if (element_infos->param_rank == 5U)
      {
        uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                    element_infos->str_size, CI_SIZE);
        p_modem_ctxt->persist.eps_location_info.ci = (uint32_t)ci;
        PRINT_INFO("+CEREG: ci=%ld =0x%lx", ci, ci)
      }
      if (element_infos->param_rank == 6U)
      {
        /* param traced only */
        PRINT_INFO("+CEREG: act=%ld",
                   ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
      }
      /* for other parameters, two cases to consider:
       * n=(0,1,2 or 3) or n=(4 or 5)
       */
      if (n_val <= 3U)
      {
        if (element_infos->param_rank == 7U)
        {
          /* param traced only */
          PRINT_DBG("+CEREG: cause_type=%ld",
                    ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        }
        if (element_infos->param_rank == 8U)
        {
          /* param traced only */
          PRINT_DBG("+CEREG: reject_cause=%ld",
                    ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        }
      }
      else if ((n_val == 4U) || (n_val == 5U))
      {
        if (element_infos->param_rank == 7U)
        {
          /* param traced only */
          PRINT_DBG("+CEREG: rac=%ld",
                    extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size, RAC_SIZE))
        }
        if (element_infos->param_rank == 8U)
        {
          /* param traced only */
          PRINT_DBG("+CEREG: cause_type=%ld",
                    ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        }
        if (element_infos->param_rank == 9U)
        {
          /* param traced only */
          PRINT_DBG("+CEREG: reject_cause=%ld",
                    ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
        }
        if (element_infos->param_rank == 10U)
        {
          /* param traced only */
          uint32_t active_time = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                               element_infos->str_size, 8);
          PRINT_INFO("+CEREG: active_time= %ld (=0x%lx)", active_time, active_time)
          /* TODO report to upper layers */
        }
        if (element_infos->param_rank == 11U)
        {
          /* param traced only */
          uint32_t periodic_tau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 8);
          PRINT_INFO("+CEREG: periodic_tau= %ld (=0x%lx)", periodic_tau, periodic_tau)
          /* TODO report to upper layers */
        }
      }
      else
      { /* unexpecetd n value, ignore it */}
      END_PARAM_LOOP()
    }
    /* analyze parameters for +CEREG */
    else if (p_atp_ctxt->current_atcmd.type == ATTYPE_TEST_CMD)
    {
      PRINT_DBG("+CEREG for test cmd NOT IMPLEMENTED")
    }
    else
    {
      /* nothing to do */
    }
  }
  else
  {
    /* this is an URC */
    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t stat = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      p_modem_ctxt->persist.urc_avail_eps_network_registration = AT_TRUE;
      p_modem_ctxt->persist.eps_network_state = convert_NetworkState(stat, EPS_NETWORK_TYPE);
      PRINT_DBG("+CEREG URC: stat=%ld", stat)
    }
    if (element_infos->param_rank == 3U)
    {
      uint32_t tac = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                   element_infos->str_size, LAC_TAC_SIZE);
      p_modem_ctxt->persist.urc_avail_eps_location_info_tac = AT_TRUE;
      p_modem_ctxt->persist.eps_location_info.lac = (uint16_t)tac;
      PRINT_INFO("+CEREG URC: tac=%ld =0x%lx", tac, tac)
    }
    if (element_infos->param_rank == 4U)
    {
      uint32_t ci = extract_hex_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size, CI_SIZE);
      p_modem_ctxt->persist.urc_avail_eps_location_info_ci = AT_TRUE;
      p_modem_ctxt->persist.eps_location_info.ci = (uint32_t)ci;
      PRINT_INFO("+CEREG URC: ci=%ld =0x%lx", ci, ci)
    }
    if (element_infos->param_rank == 5U)
    {
      /* param traced only */
      PRINT_DBG("+CEREG URC: act=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 6U)
    {
      /* param traced only */
      PRINT_DBG("+CEREG URC: cause_type=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 7U)
    {
      /* param traced only */
      PRINT_DBG("+CEREG URC: reject_cause=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
    if (element_infos->param_rank == 8U)
    {
      /* param traced only */
      uint32_t active_time = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                           element_infos->str_size, 8);
      PRINT_INFO("+CEREG URC: active_time= %ld (=0x%lx)", active_time, active_time)
      /* TODO report to upper layers */
    }
    if (element_infos->param_rank == 9U)
    {
      /* param traced only */
      uint32_t periodic_tau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                            element_infos->str_size, 8);
      PRINT_INFO("+CEREG URC: periodic_tau= %ld (=0x%lx)", periodic_tau, periodic_tau)
      /* TODO report to upper layers */
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGEV(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                 const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGEV()")

  /* cf 3GPP TS 27.007 */
  /* this is an URC */
  START_PARAM_LOOP()
  if (element_infos->param_rank == 2U)
  {
    /* due to parser implementation (spaces are not considered as a split character) and the format of +CGEV,
    *  we can receive an additional paramater with the PDN event name in the 1st CGEV parameter.
    *  For example:
    *    +CGEV: NW DETACH                                 => no additionnal parameter
    *    +CGEV: NW PDN DEACT <cid>[,<WLAN_Offload>]       => <cid> will be present here
    *    +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]  => <PDP_type>  will be present here
    *
    * In consequence, a specific treatment is done here.

    * Here is a list of different events we can receive:
    *   +CGEV: NW DETACH
    *   +CGEV: ME DETACH
    *   +CGEV: NW CLASS <class>
    *   +CGEV: ME CLASS <class>
    *   +CGEV: NW PDN ACT <cid>[,<WLAN_Offload>]
    *   +CGEV: ME PDN ACT <cid>[,<reason>[,<cid_other>]][,<WLAN_Offload>]
    *   +CGEV: NW ACT <p_cid>, <cid>, <event_type>[,<WLAN_Offload>]
    *   +CGEV: ME ACT <p_cid>, <cid>, <event_type>[,<WLAN_Offload>]
    *   +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]
    *   +CGEV: ME DEACT <PDP_type>, <PDP_addr>, [<cid>]
    *   +CGEV: NW PDN DEACT <cid>[,<WLAN_Offload>]
    *   +CGEV: ME PDN DEACT <cid>
    *   +CGEV: NW DEACT <p_cid>, <cid>, <event_type>[,<WLAN_Offload>]
    *   +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>].
    *   +CGEV: ME DEACT <p_cid>, <cid>, <event_type>
    *   +CGEV: ME DEACT <PDP_type>, <PDP_addr>, [<cid>].
    *   +CGEV: NW MODIFY <cid>, <change_reason>, <event_type>[,<WLAN_Offload>]
    *   +CGEV: ME MODIFY <cid>, <change_reason>, <event_type>[,<WLAN_Offload>]
    *   +CGEV: REJECT <PDP_type>, <PDP_addr>
    *   +CGEV: NW REACT <PDP_type>, <PDP_addr>, [<cid>]
    *
    *  We are only interested by following events:
    *   +CGEV: NW DETACH : the network has forced a Packet domain detach (all contexts)
    *   +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>] : the nw has forced a contex deactivation
    *   +CGEV: NW PDN DEACT <cid>[,<WLAN_Offload>] : context deactivated
    */

    /* check that previous PDN URC has been reported
    *  if this is not the case, we can not report this one
    */
    if (p_modem_ctxt->persist.urc_avail_pdn_event != AT_TRUE)
    {
      /* reset event params */
      reset_pdn_event(&p_modem_ctxt->persist);

      /* create a copy of params */
      uint8_t copy_params[MAX_CGEV_PARAM_SIZE] = {0};
      AT_CHAR_t *found;
      size_t size_mini = ATC_GET_MINIMUM_SIZE(element_infos->str_size, MAX_CGEV_PARAM_SIZE);
      (void) memcpy((void *)copy_params,
                    (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                    size_mini);
      found = (AT_CHAR_t *)strtok((CRC_CHAR_t *)copy_params, " ");
      while (found  != NULL)
      {
        /* analyze of +CGEV event received */
        if (0 == strcmp((CRC_CHAR_t *)found, "NW"))
        {
          PRINT_DBG("<NW>")
          p_modem_ctxt->persist.pdn_event.event_origine = CGEV_EVENT_ORIGINE_NW;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "ME"))
        {
          PRINT_DBG("<ME>")
          p_modem_ctxt->persist.pdn_event.event_origine = CGEV_EVENT_ORIGINE_ME;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "PDN"))
        {
          PRINT_DBG("<PDN>")
          p_modem_ctxt->persist.pdn_event.event_scope = CGEV_EVENT_SCOPE_PDN;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "ACT"))
        {
          PRINT_DBG("<ACT>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_ACTIVATION;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "DEACT"))
        {
          PRINT_DBG("<DEACT>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_DEACTIVATION;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "REJECT"))
        {
          PRINT_DBG("<REJECT>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_REJECT;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "DETACH"))
        {
          PRINT_DBG("<DETACH>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_DETACH;
          /* all PDN are concerned */
          p_modem_ctxt->persist.pdn_event.conf_id = CS_PDN_ALL;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "CLASS"))
        {
          PRINT_DBG("<CLASS>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_CLASS;
        }
        else if (0 == strcmp((CRC_CHAR_t *)found, "MODIFY"))
        {
          PRINT_DBG("<MODIFY>")
          p_modem_ctxt->persist.pdn_event.event_type = CGEV_EVENT_TYPE_MODIFY;
        }
        else
        {
          /* if falling here, this is certainly an additional paramater (cf above explanation) */
          if (p_modem_ctxt->persist.pdn_event.event_origine == CGEV_EVENT_ORIGINE_NW)
          {
            if (p_modem_ctxt->persist.pdn_event.event_type == CGEV_EVENT_TYPE_DETACH)
            {
              /*  we are in the case:
              *  +CGEV: NW DETACH
              *  => no parameter to analyze
              */
              PRINT_ERR("No parameter expected for  NW DETACH")
            }
            else if (p_modem_ctxt->persist.pdn_event.event_type == CGEV_EVENT_TYPE_DEACTIVATION)
            {
              if (p_modem_ctxt->persist.pdn_event.event_scope == CGEV_EVENT_SCOPE_PDN)
              {
                /* we are in the case:
                *   +CGEV: NW PDN DEACT <cid>[,<WLAN_Offload>]
                *   => parameter to analyze = <cid>
                */
                uint32_t cgev_cid = ATutil_convertStringToInt((uint8_t *)found,
                                                              (uint16_t)strlen((CRC_CHAR_t *)found));
                p_modem_ctxt->persist.pdn_event.conf_id = atcm_get_configID_for_modem_cid(&p_modem_ctxt->persist,
                                                                                          (uint8_t)cgev_cid);
                PRINT_DBG("+CGEV modem cid=%ld (user conf Id =%d)", cgev_cid, p_modem_ctxt->persist.pdn_event.conf_id)
              }
              else
              {
                /* we are in the case:
                *   +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]
                *   => parameter to analyze = <PDP_type>
                *
                *   skip, parameter not needed in this case
                */
              }
            }
            else
            {
              PRINT_DBG("event type (= %d) ignored", p_modem_ctxt->persist.pdn_event.event_type)
            }
          }
          else
          {
            PRINT_DBG("ME events ignored")
          }
        }

        PRINT_DBG("(%d) ---> %s", strlen((CRC_CHAR_t *)found), (uint8_t *) found)
        found = (AT_CHAR_t *)strtok(NULL, " ");
      }

      /* Indicate that a +CGEV URC has been received */
      p_modem_ctxt->persist.urc_avail_pdn_event = AT_TRUE;
    }
    else
    {
      PRINT_ERR("an +CGEV URC still not reported, ignore this one")
      retval = ATACTION_RSP_ERROR;
    }

  }
  else if (element_infos->param_rank == 3U)
  {
    if ((p_modem_ctxt->persist.pdn_event.event_origine == CGEV_EVENT_ORIGINE_NW) &&
        (p_modem_ctxt->persist.pdn_event.event_type == CGEV_EVENT_TYPE_DEACTIVATION))
    {
      /* receive <PDP_addr> for:
      * +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]
      */

      /* analyze <IP_address> and try to find a matching user cid */
      csint_ip_addr_info_t  ip_addr_info;
      (void) memset((void *)&ip_addr_info, 0, sizeof(csint_ip_addr_info_t));

      /* recopy IP address value, ignore type */
      ip_addr_info.ip_addr_type = CS_IPAT_INVALID;
      (void) memcpy((void *) & (ip_addr_info.ip_addr_value),
                    (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                    (size_t) element_infos->str_size);
      PRINT_DBG("<PDP_addr>=%s", (AT_CHAR_t *)&ip_addr_info.ip_addr_value)

      /* find user cid matching this IP addr (if any) */
      p_modem_ctxt->persist.pdn_event.conf_id = find_user_cid_with_matching_ip_addr(&p_modem_ctxt->persist,
                                                                                    &ip_addr_info);

    }
    else
    {
      PRINT_DBG("+CGEV parameter rank %d ignored", element_infos->param_rank)
    }
  }
  else if (element_infos->param_rank == 4U)
  {
    if ((p_modem_ctxt->persist.pdn_event.event_origine == CGEV_EVENT_ORIGINE_NW) &&
        (p_modem_ctxt->persist.pdn_event.event_type == CGEV_EVENT_TYPE_DEACTIVATION))
    {
      /* receive <cid> for:
      * +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]
      */
      /* CID not used: we could use it if problem with <PDP_addr> occured */
    }
    else
    {
      PRINT_DBG("+CGEV parameter rank %d ignored", element_infos->param_rank)
    }
  }
  else
  {
    PRINT_DBG("+CGEV parameter rank %d ignored", element_infos->param_rank)
  }
  END_PARAM_LOOP()

  return (retval);
}

at_action_rsp_t fRspAnalyze_CSQ(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CSQ()")

  /* analyze parameters for CSQ */
  /* for EXECUTION COMMAND only  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    /* 3GP TS27.007
    *  format: +CSQ: <rssi>,<ber>
    *
    *  <rssi>: integer type
    *          0  -113dBm or less
    *          1  -111dBm
    *          2...30  -109dBm to -53dBm
    *          31  -51dBm or greater
    *          99  unknown or not detectable
    *  <ber>: integer type (channel bit error rate in percent)
    *          0...7  as RXQUAL values in the table 3GPP TS 45.008
    *          99     not known ot not detectable
    */

    START_PARAM_LOOP()
    if (element_infos->param_rank == 2U)
    {
      uint32_t rssi = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      PRINT_DBG("+CSQ rssi=%ld", rssi)
      p_modem_ctxt->SID_ctxt.signal_quality->rssi = (uint8_t)rssi;
    }
    if (element_infos->param_rank == 3U)
    {
      uint32_t ber = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                               element_infos->str_size);
      PRINT_DBG("+CSQ ber=%ld", ber)
      p_modem_ctxt->SID_ctxt.signal_quality->ber = (uint8_t)ber;
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CGPADDR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                    const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CGPADDR()")

  /* analyze parameters for CGPADDR */
  /* for WRITE COMMAND only  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_WRITE_CMD)
  {
    /* 3GP TS27.007
    *  format: +CGPADDR: <cid>[,<PDP_addr_1>[,>PDP_addr_2>]]]
    *
    *  <cid>: integer type
    *         specifies a particular PDP context definition
    *  <PDP_addr_1> and <PDP_addr_2>: string type
    *         format = a1.a2.a3.a4 for IPv4
    *         format = a1.a2.a3.a4.a5a.a6.a7.a8.a9.a10.a11.a12a.a13.a14.a15.a16 for IPv6
    */

    START_PARAM_LOOP()
    PRINT_DBG("+CGPADDR param_rank = %d", element_infos->param_rank)
    if (element_infos->param_rank == 2U)
    {
      uint32_t modem_cid = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                     element_infos->str_size);
      PRINT_DBG("+CGPADDR cid=%ld", modem_cid)
      p_modem_ctxt->CMD_ctxt.modem_cid = modem_cid;
    }
    else if ((element_infos->param_rank == 3U) || (element_infos->param_rank == 4U))
    {
      /* analyze <PDP_addr_1> and <PDP_addr_2> */
      csint_ip_addr_info_t  ip_addr_info;
      (void) memset((void *)&ip_addr_info, 0, sizeof(csint_ip_addr_info_t));

      /* retrive IP address value */
      (void) memcpy((void *) & (ip_addr_info.ip_addr_value),
                    (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                    (size_t) element_infos->str_size);
      PRINT_DBG("+CGPADDR addr=%s", (AT_CHAR_t *)&ip_addr_info.ip_addr_value)

      /* determine IP address type */
      ip_addr_info.ip_addr_type = atcm_get_ip_address_type((AT_CHAR_t *)&ip_addr_info.ip_addr_value);

      /* save IP address infos in modem_cid_table */
      if (element_infos->param_rank == 3U)
      {
        atcm_put_IP_address_infos(&p_modem_ctxt->persist, (uint8_t)p_modem_ctxt->CMD_ctxt.modem_cid, &ip_addr_info);
      }
    }
    else
    {
      /* parameters ignored */
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CPSMS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                  const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CPSMS()")

  /* analyze parameters for CPSMS */
  /* for READ COMMAND only  */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
  {
    /* 3GP TS27.007
    *  format: +CPSMS: <mode>,[<Requested_Periodic-RAU>],[<Requested_GPRS-READYtimer>],
                       [<Requested_Periodic-TAU>],[<Requested_Active-Time>]
    */

    START_PARAM_LOOP()
    PRINT_DBG("+CPSMS param_rank = %d", element_infos->param_rank)
    if (element_infos->param_rank == 2U)
    {
      uint32_t mode = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                element_infos->str_size);
      PRINT_INFO("+CPSMS: mode= %ld", mode)
      /* TODO report to upper layers */
    }
    else if (element_infos->param_rank == 3U)
    {
      uint32_t req_periodic_rau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 8);
      PRINT_INFO("+CPSMS: req_periodic_rau= %ld (=0x%lx)", req_periodic_rau, req_periodic_rau)
      /* TODO report to upper layers */
    }
    else if (element_infos->param_rank == 4U)
    {
      uint32_t req_gprs_ready_timer = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                    element_infos->str_size, 8);
      PRINT_INFO("+CPSMS: req_gprs_ready_timer= %ld (=0x%lx)", req_gprs_ready_timer, req_gprs_ready_timer)
      /* TODO report to upper layers */
    }
    else if (element_infos->param_rank == 5U)
    {
      uint32_t req_periodic_tau = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 8);
      PRINT_INFO("+CPSMS: req_periodic_tau= %ld (=0x%lx)", req_periodic_tau, req_periodic_tau)
      /* TODO report to upper layers */
    }
    else if (element_infos->param_rank == 6U)
    {
      uint32_t req_active_time = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                               element_infos->str_size, 8);
      PRINT_INFO("+CPSMS: req_active_time= %ld (=0x%lx)", req_active_time, req_active_time)
      /* TODO report to upper layers */
    }
    else
    {
      /* parameters ignored */
    }
    END_PARAM_LOOP()
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CEDRXS(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CEDRXS()")

  /* analyze parameters for CEDRXS */
  if (p_atp_ctxt->current_atcmd.id == (CMD_ID_t) CMD_AT_CEDRXS)
  {
    /* for READ COMMAND only  */
    if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
    {
      /* 3GP TS27.007
      *  eDRX URC
      *
      *  format: +CEDRXS: <AcT-type>,<Requested_eDRX_value>
      *                   [<CR><LF>+CEDRXS:<AcT-type>,<Requested_eDRX_value>[...]]]
      */

      START_PARAM_LOOP()
      PRINT_DBG("+CEDRXS param_rank = %d", element_infos->param_rank)
      if (element_infos->param_rank == 2U)
      {
        uint32_t act_type = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                      element_infos->str_size);
        PRINT_DBG("+CEDRXS: act_type= %ld", act_type)
        /* TODO report to upper layers */
      }
      else if (element_infos->param_rank == 3U)
      {
        uint32_t req_edrx_value = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 4);
        PRINT_INFO("+CEDRXS: req_edrx_value= %ld (=0x%lx)", req_edrx_value, req_edrx_value)
        /* TODO report to upper layers */
      }
      else
      {
        /* parameters ignored */
      }
      END_PARAM_LOOP()
    }
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_CEDRXP(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                   const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CEDRXP()")

  /* 3GP TS27.007
  *  format: +CEDRXP: <AcT-type>[,<Requested_eDRX_value>[,<NW-provided_eDRX_value>[,<Paging_time_window>]]]
  *
  */

  START_PARAM_LOOP()
  PRINT_DBG("+CEDRXS param_rank = %d", element_infos->param_rank)
  if (element_infos->param_rank == 2U)
  {
    uint32_t act_type = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
    PRINT_DBG("+CEDRXP URC: act_type= %ld", act_type)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 3U)
  {
    uint32_t req_edrx_value = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                            element_infos->str_size, 4);
    PRINT_INFO("+CEDRXP URC: req_edrx_value= %ld (=0x%lx)", req_edrx_value, req_edrx_value)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 4U)
  {
    uint32_t nw_provided_edrx_value =
      extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                    element_infos->str_size, 4);
    PRINT_INFO("+CEDRXP URC: nw_provided_edrx_value= %ld (=0x%lx)", nw_provided_edrx_value, nw_provided_edrx_value)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 5U)
  {
    uint32_t paging_time_window =
      extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                    element_infos->str_size, 4);
    PRINT_INFO("+CEDRXP URC: paging_time_window= %ld (=0x%lx)", paging_time_window, paging_time_window)
    /* TODO report to upper layers */
  }
  else
  {
    /* parameters ignored */
  }
  END_PARAM_LOOP()


  return (retval);
}

at_action_rsp_t fRspAnalyze_CEDRXRDP(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                     const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_CEDRXRDP()")

  /* 3GP TS27.007
  *  eDRX Read Dynamic Parameters
  *
  *  format: +CEDRXRDP: <AcT-type>[,<Requested_eDRX_value>[,<NW-provided_eDRX_value>[,<Paging_time_window>]]]
  *
  */

  START_PARAM_LOOP()
  PRINT_DBG("+CEDRXDP param_rank = %d", element_infos->param_rank)
  if (element_infos->param_rank == 2U)
  {
    uint32_t act_type = ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx],
                                                  element_infos->str_size);
    PRINT_DBG("+CEDRXRDP: act_type= %ld", act_type)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 3U)
  {
    uint32_t req_edrx_value = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                            element_infos->str_size, 4);
    PRINT_INFO("+CEDRXRDP: req_edrx_value= %ld (=0x%lx)", req_edrx_value, req_edrx_value)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 4U)
  {
    uint32_t nw_provided_edrx_value =
      extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                    element_infos->str_size, 4);
    PRINT_INFO("+CEDRXRDP: nw_provided_edrx_value= %ld (=0x%lx)", nw_provided_edrx_value, nw_provided_edrx_value)
    /* TODO report to upper layers */
  }
  else if (element_infos->param_rank == 5U)
  {
    uint32_t paging_time_window = extract_bin_value_from_quotes(&p_msg_in->buffer[element_infos->str_start_idx],
                                                                element_infos->str_size, 4);
    PRINT_INFO("+CEDRXRDP: paging_time_window= %ld (=0x%lx)", paging_time_window, paging_time_window)
    /* TODO report to upper layers */
  }
  else
  {
    /* parameters ignored */
  }
  END_PARAM_LOOP()


  return (retval);
}

/* ==========================  Analyze V.25ter commands ========================== */
at_action_rsp_t fRspAnalyze_GSN(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_GSN()")

  /* analyze parameters for +GSN */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_EXECUTION_CMD)
  {
    PRINT_DBG("IMEI:")
    PRINT_BUF((const uint8_t *)&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size)

    (void) memcpy((void *) & (p_modem_ctxt->SID_ctxt.device_info->u.imei),
                  (const void *)&p_msg_in->buffer[element_infos->str_start_idx],
                  (size_t) element_infos->str_size);
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_IPR(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_modem_ctxt);
#if (USE_TRACE_ATCUSTOM_MODEM == 0U)
  UNUSED(p_msg_in); /* for MISRA-2012 */
#endif /* USE_TRACE_ATCUSTOM_MODEM */

  atparser_context_t *p_atp_ctxt = &(p_at_ctxt->parser);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_IPR()")

  /* analyze parameters for +IPR */
  /* only for execution command, set parameters */
  if (p_atp_ctxt->current_atcmd.type == ATTYPE_READ_CMD)
  {
    PRINT_DBG("BAUD RATE:")
    if (element_infos->param_rank == 2U)
    {
      /* param trace only */
      PRINT_DBG("+IPR baud rate=%ld",
                ATutil_convertStringToInt(&p_msg_in->buffer[element_infos->str_start_idx], element_infos->str_size))
    }
  }

  return (retval);
}

at_action_rsp_t fRspAnalyze_DIRECT_CMD(at_context_t *p_at_ctxt, atcustom_modem_context_t *p_modem_ctxt,
                                       const IPC_RxMessage_t *p_msg_in, at_element_info_t *element_infos)
{
  UNUSED(p_at_ctxt);
  UNUSED(p_modem_ctxt);
  UNUSED(p_msg_in);
  UNUSED(element_infos);
  at_action_rsp_t retval = ATACTION_RSP_IGNORED;
  PRINT_API("enter fRspAnalyze_DIRECT_CMD()")

  /* NOT IMPLEMENTED YET */

  return (retval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
