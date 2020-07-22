/**
  ******************************************************************************
  * @file    cellular_service_config.c
  * @author  MCD Application Team
  * @brief   This file defines functions for Cellular Service Config
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
#include <stdbool.h>

#include "plf_config.h"
#include "dc_common.h"
#include "at_util.h"
#include "cellular_datacache.h"
#include "cellular_service.h"
#include "cellular_service_os.h"
#include "cellular_service_config.h"
#include "cellular_service_task.h"
#include "error_handler.h"
#include "cellular_service_int.h"
#include "cellular_runtime_standard.h"
#include "cellular_runtime_custom.h"

#if (!USE_DEFAULT_SETUP == 1)
#include "menu_utils.h"
#include "setup.h"
#include "app_select.h"
#endif  /* (!USE_DEFAULT_SETUP == 1)*/

/* Private defines -----------------------------------------------------------*/
#define CST_SETUP_NFMC       (1)

#define CST_MODEM_POWER_ON_LABEL "Modem power on (without application)"

#define CST_MODEM_POLLING_PERIOD_DEFAULT 5000U

#define CST_LABEL "Cellular Service"
#define CST_VERSION_APPLI     (uint16_t)5 /* V5: Adding attachment timeout */

#define CST_TEMP_STRING_SIZE    32




/* Private macros ------------------------------------------------------------*/
#if (USE_PRINTF == 0U)
/* Trace macro definition */
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_CELLULAR_SERVICE, DBL_LVL_P0, "" format "", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)                (void)printf(format , ## args);
#endif   /* (USE_PRINTF == 0U)*/
#if (USE_TRACE_CELLULAR_SERVICE == 1U)
#if (USE_PRINTF == 0U)
#define PRINT_CELLULAR_SERVICE(format, args...)      TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, \
                                                                 DBL_LVL_P0, format, ## args)
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_CELLULAR_SERVICE, \
                                                                 DBL_LVL_ERR, "ERROR " format, ## args)
#else
#define PRINT_CELLULAR_SERVICE(format, args...)      (void)printf(format , ## args);
#define PRINT_CELLULAR_SERVICE_ERR(format, args...)  (void)printf(format , ## args);
#endif /* USE_PRINTF */
#else
#define PRINT_CELLULAR_SERVICE(...)         __NOP(); /* Nothing to do */
#define PRINT_CELLULAR_SERVICE_ERR(...)     __NOP(); /* Nothing to do */
#endif /* USE_TRACE_CELLULAR_SERVICE */

/* Private typedef -----------------------------------------------------------*/
/* order of setup default param in table */
enum
{
  CST_PARAM_SIM_SLOT           = 0,
  CST_PARAM_APN                = 1,
  CST_PARAM_CID                = 2,
  CST_PARAM_USERNAME           = 3,
  CST_PARAM_PASSWORD           = 4,
  CST_PARAM_TARGET_STATE       = 5,
  CST_PARAM_ATTACHMENT_TIMEOUT = 6,
  CST_PARAM_NFMC               = 7,
  CST_PARAM_NFMC_TEMPO         = 8
} ;


/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Default setup parameter table */
static uint8_t *CST_default_setup_table[CST_DEFAULT_PARAMA_NB] =
{
  CST_DEFAULT_SIM_SLOT_STRING,
  CST_DEFAULT_APN_STRING,
  CST_DEFAULT_CID_STRING,
  CST_DEFAULT_USERNAME_STRING,
  CST_DEFAULT_PASSWORD_STRING,
  CST_DEFAULT_TARGET_STATE_STRING,
  CST_DEFAULT_ATTACHMENT_TIMEOUT_STRING,
  CST_DEFAULT_NFMC_ACTIVATION_STRING,
  CST_DEFAULT_NFMC_TEMPO1_STRING,
  CST_DEFAULT_NFMC_TEMPO2_STRING,
  CST_DEFAULT_NFMC_TEMPO3_STRING,
  CST_DEFAULT_NFMC_TEMPO4_STRING,
  CST_DEFAULT_NFMC_TEMPO5_STRING,
  CST_DEFAULT_NFMC_TEMPO6_STRING,
  CST_DEFAULT_NFMC_TEMPO7_STRING
};

#if (USE_DEFAULT_SETUP == 1)
static dc_cs_sim_slot_type_t  cst_get_sim_socket_value(uint8_t sim_slot_value);
static void CST_local_setup_handler(void);
#endif  /* (USE_DEFAULT_SETUP == 1) */
static dc_cs_target_state_t  cst_get_target_state_value(uint8_t target_state_value);

/* Global variables ----------------------------------------------------------*/
uint8_t *CST_SimSlotName_p[3] =
{
  ((uint8_t *)"MODEM SOCKET"),
  ((uint8_t *)"MODEM EMBEDDED SIM"),
  ((uint8_t *)"STM32 EMBEDDED SIM")
};

/* Private function Definition -----------------------------------------------*/
/**
  * @brief  convert sim slot integer value to DC enum value
  * @param  sim_slot_value    - sim slot integer value
  * @retval dc_cs_sim_slot_type_t  - sim slot DC enum value
  */
static dc_cs_sim_slot_type_t  cst_get_sim_socket_value(uint8_t sim_slot_value)
{
  dc_cs_sim_slot_type_t enum_value;
  switch (sim_slot_value)
  {
    case DC_SIM_SLOT_MODEM_SOCKET:
    {
      enum_value = DC_SIM_SLOT_MODEM_SOCKET;
      break;
    }
    case DC_SIM_SLOT_MODEM_EMBEDDED_SIM:
    {
      enum_value = DC_SIM_SLOT_MODEM_EMBEDDED_SIM;
      break;
    }
    case DC_SIM_SLOT_STM32_EMBEDDED_SIM:
    {
      enum_value = DC_SIM_SLOT_STM32_EMBEDDED_SIM;
      break;
    }
    default:
    {
      enum_value = DC_SIM_SLOT_MODEM_SOCKET;
      break;
    }
  }
  return enum_value;
}

/**
  * @brief  convert modem target state integer value to enum value
  * @param  target_state_value    - modem target state integer value
  * @retval dc_cs_sim_slot_type_t  - modem target state enum value
  */
static dc_cs_target_state_t  cst_get_target_state_value(uint8_t target_state_value)
{
  dc_cs_target_state_t enum_value;
  switch (target_state_value)
  {
    case DC_TARGET_STATE_OFF:
    {
      enum_value = DC_TARGET_STATE_OFF;
      break;
    }
    case DC_TARGET_STATE_SIM_ONLY:
    {
      enum_value = DC_TARGET_STATE_SIM_ONLY;
      break;
    }
    case DC_TARGET_STATE_FULL:
    {
      enum_value = DC_TARGET_STATE_FULL;
      break;
    }
    default:
    {
      enum_value = DC_TARGET_STATE_FULL;
      break;
    }
  }
  return enum_value;
}

/**
  * @brief  convert cid integer value to enum value
  * @param  target_state_value    - cid integer value
  * @retval CS_PDN_conf_id_t  - cid enum value
  */
CS_PDN_conf_id_t  cst_get_cid_value(uint8_t cid_value)
{
  CS_PDN_conf_id_t enum_value;
  switch (cid_value)
  {
    case CS_PDN_PREDEF_CONFIG:
    {
      enum_value = CS_PDN_PREDEF_CONFIG;
      break;
    }
    case CS_PDN_USER_CONFIG_1:
    {
      enum_value = CS_PDN_USER_CONFIG_1;
      break;
    }
    case CS_PDN_USER_CONFIG_2:
    {
      enum_value = CS_PDN_USER_CONFIG_2;
      break;
    }
    case CS_PDN_USER_CONFIG_3:
    {
      enum_value = CS_PDN_USER_CONFIG_3;
      break;
    }
    case CS_PDN_USER_CONFIG_4:
    {
      enum_value = CS_PDN_USER_CONFIG_4;
      break;
    }
    case CS_PDN_USER_CONFIG_5:
    {
      enum_value = CS_PDN_USER_CONFIG_5;
      break;
    }
    case CS_PDN_CONFIG_DEFAULT:
    {
      enum_value = CS_PDN_CONFIG_DEFAULT;
      break;
    }
    case CS_PDN_NOT_DEFINED:
    {
      enum_value = CS_PDN_NOT_DEFINED;
      break;
    }
    case CS_PDN_ALL:
    {
      enum_value = CS_PDN_ALL;
      break;
    }
    default:
    {
      enum_value = CS_PDN_PREDEF_CONFIG;
      break;
    }

  }
  return enum_value;
}

#if (!USE_DEFAULT_SETUP == 1)
/**
  * @brief  setup menu help
  * @param  none
  * @retval none
  */
static void CST_setup_help(void)
{
  PRINT_SETUP("\r\n")
  PRINT_SETUP("===================================\r\n")
  PRINT_SETUP("Cellular Service configuration help\r\n")
  PRINT_SETUP("===================================\r\n")
  setup_version_help();
  PRINT_SETUP("-------------\n\r")
  PRINT_SETUP("Sim slot list\r\n")
  PRINT_SETUP("-------------\n\r")
  PRINT_SETUP("Sim slot list: specifies the list and order of SIM slots to use at boot time.\r\n")
  PRINT_SETUP("- 0: socket slot.\r\n")
  PRINT_SETUP("- 1: embedded SIM slot.\r\n")
  PRINT_SETUP("- 2: host SIM slot (not implemented).\r\n")
  PRINT_SETUP("The default value of Sim slot is '%s' (socket slot)\r\n", CST_DEFAULT_SIM_SLOT_STRING)
  PRINT_SETUP("\r\n")
  PRINT_SETUP("Example: If the list of SIM slots to use is:\r\n")
  PRINT_SETUP("first 'embedded SIM slot' then 'socket slot'\r\n")
  PRINT_SETUP("the value to set is '10' ('1' for 'embedded SIM slot' and '0' for 'socket slot'\r\n")
  PRINT_SETUP("\r\n")
  PRINT_SETUP("'APN' 'CID' 'username' and 'password' parameters are requested for each SIM slot used\r\n")
  PRINT_SETUP("\r\n")
  PRINT_SETUP("---\n\r")
  PRINT_SETUP("APN\r\n")
  PRINT_SETUP("---\n\r")
  PRINT_SETUP("APN to associated with the CID for the selected SIM slot\r\n")
  PRINT_SETUP("\r\n")
  PRINT_SETUP("---\n\r")
  PRINT_SETUP("CID\r\n")
  PRINT_SETUP("---\n\r")
  PRINT_SETUP("CID to associated with the APN for the selected SIM slot\r\n")
  PRINT_SETUP("The default value is '%s'\r\n", CST_DEFAULT_CID_STRING)
  PRINT_SETUP("\r\n")
  PRINT_SETUP("--------\n\r")
  PRINT_SETUP("username\r\n")
  PRINT_SETUP("--------\n\r")
  PRINT_SETUP("Username used for the selected SIM slot (optionnal)\r\n")
  PRINT_SETUP("\r\n")
  PRINT_SETUP("--------\n\r")
  PRINT_SETUP("password\r\n")
  PRINT_SETUP("--------\n\r")
  PRINT_SETUP("Password used for the selected SIM slot (optionnal)\r\n")
  PRINT_SETUP("\r\n")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("Modem Target state\r\n")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("Modem state to reach at initialisation\r\n")
  PRINT_SETUP("- 0: modem off\r\n")
  PRINT_SETUP("- 2: full cellular data enabled\r\n")
  PRINT_SETUP("The default value is '%s'\r\n", CST_DEFAULT_TARGET_STATE_STRING)
  PRINT_SETUP("\r\n")
  PRINT_SETUP("-------------------------\n\r")
  PRINT_SETUP("Modem attachment timeout\r\n")
  PRINT_SETUP("-------------------------\n\r")
  PRINT_SETUP("Maximum duration to be attached\r\n")
  PRINT_SETUP("The default value is '%s'\r\n", CST_DEFAULT_ATTACHMENT_TIMEOUT_STRING)
  PRINT_SETUP("\r\n")
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("NFMC activation\r\n")
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("Used for enabling of disabling the NFMC feature\r\n")
  PRINT_SETUP("- 0: NFMC feature disabled\r\n")
  PRINT_SETUP("- 1: NFMC feature enabled. Base-temporization parameters are used\r\n")
  PRINT_SETUP("The default value of NFMC activation is '%s'\r\n", CST_DEFAULT_NFMC_ACTIVATION_STRING)
  PRINT_SETUP("\r\n")
  PRINT_SETUP("The default values of the base-temporization parameters are:\r\n")
  PRINT_SETUP("NFMC value 1:   %s ms\r\n", CST_DEFAULT_NFMC_TEMPO1_STRING)
  PRINT_SETUP("NFMC value 2:  %s ms\r\n", CST_DEFAULT_NFMC_TEMPO2_STRING)
  PRINT_SETUP("NFMC value 3:  %s ms\r\n", CST_DEFAULT_NFMC_TEMPO3_STRING)
  PRINT_SETUP("NFMC value 4:  %s ms\r\n", CST_DEFAULT_NFMC_TEMPO4_STRING)
  PRINT_SETUP("NFMC value 5:  %s ms\r\n", CST_DEFAULT_NFMC_TEMPO5_STRING)
  PRINT_SETUP("NFMC value 6: %s ms\r\n", CST_DEFAULT_NFMC_TEMPO6_STRING)
  PRINT_SETUP("NFMC value 7: %s ms\r\n", CST_DEFAULT_NFMC_TEMPO7_STRING)
  PRINT_SETUP("\r\n")
}

/**
  * @brief  setup config menu handler
  * @param  none
  * @retval error code  - (0:OK)
  */
static uint32_t CST_setup_handler(void)
{
  static uint8_t CST_sim_string[DC_SIM_SLOT_NB + 1];
  static uint8_t CST_temp_string[CST_TEMP_STRING_SIZE];
  uint32_t i;
  uint32_t ret;
#if (CST_SETUP_NFMC == 1)
  uint32_t j;
  uint32_t count;
#endif /* CST_SETUP_NFMC == 1 */

  ret = 0U;
  uint32_t remaining_default_sim_slot_nb;
  uint32_t sim_slot_nb;
  dc_cs_sim_slot_type_t sim_slot_type;
  dc_cellular_params_t cellular_params;

  (void)memset((void *)&cellular_params, 0, sizeof(cellular_params));
  /* PDN mode set to activated */
  cellular_params.set_pdn_mode = 1U;
  (void)menu_utils_get_next_default_value(CST_temp_string, 0);

  /* Get the new SIM slot list */
  menu_utils_get_string((uint8_t *)"Sim Slot List (0: socket / 1: embedded sim) (possible values (0 1 or 01)",
                        (uint8_t *)CST_sim_string, sizeof(CST_sim_string));

  /* the possible SIM slot list are: '0' '1' and '01'  */
  if (((CST_sim_string[0]  != (uint8_t)'0') && (CST_sim_string[0] != (uint8_t)'1'))   /* first slot: '0' or '1'   */
      || ((CST_sim_string[1] !=  0U)  && (CST_sim_string[1] != (uint8_t)'1')))        /* second slot: none or '1' */
  {
    PRINT_FORCE("Bad 'SIM slot list'. Exit from config\r\n")
    ret = 1U;
    goto end;
  }

  /* the new number of SIM slot is equal to lenght of the SIM list string */
  sim_slot_nb = crs_strlen(CST_sim_string);

  /* the possible SIM slot list are: '0' '1' and '01'  */
  if ((sim_slot_nb > 2U)                                                           /* max 2 slots              */
      || (sim_slot_nb == 0U))                                                      /* min 1 slot               */
  {
    PRINT_FORCE("Bad 'SIM slot list'. Exit from config\r\n")
    ret = 1U;
    goto end;
  }

  /* the number of SIM slot in the default config is equal to lenght of the SIM list string */
  remaining_default_sim_slot_nb = crs_strlen(CST_temp_string);
  if (remaining_default_sim_slot_nb > 2U)
  {
    /* Should never occur (default configuration is wrong) => ignore default values */
    remaining_default_sim_slot_nb = 0U;
  }

  /* For each Sim slot in the list: get sim slot parameters (type - APN - CID - username - password) */
  for (i = 0U ; (i < sim_slot_nb) && (ret == 0U); i++)
  {
    sim_slot_type = cst_get_sim_socket_value(CST_sim_string[i] - (uint8_t)'0');
    PRINT_SETUP("Sim slot %ld (%s) config:\r\n", i, CST_SimSlotName_p[sim_slot_type])
    cellular_params.sim_slot[i].sim_slot_type = sim_slot_type;

    /* compare with the default numbers of SIM slot in configuration */
    if (remaining_default_sim_slot_nb == 0U)
    {
      /* no more SIM slot defined in default config */
      /*   => new sim slot added: no default values available in previous setup config */

      /* get APN (no default value available) */
      menu_utils_get_string_without_default_value((uint8_t *)"APN: ", cellular_params.sim_slot[i].apn, DC_MAX_SIZE_APN);

      /* get CID (no default value available) */
      menu_utils_get_string_without_default_value((uint8_t *)"CID (1-9): ", (uint8_t *)CST_temp_string,
                                                  sizeof(CST_temp_string));
      if ((CST_temp_string[0]  < (uint8_t)'1') || (CST_temp_string[0] > (uint8_t)'9'))
      {
        /* wrong CID (correct values in range 1-9) */
        PRINT_FORCE("Bad 'CID' value. Exit from config\r\n")
        ret = 1U;
      }
      else
      {
        /* get CID (values in range 1-9) */
        if (CST_temp_string[0] >= (uint8_t)'0')
        {
          cellular_params.sim_slot[i].cid = CST_temp_string[0] - (uint8_t)'0';
        }
        else
        {
          cellular_params.sim_slot[i].cid = 1U;
        }
        /* get username */
        menu_utils_get_string_without_default_value((uint8_t *)"username: ", cellular_params.sim_slot[i].username,
                                                    DC_CST_USERNAME_SIZE);
        /* get password */
        menu_utils_get_string_without_default_value((uint8_t *)"password: ", cellular_params.sim_slot[i].password,
                                                    DC_CST_PASSWORD_SIZE);
      }
    }
    else
    {
      /* SIM slot already defined in default config: get new value using previous values as default values */

      /* GET APN */
      menu_utils_get_string((uint8_t *)"APN ", cellular_params.sim_slot[i].apn, DC_MAX_SIZE_APN);

      /* get CID */
      menu_utils_get_string((uint8_t *)"CID (1-9)", (uint8_t *)CST_temp_string, sizeof(CST_temp_string));
      if ((CST_temp_string[0]  < (uint8_t)'1') || (CST_temp_string[0] > (uint8_t)'9'))
      {
        /* wrong CID (correct values in range 1-9 */
        PRINT_FORCE("Bad 'CID' value. Exit from config\r\n")
        ret = 1U;
      }
      else
      {
        /* correct CID (values in range 1-9) */
        if (CST_temp_string[0] >= (uint8_t)'0')
        {
          cellular_params.sim_slot[i].cid = CST_temp_string[0] - (uint8_t)'0';
        }
        else
        {
          cellular_params.sim_slot[i].cid = 1U;
        }
        /* get username */
        menu_utils_get_string((uint8_t *)"username ", cellular_params.sim_slot[i].username, DC_CST_USERNAME_SIZE);
        /* get password */
        menu_utils_get_string((uint8_t *)"password ", cellular_params.sim_slot[i].password, DC_CST_PASSWORD_SIZE);

        /* decrement the number of default sim slots */
        remaining_default_sim_slot_nb--;
        /* increment the number of new sim slots available */
        cellular_params.sim_slot_nb++;
      }
    }
  }

  /* tests if SIM slot list configuration is OK */
  if (ret != 0U)
  {
    /* wrong SIM slot configuration parameter: return with error */
    goto end;
  }

  if (remaining_default_sim_slot_nb != 0U)
  {
    /* one sim slot removed from default config: flush associated 4 next parameters values in previous configuration */
    menu_utils_flush_next_default_value(((int16_t)remaining_default_sim_slot_nb * 4));
  }

  /* get modem target state */
  menu_utils_get_string((uint8_t *)"cellular target state (0: modem off / 1: sim only / 2: full cellular data)",
                        (uint8_t *)CST_temp_string, sizeof(CST_temp_string));
  if ((CST_temp_string[0]  < (uint8_t)'0') || (CST_temp_string[0] > (uint8_t)'2'))
  {
    /* wrong configuration parameter: return with error */
    PRINT_FORCE("Bad 'target state' value. Exit from config\r\n")
    ret = 1U;
    goto end;
  }

  cellular_params.target_state = cst_get_target_state_value(CST_temp_string[0] - (uint8_t)'0');
  if (cellular_params.target_state == DC_TARGET_STATE_UNKNOWN)
  {
    /* wrong configuration parameter: return with error */
    PRINT_FORCE("Unknown target state. Exit from config\r\n")
    ret = 1U;
    goto end;
  }

  /* get attachement timeout */
  menu_utils_get_string((uint8_t *)"attachment timeout (ms) ",
                        (uint8_t *)CST_temp_string, sizeof(CST_temp_string));
  cellular_params.attachment_timeout = (uint32_t)crs_atoi(&CST_temp_string[0]);

#if (CST_SETUP_NFMC == 1)
  /* get NFMC activation value */
  menu_utils_get_string((uint8_t *)"NFMC activation (0: inactive / 1: active) ", (uint8_t *)CST_temp_string,
                        sizeof(CST_temp_string));
  if (CST_temp_string[0] == (uint8_t)'1')
  {
    /*  NFMC enabled => get NFMC tempo values */
    cellular_params.nfmc_active = 1U;
    for (j = 0; j < CST_NFMC_TEMPO_NB ; j++)
    {
      (void)sprintf((CRC_CHAR_t *)CST_temp_string, "NFMC value %ld  ", j + 1U);
      menu_utils_get_string(CST_temp_string, (uint8_t *)CST_temp_string, sizeof(CST_temp_string));
      cellular_params.nfmc_value[j] = (uint32_t)crs_atoi(CST_temp_string);
    }
  }
  else if (CST_temp_string[0] == (uint8_t)'0')
  {
    /*  NFMC disabled => keep unused NFMC tempo values in setup config for future eventual default value  */
    cellular_params.nfmc_active = 0U;
    for (j = 0; j < CST_NFMC_TEMPO_NB ; j++)
    {
      count = menu_utils_get_next_default_value(CST_temp_string, 1);
      if ((count == 0U) || (count > 10U))
      {
        break;
      }
      menu_utils_set_sefault_value((uint8_t *)CST_temp_string);
      cellular_params.nfmc_value[j] = (uint32_t)crs_atoi(CST_temp_string);
    }
  }
  else
  {
    /* wrong configuration parameter: return with error */
    PRINT_FORCE("Unknown 'NFMC activation' mode (0 or 1). Exit from config\r\n")
    ret = 1;
    goto end;
  }

#else  /* (CST_SETUP_NFMC == 0) */
  /* setup configuration menu disabled for NFMC: NFMC default values are set */
  cellular_params.nfmc_active = 1U;
  cellular_params.nfmc_value[0] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO1_STRING);
  cellular_params.nfmc_value[1] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO2_STRING);
  cellular_params.nfmc_value[2] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO3_STRING);
  cellular_params.nfmc_value[3] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO4_STRING);
  cellular_params.nfmc_value[4] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO5_STRING);
  cellular_params.nfmc_value[5] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO6_STRING);
  cellular_params.nfmc_value[6] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO7_STRING);
