/**
  ******************************************************************************
  * @file    ping_client.c
  * @author  MCD Application Team
  * @brief   Ping a remote host
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
#include <stdbool.h>

#include "ping_client.h"
#include "plf_config.h"
#include "ping_client_config.h"

#include "cmsis_os_misrac2012.h"

#include "dc_common.h"
#include "cellular_datacache.h"
#include "dc_control.h"
#include "cellular_runtime_custom.h"

#include "error_handler.h"
#include "setup.h"
#include "menu_utils.h"

#if (USE_CMD_CONSOLE == 1)
#include "cmd.h"
#endif  /* (USE_CMD_CONSOLE == 1) */

#include "com_sockets.h"

#if (USE_PING_CLIENT == 1)

/* Private defines -----------------------------------------------------------*/

/* Ping request number for each loop */
#define PING_NUMBER_MAX         10U

/* Ping period between each request in a loop of ping request */
#define PING_SEND_PERIOD       500U  /* in millisecond */

/* Ping receive timeout */
#define PING_RCV_TIMEO_SEC      10U  /* 10 seconds */

/* Ping remote host ip number */
#define PING_REMOTE_HOSTIP_NB    3U

/* Ping is send periodically automatically */
#define PING_LONG_DURATION_TEST  0U  /* 0: not activated
                                        1: activated */

/* Application default parameters number for setup menu */
#define PING_DEFAULT_PARAM_NB       2U

#if (USE_DEFAULT_SETUP == 0)
/* Application version for setup menu */
#define PING_SETUP_VERSION          (uint16_t)1
/* Application label for setup menu */
#define PING_LABEL                  ((uint8_t*)"Ping")
/* Application remote IP size max for setup menu */
/* SIZE_MAX for remote host IP - even if IPv6 not supported
   IPv4 : xxx.xxx.xxx.xxx=15+/0
   IPv6 : xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx = 39+/0*/
#define PING_REMOTE_IP_SIZE_MAX     40U
#endif /* USE_DEFAULT_SETUP */

#if (USE_CMD_CONSOLE == 1)
#define PING_INDEX_CMD  2U
#endif  /* (USE_CMD_CONSOLE == 1) */

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_PING_CLIENT == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_APP(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_PING, DBL_LVL_P0, "" format, ## args)
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_PING, DBL_LVL_P0, "" format, ## args)
#define PRINT_DBG(format, args...) \
  TRACE_PRINT(DBG_CHAN_PING, DBL_LVL_P1, "Ping: " format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_APP(format, args...)   (void)printf("" format, ## args);
#define PRINT_INFO(format, args...)  (void)printf("" format, ## args);
#define PRINT_DBG(format, args...)   (void)printf("\n\r Ping: " format "\n\r", ## args);
#endif  /* (USE_PRINTF == 0U) */
#else /* USE_TRACE_PING_CLIENT == 0 */
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_APP(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_PING, DBL_LVL_P0, "" format, ## args)
#else
#include <stdio.h>
#define PRINT_APP(format, args...)   (void)printf("" format, ## args);
#endif  /* (USE_PRINTF == 0U) */
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_PING_CLIENT */

/* Private variables ---------------------------------------------------------*/
static osMessageQId ping_client_queue;

/* State of Ping process */
static bool ping_process_flag; /* false: inactive,
                                  true: active
                                  default value : see ping_client_init() */
static bool ping_next_process_flag; /* false: inactive,
                                       true: active
                                       default value : ping_process_flag */

/* To Ping different remote host */
static uint8_t ping_remote_hostip_index;    /* 0: ping_remote_host_ip[0]
                                               1: ping_remote_host_ip[1]
                                               ... */

static uint32_t ping_remote_hostip[PING_REMOTE_HOSTIP_NB][4];

/* Number of Ping to do in each loop */
static uint8_t  ping_number;
/* Ping handle */
static int32_t  ping_handle;

/* State of Network connection */
static bool ping_network_is_on;

#if (PING_LONG_DURATION_TEST == 1U)
/* To follow the occurence number */
static uint16_t ping_occurence;
#endif /* PING_LONG_DURATION_TEST */

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Callback */
static void ping_client_notif_cb(dc_com_event_id_t dc_event_id,
                                 const void *private_gui_data);

static void ping_client_socket_thread(void const *argument);

static void ping_client_configuration(void);
#if (USE_CMD_CONSOLE == 1)
static void ping_client_cmd_help(void);
static cmd_status_t ping_cmd_processing(void);
static cmd_status_t ping_client_cmd(uint8_t *cmd_line_p);
#endif  /*  (USE_CMD_CONSOLE == 0) */

#if (USE_DEFAULT_SETUP == 0)
static uint32_t ping_client_setup_handler(void);
static void ping_client_setup_dump(void);
static void ping_client_setup_help(void);
#endif  /* (USE_DEFAULT_SETUP == 0) */

/* Private functions ---------------------------------------------------------*/
#if (USE_CMD_CONSOLE == 1)
/**
  * @brief  help cmd management
  * @param  -
  * @retval -
  */
static void ping_client_cmd_help(void)
{
  CMD_print_help((uint8_t *)"ping");
  PRINT_APP("ping help\n\r")
  PRINT_APP("ping                 : if no ping in progress,\n\r")
  PRINT_APP("                       start a 10 pings session to IP address pointed by Ping index\n\r")
  PRINT_APP("                       else stop the ping session and set Ping index to the next defined IP\n\r")
  PRINT_APP("ping ip1             : if no ping in progress, set Ping index to IP1 and start a 10 pings session\n\r")
  PRINT_APP("ping ip2             : if no ping in progress, set Ping index to IP2 and start a 10 pings session\n\r")
  PRINT_APP("ping ddd.ddd.ddd.ddd : if no ping in progress, \n\r")
  PRINT_APP("                       set Dynamic IP address to ddd.ddd.ddd.ddd, \n\r")
  PRINT_APP("                       set Ping index to Dynamic IP and start a 10 pings session\n\r")
  PRINT_APP("ping status          : display addresses for IP1, IP2, Dynamic IP, current Ping index and Ping state\n\r")
}

/**
  * @brief  ping cmd management
  * @param  -
  * @retval -
  */
static cmd_status_t ping_cmd_processing(void)
{
  cmd_status_t cmd_status ;
  cmd_status = CMD_OK;

  /* Can't wait the end of session Ping to return the command status
     because if so then cmd is blocked until the end of Ping session */
  if (ping_process_flag == false)
  {
    ping_next_process_flag = true;
    PRINT_APP("Ping Start requested ... \n\r")
  }
  else
  {
    if (ping_next_process_flag == false)
    {
      PRINT_APP("Ping Stop in progress ...\n\r")
    }
    else
    {
      ping_next_process_flag = false;
      PRINT_APP("Ping Stop requested ...\n\r")

      /* Increment ping_remote_hostip_index */
      ping_remote_hostip_index++;
      /* Check validity of ping_remote_hostip_index */
      if (ping_remote_hostip_index > (PING_REMOTE_HOSTIP_NB - 1U))
      {
        /* Loop at the beginning of the remote hostip array */
        ping_remote_hostip_index = 0U;
      }
      /* If Dynamic IP is undefined and index is on it */
      if ((ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0] == 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1] == 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2] == 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3] == 0U)
          && (ping_remote_hostip_index == (PING_REMOTE_HOSTIP_NB - 1U)))
      {
        /* Loop at the beginning of the remote hostip array */
        ping_remote_hostip_index = 0U;
      }
      if (ping_remote_hostip_index < (PING_REMOTE_HOSTIP_NB - 1U))
      {
        PRINT_APP("Ping index is now on: IP%d\n\r", (ping_remote_hostip_index + 1U))
      }
      else
      {
        PRINT_APP("Ping index is now on: dynamic IP\n\r")
      }
    }
  }
  return cmd_status;
}


/**
  * @brief  cmd management
  * @param  cmd_line_p - command parameters
  * @retval cmd_status_t - status of cmd management
  */