#endif /* (CST_SETUP_NFMC == 1) */

  /* cellular parameter data cache entry ON */
  cellular_params.rt_state = DC_SERVICE_ON;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_CONFIG, (void *)&cellular_params, sizeof(cellular_params));

end:
  return ret;
}

/**
  * @brief  setup config dump
  * @param  none
  * @retval none
  */
static void CST_setup_dump(void)
{
  uint32_t i;

  dc_cellular_params_t cellular_params;
  (void)dc_com_read(&dc_com_db,  DC_COM_CELLULAR_PARAM, (void *)&cellular_params, sizeof(cellular_params));

  PRINT_FORCE("\n\r")

  for (i = 0 ; i < cellular_params.sim_slot_nb ; i++)
  {
    PRINT_FORCE("Sim Slot %ld: %d (%s)\n\r", i, cellular_params.sim_slot[i].sim_slot_type,
                CST_SimSlotName_p[cellular_params.sim_slot[i].sim_slot_type]);
    PRINT_FORCE("APN: %s\n\r",  cellular_params.sim_slot[i].apn)
    PRINT_FORCE("CID: %d\n\r",  cellular_params.sim_slot[i].cid)
    PRINT_FORCE("username: %s\n\r",  cellular_params.sim_slot[i].username)
    PRINT_FORCE("password: %s\n\r",  cellular_params.sim_slot[i].password)
  }
  PRINT_FORCE("modem target_state: %d\n\r",  cellular_params.target_state)

  PRINT_FORCE("attachment timeout: %ld ms\n\r",  cellular_params.attachment_timeout)

#if (CST_SETUP_NFMC == 1)
  PRINT_FORCE("NFMC activation : %d\n\r",  cellular_params.nfmc_active)

  if (cellular_params.nfmc_active == 1U)
  {
    for (i = 0U; i < CST_NFMC_TEMPO_NB ; i++)
    {
      PRINT_FORCE("NFMC value %ld : %ld\n\r", i + 1U, cellular_params.nfmc_value[i])
    }
  }
#endif  /* (CST_SETUP_NFMC == 1) */
}