static cmd_status_t ping_client_cmd(uint8_t *cmd_line_p)
{
  uint32_t argc;
  uint8_t ip_addr[4];
  uint8_t  *argv_p[10];
  uint8_t  *cmd_p;
  cmd_status_t cmd_status ;
  cmd_status = CMD_OK;

  PRINT_APP("\n\r")
  cmd_p = (uint8_t *)strtok((CRC_CHAR_t *)cmd_line_p, " \t");

  if (memcmp((CRC_CHAR_t *)cmd_p,
             "ping",
             crs_strlen(cmd_p))
      == 0)
  {
    /* parameters parsing */
    for (argc = 0U; argc < 10U; argc++)
    {
      argv_p[argc] = (uint8_t *)strtok(NULL, " \t");
      if (argv_p[argc] == NULL)
      {
        break;
      }
    }

    if (argc == 0U)
    {
      cmd_status = ping_cmd_processing();
    }
    /*  1st parameter analysis */
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "help",
                    crs_strlen(argv_p[0])) == 0)
    {
      ping_client_cmd_help();
    }
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "ip1",
                    crs_strlen(argv_p[0])) == 0)
    {
      if (ping_process_flag == false)
      {
        ping_remote_hostip_index = 0U;
        cmd_status = ping_cmd_processing();
      }
      else
      {
        PRINT_APP("Stop or Wait end of current Ping session before to call this cmd\n\r")
      }
    }
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "ip2",
                    crs_strlen(argv_p[0])) == 0)
    {
      if (ping_process_flag == false)
      {
        ping_remote_hostip_index = 1U;
        cmd_status = ping_cmd_processing();
      }
      else
      {
        PRINT_APP("Stop or Wait end of current Ping session before to call this cmd\n\r")
      }
    }
    else if (memcmp((CRC_CHAR_t *)argv_p[0],
                    "status",
                    crs_strlen(argv_p[0])) == 0)
    {
      PRINT_APP("<<< Ping status >>>\n\r")
      for (uint8_t i = 0; i < (PING_REMOTE_HOSTIP_NB - 1U); i++)
      {
        PRINT_APP("IP%d: %ld.%ld.%ld.%ld\n\r",
                  i + 1U,
                  ping_remote_hostip[i][0],
                  ping_remote_hostip[i][1],
                  ping_remote_hostip[i][2],
                  ping_remote_hostip[i][3])
      }
      if ((ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0] != 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1] != 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2] != 0U)
          && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3] != 0U))
      {
        PRINT_APP("Dynamic IP: %ld.%ld.%ld.%ld\n\r",
                  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0],
                  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1],
                  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2],
                  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3])
      }
      else
      {
        PRINT_APP("Dynamic IP: undefined (use command 'ping ddd.ddd.ddd.ddd' to define/start it)\n\r")
      }
      if (ping_remote_hostip_index < (PING_REMOTE_HOSTIP_NB - 1U))
      {
        PRINT_APP("Ping index on: IP%d\n\r", (ping_remote_hostip_index + 1U))
      }
      else
      {
        PRINT_APP("Ping index on: dynamic IP\n\r")
      }
      if (ping_process_flag == true)
      {
        if (ping_next_process_flag == false)
        {
          PRINT_APP("A Ping session is in progress and stop has been requested\n\r")
        }
        else
        {
          PRINT_APP("A Ping session is in progress\n\r")
        }
      }
      else
      {
        if (ping_next_process_flag == true)
        {
          PRINT_APP("A Ping session start has been requested\n\r")
        }
        else
        {
          PRINT_APP("Ping is stopped\n\r")
        }
      }
    }
    else
    {
      if ((crc_get_ip_addr(argv_p[0], ip_addr, NULL) == 0U)
          && (ip_addr[0] != 0U)
          && (ip_addr[1] != 0U)
          && (ip_addr[2] != 0U)
          && (ip_addr[3] != 0U))
      {
        if (ping_process_flag == false)
        {
          ping_remote_hostip_index = PING_INDEX_CMD;
          for (uint8_t i = 0U; i < 4U; i++)
          {
            ping_remote_hostip[ping_remote_hostip_index][i] = ip_addr[i];
          }
          cmd_status = ping_cmd_processing();
        }
        else
        {
          PRINT_APP("Stop or Wait end of current Ping session before to call this cmd\n\r")
        }
      }
      else
      {
        PRINT_APP("%s bad parameter %s>>>\n\r", cmd_p, argv_p[0])
        cmd_status = CMD_SYNTAX_ERROR;
        ping_client_cmd_help();
      }
    }
  }
  return cmd_status;
}
#endif /* USE_CMD_CONSOLE */

/**
  * @brief  Configuration handler
  * @note   At initialization update ping client parameters by menu or default value
  * @param  -
  * @retval -
  */
static void ping_client_configuration(void)
{
  static uint8_t *ping_default_setup_table[PING_DEFAULT_PARAM_NB] =
  {
    PING_DEFAULT_REMOTE_HOSTIP1,
    PING_DEFAULT_REMOTE_HOSTIP2
  };

#if (USE_DEFAULT_SETUP == 0)
  (void)setup_record(SETUP_APPLI_PING_CLIENT,
                     PING_SETUP_VERSION,
                     PING_LABEL,
                     ping_client_setup_handler,
                     ping_client_setup_dump,
                     ping_client_setup_help,
                     ping_default_setup_table,
                     PING_DEFAULT_PARAM_NB);

#else /* (USE_DEFAULT_SETUP == 1) */
  bool ip_valid;
  uint8_t table_indice;
  uint32_t res;
  uint8_t ip_addr[4];

  table_indice = 0U;

  for (uint8_t i = 0U; i < (PING_REMOTE_HOSTIP_NB - 1U); i++)
  {
    ip_valid = false;

    /* Check validity IP address - Only IPv4 is managed */
    res = crc_get_ip_addr(ping_default_setup_table[table_indice], ip_addr, NULL);

    if (res == 0U)
    {
      if ((ip_addr[0] != 0U)
          && (ip_addr[1] != 0U)
          && (ip_addr[2] != 0U)
          && (ip_addr[3] != 0U))
      {
        ip_valid = true;
      }
    }
    if (ip_valid == false)
    {
      ip_addr[0] = 0U;
      ip_addr[1] = 0U;
      ip_addr[2] = 0U;
      ip_addr[3] = 0U;
    }

    for (uint8_t j = 0U; j < 4U; j++)
    {
      ping_remote_hostip[i][j] = ip_addr[j];
    }
    table_indice++;
  }
#endif /* (USE_DEFAULT_SETUP == 0) */
  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0] = 0U;
  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1] = 0U;
  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2] = 0U;
  ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3] = 0U;
}

#if (USE_DEFAULT_SETUP == 0)
/**
  * @brief  Setup handler
  * @note   At initialization update ping client parameters by menu
  * @param  -
  * @retval uint32_t - always 0
  */