#else /* (!USE_DEFAULT_SETUP == 1) */

/**
  * @brief  default setup config menu handler
  * @note  used only if USE_DEFAULT_SETUP compilation flag is defined
  * @param  none
  * @retval none
  */
static void CST_local_setup_handler(void)
{
  const uint8_t *tmp_string;
  dc_cellular_params_t cellular_params;

  (void)memset((void *)&cellular_params, 0, sizeof(cellular_params));

  cellular_params.set_pdn_mode = 1U;
  cellular_params.sim_slot_nb  = 1U;

  /* SIM slot 0 parameters BEGIN*/
  tmp_string = CST_default_setup_table[CST_PARAM_SIM_SLOT];
  cellular_params.sim_slot[0].sim_slot_type = cst_get_sim_socket_value(tmp_string[0] - (uint8_t)'0');
  (void)memcpy((CRC_CHAR_t *)cellular_params.sim_slot[0].apn,
               (CRC_CHAR_t *)CST_default_setup_table[CST_PARAM_APN],
               crs_strlen(CST_default_setup_table[CST_PARAM_APN]) + 1U);

  tmp_string = CST_default_setup_table[CST_PARAM_CID];
  cellular_params.sim_slot[0].cid = cst_get_cid_value(tmp_string[0] - (uint8_t)'0');

  (void)memcpy((CRC_CHAR_t *)cellular_params.sim_slot[0].username,
               (CRC_CHAR_t *)CST_default_setup_table[CST_PARAM_USERNAME],
               crs_strlen(CST_default_setup_table[CST_PARAM_USERNAME]) + 1U);
  (void)memcpy((CRC_CHAR_t *)cellular_params.sim_slot[0].password,
               (CRC_CHAR_t *)CST_default_setup_table[CST_PARAM_PASSWORD],
               crs_strlen(CST_default_setup_table[CST_PARAM_PASSWORD]) + 1U);
  /* SIM slot 0 parameters END*/

  tmp_string = CST_default_setup_table[CST_PARAM_TARGET_STATE];
  cellular_params.target_state = cst_get_target_state_value(tmp_string[0] - (uint8_t)'0');

  tmp_string = CST_default_setup_table[CST_PARAM_ATTACHMENT_TIMEOUT];
  cellular_params.attachment_timeout = (uint32_t)crs_atoi(tmp_string);

#if (CST_SETUP_NFMC == 1)
  tmp_string = CST_default_setup_table[CST_PARAM_NFMC];
  if (tmp_string[0] == (uint8_t)'1')
  {
    cellular_params.nfmc_active = 1U;
  }
  else
  {
    cellular_params.nfmc_active = 0U;
  }

  for (uint32_t i = 0U; i < CST_NFMC_TEMPO_NB ; i++)
  {
    cellular_params.nfmc_value[i] = (uint32_t)crs_atoi(CST_default_setup_table[CST_PARAM_NFMC_TEMPO]);
  }
#else /* CST_SETUP_NFMC == 1 */
  cellular_params.nfmc_active = 1;
  cellular_params.nfmc_value[0] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO1_STRING);
  cellular_params.nfmc_value[1] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO2_STRING);
  cellular_params.nfmc_value[2] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO3_STRING);
  cellular_params.nfmc_value[3] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO4_STRING);
  cellular_params.nfmc_value[4] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO5_STRING);
  cellular_params.nfmc_value[5] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO6_STRING);
  cellular_params.nfmc_value[6] = (uint32_t)crs_atoi(CST_DEFAULT_NFMC_TEMPO7_STRING);
#endif /* (CST_SETUP_NFMC == 1) */
  cellular_params.rt_state = DC_SERVICE_ON;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_CONFIG, (void *)&cellular_params, sizeof(cellular_params));
}

#endif /* (!USE_DEFAULT_SETUP == 1) */

/* Public function Definition -----------------------------------------------*/
#if (!USE_DEFAULT_SETUP == 1)

/**
  * @brief  update setup apn config of active sim slot in flash
  * @param  apn_config   - new apn config
  * @retval active_slot  - active slot
  */
uint32_t CST_update_config_setup_handler(dc_apn_config_t *apn_config, dc_cs_sim_slot_type_t  active_slot)
{
  static uint8_t CST_sim_string[DC_SIM_SLOT_NB + 1];

  uint32_t i;
  uint32_t ret;
  uint32_t sim_slot_nb;
  dc_cs_sim_slot_type_t sim_slot_type;
  uint8_t str_cid[3];

  /* get current setup configuration */
  setup_set_config(SETUP_APPLI_CST, CST_VERSION_APPLI, CST_default_setup_table, CST_DEFAULT_PARAMA_NB);

  /* get SIM slot list */
  (void)menu_utils_get_next_default_value(CST_sim_string, 0);
  sim_slot_nb = (uint32_t)crs_strlen(CST_sim_string);
  if (sim_slot_nb > DC_SIM_SLOT_NB)
  {
    sim_slot_nb = 0U;
  }

  (void)menu_utils_copy_next_value();

  /* find active SIM slot in the list */
  for (i = 0U ; i < sim_slot_nb ; i++)
  {
    sim_slot_type = cst_get_sim_socket_value(CST_sim_string[i] - (uint8_t)'0');
    if (active_slot == sim_slot_type)
    {
      /* Active SIM slot found: replace associated APN config by the new config*/
      /* APN */
      (void)menu_utils_replace_next_value(apn_config->apn);

      /* CID */
      (void)crs_itoa((int32_t)apn_config->cid, str_cid, 10U);
      (void)menu_utils_replace_next_value(str_cid);

      /* USERNAME */
      (void)menu_utils_replace_next_value(apn_config->username);

      /* PASSWORD */
      (void)menu_utils_replace_next_value(apn_config->password);
    }
    else
    {
      /* Not the Active SIM: keep the current SIM slot config not modified */
      /* APN */
      (void)menu_utils_copy_next_value();

      /* CID */
      (void)menu_utils_copy_next_value();

      /* USERNAME */
      (void)menu_utils_copy_next_value();

      /* PASSWORD */
      (void)menu_utils_copy_next_value();
    }
  }

  /* parse until configuration end */
  while (menu_utils_copy_next_value() != 0U)
  {
  }

  /* store new coniguration in flash  */
  ret = setup_save_config_flash(SETUP_APPLI_CST, CST_VERSION_APPLI);

  return ret;
}
#endif /* (!USE_DEFAULT_SETUP == 1) */

/**
  * @brief  initialize the cellular configuration service
  * @note  this function is called by cellular service task init
  * @param  cellular_params   - cellular configuration
  * @retval eror code (O:OK)
  */
CS_Status_t CST_config_init(void)
{
#if (!USE_DEFAULT_SETUP == 1)
  /* add boot menu choice to boot modem only */
  (void)app_select_record((uint8_t *)CST_MODEM_POWER_ON_LABEL, APP_SELECT_CST_MODEM_START);

  /* record cellular service componenet to setup configuration  */
  (void)setup_record(SETUP_APPLI_CST, CST_VERSION_APPLI,
                     (uint8_t *)CST_LABEL, CST_setup_handler,
                     CST_setup_dump,
                     CST_setup_help,
                     CST_default_setup_table, CST_DEFAULT_PARAMA_NB);
#else
  /* In case on default setup (without menu) calls default configuration setting */
  CST_local_setup_handler();
#endif   /* (!USE_DEFAULT_SETUP == 1) */

  return CELLULAR_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