static uint32_t ping_client_setup_handler(void)
{
  bool ip_valid;
  uint8_t i;
  uint8_t display_string[80];
  uint32_t res;
  uint8_t ip_addr[4];

  static uint8_t ping_remote_ip_string[PING_REMOTE_IP_SIZE_MAX];

  for (i = 0U; i < (PING_REMOTE_HOSTIP_NB - 1U); i++)
  {
    /* Request Remote host IPi */
    (void)sprintf((CRC_CHAR_t *)display_string,
                  "Remote host IP%d to ping(xxx.xxx.xxx.xxx)",
                  i + 1U);
    ip_valid = false;
    menu_utils_get_string(display_string,
                          ping_remote_ip_string,
                          PING_REMOTE_IP_SIZE_MAX);

    /* Check validity IP address - Only IPv4 is managed */
    res = crc_get_ip_addr(ping_remote_ip_string, ip_addr, NULL);
    if (res == 0U)
    {
      if ((ip_addr[0] != 0U)
          && (ip_addr[1] != 0U)
          && (ip_addr[2] != 0U)
          && (ip_addr[3] != 0U))
      {
        ip_valid = true;
        (void)sprintf((CRC_CHAR_t *)display_string,
                      "Remote host IP%d to ping: %d.%d.%d.%d\n\r",
                      i + 1U,
                      ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
        PRINT_SETUP("%s", display_string)
      }
    }
    if (ip_valid == false)
    {
      (void)sprintf((CRC_CHAR_t *)display_string,
                    "Remote host IP%d to ping: syntax NOK\n\r",
                    i + 1U);
      PRINT_SETUP("%s", display_string)
      ip_addr[0] = 0U;
      ip_addr[1] = 0U;
      ip_addr[2] = 0U;
      ip_addr[3] = 0U;
    }

    for (uint8_t j = 0U; j < 4U; j++)
    {
      ping_remote_hostip[i][j] = ip_addr[j];
    }
  }
  return 0U;
}

/**
  * @brief  Setup help
  * @param  -
  * @retval -
  */
static void ping_client_setup_help(void)
{
  PRINT_SETUP("\r\n")
  PRINT_SETUP("===================================\r\n")
  PRINT_SETUP("Ping configuration help\r\n")
  PRINT_SETUP("===================================\r\n")
  setup_version_help();
  PRINT_SETUP("------------------------\n\r")
  PRINT_SETUP(" Remote host IP1 to ping\n\r")
  PRINT_SETUP("------------------------\n\r")
  PRINT_SETUP("Default IP addr of remote host 1\n\r")
  PRINT_SETUP("Default value: (%s)\n\r", PING_DEFAULT_REMOTE_HOSTIP1)
  PRINT_SETUP("\n\r")
  PRINT_SETUP("------------------------\n\r")
  PRINT_SETUP(" Remote host IP2 to ping\n\r")
  PRINT_SETUP("------------------------\n\r")
  PRINT_SETUP("Default IP addr of remote host 2\n\r")
  PRINT_SETUP("Default value: (%s)\n\r", PING_DEFAULT_REMOTE_HOSTIP2)
  PRINT_SETUP("\n\r")
}

/**
  * @brief  Setup dump
  * @param  -
  * @retval -
  */
static void ping_client_setup_dump(void)
{
  uint8_t display_string[80];

  for (uint8_t i = 0U; i < (PING_REMOTE_HOSTIP_NB - 1U); i++)
  {
    if ((ping_remote_hostip[i][0] <= 255U)
        && (ping_remote_hostip[i][1] <= 255U)
        && (ping_remote_hostip[i][2] <= 255U)
        && (ping_remote_hostip[i][3] <= 255U))
    {
      (void)sprintf((CRC_CHAR_t *)display_string,
                    "Remote host IP%d to ping: %ld.%ld.%ld.%ld\n\r",
                    i + 1U,
                    ping_remote_hostip[i][0],
                    ping_remote_hostip[i][1],
                    ping_remote_hostip[i][2],
                    ping_remote_hostip[i][3]);
      PRINT_APP("%s", display_string)
    }
    else
    {
      (void)sprintf((CRC_CHAR_t *)display_string,
                    "Remote host IP%d to ping: syntax NOK\n\r",
                    i + 1U);
      PRINT_APP("%s", display_string)
    }
  }
}
#endif /* USE_DEFAULT_SETUP == 0 */

/**
  * @brief  Callback called when a value in datacache changed
  * @note   Managed datacache value changed
  * @param  dc_event_id - value changed
  * @note   -
  * @param  private_gui_data - value provided at service subscription
  * @note   Unused parameter
  * @retval -
  */
//static void ping_client_notif_cb(dc_com_event_id_t dc_event_id,
//                                 const void *private_gui_data)
//{
//  UNUSED(private_gui_data);
//
//  if (dc_event_id == DC_CELLULAR_NIFMAN_INFO)
//  {
//    dc_nifman_info_t  dc_nifman_info;
//    (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO,
//                      (void *)&dc_nifman_info,
//                      sizeof(dc_nifman_info));
//    if (dc_nifman_info.rt_state == DC_SERVICE_ON)
//    {
//      PRINT_APP("Ping: Network is up\n\r")
//      ping_network_is_on = true;
//      (void)osMessagePut(ping_client_queue, (uint32_t)dc_event_id, 0U);
//    }
//    else
//    {
//      PRINT_APP("Ping: Network is down\n\r")
//      ping_network_is_on = false;
//    }
//  }
//  else if (dc_event_id == DC_COM_BUTTON_DN)
//  {
//    if (ping_process_flag == false)
//    {
//      ping_next_process_flag = true;
//      PRINT_APP("Ping Start requested ... \n\r")
//    }
//    else
//    {
//      if (ping_next_process_flag == false)
//      {
//        PRINT_APP("Ping Stop in progress ...\n\r")
//      }
//      else
//      {
//        ping_next_process_flag = false;
//        PRINT_APP("Ping Stop requested ...\n\r")
//
//        /* Increment ping_remote_hostip_index */
//        ping_remote_hostip_index++;
//        /* Check validity of ping_remote_hostip_index */
//        if (ping_remote_hostip_index > (PING_REMOTE_HOSTIP_NB - 1U))
//        {
//          /* Loop at the beginning of the remote hostip array */
//          ping_remote_hostip_index = 0U;
//        }
//        /* If Dynamic IP is undefined and index is on it */
//        if ((ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0] == 0U)
//            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1] == 0U)
//            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2] == 0U)
//            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3] == 0U)
//            && (ping_remote_hostip_index == (PING_REMOTE_HOSTIP_NB - 1U)))
//        {
//          /* Loop at the beginning of the remote hostip array */
//          ping_remote_hostip_index = 0U;
//        }
//        if (ping_remote_hostip_index < (PING_REMOTE_HOSTIP_NB - 1U))
//        {
//          PRINT_APP("Ping index is now on: IP%d\n\r", (ping_remote_hostip_index + 1U))
//        }
//        else
//        {
//          PRINT_APP("Ping index is now on: dynamic IP\n\r")
//        }
//      }
//    }
//  }
//  else
//  {
//    /* Nothing to do */
//  }
//}


/**
  * @brief  Socket thread
  * @note   Infinite loop Ping client body
  * @param  argument - parameter osThread
  * @note   Unused parameter
  * @retval -
  */
static void ping_client_socket_thread(void const *argument)
{
  uint8_t  ping_counter;
  uint8_t  ping_index;
  /* Ping Statistic */
  uint8_t  ping_rsp_ok;
  uint32_t ping_rsp_min;
  uint32_t ping_rsp_max;
  uint32_t ping_rsp_tot;
  int32_t  result;
  com_ip_addr_t     ping_targetip;
  com_sockaddr_in_t ping_target;
  com_ping_rsp_t    ping_rsp;

  UNUSED(argument);

  for (;;)
  {
    /* Wait Network is up indication */
    (void)osMessageGet(ping_client_queue, RTOS_WAIT_FOREVER);

    (void)osDelay(1000U); /* just to let apps to show menu */

    /* Release Ping handle */
    if (ping_handle >= 0)
    {
      if (com_closeping(ping_handle) == COM_SOCKETS_ERR_OK)
      {
        ping_handle = COM_SOCKET_INVALID_ID;
      }
    }

    /* Re-init counter for next session */
    ping_counter = 0U;
    ping_rsp_min = 0xFFFFFFFFU;
    ping_rsp_max = 0U;
    ping_rsp_tot = 0U;
    ping_rsp_ok  = 0U;

    while (ping_network_is_on == true)
    {
      /* Update ping_process_flag value */
      ping_process_flag = ping_next_process_flag;

      if (ping_process_flag == false)
      {
        /* PING CLIENT NOT ACTIVE */
        /* Release Ping handle */
        if (ping_handle >= 0)
        {
          if (com_closeping(ping_handle) == COM_SOCKETS_ERR_OK)
          {
            ping_handle = COM_SOCKET_INVALID_ID;
          }
        }
        /* Re-init counter for next session */
        ping_counter = 0U;
        ping_rsp_min = 0xFFFFFFFFU;
        ping_rsp_max = 0U;
        ping_rsp_tot = 0U;
        ping_rsp_ok  = 0U;
        (void)osDelay(1000U);
#if (PING_LONG_DURATION_TEST == 1U)
        (void)osDelay(1000U);
        ping_occurence++;
        /* Restart automatically a Ping Sequence on next Remote HostIP */
        ping_process_flag = true;
        PRINT_APP("<<< Ping Start Auto %lu>>>\n\r",
                  ping_occurence)
        ping_remote_hostip_index++;
        /* Check validity of ping_remote_hostip_index */
        if (ping_remote_hostip_index > (PING_REMOTE_HOSTIP_NB - 1U))
        {
          /* Loop at the beginning of the remote hostip array */
          ping_remote_hostip_index = 0U;
        }
        /* If Dynamic IP is undefined and index is on it */
        if ((ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][0] == 0U)
            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][1] == 0U)
            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][2] == 0U)
            && (ping_remote_hostip[PING_REMOTE_HOSTIP_NB - 1U][3] == 0U)
            && (ping_remote_hostip_index == (PING_REMOTE_HOSTIP_NB - 1U)))
        {
          /* Loop at the beginning of the remote hostip array */
          ping_remote_hostip_index = 0U;
        }
#endif  /*  (PING_LONG_DURATION_TEST == 1U) */
      }
      else
      {
        ping_index = ping_remote_hostip_index;
        if (ping_counter == 0U)
        {
          PRINT_APP("\n\r<<< Ping Started on %ld.%ld.%ld.%ld>>>\n\r",
                    ping_remote_hostip[ping_index][0],
                    ping_remote_hostip[ping_index][1],
                    ping_remote_hostip[ping_index][2],
                    ping_remote_hostip[ping_index][3])
        }
        if (ping_counter < ping_number)
        {
          /* Create the ping */
          if (ping_handle < 0)
          {
            ping_handle = com_ping();
          }
          if (ping_handle >= 0)
          {
            PRINT_DBG("open OK")
            COM_IP4_ADDR(&ping_targetip,
                         ping_remote_hostip[ping_index][0],
                         ping_remote_hostip[ping_index][1],
                         ping_remote_hostip[ping_index][2],
                         ping_remote_hostip[ping_index][3]);

            ping_target.sin_family      = (uint8_t)COM_AF_INET;
            ping_target.sin_port        = (uint16_t)0U;
            ping_target.sin_addr.s_addr = ping_targetip.addr;
            ping_target.sin_len = (uint8_t)sizeof(com_sockaddr_in_t);

            result = com_ping_process(ping_handle,
                                      (const com_sockaddr_t *)&ping_target,
                                      (int32_t)ping_target.sin_len,
                                      PING_RCV_TIMEO_SEC, &ping_rsp);

            if ((result == COM_SOCKETS_ERR_OK)
                && (ping_rsp.status == COM_SOCKETS_ERR_OK))
            {
              PRINT_APP("Ping: %d bytes from %d.%d.%d.%d: seq=%.2d time= %ldms ttl=%d\n\r",
                        ping_rsp.size,
                        COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                        COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                        ping_counter + 1U,
                        ping_rsp.time,
                        ping_rsp.ttl)

              if (ping_rsp.time > ping_rsp_max)
              {
                ping_rsp_max = ping_rsp.time;
              }
              if (ping_rsp.time < ping_rsp_min)
              {
                ping_rsp_min = ping_rsp.time;
              }
              if (ping_rsp_tot < (0xFFFFFFFFU - ping_rsp.time))
              {
                ping_rsp_tot += ping_rsp.time;
              }
              else
              {
                ping_rsp_tot = 0xFFFFFFFFU;
              }
              ping_rsp_ok++;
            }
            else
            {
              if (result == COM_SOCKETS_ERR_TIMEOUT)
              {
                PRINT_APP("Ping: timeout from %d.%d.%d.%d: seq=%.2d\n\r",
                          COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                          COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                          ping_counter + 1U)
              }
              else
              {
                PRINT_APP("Ping: error from %d.%d.%d.%d: seq=%.2d\n\r",
                          COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                          COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                          ping_counter + 1U)
              }
            }
            ping_counter++;
            (void)osDelay(PING_SEND_PERIOD);
          }
          else
          {
            /* Ping handle not received */
            PRINT_INFO("Ping: low-level not ready - Wait before to try again\n\r")
            (void)osDelay(1000U);
          }
        }

        /* Display the result */
        if ((ping_counter == ping_number)               /* Session completed                  */
            || (((ping_next_process_flag == false)      /* Session stopped by user            */
                 || (ping_network_is_on == false))      /* Session stopped because no network */
                && (ping_counter > 0U)))                /* at least one Ping has been send    */
        {
          /* Display the result even if it is partial */
          if ((ping_rsp_ok  != 0U)
              && (ping_rsp_tot != 0xFFFFFFFFU))
          {
            PRINT_APP("--- %d.%d.%d.%d Ping Statistics ---\n\rPing: min/avg/max = %ld/%ld/%ld ms ok = %d/%d\n\r",
                      COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                      COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                      ping_rsp_min, ping_rsp_tot / ping_rsp_ok, ping_rsp_max,
                      ping_rsp_ok, ping_counter)

            TRACE_VALID("@valid@:ping:state:%d/%d\n\r", ping_rsp_ok, ping_counter)
          }
          else
          {
            if (ping_rsp_ok  == 0U)
            {
              PRINT_APP("--- %d.%d.%d.%d Ping Statistics ---\n\rPing: min/avg/max = 0/0/0 ms ok = 0/%d\n\r",
                        COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                        COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                        ping_counter)

              TRACE_VALID("@valid@:ping:state:0/%d\n\r", ping_counter)
            }
            else
            {
              PRINT_APP("--- %d.%d.%d.%d Ping Statistics ---\n\rPing: min/avg/max = %ld/Overrun/%ld ms ok = %d/%d\n\r",
                        COM_IP4_ADDR1(&ping_targetip), COM_IP4_ADDR2(&ping_targetip),
                        COM_IP4_ADDR3(&ping_targetip), COM_IP4_ADDR4(&ping_targetip),
                        ping_rsp_min, ping_rsp_max,
                        ping_rsp_ok, ping_counter)

              TRACE_VALID("@valid@:ping:state:%d/%d\n\r", ping_rsp_ok, ping_counter)
            }
          }
          if ((ping_counter == ping_number)
              && (ping_next_process_flag == true))
          {
            PRINT_APP("\n\r<<< Ping Completed >>>\n\r")
          }
          else
          {
            PRINT_APP("\n\r<<< Ping Stopped before the end >>>\n\r")
          }
          if (ping_handle >= 0)
          {
            if (com_closeping(ping_handle)
                == COM_SOCKETS_ERR_OK)
            {
              ping_handle = COM_SOCKET_INVALID_ID;
            }
          }
          ping_process_flag = false;
          ping_next_process_flag = false;
        }
      }
    }
  }
}

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialization
  * @note   Ping client initialization
  * @param  -
  * @retval -
  */
void pingclient_init(void)
{
  /* Configuration by menu or default value */
  ping_client_configuration();

  /* Ping initialization */
  ping_handle = COM_SOCKET_INVALID_ID;
  ping_remote_hostip_index  = 0U;
  ping_number = PING_NUMBER_MAX;

  /* Ping deactivated by default */
  ping_process_flag = false;
  ping_next_process_flag = ping_process_flag;

#if (PING_LONG_DURATION_TEST == 1U)
  /* Init Ping occurence number */
  ping_occurence = 0U;
#endif  /*  (PING_LONG_DURATION_TEST == 1U) */

  osMessageQDef(ping_client_queue, 1, uint32_t);
  ping_client_queue = osMessageCreate(osMessageQ(ping_client_queue), NULL);

  if (ping_client_queue == NULL)
  {
    ERROR_Handler(DBG_CHAN_PING, 2, ERROR_FATAL);
  }
}

/**
  * @brief  Start
  * @note   Ping client start
  * @param  -
  * @retval -
  */
void pingclient_start(void)
{
  static osThreadId pingClientTaskHandle;

  /* Registration to datacache - for Network On/Off and Button */
//  (void)dc_com_register_gen_event_cb(&dc_com_db, ping_client_notif_cb, (void *) NULL);

#if (USE_CMD_CONSOLE == 1)
  CMD_Declare((uint8_t *)"ping", ping_client_cmd, (uint8_t *)"ping commands");
#endif  /*  (USE_CMD_CONSOLE == 1) */

  /* Create Ping thread  */
  osThreadDef(pingClientTask,
              ping_client_socket_thread,
              PINGCLIENT_THREAD_PRIO, 0,
              USED_PINGCLIENT_THREAD_STACK_SIZE);
  pingClientTaskHandle = osThreadCreate(osThread(pingClientTask), NULL);

  if (pingClientTaskHandle == NULL)
  {
    ERROR_Handler(DBG_CHAN_PING, 3, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(pingClientTaskHandle,
                                             USED_PINGCLIENT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }
}


#endif /* USE_PING_CLIENT */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
