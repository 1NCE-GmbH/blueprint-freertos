/**
  ******************************************************************************
  * @file    httpclient.c
  * @author  MCD Application Team
  * @brief   Example of http client to send infos to grovestreams cloud
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

#include "httpclient.h"
#include "plf_config.h"
#include "httpclient_config.h"

#include "cmsis_os_misrac2012.h"
#include "error_handler.h"

#include "com_sockets.h"
#include "board_leds.h"

#include "dc_common.h"
#include "cellular_datacache.h"
#if ((USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1))
#include "dc_mems.h"
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */
#if (USE_DC_GENERIC == 1)
#include "dc_generic.h"
#endif /* USE_DC_GENERIC == 1 */
#include "dc_control.h"
#if (USE_DC_EMUL == 1)
#include "dc_emul.h"
#endif /* USE_DC_EMUL == 1 */
#include "dc_time.h"

#include "setup.h"
#include "app_select.h"
#include "menu_utils.h"

#include "time_date.h"
#include "cellular_runtime_custom.h"

#if (USE_CMD_CONSOLE == 1)
#include "cmd.h"
#endif /* USE_CMD_CONSOLE == 1 */

/* Private defines -----------------------------------------------------------*/
#if (USE_TRACE_HTTP_CLIENT == 1U)

#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_HTTP, DBL_LVL_P0, format "\n\r", ## args)
#define PRINT_INFO_MENU(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_HTTP, DBL_LVL_P0, format "\n\r", ## args)
#define PRINT_INFO(format, args...) \
  TRACE_PRINT(DBG_CHAN_HTTP, DBL_LVL_P0, "HTTP: " format "\n\r", ## args)
#define PRINT_DBG(format, args...) \
  TRACE_PRINT(DBG_CHAN_HTTP, DBL_LVL_P1, "HTTP: " format "\n\r", ## args)
#define PRINT_ERR(format, args...) \
  TRACE_PRINT(DBG_CHAN_HTTP, DBL_LVL_ERR, "HTTP ERROR: " format "\n\r", ## args)

#else /* USE_PRINTF == 1 */
#include <stdio.h>
#define PRINT_FORCE(format, args...)      (void)printf(format "\n\r", ## args);
#define PRINT_INFO_MENU(format, args...)  (void)printf(format "\n\r", ## args);
#define PRINT_INFO(format, args...)       (void)printf("HTTP: " format "\n\r", ## args);
/*#define PRINT_DBG(format, args...)        (void)printf("HTTP: " format "\n\r", ## args);*/
/* To reduce trace PRINT_DBG is deactivated when using printf */
#define PRINT_DBG(...)                     __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)        (void)printf("HTTP ERROR: " format "\n\r", ## args);

#endif /* USE_PRINTF == 0U */

#else /* USE_TRACE_HTTP_CLIENT == 0 */
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_HTTP, DBL_LVL_P0, format "\n\r", ## args)
#define PRINT_INFO_MENU(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_HTTP, DBL_LVL_P0, format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)      (void)printf(format "\n\r", ## args);
#define PRINT_INFO_MENU(format, args...)  (void)printf(format "\n\r", ## args);
#endif /* USE_PRINTF == 0U */
#define PRINT_INFO(...)      __NOP(); /* Nothing to do */
#define PRINT_DBG(...)       __NOP(); /* Nothing to do */
#define PRINT_ERR(...)       __NOP(); /* Nothing to do */

#endif /* USE_TRACE_HTTP_CLIENT == 1U */


#define HTTP_CLIENT_SETUP_VERSION      (uint16_t)2

#define HTTP_CLIENT_DEFAULT_LOCALPORT  ((uint16_t)0U)

/*  Modem reset feature
if:
- the feature is activated (http_client_modem_reset set to true)
- NFMC is deactivated (setup configuration)
- there are too many consecutives errors (errors > HTTP_CLIENT_ERROR_SHORT_LIMIT_MAX)
error take into account : connect, send, read failure
then :
a Modem off/on is ordered
*/
#define HTTP_CLIENT_DEFAULT_MODEM_RESET   ((bool)false)
/* #define HTTP_CLIENT_DEFAULT_MODEM_RESET   ((bool)true) */

/* Error counter implementation : nb of consecutive errors before to start
   NFMC feature or Modem reset */
#define HTTP_CLIENT_ERROR_SHORT_LIMIT_MAX        5U

#define DNS_SERVER_NAME        ((uint8_t*)"gandi.net")
#define DNS_SERVER_IP          ((uint32_t)0x41B946D9U)   /* 217.70.185.65 */
#define DNS_SERVER_PORT        ((uint16_t)443U)

#define CLOUD_SERVER_NAME      ((uint8_t*)"liveobjects.orange-business.com")
#define CLOUD_SERVER_IP        ((uint32_t)0xD02A2754U)   /*  84.39.42.208 */
#define CLOUD_SERVER_PORT      ((uint16_t)8883U)

#define ST_UNKNOWN_NAME        ((uint8_t*)"stunknown.st")
#define ST_UNKNOWN_IP          ((uint32_t)0x9B36379AU)   /* 154.55.54.155 */
#define ST_UNKNOWN_PORT        ((uint16_t)1234U)

#define NAME_SIZE_MAX          (uint8_t)(64U) /* MAX_SIZE_IPADDR of cellular_service.h */
#define HTTP_ENABLED_SIZE_MAX  (uint8_t)(2U)


#define HTTPCLIENT_LABEL  ((uint8_t*)"Grovestreams")


#define HTTPCLIENT_DEFAULT_PARAM_NB 21U

#define HTTPCLIENT_PUT_BATLEVEL_CHANNEL_ID         0U
#define HTTPCLIENT_PUT_SIGLEVEL_CHANNEL_ID         1U
#define HTTPCLIENT_PUT_ICCID_CHANNEL_ID            2U
#define HTTPCLIENT_PUT_IMSI_CHANNEL_ID             3U
#define HTTPCLIENT_PUT_HUM_CHANNEL_ID              4U
#define HTTPCLIENT_PUT_TEMP_CHANNEL_ID             5U
#define HTTPCLIENT_PUT_PRESS_CHANNEL_ID            6U
#define HTTPCLIENT_PUT_ACCX_CHANNEL_ID             7U
#define HTTPCLIENT_PUT_ACCY_CHANNEL_ID             8U
#define HTTPCLIENT_PUT_ACCZ_CHANNEL_ID             9U
#define HTTPCLIENT_PUT_GYRX_CHANNEL_ID            10U
#define HTTPCLIENT_PUT_GYRY_CHANNEL_ID            11U
#define HTTPCLIENT_PUT_GYRZ_CHANNEL_ID            12U
#define HTTPCLIENT_PUT_MAGX_CHANNEL_ID            13U
#define HTTPCLIENT_PUT_MAGY_CHANNEL_ID            14U
#define HTTPCLIENT_PUT_MAGZ_CHANNEL_ID            15U
#if (USE_DC_GENERIC == 1)
#define HTTPCLIENT_PUT_GEN_BOOL1_CHANNEL_ID       16U
#define HTTPCLIENT_PUT_GEN_BOOL2_CHANNEL_ID       17U
#define HTTPCLIENT_PUT_GEN_BYTE1_CHANNEL_ID       18U
#define HTTPCLIENT_PUT_GEN_BYTE2_CHANNEL_ID       19U
#define HTTPCLIENT_PUT_GEN_LONG1_CHANNEL_ID       20U
#define HTTPCLIENT_PUT_GEN_LONG2_CHANNEL_ID       21U
#define HTTPCLIENT_PUT_GEN_FLOAT1_CHANNEL_ID      22U
#define HTTPCLIENT_PUT_GEN_FLOAT2_CHANNEL_ID      23U
#define PUT_CHANNEL_ID_MAX  ((uint8_t)24U)
#else
#define PUT_CHANNEL_ID_MAX  ((uint8_t)16U)
#endif /* USE_DC_GENERIC == 1 */

#define HTTPCLIENT_GET_LEDLIGHT_CHANNEL_ID        0U
#if (USE_DC_GENERIC == 1)
#define HTTPCLIENT_GET_GEN_BOOL1_CHANNEL_ID       1U
#define HTTPCLIENT_GET_GEN_BOOL2_CHANNEL_ID       2U
#define HTTPCLIENT_GET_GEN_BYTE1_CHANNEL_ID       3U
#define HTTPCLIENT_GET_GEN_BYTE2_CHANNEL_ID       4U
#define HTTPCLIENT_GET_GEN_LONG1_CHANNEL_ID       5U
#define HTTPCLIENT_GET_GEN_LONG2_CHANNEL_ID       6U
#define HTTPCLIENT_GET_GEN_FLOAT1_CHANNEL_ID      7U
#define HTTPCLIENT_GET_GEN_FLOAT2_CHANNEL_ID      8U
#define GET_CHANNEL_ID_MAX  ((uint8_t)9U)
#else
#define GET_CHANNEL_ID_MAX  ((uint8_t)2U)
#endif /* USE_DC_GENERIC == 1 */

#define HTTPCLIENT_SEND_PERIOD        1000U
#define CLIENT_MESSAGE_SIZE           1500U
#define KEY_GET_INPUT_SIZE              50U
#define KEY_PUT_INPUT_SIZE              KEY_GET_INPUT_SIZE
#define CHANNEL_ID_SIZE_MAX             15U
#define COMPONENT_ID_STRING_SIZE_MAX    15U
#define SELECT_INPUT_STRING_SIZE         5U

#define PUT_REQUEST_PERIOD_SIZE_MAX      5U
#define GET_REQUEST_PERIOD_SIZE_MAX      5U

#define PUT_DEFAULT_CHANNEL_ID_NUMBER    4U
#define GET_DEFAULT_CHANNEL_ID_NUMBER    2U

#define RCV_SND_TIMEOUT              10000U /* Timeout to send/receive data */

/* Duration max Socket with Remote Server maintained open in sec */
/* To take into 2G Network latency */
#define SOCKET_OPEN_DURATION_MAX        15U

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  SOCKET_INVALID = 0,
  SOCKET_CREATED,
  SOCKET_CONNECTED,
  SOCKET_SENDING,
  SOCKET_WAITING_RSP,
  SOCKET_CLOSING
} socket_state_t;

typedef struct
{
  com_char_t *hostname;
  com_ip_addr_t hostip;
  uint16_t      hostport;
} dns_resolver_t;

typedef struct
{
  uint32_t count;
  uint32_t cnt_ok;
  uint32_t cnt_ko;
  uint32_t snd_ok;
  uint32_t snd_ko;
  uint32_t rcv_ok;
  uint32_t rcv_ko;
  uint32_t cls_ok;
  uint32_t cls_ko;
} httpclient_stat_t;

/* Private macro -------------------------------------------------------------*/
#define HTTPCLIENT_MIN(a,b) (((a) < (b))  ? (a) : (b))
#define HTTPCLIENT_MAX(a,b) (((a) >= (b)) ? (a) : (b))

/* Private variables ---------------------------------------------------------*/
/* static osMessageQId http_client_queue; */

static com_ip_addr_t httpclient_distantip;
static uint16_t      httpclient_distantport;
static com_char_t    httpclient_distantname[NAME_SIZE_MAX];

static bool socket_closing;
static socket_state_t socket_state;
static int32_t socket_http_client;

static bool http_client_modem_reset_in_progress;
static bool http_client_modem_restart_in_progress;
static bool http_client_modem_reset;

/* input strings from console */
static uint8_t key_get_input[KEY_GET_INPUT_SIZE];
static uint8_t key_put_input[KEY_PUT_INPUT_SIZE];
static uint8_t component_id_string[COMPONENT_ID_STRING_SIZE_MAX];
static uint8_t put_channel_id_string_tab[PUT_CHANNEL_ID_MAX][CHANNEL_ID_SIZE_MAX];
static uint8_t get_channel_id_string_tab[GET_CHANNEL_ID_MAX][CHANNEL_ID_SIZE_MAX];

static com_char_t httpclient_tmp_buffer[CLIENT_MESSAGE_SIZE];

static com_ip_addr_t httpclient_localip_addr;

/* FORMATTED http GET or PUT request  */
static com_char_t formatted_http_request[400U];
static com_char_t formatted_channel_http_request[200U];

/* partial http PUT and GET request definition */
static com_char_t *header2       = (com_char_t *)"Host:www.grovestreams.com\r\n";
static com_char_t *header3       = (com_char_t *)"Content-type:application/json\r\n";
static com_char_t *header4;
/*
static com_char_t *header4       = "Connection:close\r\nContent-Type:application/json\r\nContent-Length:0\r\n";
*/
static com_char_t *header5       = (com_char_t *)"Content-Type:application/json\r\nContent-Length:0\r\n";
static com_char_t *header6       = (com_char_t *)"Cookie:api_key=";
static com_char_t *header_end    = (com_char_t *)"\r\n";

static bool http_client_set_date_time;

static uint32_t http_client_put_period;
static uint32_t http_client_get_period;

/* Error counter implementation */
/* limit nb of errors before to activate nfmc or reset modem */
static uint8_t http_client_nb_error_short_limit;
/* current nb of errors */
static uint8_t http_client_nb_error_short;
/* sleep timer index in the NFMC array */
static uint8_t http_client_nfmc_tempo_index;

/* State of HTTP_CLIENT process */
static bool http_client_process_flag; /* false: inactive,
                                         true: active */

/* State of Network connection */
static bool network_is_on;

/* HTTP client statistic */
static httpclient_stat_t httpclient_stat;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Callback */
static void http_client_notif_cb(dc_com_event_id_t dc_event_id,
                                 const void *private_gui_data);

static bool http_client_dns_resolver(uint8_t  *hoststring,
                                     uint8_t  *hostname,
                                     uint16_t *hostport,
                                     com_ip_addr_t *hostip);

/* Action over the network is not authorized
if :
- there are too many consecutives errors (errors > HTTP_CLIENT_ERROR_SHORT_LIMIT_MAX)
error take into account : connect, send, read failure
and :
(- NFMC is activated (setup configuration)
or :
- Reset Modem is activated (http_client_modem_reset set to true))
*/
static bool http_client_is_action_authorized(void);

static void http_client_put_request_format(void);
static void http_client_check_distantname(void);
static void http_client_create_socket(void);
static bool http_client_receive_a_full_trame(com_char_t *buf_rsp);
static bool http_client_process(com_char_t *buf_snd,
                                com_char_t *buf_rsp);
static void http_client_get_process(void);
static void http_client_put_process(void);
static bool http_client_get_nfmc_parameters(uint8_t index,
                                            uint32_t *value);
static void http_client_socket_thread(void const *argument);
static bool http_client_get_request(uint32_t channel_id);
static uint8_t *http_client_find_string(const uint8_t *string,
                                        const uint8_t *string_to_find);

static void http_client_configuration(void);

#if (USE_DEFAULT_SETUP == 0)
static uint32_t http_client_setup_handler(void);
static void http_client_setup_dump(void);
static void http_client_setup_help(void);
static uint32_t http_client_sensor_list_config(const com_char_t **channelIdName,
                                               uint8_t (*channelIdStringInput)[CHANNEL_ID_SIZE_MAX],
                                               uint8_t channel_id_max,
                                               uint8_t channel_id_size_max);
#endif /* USE_DEFAULT_SETUP == 0 */

#if (USE_CMD_CONSOLE == 1)
static void http_client_cmd_help(void);
static cmd_status_t http_client_cmd(uint8_t *cmd_line_p);
#endif /* USE_CMD_CONSOLE == 1 */

#if (USE_DC_GENERIC == 1)
static uint32_t httpclient_find_uint32_value(uint8_t *buf,
                                             const uint8_t *str_find,
                                             uint32_t str_find_len,
                                             uint32_t *data_value);
#endif /* USE_DC_GENERIC == 1 */

/* Private functions ---------------------------------------------------------*/

#if (USE_DC_GENERIC == 1)
/**
  * @brief  find data value in a string
  * @param  buf - string
  * @param  str_find - string to find
  * @param  str_find_len - string length to find
  * @param  (out) data_value - value returned
  * @retval uint32_t ret==0: value not found - ret!=0: value found
  */
static uint32_t httpclient_find_uint32_value(uint8_t *buf,
                                             const uint8_t *str_find,
                                             uint32_t str_find_len,
                                             uint32_t *data_value)
{
  uint32_t i;
  uint32_t j;
  uint32_t offset;
  uint32_t buf_len;

  offset = 0;
  buf_len      = crs_strlen(buf);

  if (buf_len > str_find_len)
  {
    for (i = 0U ; i < (buf_len - str_find_len) ; i++)
    {
      if (buf[i] == str_find[0])
      {
        for (j = 1U ; j < str_find_len ; j++)
        {
          if (buf[i + j] != str_find[j])
          {
            break;
          }
        }

        if (j == str_find_len)
        {
          offset = i + j;
          break;
        }
      }
    }
  }

  if (offset != 0U)
  {
    *data_value = (uint32_t)crs_atoi(&buf[offset]);
  }
  else
  {
    *data_value = 0U;
  }
  return offset;
}
#endif /* USE_DC_GENERIC == 1 */

#if (USE_CMD_CONSOLE == 1)
/**
  * @brief  help cmd management
  * @param  -
  * @retval -
  */
static void http_client_cmd_help(void)
{
  CMD_print_help((uint8_t *)"httpclient");
  PRINT_FORCE("httpclient help")
  PRINT_FORCE("httpclient on  (enable http client process)")
  PRINT_FORCE("httpclient off (disable http client process)")
  PRINT_FORCE("httpclient stat (display transfer statistics)")
  PRINT_FORCE("httpclient resetstat (statistic reset)")
}

/**
  * @brief  cmd management
  * @param  cmd_line_p - command parameters
  * @retval cmd_status_t - status of cmd management
  */
static cmd_status_t http_client_cmd(uint8_t *cmd_line_p)
{
  uint32_t argc;
  uint8_t  *argv_p[10];
  const uint8_t *cmd_p;
  cmd_status_t cmd_status ;
  cmd_status = CMD_OK;
  uint32_t total_ko;

  cmd_p = (uint8_t *)strtok((CRC_CHAR_t *)cmd_line_p, " \t");

  if (cmd_p != NULL)
  {
    if (memcmp((const CRC_CHAR_t *)cmd_p,
               "httpclient",
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
        http_client_cmd_help();
      }

      /*  1st parameter analysis */
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "help",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        http_client_cmd_help();
      }
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "on",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        http_client_process_flag = true;
        PRINT_FORCE("\n\r<<< HTTPCLIENT ACTIVE >>>")
      }
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "off",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        http_client_process_flag = false;
        PRINT_FORCE("\n\r<<< HTTPCLIENT NOT ACTIVE >>>")
      }
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "valid",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        /* automatical test */
        if (memcmp((CRC_CHAR_t *)argv_p[1],
                   "stat",
                   crs_strlen(argv_p[1]))
            == 0)
        {
          total_ko =  httpclient_stat.cnt_ko
                      + httpclient_stat.snd_ko
                      + httpclient_stat.rcv_ko
                      + httpclient_stat.cls_ko;

          TRACE_VALID("@valid@:http:stat:%ld/%ld\n\r", httpclient_stat.rcv_ok, total_ko)
        }
      }
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "stat",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        PRINT_FORCE("\n\rHTTPCLIENT Statistics\r\n")
        total_ko =  httpclient_stat.cnt_ko
                    + httpclient_stat.snd_ko
                    + httpclient_stat.rcv_ko
                    + httpclient_stat.cls_ko;

        PRINT_FORCE("test count:   %ld", httpclient_stat.count)
        PRINT_FORCE("ok count:     %ld", httpclient_stat.rcv_ok)
        PRINT_FORCE("ko count:     %ld\r\n", total_ko)

        PRINT_FORCE("connect ok:   %ld", httpclient_stat.cnt_ok)
        PRINT_FORCE("connect ko:   %ld", httpclient_stat.cnt_ko)
        PRINT_FORCE("send ok:      %ld", httpclient_stat.snd_ok)
        PRINT_FORCE("send ko:      %ld", httpclient_stat.snd_ko)
        PRINT_FORCE("recv ok:      %ld", httpclient_stat.rcv_ok)
        PRINT_FORCE("recv ko:      %ld", httpclient_stat.rcv_ko)
        PRINT_FORCE("close ok:     %ld", httpclient_stat.cls_ok)
        PRINT_FORCE("close ko:     %ld", httpclient_stat.cls_ko)
      }
      else if (memcmp((CRC_CHAR_t *)argv_p[0],
                      "resetstats",
                      crs_strlen(argv_p[0]))
               == 0)
      {
        (void)memset((void *)&httpclient_stat, 0, sizeof(httpclient_stat));

        PRINT_FORCE("Httpclient statistics reseted")
      }
      else
      {
        PRINT_FORCE("\n\r HTTPCLIENT: Unrecognized command. Usage:")
        http_client_cmd_help();
      }
    }
    else
    {
      PRINT_FORCE("\n\r HTTPCLIENT: Unrecognized command. Usage:")
      http_client_cmd_help();
    }
  }
  return cmd_status;
}
#endif /* USE_CMD_CONSOLE == 1 */

/**
  * @brief  Callback called when a value in datacache changed
  * @note   Managed datacache value changed
  * @param  dc_event_id - value changed
  * @note   -
  * @param  private_gui_data - value provided at service subscription
  * @note   Unused parameter
  * @retval -
  */
static void http_client_notif_cb(dc_com_event_id_t dc_event_id,
                                 const void *private_gui_data)
{
  UNUSED(private_gui_data);

  if (dc_event_id == DC_CELLULAR_NIFMAN_INFO)
  {
    dc_nifman_info_t  dc_nifman_info;
    (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO,
                      (void *)&dc_nifman_info,
                      sizeof(dc_nifman_info));

    if (dc_nifman_info.rt_state == DC_SERVICE_ON)
    {
      (void)memcpy((void *)&httpclient_localip_addr,
                   (void *)&dc_nifman_info.ip_addr,
                   sizeof(com_ip_addr_t));
      network_is_on = true;

      /*      (void)osMessagePut(http_client_queue, (uint32_t)dc_event_id, 0U);*/
    }
    else if (dc_nifman_info.rt_state == DC_SERVICE_SHUTTING_DOWN)
    {
      /* Modem shutdown in progress :
         if a socket was opened mark it has closing */
      if (socket_state != SOCKET_INVALID)
      {
        socket_closing = true;
      }
      network_is_on = false;
    }
    else
    {
      network_is_on = false;
    }
  }
  else if (dc_event_id == DC_CELLULAR_INFO)
  {
    dc_cellular_info_t dc_cellular_info;

    (void)dc_com_read(&dc_com_db,
                      DC_CELLULAR_INFO,
                      (void *)&dc_cellular_info,
                      sizeof(dc_cellular_info));
    if (dc_cellular_info.modem_state == DC_MODEM_STATE_OFF)
    {
      if (http_client_modem_reset_in_progress == true)
      {
        http_client_modem_reset_in_progress = false;
        PRINT_INFO("Reset Modem in progress => Modem is off, will ask for a restart")
        http_client_modem_restart_in_progress = true;
      }
    }
  }
  else if (dc_event_id == DC_COM_BUTTON_UP)
  {
    if (http_client_process_flag == false)
    {
      http_client_process_flag = true;
      PRINT_FORCE("\n\r<<< HTTPCLIENT ACTIVE >>>")
    }
    else
    {
      http_client_process_flag = false;
      PRINT_FORCE("\n\r<<< HTTPCLIENT NOT ACTIVE >>>")
    }
  }
  else
  {
    /* Nothing to do */
  }
}


/**
  * @brief  search a substring in a string
  * @param  string         - initial string
  * @param  string_to_find - string to find
  * @note   -
  * @retval uint8_t* pointer on the start of string
  *         or NULL if string not find
  */
static uint8_t *http_client_find_string(const uint8_t *string,
                                        const uint8_t *string_to_find)
{
  return ((uint8_t *)strstr((const CRC_CHAR_t *)string,
                            (const CRC_CHAR_t *)string_to_find));
}

/**
  * @brief  try to match a substring in a string
  * @note
  * @param  string    - initial string
  * @param  str_begin - header
  * @param  str_end   - match end
  * @param  str_result - string returned
  * @param  str_result_len - lenght of returned string
  * @note   -
  * @retval uint32_t
  */
static uint32_t http_client_match_string2(uint8_t *string, uint8_t *str_result,
                                          uint32_t *str_result_len)
{
  uint32_t i;
  uint32_t res;

  res = 1;

  for (i = 0U ; (i < *str_result_len) && (res != 0U) ; i++)
  {
    if (string[i] == (uint8_t)' ')
    {
      res = 0U;
      str_result[i] = 0U;
      *str_result_len = i + 1U;
    }
    else if (string[i] == 0U)
    {
      res = 0U;
      str_result[i] = 0U;
      *str_result_len = 0U;
    }
    else
    {
      str_result[i] = string[i];
    }
  }

  return res;
}


/**
  * @brief  Internal DNS resolver
  * @note   Used to separate remote host information (URL from port)
            and to complete information (remote port) if not provided
  * @param  hoststring - string containing url and port
  * @note   -
  * @param  hostname - string that will contain host name
  * @note   only set
  * @param  hostport - string that will contain host port
  * @note   -
  * @param  hostip - string that will contain host IP
  * @note   -
  * @retval bool - false : semantic is NOK
                   true  : semantic is OK
  */
static bool http_client_dns_resolver(uint8_t  *hoststring,
                                     uint8_t  *hostname,
                                     uint16_t *hostport,
                                     com_ip_addr_t *hostip)
{
  bool result;
  uint8_t i;
  uint8_t begin;
  uint8_t dns_resolver_size;
  uint8_t ip_string[NAME_SIZE_MAX];
  uint32_t res;
  uint32_t str_result_len;
  uint32_t ip_port;

  static const dns_resolver_t dns_resolver[] =
  {
#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 1)
    { HTTP_SERVER_TEST_NAME, {HTTP_SERVER_TEST_IP},  HTTP_SERVER_TEST_PORT },
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 1 */
    { DNS_SERVER_NAME,       {DNS_SERVER_IP},        DNS_SERVER_PORT },
    { CLOUD_SERVER_NAME,     {CLOUD_SERVER_IP},      CLOUD_SERVER_PORT },
    { ST_UNKNOWN_NAME,       {ST_UNKNOWN_IP},        ST_UNKNOWN_PORT }
  };

  i = 0U;
  dns_resolver_size = (uint8_t)(sizeof(dns_resolver) / sizeof(dns_resolver_t));
  result = false;

  /* Separate the NAME from the Port */
  str_result_len = sizeof(ip_string);
  res = http_client_match_string2(hoststring, ip_string, &str_result_len);
  ip_port = 0U;
  if (str_result_len != 0U)
  {
    ip_port = (uint32_t)crs_atoi(&hoststring[str_result_len]);
    if (ip_port > 65535U)
    {
      ip_port = 0U;
    }
  }

  if (res == 0U)
  {
    /* Check NAME known ? */
    if (memcmp((CRC_CHAR_t *)hoststring, "www.", 4U)
        == 0)
    {
      begin = 4U;
    }
    else
    {
      begin = 0U;
    }

    while ((i < dns_resolver_size)
           && (result == false))
    {
      if (crs_strlen((const uint8_t *)&ip_string[begin])
          == crs_strlen(&dns_resolver[i].hostname[0]))
      {
        if (memcmp((CRC_CHAR_t *)&ip_string[begin],
                   (const CRC_CHAR_t *)&dns_resolver[i].hostname[0],
                   crs_strlen(&dns_resolver[i].hostname[0]))
            == 0)
        {
          /* If port is set, it overwrites the preconfigured value */
          if (ip_port != 0U)
          {
            *hostport = (uint16_t)ip_port;
          }
          else
          {
            *hostport = (uint16_t)dns_resolver[i].hostport;
          }
          hostip->addr = dns_resolver[i].hostip.addr;
          /* NAME without port is copy in hostname */
          /* Sure that length is ok for next copy */
          (void)strcpy((CRC_CHAR_t *)hostname,
                       (const CRC_CHAR_t *)ip_string);
          result = true;
        }
        else
        {
          i++;
        }
      }
      else
      {
        i++;
      }
    }
    /* Internal DNS resolver didn't find the NAME */
    if (result == false)
    {
      uint32_t ip_string_len = crs_strlen(&ip_string[0]);
      /* Copy NAME without port in hostname */

      (void)memcpy(hostname,
                   ip_string,
                   HTTPCLIENT_MIN(ip_string_len,
                                  ((uint32_t)(NAME_SIZE_MAX))));
      if ((ip_port != 0U)
          && (ip_port < 65535U))
      {
        *hostport = (uint16_t)ip_port;
      }
      else
      {
        /* Put the default distant port value */
        *hostport = HTTPCLIENT_DEFAULT_DISTANTPORT;
      }
      hostip->addr = 0U;
      result  = true;
    }
  }

  return result;
}

/**
  * @brief  Check if PUT/GET - interaction with modem is authorized
  * @note   NFMC sleep or Modem reset could be requested
  * @param  -
  * @retval bool - false/true Interaction blocked/authorized
  */
static bool http_client_is_action_authorized(void)
{
  bool result;

  if (http_client_nb_error_short >= http_client_nb_error_short_limit)
  {
    result = false;
  }
  else
  {
    result = true;
  }

  return result;
}

/**
  * @brief  PUT request format
  * @note   Format the PUT message
  * @param  -
  * @retval -
  */
static void http_client_put_request_format(void)
{
  uint8_t i;
  dc_cellular_info_t      cellular_info;
  dc_sim_info_t           sim_info;
#if (USE_DC_EMUL == 1)
  dc_battery_info_t       battery_info;
#endif /* USE_DC_EMUL == 1 */
#if ((USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1))
  dc_pressure_info_t      pressure_info;
  dc_humidity_info_t      humidity_info;
  dc_temperature_info_t   temperature_info;
  dc_accelerometer_info_t accelerometer_info;
  dc_gyroscope_info_t     gyroscope_info;
  dc_magnetometer_info_t  magnetometer_info;
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */
#if (USE_DC_GENERIC == 1)
  dc_generic_bool_info_t    generic_bool;
  dc_generic_uint8_info_t   generic_uint8;
  dc_generic_uint32_info_t  generic_uint32;
  dc_generic_float_info_t   generic_float;
#endif /* USE_DC_GENERIC == 1 */

  formatted_channel_http_request[0] = 0U;

  for (i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    if (put_channel_id_string_tab[i][0] != (uint8_t)'\0')
    {
      PRINT_DBG("Channel %d %s", i, put_channel_id_string_tab[i])
      switch (i)
      {
        case HTTPCLIENT_PUT_SIGLEVEL_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO,
                            (void *)&cellular_info,
                            sizeof(cellular_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        cellular_info.cs_signal_level_db);
          break;
        }
        case HTTPCLIENT_PUT_ICCID_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_INFO,
                            (void *)&cellular_info,
                            sizeof(cellular_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%s",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        cellular_info.iccid);
          break;
        }
        case HTTPCLIENT_PUT_IMSI_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_SIM_INFO, (void *)&sim_info, sizeof(sim_info));

          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%s",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        sim_info.imsi);
          break;
        }
#if (USE_DC_EMUL == 1)
        case HTTPCLIENT_PUT_BATLEVEL_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_BATTERY,
                            (void *)&battery_info,
                            sizeof(battery_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%d",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        battery_info.current_battery_percentage);
          break;
        }
#endif /* USE_DC_EMUL == 1 */
#if ((USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1))
        case HTTPCLIENT_PUT_ACCX_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                            (void *)&accelerometer_info,
                            sizeof(accelerometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        accelerometer_info.accelerometer.AXIS_X);
          break;
        }
        case HTTPCLIENT_PUT_ACCY_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                            (void *)&accelerometer_info,
                            sizeof(accelerometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        accelerometer_info.accelerometer.AXIS_Y);
          break;
        }
        case HTTPCLIENT_PUT_ACCZ_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_ACCELEROMETER,
                            (void *)&accelerometer_info,
                            sizeof(accelerometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        accelerometer_info.accelerometer.AXIS_Z);
          break;
        }
        case HTTPCLIENT_PUT_GYRX_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                            (void *)&gyroscope_info,
                            sizeof(gyroscope_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        gyroscope_info.gyroscope.AXIS_X);
          break;
        }
        case HTTPCLIENT_PUT_GYRY_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                            (void *)&gyroscope_info,
                            sizeof(gyroscope_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        gyroscope_info.gyroscope.AXIS_Y);
          break;
        }
        case HTTPCLIENT_PUT_GYRZ_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_GYROSCOPE,
                            (void *)&gyroscope_info,
                            sizeof(gyroscope_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        gyroscope_info.gyroscope.AXIS_Z);
          break;
        }
        case HTTPCLIENT_PUT_MAGX_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                            (void *)&magnetometer_info,
                            sizeof(magnetometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        magnetometer_info.magnetometer.AXIS_X);
          break;
        }
        case HTTPCLIENT_PUT_MAGY_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                            (void *)&magnetometer_info,
                            sizeof(magnetometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        magnetometer_info.magnetometer.AXIS_Y);
          break;
        }
        case HTTPCLIENT_PUT_MAGZ_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_MAGNETOMETER,
                            (void *)&magnetometer_info,
                            sizeof(magnetometer_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        magnetometer_info.magnetometer.AXIS_Z);
          break;
        }
        case HTTPCLIENT_PUT_HUM_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_HUMIDITY,
                            (void *)&humidity_info,
                            sizeof(humidity_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%f",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        humidity_info.humidity);
          break;
        }
        case HTTPCLIENT_PUT_TEMP_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_TEMPERATURE,
                            (void *)&temperature_info,
                            sizeof(temperature_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%f",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        temperature_info.temperature);
          break;
        }
        case HTTPCLIENT_PUT_PRESS_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_COM_PRESSURE,
                            (void *)&pressure_info,
                            sizeof(pressure_info));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%f",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        pressure_info.pressure);
          break;
        }
#endif  /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */
#if (USE_DC_GENERIC == 1)
        case HTTPCLIENT_PUT_GEN_BOOL1_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_BOOL_1,
                            (void *)&generic_bool,
                            sizeof(generic_bool));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%d",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_bool.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_BOOL2_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_BOOL_2,
                            (void *)&generic_bool,
                            sizeof(generic_bool));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%d",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_bool.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_BYTE1_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_UINT8_1,
                            (void *)&generic_uint8,
                            sizeof(generic_uint8));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%d",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_uint8.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_BYTE2_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_UINT8_2,
                            (void *)&generic_uint8,
                            sizeof(generic_uint8));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%d",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_uint8.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_LONG1_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_UINT32_1,
                            (void *)&generic_uint32,
                            sizeof(generic_uint32));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_uint32.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_LONG2_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_UINT32_2,
                            (void *)&generic_uint32,
                            sizeof(generic_uint32));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%ld",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_uint32.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_FLOAT1_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_FLOAT_1,
                            (void *)&generic_float,
                            sizeof(generic_float));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%f",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_float.value);
          break;
        }
        case HTTPCLIENT_PUT_GEN_FLOAT2_CHANNEL_ID:
        {
          (void)dc_com_read(&dc_com_db, DC_GENERIC_FLOAT_2,
                            (void *)&generic_float,
                            sizeof(generic_float));
          (void)sprintf((CRC_CHAR_t *)formatted_channel_http_request,
                        "%s&%s=%f",
                        formatted_channel_http_request,
                        put_channel_id_string_tab[i],
                        generic_float.value);
          break;
        }
#endif /* USE_DC_GENERIC == 1 */
        default:
        {
          break;
        }
      }
    }
    PRINT_DBG("formatted_channel_http_request %s",
              formatted_channel_http_request)
  }
}

/**
  * @brief  Check Distant name
  * @note   Decide if Network DNS resolver has to be called
  * @param  -
  * @retval -
  */
static void http_client_check_distantname(void)
{
  /* If distantname is provided and distantip is unknown
     call DNS network resolution service */
  if ((crs_strlen(httpclient_distantname) > 0U)
      && (httpclient_distantip.addr == 0U))
  {
    com_sockaddr_t distantaddr;
    /* DNS network resolution request */
    PRINT_INFO("Distant Name provided %s. DNS resolution started", httpclient_distantname)
    if (com_gethostbyname(httpclient_distantname,
                          &distantaddr)
        == COM_SOCKETS_ERR_OK)
    {
      /* Even next test doesn't suppress MISRA Warnings */
      /*
            if ((distantaddr.sa_len == (uint8_t)sizeof(com_sockaddr_in_t))
                && ((distantaddr.sa_family == (uint8_t)COM_AF_UNSPEC)
                    || (distantaddr.sa_family == (uint8_t)COM_AF_INET)))
      */
      {
        httpclient_distantip.addr = ((com_sockaddr_in_t *)&distantaddr)->sin_addr.s_addr;
        PRINT_INFO("DNS resolution OK - Http Remote IP: %d.%d.%d.%d",
                   COM_IP4_ADDR1(&httpclient_distantip),
                   COM_IP4_ADDR2(&httpclient_distantip),
                   COM_IP4_ADDR3(&httpclient_distantip),
                   COM_IP4_ADDR4(&httpclient_distantip))
        /* No reset of nb_error_short wait to see if distant can be reached */
      }
    }
    else
    {
      httpclient_stat.cnt_ko++;
      http_client_nb_error_short++;
      PRINT_ERR("DNS resolution NOK. Will retry later")
    }
  }
}

/**
  * @brief  Create socket
  * @note   If requested close and create socket
  * @param  -
  * @retval -
  */
static void http_client_create_socket(void)
{
  com_sockaddr_in_t address;
  uint32_t timeout;

  if ((socket_closing == true)
      || (socket_state == SOCKET_CLOSING))
  {
    PRINT_INFO("socket in closing mode request the close")
    if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
    {
      socket_state = SOCKET_CLOSING;
      PRINT_ERR("socket close NOK")
      httpclient_stat.cls_ko++;
    }
    else
    {
      socket_state = SOCKET_INVALID;
      PRINT_INFO("socket close OK")
      httpclient_stat.cls_ok++;
    }
    socket_closing = false;
  }

  if (socket_state == SOCKET_INVALID)
  {
    /* Create a TCP socket */
    PRINT_INFO("socket creation in progress")
    socket_http_client = com_socket(COM_AF_INET,
                                    COM_SOCK_STREAM,
                                    COM_IPPROTO_TCP);

    if (socket_http_client >= 0)
    {
      address.sin_family      = (uint8_t)COM_AF_INET;
      address.sin_port        = COM_HTONS(HTTP_CLIENT_DEFAULT_LOCALPORT);
      address.sin_addr.s_addr = COM_INADDR_ANY;
      PRINT_INFO("socket bind in progress")
      if (com_bind(socket_http_client,
                   (com_sockaddr_t *)&address,
                   (int32_t)sizeof(com_sockaddr_in_t))
          == COM_SOCKETS_ERR_OK)
      {
        timeout = RCV_SND_TIMEOUT;

        PRINT_INFO("socket setsockopt in progress")
        if (com_setsockopt(socket_http_client,
                           COM_SOL_SOCKET, COM_SO_RCVTIMEO,
                           &timeout,
                           (int32_t)sizeof(timeout))
            == COM_SOCKETS_ERR_OK)
        {
          if (com_setsockopt(socket_http_client,
                             COM_SOL_SOCKET, COM_SO_SNDTIMEO,
                             &timeout,
                             (int32_t)sizeof(timeout))
              == COM_SOCKETS_ERR_OK)
          {
            socket_state = SOCKET_CREATED;
            PRINT_INFO("socket create OK")
          }
          else
          {
            PRINT_ERR("socket setsockopt SNDTIMEO NOK")
          }
        }
        else
        {
          PRINT_ERR("socket setsockopt RCVTIMEO NOK")
        }
      }
      else
      {
        PRINT_ERR("socket bind NOK")
      }

      if (socket_state != SOCKET_CREATED)
      {
        PRINT_ERR("socket bind or setsockopt NOK - close the socket")
        if (com_closesocket(socket_http_client)
            != COM_SOCKETS_ERR_OK)
        {
          socket_state = SOCKET_CLOSING;
          PRINT_ERR("socket close NOK")
        }
        else
        {
          socket_state = SOCKET_INVALID;
          PRINT_INFO("socket close OK")
        }
      }
    }
    else
    {
      PRINT_ERR("socket create NOK")
    }
  }

  if (socket_state == SOCKET_CREATED)
  {
    address.sin_family      = (uint8_t)COM_AF_INET;
    address.sin_port        = COM_HTONS(httpclient_distantport);
    address.sin_addr.s_addr = httpclient_distantip.addr;
    if (com_connect(socket_http_client,
                    (com_sockaddr_t const *)&address,
                    (int32_t)sizeof(com_sockaddr_in_t))
        == COM_SOCKETS_ERR_OK)
    {
      socket_state = SOCKET_CONNECTED;
      http_client_nb_error_short = 0U;
      http_client_nfmc_tempo_index = 0U;
      PRINT_INFO("socket connect OK")
      httpclient_stat.cnt_ok++;
    }
    else
    {
      http_client_nb_error_short++;
      httpclient_stat.cnt_ko++;
      PRINT_ERR("socket connect NOK closing the socket")
      /* force next time to recreate socket */
      /* because if close is not done, next connect might be refused by LwIP */
      if (com_closesocket(socket_http_client)
          != COM_SOCKETS_ERR_OK)
      {
        socket_state = SOCKET_CLOSING;
        PRINT_ERR("socket close NOK")
      }
      else
      {
        socket_state = SOCKET_INVALID;
        PRINT_INFO("socket close OK")
      }
      /* maybe distantip is no more ok
         if distantname is known force next time a DNS network resolution */
#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 0)
      if (crs_strlen(httpclient_distantname) > 0U)
      {
        PRINT_INFO("distant ip is reset to force a new DNS network resolution of distant url")
        httpclient_distantip.addr = (uint32_t)0U;
      }
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 0 */
    }
  }
}

/**
  * @brief  Receive a full trame
  * @note   Read loop on socket until a full trame is received or an error occur
  * @param  buf_rsp - pointer on data buffer for the response
  * @note   -
  * @retval bool    - false : error occured before to receive a full trame
  *                   true  : full trame received
  */
static bool http_client_receive_a_full_trame(com_char_t *buf_rsp)
{
  bool result;    /* no error occured when reading data ? true */
  bool exit;      /* a complete trame has been received ? true */
  uint32_t index;          /* index in response string */
  int32_t content_length;  /* length of the content received in the trame */
  int32_t rsp_size;        /* length of data received on the socket */
  uint8_t *p_string;       /* string is available in the reponse or not */

  index = 0U;
  result = true; /* result == false when an error in reception is received */
  exit = false;  /* exit == true when all data are received */

  /* Clean-Up the response buffer
     Put '\0' to be sure that even if the string is not completly read string is ended
     Assume response length < CLIENT_MESSAGE_SIZE
   */
  (void)memset((void *)buf_rsp,
               (int32_t)'\0',
               CLIENT_MESSAGE_SIZE);

  do
  {
    rsp_size = com_recv(socket_http_client,
                        (uint8_t *)&buf_rsp[index], /* point on the next byte available in response */
                        (int32_t)(CLIENT_MESSAGE_SIZE) - (int32_t)(index),  /* the more we progress in response,
                                                                             the less bytes are still available */
                        COM_MSG_WAIT);

    /* Data received ? */
    if (rsp_size > 0)
    {
      /* Data received */
      if (index != 0U)
      {
        PRINT_INFO("Multiple com_recv to have complete answer")
      }
      index += (uint32_t)rsp_size;
      /* PRINT_INFO("%s\r\n", buf_rsp) */

      /* Scan the response to see if a trame is complete */
      /* What has to be find ?
       *  1) Header
       *     HTTP/1.1 200 OK "48 54 54 50 2f 31 2e 31 20 32 30 30 20 4f 4b 0d 0a"
       *  2) Content Length: 0 or length
       *     Content-Length: "43 6f 6e 74 65 6e 74 2d 4c 65 6e 67 74 68 3a 20 xx xx 0d 0a"
       *  3) Content Type / End of protocol trame
       *     Content-Type: "43 6f 6e 74 65 6e 74 2d 54 79 70 65 3a xx xx xx 0d 0a 0d 0a"
       *  4) if Content Length != 0 data are avaialble in the reponse
       *     [{"data": .... }]: "5b 7b 22 64 61 74 61 22 3a" ... "7d 5d"
       */
      p_string = NULL;

      /* Enough data received ? */
      if (index > (crs_strlen((uint8_t *)"HTTP/1.1 200 ") + 4U))
      {
        /* Is header received ? */
        p_string = http_client_find_string((const uint8_t *)buf_rsp,
                                           (const uint8_t *)"HTTP/1.1 200 ");
        /* Test the Header */
        if (p_string != NULL)
        {
          /* Header is received - is it OK ?*/
          if (memcmp((const CRC_CHAR_t *)&p_string[crs_strlen((uint8_t *)"HTTP/1.1 200 ")],
                     (uint8_t *)"OK",
                     crs_strlen((uint8_t *)"OK"))
              == 0)
          {
            PRINT_DBG("socket rsp received HTTP header OK")
          }
          else
          {
            PRINT_INFO("socket rsp received NOK - HTTP header NOK")
            result = false; /* exit from the research */
            p_string = NULL; /* Don't continue the current treatment */
          }
        }
      }

      /* p_string != NULL: Header received and OK */
      if (p_string != NULL)
      {
        /* Is end of protocol trame received ? */
        /* Content-Type is after Content-Length,
           so having Content-Type means Content-Length available */
        p_string = http_client_find_string((const uint8_t *)buf_rsp,
                                           (const uint8_t *)"Content-Type: ");
        if (p_string != NULL)
        {
          /* End of protocol maybe reached  - at least data length available */
          p_string = http_client_find_string((const uint8_t *)buf_rsp,
                                             (const uint8_t *)"Content-Length: ");

          if (p_string == NULL)
          {
            /* End of protocol maybe reached but no tag for Data length */
            PRINT_INFO("socket rsp received NOK - No data length indication")
            result = false; /* exit from the research */
          }
        }
        /* else Some data received but not yet enough... continue to read the socket */
      }

      /* p_string != NULL: Header, Content-Type and Content-Length received and OK  */
      /* Test if some data are available in the trame */
      if (p_string != NULL)
      {
        PRINT_DBG("socket rsp received Content-Length OK")
        content_length = (int32_t)crs_atoi((uint8_t *)&p_string[crs_strlen((uint8_t *)"Content-Length: ")]);
        if (content_length > 0)
        {
          const uint8_t *p_data_begin;

          /* Data are expected - Check if all datas are received */
          PRINT_DBG("socket rsp received Content-Length: %ld",  content_length)
          p_data_begin = http_client_find_string((const uint8_t *)buf_rsp,
                                                 (const uint8_t *)"[{\"data\":");
          if (p_data_begin != NULL)
          {
            if (http_client_find_string((const uint8_t *)p_data_begin,
                                        (const uint8_t *)"}]")
                != NULL)
            {
              /* End of protocol reached trame with data */
              PRINT_DBG("socket rsp received OK - full trame with data")
              exit = true;
            }
          }
          /* else Some data received but not yet enough... continue to read the socket */
        }
        else
        {
          /* Data not expected */
          if (http_client_find_string((const uint8_t *)p_string,
                                      (const uint8_t *)"\r\n\r\n")
              != NULL)
          {
            /* End of protocol reached trame with no data */
            PRINT_DBG("socket rsp received OK - full trame with no data")
            exit = true;
          }
          /* else Some data received but not yet enough... continue to read the socket */
        }
      }
    }
    else
    {
      /* Timeout or another error => exit */
      PRINT_INFO("socket rsp received NOK - error or timeout")
      result = false;
    }
    /* exit if:
      an error is detected => result == false
      or trame is complete => exit == true */
  } while ((result != false) && (exit != true));

  return result;
}

/**
  * @brief  Process a request
  * @note   Create, Send, Receive and Close socket
  * @param  buf_snd  - pointer on data buffer to send
  * @note   -
  * @param  buf_rsp - pointer on data buffer for the response
  * @note   -
  * @retval bool    - false/true : process NOK/OK
  */
static bool http_client_process(com_char_t *buf_snd,
                                com_char_t *buf_rsp)
{
  bool result;

  result = false;
  /* If distantip to contact is unknown
     call DNS resolver service */
  httpclient_stat.count++;

  if (httpclient_distantip.addr == (uint32_t)0U)
  {
    http_client_check_distantname();
  }

  if (httpclient_distantip.addr != (uint32_t)0U)
  {
    http_client_create_socket();

    if (socket_state == SOCKET_CONNECTED)
    {
      if (com_send(socket_http_client,
                   (const com_char_t *)buf_snd,
                   (int32_t)crs_strlen(buf_snd),
                   COM_MSG_WAIT)
          > 0)
      {
        PRINT_INFO("socket send data OK")
        http_client_nb_error_short = 0U;
        http_client_nfmc_tempo_index = 0U;
        httpclient_stat.snd_ok++;

        socket_state = SOCKET_WAITING_RSP;

        result = http_client_receive_a_full_trame(buf_rsp);

        socket_state = SOCKET_CONNECTED;

        if (result == false)
        {
          socket_closing = true;
          httpclient_stat.rcv_ko++;
        }
        else
        {
          /* Even if date/time can't be extracted considered answer is ok */
          httpclient_stat.rcv_ok++;

          /* Analyze the response */
          if (http_client_set_date_time == true)
          {
            dc_time_date_rt_info_t dc_time_date_rt_info;

            (void)dc_time_get_time_date(&dc_time_date_rt_info,
                                        DC_DATE_AND_TIME);

            if (dc_time_date_rt_info.rt_state != DC_SERVICE_ON)
            {
              static uint8_t *date_header  = (uint8_t *)"HTTP/1.1 200 OK\r\nDate: ";
              uint8_t header_size;

              header_size = (uint8_t)crs_strlen(date_header);
              if (memcmp(buf_rsp, date_header, header_size) == 0)
              {
                PRINT_INFO("Update date and time")
                if (timedate_set((uint8_t *)&buf_rsp[header_size]) != 0)
                {
                  PRINT_INFO("Update date and time NOK")
                }
                else
                {
                  PRINT_INFO("Update date and time OK")
                }
              }
            }
            /* Try to set date and time only one time */
            http_client_set_date_time = false;
          }
        }
      }
      else
      {
        httpclient_stat.snd_ko++;
        http_client_nb_error_short++;
        socket_closing = true;
        PRINT_ERR("socket send data NOK - closing the socket")
      }

      if ((socket_closing == true)
          || (socket_state == SOCKET_WAITING_RSP))
      {
        /* Timeout to receive an answer or Closing has been requested */
        if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
        {
          socket_state = SOCKET_CLOSING;
          PRINT_ERR("socket close NOK - stay in closing state")
          httpclient_stat.cls_ko++;
        }
        else
        {
          socket_state = SOCKET_INVALID;
          PRINT_INFO("socket close OK")
          httpclient_stat.cls_ok++;
        }
        socket_closing = false;
      }
    }
  }
  return result;
}

/**
  * @brief  Process a get request
  * @param  channel_id - channel id to process
  * @retval bool       - false/true : process NOK/OK
  */
static bool http_client_get_request(uint32_t channel_id)
{
  static com_char_t *header1_get_a = (com_char_t *)"GET /api/comp/";
  static com_char_t *header1_get_b = (com_char_t *)"/stream/";
  static com_char_t *header1_get_c = (com_char_t *)"/last_value HTTP/1.1\r\n";

  bool res;

  (void)sprintf((CRC_CHAR_t *)formatted_http_request,
                "%s%s%s%s%s%s%s%s%s%s%s%s%s",
                header1_get_a,
                component_id_string,
                header1_get_b,
                get_channel_id_string_tab[channel_id],
                header1_get_c,
                header2, header3, header4, header5, header6,
                key_get_input,
                header_end, header_end);

  res = http_client_process(formatted_http_request,
                            httpclient_tmp_buffer);


  return res;
}

/**
  * @brief  Process the Get
  * @note   Loop on the different channel id to process
  * @param  -
  * @retval -
  */
static void http_client_get_process(void)
{
  static com_char_t buf_led_on[]  = {0x64, 0x61, 0x74, 0x61, 0x22, 0x3a, 0x74, 0x72, 0x75, 0x65, 0x00};
  static com_char_t buf_led_off[] = {0x64, 0x61, 0x74, 0x61, 0x22, 0x3a, 0x66, 0x61, 0x6c, 0x73, 0x65, 0x00};

#if (USE_DC_GENERIC == 1)
  static uint8_t *data_string = "\"data\":";
  dc_generic_bool_info_t    generic_bool;
  dc_generic_uint8_info_t   generic_uint8;
  dc_generic_uint32_info_t  generic_uint32;
  uint32_t data_value;
  uint32_t ret;
#endif /* USE_DC_GENERIC == 1 */
  uint8_t i;
  bool res;

  for (i = 0U; i < GET_CHANNEL_ID_MAX; i++)
  {
    if (get_channel_id_string_tab[i][0] != (uint8_t)'\0')
    {
      PRINT_INFO("GET request Channel %d %s", i, put_channel_id_string_tab[i])

      res = http_client_get_request(i);
      if (res == true)
      {
        PRINT_INFO("GET request OK")

        switch (i)
        {
          case HTTPCLIENT_GET_LEDLIGHT_CHANNEL_ID:
          {
            if (strstr((const CRC_CHAR_t *)httpclient_tmp_buffer,
                       (const CRC_CHAR_t *)buf_led_on)
                != 0)
            {
              PRINT_INFO("led on")
              BL_LED_On(HTTPCLIENT_LED);
            }
            else
            {
              if (strstr((const CRC_CHAR_t *)httpclient_tmp_buffer,
                         (const CRC_CHAR_t *)buf_led_off)
                  != 0)
              {
                PRINT_INFO("led off")
                BL_LED_Off(HTTPCLIENT_LED);
              }
              else
              {
                PRINT_INFO("GET request rsp NOK")
              }
            }
            break;
          }
#if (USE_DC_GENERIC == 1)
          case HTTPCLIENT_GET_GEN_BOOL1_CHANNEL_ID:
          {
            if (strstr((const CRC_CHAR_t *)httpclient_tmp_buffer,
                       (const CRC_CHAR_t *)buf_led_on)
                != 0)
            {
              generic_bool.value = 1U;
            }
            else
            {
              generic_bool.value = 0U;
            }
            generic_bool.rt_state = DC_SERVICE_ON;
            (void)dc_com_write(&dc_com_db,
                               DC_GENERIC_BOOL_1,
                               (void *)&generic_bool,
                               sizeof(generic_bool));
            break;
          }
          case HTTPCLIENT_GET_GEN_BOOL2_CHANNEL_ID:
          {
            if (strstr((const CRC_CHAR_t *)httpclient_tmp_buffer,
                       (const CRC_CHAR_t *)buf_led_on)
                != 0)
            {
              generic_bool.value = 1U;
            }
            else
            {
              generic_bool.value = 0U;
            }
            generic_bool.rt_state = DC_SERVICE_ON;
            (void)dc_com_write(&dc_com_db,
                               DC_GENERIC_BOOL_2,
                               (void *)&generic_bool,
                               sizeof(generic_bool));
            break;
          }
          case HTTPCLIENT_GET_GEN_BYTE1_CHANNEL_ID:
          {
            ret = httpclient_find_uint32_value(httpclient_tmp_buffer,
                                               data_string,
                                               crs_strlen(data_string),
                                               &data_value);
            if (ret != 0U)
            {
              generic_uint8.rt_state = DC_SERVICE_ON;
              generic_uint8.value = (uint8_t)data_value;
              (void)dc_com_write(&dc_com_db,
                                 DC_GENERIC_UINT8_1,
                                 (void *)&generic_uint8,
                                 sizeof(generic_uint8));
            }
            else
            {
              PRINT_INFO("httpclient: Byte1 invalid\r\n");
            }
            break;
          }
          case HTTPCLIENT_GET_GEN_BYTE2_CHANNEL_ID:
          {
            ret = httpclient_find_uint32_value(httpclient_tmp_buffer,
                                               data_string,
                                               crs_strlen(data_string),
                                               &data_value);
            if (ret == 0U)
            {
              generic_uint8.rt_state = DC_SERVICE_ON;
              generic_uint8.value = (uint8_t)data_value;
              (void)dc_com_write(&dc_com_db,
                                 DC_GENERIC_UINT8_2,
                                 (void *)&generic_uint8,
                                 sizeof(generic_uint8));
            }
            else
            {
              PRINT_INFO("httpclient: Byte1 invalid\r\n");
            }
            break;
          }
          case HTTPCLIENT_GET_GEN_LONG1_CHANNEL_ID:
          {
            ret = httpclient_find_uint32_value(httpclient_tmp_buffer,
                                               data_string,
                                               crs_strlen(data_string),
                                               &data_value);
            if (ret == 0U)
            {
              generic_uint32.rt_state = DC_SERVICE_ON;
              generic_uint32.value = data_value;
              (void)dc_com_write(&dc_com_db,
                                 DC_GENERIC_UINT32_1,
                                 (void *)&generic_uint32,
                                 sizeof(generic_uint32));
            }
            else
            {
              PRINT_INFO("httpclient: Long1 invalid\r\n");
            }
            break;
          }
          case HTTPCLIENT_GET_GEN_LONG2_CHANNEL_ID:
          {
            ret = httpclient_find_uint32_value(httpclient_tmp_buffer,
                                               data_string,
                                               crs_strlen(data_string),
                                               &data_value);
            if (ret == 0U)
            {
              generic_uint32.rt_state = DC_SERVICE_ON;
              generic_uint32.value = data_value;
              (void)dc_com_write(&dc_com_db,
                                 DC_GENERIC_UINT32_2,
                                 (void *)&generic_uint32,
                                 sizeof(generic_uint32));
            }
            else
            {
              PRINT_INFO("httpclient: Long1 invalid\r\n");
            }
            break;
          }
#endif /* USE_DC_GENERIC == 1 */
          default:
          {
            /* Nothing to do */
            break;
          }
        }
      }
      else
      {
        PRINT_INFO("GET request NOK")
      }
    }
  }
}


/**
  * @brief  Process a put request
  * @param  -
  * @retval -
  */
static void http_client_put_process(void)
{
  static com_char_t *header1a_put  = (com_char_t *)"PUT /api/feed?compId=";
  static com_char_t *header1b_put  = (com_char_t *)" HTTP/1.1\r\n";
  bool process_ok;

  http_client_put_request_format();
  (void)sprintf((CRC_CHAR_t *)formatted_http_request,
                "%s%s%s%s%s%s%s%s%s%s%s%s",
                header1a_put,
                component_id_string,
                formatted_channel_http_request,
                header1b_put,
                header2, header3, header4, header5, header6,
                key_put_input,
                header_end, header_end);

  PRINT_INFO("PUT request")
  process_ok = http_client_process(formatted_http_request,
                                   httpclient_tmp_buffer);
  if (process_ok == true)
  {
    PRINT_INFO("PUT request OK")
  }
  else
  {
    PRINT_INFO("PUT request NOK")
  }
}

/**
  * @brief  Get NFMC parameters
  * @param  index - current index of NFMC
  * @param  value - NFMC timer value according to the current index
  * @retval bool  - true/false : NFMC activated/not activated
  */
static bool http_client_get_nfmc_parameters(uint8_t index,
                                            uint32_t *value)
{
  bool result;
  dc_nfmc_info_t dc_nfmc_info;

  (void)dc_com_read(&dc_com_db, DC_CELLULAR_NFMC_INFO,
                    (void *)&dc_nfmc_info,
                    sizeof(dc_nfmc_info));

  if ((dc_nfmc_info.rt_state == DC_SERVICE_ON)
      && (dc_nfmc_info.activate == 1U))
  {
    result = true;
    *value = dc_nfmc_info.tempo[index];
    PRINT_INFO("http_client NFMC activated - timer read: %d-%ld", index, *value)
  }
  else
  {
    result = false;
  }

  return result;
}


/**
  * @brief  Configuration handler
  * @note   At initialization update http client parameters by menu or default value
  * @param  -
  * @retval -
  */
static void http_client_configuration(void)
{
  static uint8_t *httpclient_default_setup_table[HTTPCLIENT_DEFAULT_PARAM_NB] =
  {
    HTTPCLIENT_DEFAULT_ENABLED,
    HTTPCLIENT_DEFAULT_DISTANTNAME,
    HTTPCLIENT_DEFAULT_CLOUD_KEY_GET,
    HTTPCLIENT_DEFAULT_CLOUD_KEY_PUT,
    HTTPCLIENT_DEFAULT_COMPONENT_ID,
    HTTPCLIENT_DEFAULT_PUT_REQUEST_PERIOD,
    HTTPCLIENT_DEFAULT_SENSOR1_TYPE,
    HTTPCLIENT_DEFAULT_SENSOR1_ID,
    HTTPCLIENT_DEFAULT_SENSOR2_TYPE,
    HTTPCLIENT_DEFAULT_SENSOR2_ID,
    HTTPCLIENT_DEFAULT_SENSOR3_TYPE,
    HTTPCLIENT_DEFAULT_SENSOR3_ID,
    HTTPCLIENT_DEFAULT_SENSOR4_TYPE,
    HTTPCLIENT_DEFAULT_SENSOR4_ID,
    HTTPCLIENT_DEFAULT_SENSOR5_TYPE,
    HTTPCLIENT_DEFAULT_SENSOR5_ID,
    HTTPCLIENT_DEFAULT_SENSOR_END,
    HTTPCLIENT_DEFAULT_GET_REQUEST_PERIOD,
    HTTPCLIENT_DEFAULT_GET_TYPE,
    HTTPCLIENT_DEFAULT_GET_ID,
    HTTPCLIENT_DEFAULT_GET_END
  };

#if (USE_DEFAULT_SETUP == 0)
  (void)setup_record(SETUP_APPLI_HTTP_CLIENT,
                     HTTP_CLIENT_SETUP_VERSION,
                     HTTPCLIENT_LABEL,
                     http_client_setup_handler,
                     http_client_setup_dump,
                     http_client_setup_help,
                     httpclient_default_setup_table,
                     HTTPCLIENT_DEFAULT_PARAM_NB);

#else /* (USE_DEFAULT_SETUP == 1) */
  bool ip_valid;
  uint8_t i;
  uint32_t table_indice;
  int32_t  atoi_res;

  table_indice = 0U;
  if (httpclient_default_setup_table[table_indice][0] == (uint8_t)'1')
  {
    http_client_process_flag = true;
  }
  else
  {
    http_client_process_flag = false;
  }

  ip_valid = false;
  if (http_client_dns_resolver(httpclient_default_setup_table[table_indice++],
                               &httpclient_distantname[0],
                               &httpclient_distantport,
                               &httpclient_distantip)
      == true)
  {
    ip_valid = true;
  }

  if (ip_valid == false)
  {
    httpclient_distantport = HTTPCLIENT_DEFAULT_DISTANTPORT;
    httpclient_distantip.addr = HTTPCLIENT_DEFAULT_DISTANTIP;
    (void)memcpy(httpclient_distantname,
                 HTTPCLIENT_DEFAULT_DISTANTNAME,
                 crs_strlen(HTTPCLIENT_DEFAULT_DISTANTNAME));
  }

#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 1)
  /* Force value to unitary test */
  (void)strcpy((CRC_CHAR_t *)distanthost_string,
               (const CRC_CHAR_t *)HTTPCLIENT_DEFAULT_DISTANTNAME);
  httpclient_distantip.addr = HTTPCLIENT_DEFAULT_DISTANTIP;
  httpclient_distantport = HTTPCLIENT_DEFAULT_DISTANTPORT;
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 1 */

  (void)strcpy((CRC_CHAR_t *)key_get_input,
               (CRC_CHAR_t *)httpclient_default_setup_table[table_indice++]);
  (void)strcpy((CRC_CHAR_t *)key_put_input,
               (CRC_CHAR_t *)httpclient_default_setup_table[table_indice++]);
  (void)strcpy((CRC_CHAR_t *)component_id_string,
               (CRC_CHAR_t *)httpclient_default_setup_table[table_indice++]);

  atoi_res = crs_atoi(httpclient_default_setup_table[table_indice++]);
  if (atoi_res > 0)
  {
    http_client_put_period = (uint32_t)atoi_res;
  }
  else
  {
    http_client_put_period = 0U;
  }
  for (i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    atoi_res = crs_atoi(httpclient_default_setup_table[table_indice++]);
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    (void)strcpy((CRC_CHAR_t *)(put_channel_id_string_tab[atoi_res]),
                 (CRC_CHAR_t *)(httpclient_default_setup_table[table_indice++]));
  }

  atoi_res = crs_atoi(httpclient_default_setup_table[table_indice++]);
  if (atoi_res > 0)
  {
    http_client_get_period = (uint32_t)atoi_res;
  }
  else
  {
    http_client_get_period = 0U;
  }

  for (i = 0U; i < GET_CHANNEL_ID_MAX; i++)
  {
    atoi_res = crs_atoi(httpclient_default_setup_table[table_indice++]);
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    (void)strcpy((CRC_CHAR_t *)(get_channel_id_string_tab[atoi_res]),
                 (CRC_CHAR_t *)(httpclient_default_setup_table[table_indice++]));
  }
#endif /* (USE_DEFAULT_SETUP == 0) */
}

#if (USE_DEFAULT_SETUP == 0)
/**
  * @brief  Sensor list configuration
  * @note   Configure sensor list
  * @param  channelIdName - Name of channelId
  * @param  channelIdStringInput - channelId
  * @param  channel_id_max - maximun channelId number
  * @param  channel_id_size_max-  maximun channelId size
  * @retval uint32_t - number of sensor configured
  */
static uint32_t http_client_sensor_list_config(const com_char_t **channelIdName,
                                               uint8_t (*channelIdStringInput)[CHANNEL_ID_SIZE_MAX],
                                               uint8_t channel_id_max,
                                               uint8_t channel_id_size_max)
{
  uint32_t count;
  int32_t  atoi_res;
  uint8_t  input_string[SELECT_INPUT_STRING_SIZE];

  input_string[0] = 0U;

  UNUSED(channelIdName);
  count = 0U;
  for (uint8_t i = 0U; i < channel_id_max; i++)
  {
    PRINT_SETUP("\r\nConfig a sensor:\r\n")
    PRINT_SETUP("Select sensor type")
    PRINT_SETUP("  0 : quit")
    /*
         for(uint8_t j=0U; j<channel_id_max; j++)
       {
         PRINT_INFO_MENU("  %d : %s\r\n",j+1, channelIdName[j])
       }
    */
    menu_utils_get_string((uint8_t *)"Selection (0 to quit) ",
                          input_string,
                          SELECT_INPUT_STRING_SIZE);

    atoi_res = crs_atoi(input_string);
    if (atoi_res <= 0)
    {
      break;
    }
    atoi_res--;
    if (atoi_res >= (int32_t)channel_id_max)
    {
      continue;
    }
    PRINT_SETUP("Enter channel ID of sensor")

    menu_utils_get_string((uint8_t *)"Channel ID ",
                          channelIdStringInput[(uint8_t)atoi_res],
                          (uint32_t)channel_id_size_max);
    count++;
  }

  return count;
}

/**
  * @brief  Setup handler
  * @note   At initialization update http client parameters by menu
  * @param  -
  * @retval -
  */
static uint32_t http_client_setup_handler(void)
{
  bool ip_valid;
  int32_t  atoi_res;
  uint32_t  res;
  uint16_t  ip_port;
  uint8_t  ip_addr[4];
  uint32_t put_request_channel_count;
  uint32_t get_request_channel_count;
  uint8_t input_period_string[HTTPCLIENT_MAX((PUT_REQUEST_PERIOD_SIZE_MAX + 1), \
                                             (GET_REQUEST_PERIOD_SIZE_MAX + 1))];
  uint8_t  i;

  static uint8_t http_enabled[2];
  static com_char_t distanthost_string[NAME_SIZE_MAX];
  /* list of available types of PUT and GET channels */
  /*
  static com_char_t *put_channel_id_name_tab[PUT_CHANNEL_ID_MAX] = {
         "battery level","signal level","iccid","imsi"
         };
  */
  static const com_char_t *put_channel_id_name_tab[PUT_CHANNEL_ID_MAX] =
  {
    (com_char_t *)"battery level", (com_char_t *)"signal level",
    (com_char_t *)"iccid", (com_char_t *)"imsi",
    (com_char_t *)"humidity", (com_char_t *)"temperature",
    (com_char_t *)"pressure",
    (com_char_t *)"acceleration x", (com_char_t *)"acceleration y", (com_char_t *)"acceleration z",
    (com_char_t *)"angular velocity x", (com_char_t *)"angular velocity y", (com_char_t *)"angular velocity z",
    (com_char_t *)"magnetic flux x", (com_char_t *)"magnetic flux y", (com_char_t *)"magnetic flux z"
  };

  static const com_char_t *get_channel_id_name_tab[GET_CHANNEL_ID_MAX] =
  {
    (com_char_t *)"led state", (com_char_t *)"bool1 state"
  };

  input_period_string[0] = 0U;
  ip_valid = false;

  for (i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    put_channel_id_string_tab[i][0] = (uint8_t)('\0');
  }
  for (i = 0U; i < GET_CHANNEL_ID_MAX; i++)
  {
    get_channel_id_string_tab[i][0] = (uint8_t)('\0');
  }


  menu_utils_get_string((uint8_t *)"httpclient enabled at boot (0: not enable - 1: enabled)",
                        http_enabled,
                        HTTP_ENABLED_SIZE_MAX);

  if (http_enabled[0] == (uint8_t)'1')
  {
    http_client_process_flag = true;
  }
  else
  {
    http_client_process_flag = false;
  }

  menu_utils_get_string((uint8_t *)"Host to contact IP(xxx.xxx.xxx.xxx:xxxxx)or URL(www.url.com port)",
                        distanthost_string,
                        NAME_SIZE_MAX);
#if (HTTPCLIENT_CONFIG_UNITARY_TEST == 1)
  /* Force value to unitary test */
  (void)strcpy((CRC_CHAR_t *)distanthost_string,
               (const CRC_CHAR_t *)HTTPCLIENT_DEFAULT_DISTANTNAME);
  httpclient_distantip.addr = HTTPCLIENT_DEFAULT_DISTANTIP;
  httpclient_distantport = HTTPCLIENT_DEFAULT_DISTANTPORT;
#endif /* HTTPCLIENT_CONFIG_UNITARY_TEST == 1 */

  res =  crc_get_ip_addr(distanthost_string, ip_addr, &ip_port);
  if (res == 0U)

  {
    if ((ip_addr[0] != 0U)
        && (ip_addr[1] != 0U)
        && (ip_addr[2] != 0U)
        && (ip_addr[3] != 0U)
        && (ip_port    != 0U))
    {
      COM_IP4_ADDR(&httpclient_distantip,
                   (uint32_t)ip_addr[0], (uint32_t)ip_addr[1],
                   (uint32_t)ip_addr[2], (uint32_t)ip_addr[3]);
      httpclient_distantport = ip_port;
      httpclient_distantname[0] = (com_char_t)('\0');
      ip_valid = true;
    }
  }
  else
  {
    if (http_client_dns_resolver(distanthost_string,
                                 &httpclient_distantname[0],
                                 &httpclient_distantport,
                                 &httpclient_distantip)
        == true)
    {
      ip_valid = true;
    }
  }

  if (ip_valid == false)
  {
    PRINT_SETUP("Host to contact IP address syntax NOK - Backup to default value")
    httpclient_distantport = (uint16_t)HTTPCLIENT_DEFAULT_DISTANTPORT;
    httpclient_distantip.addr   = HTTPCLIENT_DEFAULT_DISTANTIP;
    (void)memcpy(httpclient_distantname,
                 HTTPCLIENT_DEFAULT_DISTANTNAME,
                 crs_strlen(HTTPCLIENT_DEFAULT_DISTANTNAME));
  }
  if (httpclient_distantip.addr != 0U)
  {
    PRINT_SETUP("Host to contact: IP:%d.%d.%d.%d URL:%s Port:%d ",
                COM_IP4_ADDR1(&httpclient_distantip),
                COM_IP4_ADDR2(&httpclient_distantip),
                COM_IP4_ADDR3(&httpclient_distantip),
                COM_IP4_ADDR4(&httpclient_distantip),
                (CRC_CHAR_t *)&httpclient_distantname[0],
                httpclient_distantport)
  }
  else
  {
    PRINT_SETUP("Host to contact: URL:%s Port:%d ",
                (CRC_CHAR_t *)&httpclient_distantname[0],
                httpclient_distantport)
  }

  menu_utils_get_string((uint8_t *)"Grovestreams GET Key ",
                        key_get_input,
                        KEY_GET_INPUT_SIZE);
  menu_utils_get_string((uint8_t *)"Grovestreams PUT Key ",
                        key_put_input,
                        KEY_PUT_INPUT_SIZE);

  menu_utils_get_string((uint8_t *)"Component ID",
                        component_id_string,
                        COMPONENT_ID_STRING_SIZE_MAX);

  PRINT_SETUP("\r\nPUT request")
  menu_utils_get_string((uint8_t *)"PUT request period ",
                        input_period_string,
                        PUT_REQUEST_PERIOD_SIZE_MAX);
  atoi_res = crs_atoi(input_period_string);
  if (atoi_res > 0)
  {
    http_client_put_period = (uint32_t)atoi_res;
    put_request_channel_count = http_client_sensor_list_config(put_channel_id_name_tab,
                                                               put_channel_id_string_tab,
                                                               PUT_CHANNEL_ID_MAX,
                                                               CHANNEL_ID_SIZE_MAX);
    if (put_request_channel_count == 0U)
    {
      http_client_put_period = 0U;
    }
  }
  else
  {
    http_client_put_period = 0U;
  }

  PRINT_SETUP("\r\nGET request")
  menu_utils_get_string((uint8_t *)"GET request period ",
                        input_period_string,
                        GET_REQUEST_PERIOD_SIZE_MAX);
  atoi_res = crs_atoi(input_period_string);
  if (atoi_res > 0)
  {
    http_client_get_period = (uint32_t)atoi_res;
    get_request_channel_count = http_client_sensor_list_config(get_channel_id_name_tab,
                                                               get_channel_id_string_tab,
                                                               GET_CHANNEL_ID_MAX,
                                                               CHANNEL_ID_SIZE_MAX);
    if (get_request_channel_count == 0U)
    {
      http_client_get_period = 0U;
    }
  }
  else
  {
    http_client_get_period = 0U;
  }
  return 0;
}

/**
  * @brief  Setup help
  * @param  -
  * @retval -
  */
static void http_client_setup_help(void)
{
  PRINT_SETUP("\r\n")
  PRINT_SETUP("===================================\r\n")
  PRINT_SETUP("Grovestreams configuration help\r\n")
  PRINT_SETUP("===================================\r\n")
  setup_version_help();
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("Enabled after boot\n\r")
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("Allow to enable Grovestreams application after boot\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("Host to contact\n\r")
  PRINT_SETUP("---------------\n\r")
  PRINT_SETUP("URL or IP addr of grovestreams server\n\r")
  PRINT_SETUP("The default value of 'Host to contact' is '%s'\n\r", IOT_SERVER_NAME)
  PRINT_SETUP("\n\r")
  PRINT_SETUP("-----------\n\r")
  PRINT_SETUP("PUT API Key\n\r")
  PRINT_SETUP("-----------\n\r")
  PRINT_SETUP("API key for the HTTP PUT request\n\r")
  PRINT_SETUP("Used to send sensor values to the Grovestreams server\n\r")
  PRINT_SETUP("The API key is obtained from the Grovestreams server\n\r")
  PRINT_SETUP("(refer user manual: How to configure a Grovestreams account)\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("-----------\n\r")
  PRINT_SETUP("GET API Key\n\r")
  PRINT_SETUP("-----------\n\r")
  PRINT_SETUP("API key for the HTTP GET request\n\r")
  PRINT_SETUP("Used to get values set on the Grovestreams server (ex: LED state)\n\r")
  PRINT_SETUP("The API key is obtained from the Grovestreams server\n\r")
  PRINT_SETUP("(refer user manual: How to configure a Grovestreams account)\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("------------\n\r")
  PRINT_SETUP("Component ID\n\r")
  PRINT_SETUP("------------\n\r")
  PRINT_SETUP("Component ID of the device used in Grovestreams configuration\n\r")
  PRINT_SETUP("The default value of Component ID is '%s'\n\r", HTTPCLIENT_DEFAULT_COMPONENT_ID)
  PRINT_SETUP("\n\r")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("PUT request period\n\r")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("Period of the PUT request expressed in seconds\n\r")
  PRINT_SETUP("If no PUT request is needed, set this parameter to '0'\n\r")
  PRINT_SETUP("The default value of 'PUT request' period is '%s's while the minimum value is '10's\n\r",
              HTTPCLIENT_DEFAULT_PUT_REQUEST_PERIOD)
  PRINT_SETUP("\n\r")
  PRINT_SETUP("----------------------------\n\r")
  PRINT_SETUP("List of sensor values to PUT\n\r")
  PRINT_SETUP("----------------------------\n\r")
  PRINT_SETUP("This is a list of associations between the sensor types \n\r")
  PRINT_SETUP("and the associated identifiers configured in Grovestreams\n\r")
  PRINT_SETUP("Enter '0' as sensor type to exit the list\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("List of sensor types:\n\r")
  PRINT_SETUP("- 0 : End of PUT request configuration\n\r")
  PRINT_SETUP("- %d : Battery level\n\r", HTTPCLIENT_PUT_BATLEVEL_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Modem signal level\n\r", HTTPCLIENT_PUT_SIGLEVEL_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Cellular ICCID\n\r", HTTPCLIENT_PUT_ICCID_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Cellular IMSI\n\r", HTTPCLIENT_PUT_IMSI_CHANNEL_ID + 1U)
  PRINT_SETUP("\n\r")
#if ((USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1))
  PRINT_SETUP("MEMS sensors\n\r")
#if (USE_SIMU_MEMS == 1)
  PRINT_SETUP("If the sensor shield is not plugged simulated values are calculated\n\r")
#endif /* USE_SIMU_MEMS == 1 */
  PRINT_SETUP("- %d : Humidity\n\r", HTTPCLIENT_PUT_HUM_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Temperature\n\r", HTTPCLIENT_PUT_TEMP_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Pressure\n\r", HTTPCLIENT_PUT_PRESS_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Acceleration X axis\n\r", HTTPCLIENT_PUT_ACCX_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Acceleration Y axis\n\r", HTTPCLIENT_PUT_ACCY_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Acceleration Z axis\n\r", HTTPCLIENT_PUT_ACCZ_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Gyroscope X axis\n\r", HTTPCLIENT_PUT_GYRX_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Gyroscope Y axis\n\r", HTTPCLIENT_PUT_GYRY_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Gyroscope Z axis\n\r", HTTPCLIENT_PUT_GYRZ_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Magnetometer X axis\n\r", HTTPCLIENT_PUT_MAGX_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Magnetometer Y axis\n\r", HTTPCLIENT_PUT_MAGY_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Magnetometer Z axis\n\r", HTTPCLIENT_PUT_MAGZ_CHANNEL_ID + 1U)
  PRINT_SETUP("\n\r")
#endif /* (USE_DC_MEMS == 1) || (USE_SIMU_MEMS == 1) */
#if (USE_DC_GENERIC == 1)
  PRINT_SETUP("Generic Data Cache enties\n\r")
  PRINT_SETUP("- %d : Bool 1  generic entry\n\r", HTTPCLIENT_PUT_GEN_BOOL1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Bool 2  generic entry\n\r", HTTPCLIENT_PUT_GEN_BOOL2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Byte 1  generic entry\n\r", HTTPCLIENT_PUT_GEN_BYTE1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Byte 2  generic entry\n\r", HTTPCLIENT_PUT_GEN_BYTE2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Long 1  generic entry\n\r", HTTPCLIENT_PUT_GEN_LONG1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Long 2  generic entry\n\r", HTTPCLIENT_PUT_GEN_LONG2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Float 1 generic entry\n\r", HTTPCLIENT_PUT_GEN_FLOAT1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Float 2 generic entry\n\r", HTTPCLIENT_PUT_GEN_FLOAT2_CHANNEL_ID + 1U)
  PRINT_SETUP("\n\r")
#endif /* USE_DC_GENERIC == 1 */
  PRINT_SETUP("Example: on the device the 'Battery level' type is '%d'\n\r",
              HTTPCLIENT_PUT_BATLEVEL_CHANNEL_ID + 1U)
  PRINT_SETUP("In the Grovestreams configuration, if the 'battery level' identifier is set to 'batlevel',\n\r")
  PRINT_SETUP("the association is:\n\r")
  PRINT_SETUP("- Sensor type: %d\n\r", HTTPCLIENT_PUT_BATLEVEL_CHANNEL_ID + 1U)
  PRINT_SETUP("- Channel ID: batlevel\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("GET request period\n\r")
  PRINT_SETUP("------------------\n\r")
  PRINT_SETUP("Period of the GET request expressed in seconds\n\r")
  PRINT_SETUP("If no GET request is needed, set this parameter to '0'\n\r")
  PRINT_SETUP("The default value of 'GET request' period is '%s's\n\r",
              HTTPCLIENT_DEFAULT_GET_REQUEST_PERIOD)
  PRINT_SETUP("\n\r")
  PRINT_SETUP("----------------------------\n\r")
  PRINT_SETUP("List of sensor values to GET\n\r")
  PRINT_SETUP("----------------------------\n\r")
  PRINT_SETUP("This is a list of associations between the type of values to get from Grovestreams\n\r")
  PRINT_SETUP("and the associated identifiers configured in Grovestreams\n\r")
  PRINT_SETUP("Enter '0' as sensor type to exit the list\n\r")
  PRINT_SETUP("\n\r")
  PRINT_SETUP("List of values to GET:\n\r")
  PRINT_SETUP("- 0 : End of GET request configuration\n\r")
  PRINT_SETUP("- %d : LED state\n\r", HTTPCLIENT_GET_LEDLIGHT_CHANNEL_ID + 1U)
  PRINT_SETUP("\n\r")
#if (USE_DC_GENERIC == 1)
  PRINT_SETUP("Generic Data Cache enties\n\r")
  PRINT_SETUP("- %d : Bool 1  generic entry\n\r", HTTPCLIENT_GET_GEN_BOOL1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Bool 2  generic entry\n\r", HTTPCLIENT_GET_GEN_BOOL2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Byte 1  generic entry\n\r", HTTPCLIENT_GET_GEN_BYTE1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Byte 2  generic entry\n\r", HTTPCLIENT_GET_GEN_BYTE2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Long 1  generic entry\n\r", HTTPCLIENT_GET_GEN_LONG1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Long 2  generic entry\n\r", HTTPCLIENT_GET_GEN_LONG2_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Float 1 generic entry\n\r", HTTPCLIENT_GET_GEN_FLOAT1_CHANNEL_ID + 1U)
  PRINT_SETUP("- %d : Float 2 generic entry\n\r", HTTPCLIENT_GET_GEN_FLOAT2_CHANNEL_ID + 1U)
  PRINT_SETUP("\n\r")
#endif /* USE_DC_GENERIC == 1 */
  PRINT_SETUP("Example: on the device, the 'LED state' type is '1'\n\r")
  PRINT_SETUP("In the Grovestreams configuration, if the 'LED' identifier is set to 'ledlight',\n\r")
  PRINT_SETUP(" the association is:\n\r")
  PRINT_SETUP("- Sensor type: %d\n\r", HTTPCLIENT_GET_LEDLIGHT_CHANNEL_ID + 1U)
  PRINT_SETUP("- Channel ID: ledlight\n\r")
  PRINT_SETUP("\n\r")
}

/**
  * @brief  Sensor list dump
  * @note   At initialization dump the sensors value
  * @param  channelIdStringInput - channel id string table
  * @param  channel_id_max - channel id max
  * @retval -
  */
static void http_client_sensor_list_dump(uint8_t (*channelIdStringInput)[CHANNEL_ID_SIZE_MAX],
                                         uint8_t channel_id_max)
{
  for (uint8_t i = 0U; i < channel_id_max; i++)
  {
    if (crs_strlen(channelIdStringInput[i]) != 0U)
    {
      PRINT_FORCE("Channel: Type '%d' / ID '%s'", i + 1U, channelIdStringInput[i]);
    }
  }
}

/**
  * @brief  Setup dump
  * @note   At initialization dump the value
  * @param  -
  * @retval -
  */

static void http_client_setup_dump(void)
{
  if (http_client_process_flag == true)
  {
    PRINT_SETUP("Application enabled after boot\n\r")
  }
  else
  {
    PRINT_SETUP("Application not enabled after boot\n\r")
  }

  if (httpclient_distantip.addr == 0U)
  {
    PRINT_FORCE("Host to contact: %s", httpclient_distantname)
  }
  else
  {
    PRINT_FORCE("Host to contact: %lx (%d)", httpclient_distantip.addr, httpclient_distantport)
  }

  PRINT_FORCE("Grovestreams GET Key: %s", key_get_input)
  PRINT_FORCE("Grovestreams PUT Key: %s", key_put_input)
  PRINT_FORCE("Component ID: %s", component_id_string)

  PRINT_FORCE("\r\nPUT request configuration")
  PRINT_FORCE("PUT request period: %ld", http_client_put_period)
  if (http_client_put_period > 0U)
  {
    http_client_sensor_list_dump(put_channel_id_string_tab, PUT_CHANNEL_ID_MAX);
  }

  PRINT_FORCE("\r\nGET request configuration")
  PRINT_FORCE("GET request period: %ld", http_client_get_period);
  if (http_client_get_period > 0U)
  {
    http_client_sensor_list_dump(get_channel_id_string_tab, GET_CHANNEL_ID_MAX);
  }
}
#endif /* USE_DEFAULT_SETUP == 0 */

/**
  * @brief  Socket thread
  * @note   Infinite loop HTTP client body
  * @param  argument - parameter osThread
  * @note   Unused parameter
  * @retval -
  */
static void http_client_socket_thread(void const *argument)
{
  static com_char_t *header4_min   = (com_char_t *)"Connection:Keep-Alive\r\nKeep-Alive:timeout=20\r\n";
  static com_char_t *header4_max   = (com_char_t *)"Connection:Keep-Alive\r\nKeep-Alive:timeout=40\r\n";

  bool systematic_close;
  bool is_nfmc_activated;
  bool local_ip_displayed;
  uint32_t put_period_count;
  uint32_t get_period_count;
  uint32_t nfmc_tempo;

  dc_cellular_target_state_t target_state;

  UNUSED(argument);

  /* Force Local IP displaying before to start HTTP Get/Put process */
  local_ip_displayed = false;

  /* Calculate the first value for get/put period
  in order to start with the first one */
  if (http_client_put_period < http_client_get_period)
  {
    put_period_count = http_client_put_period;
    get_period_count = http_client_get_period - http_client_put_period;
  }
  else if (http_client_put_period > http_client_get_period)
  {
    put_period_count = http_client_put_period - http_client_get_period;
    get_period_count = http_client_get_period;
  }
  else
  {
    put_period_count = http_client_put_period;
    get_period_count = http_client_get_period;
  }

  systematic_close = false;

  BL_LED_Init(NETWORK_LED);
  BL_LED_Init(HTTPCLIENT_LED);

  /* Initialize header4 with the timeout connection
  according to value Put/Get period */
  if ((http_client_put_period != 0U)
      && (http_client_get_period != 0U))
  {
    if (HTTPCLIENT_MIN(http_client_put_period, http_client_get_period) >= SOCKET_OPEN_DURATION_MAX)
    {
      header4 = &header4_min[0];
      systematic_close = true;
    }
    else
    {
      header4 = &header4_max[0];
    }
  }
  else /* one of Put/Get is 0 */
  {
    if ((http_client_put_period >= SOCKET_OPEN_DURATION_MAX)
        || (http_client_get_period >= SOCKET_OPEN_DURATION_MAX))
    {
      header4 = &header4_min[0];
      systematic_close = true;
    }
    else
    {
      header4 = &header4_max[0];
    }
  }

  for (;;)
  {
    /* Waiting for network ready */
    BL_LED_Off(NETWORK_LED);
    /*    (void)osMessageGet(http_client_queue, RTOS_WAIT_FOREVER);*/
    if (network_is_on == true)
    {
      BL_LED_On(NETWORK_LED);
      (void)osDelay(1000U);
      BL_LED_Off(NETWORK_LED);
      (void)osDelay(500U);
      BL_LED_On(NETWORK_LED);
      (void)osDelay(1000U);
      BL_LED_Off(NETWORK_LED);
      PRINT_FORCE("HTTP Client - Local IP: %d.%d.%d.%d",
                  COM_IP4_ADDR1(&httpclient_localip_addr),
                  COM_IP4_ADDR2(&httpclient_localip_addr),
                  COM_IP4_ADDR3(&httpclient_localip_addr),
                  COM_IP4_ADDR4(&httpclient_localip_addr))
      local_ip_displayed = true;
      /* Check distantname as soon as possible to gain time on first connect */
      if ((httpclient_distantip.addr == (uint32_t)0U)
          && (http_client_process_flag == true))
      {
        http_client_check_distantname();
      }
    }
    else
    {
      BL_LED_On(NETWORK_LED);
      (void)osDelay(300U);
      BL_LED_Off(NETWORK_LED);

      if (http_client_modem_restart_in_progress == true)
      {
        PRINT_INFO("Reset Modem in progress => Modem restart request send")
        http_client_modem_restart_in_progress = false;
        target_state.rt_state     = DC_SERVICE_ON;
        target_state.target_state = DC_TARGET_STATE_FULL;
        (void)dc_com_write(&dc_com_db,
                           DC_CELLULAR_TARGET_STATE_CMD,
                           (void *)&target_state,
                           sizeof(target_state));
      }
    }

    /* Network can changed quickly from on/off */
    if (local_ip_displayed == true)
    {
      local_ip_displayed = false;
      while (network_is_on == true)
      {
        if (http_client_process_flag == false)
        {
          /* Close the socket if it was opened */
          if (socket_state != SOCKET_INVALID)
          {
            PRINT_INFO("socket in closing mode request the close")
            if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
            {
              socket_state = SOCKET_CLOSING;
              PRINT_ERR("socket close NOK")
            }
            else
            {
              socket_state = SOCKET_INVALID;
              PRINT_INFO("socket close OK")
            }
            socket_closing = false;
          }

          /* HTTP CLIENT NOT ACTIVE */
          (void)osDelay(1000U);
        }
        else
        {
          /*  HTTP CLIENT ACTIVE */
          BL_LED_On(NETWORK_LED);
          if ((http_client_is_action_authorized() == true)
              && (http_client_put_period > 0U))
          {
            put_period_count++;
            if (put_period_count >= http_client_put_period)
            {
              http_client_put_process();
              put_period_count = 0U;
              if ((systematic_close == true)
                  && (socket_state != SOCKET_INVALID))
              {
                PRINT_INFO("socket in closing mode request the close")
                if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
                {
                  socket_state = SOCKET_CLOSING;
                  PRINT_ERR("socket close NOK")
                }
                else
                {
                  socket_state = SOCKET_INVALID;
                  PRINT_INFO("socket close OK")
                }
                socket_closing = false;
              }
            }
            else
            {
              PRINT_DBG("Skip PUT process: not its time")
            }
          }
          else
          {
            PRINT_DBG("Skip PUT process: Not authorized or Period=0")
          }

          if ((http_client_is_action_authorized() == true)
              && (http_client_get_period > 0U))
          {
            get_period_count++;
            if (get_period_count >= http_client_get_period)
            {
              http_client_get_process();
              get_period_count = 0U;
              if ((systematic_close == true)
                  && (socket_state != SOCKET_INVALID))
              {
                PRINT_INFO("socket in closing mode request the close")
                if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
                {
                  socket_state = SOCKET_CLOSING;
                  PRINT_ERR("socket close NOK")
                }
                else
                {
                  socket_state = SOCKET_INVALID;
                  PRINT_INFO("socket close OK")
                }
                socket_closing = false;
              }
            }
            else
            {
              PRINT_DBG("Skip GET process: not its time")
            }
          }
          else
          {
            PRINT_DBG("Skip GET process: Not authorized or Period=0")
          }

          BL_LED_Off(NETWORK_LED);

          if (((systematic_close == true)
               || (http_client_is_action_authorized() == false))
              && (socket_state != SOCKET_INVALID))
          {
            PRINT_INFO("socket in closing mode request the close")
            if (com_closesocket(socket_http_client) != COM_SOCKETS_ERR_OK)
            {
              socket_state = SOCKET_CLOSING;
              PRINT_ERR("socket close NOK")
            }
            else
            {
              socket_state = SOCKET_INVALID;
              PRINT_INFO("socket close OK")
            }
            socket_closing = false;
          }

          if (http_client_is_action_authorized() == false)
          {
            /*
               Action over the network is no more authorized because
               there are too many consecutives errors (errors > HTTP_CLIENT_ERROR_SHORT_LIMIT_MAX)
            */
            /* is NFM activated (setup configuration) */
            is_nfmc_activated = http_client_get_nfmc_parameters(http_client_nfmc_tempo_index,
                                                                &nfmc_tempo);
            if (is_nfmc_activated == true)
            {
              PRINT_ERR("Too many consecutive errors: error/limit:%d/%d",
                        http_client_nb_error_short, http_client_nb_error_short_limit)
              PRINT_ERR("and NFMC activated => HTTP sleep timer activation:%ld ms",
                        nfmc_tempo)
              (void)osDelay(nfmc_tempo);
              if (http_client_nfmc_tempo_index < ((uint8_t)DC_NFMC_TEMPO_NB - 1U))
              {
                /* Increase nfmc index */
                http_client_nfmc_tempo_index++;
              }
              /* If nfmc index is at its maximum value, nothing to do :
                 http_client_nfmc_tempo_index is let to its maximum value
                 and continue to use the tempo max value
              */
            }
            /* is Modem reset activated (http_client_modem_reset */
            else if (http_client_modem_reset == true)
            {
              if (http_client_modem_reset_in_progress == false)
              {
                PRINT_ERR("Too many consecutive errors: error/limit:%d/%d",
                          http_client_nb_error_short, http_client_nb_error_short_limit)
                PRINT_ERR("and Reset Modem activated => Modem off request")
                http_client_modem_reset_in_progress = true;
                network_is_on = false;
                target_state.rt_state     = DC_SERVICE_ON;
                target_state.target_state = DC_TARGET_STATE_OFF;
                (void)dc_com_write(&dc_com_db,
                                   DC_CELLULAR_TARGET_STATE_CMD,
                                   (void *)&target_state,
                                   sizeof(target_state));
              }
            }
            else
            {
              PRINT_ERR("Too many consecutive errors: error/limit:%d/%d",
                        http_client_nb_error_short, http_client_nb_error_short_limit)
              PRINT_ERR("but nor NFMC nor Reset Modem activated - Continue as before")
            }
            http_client_nb_error_short = 0U;
          }
          else
          {
            PRINT_DBG("Delay")
            (void)osDelay(HTTPCLIENT_SEND_PERIOD);
          }
        }
      }
    }
  }
}

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialization
  * @note   HTTP client initialization
  * @param  -
  * @retval -
  */
void httpclient_init(void)
{
  for (uint8_t i = 0U; i < PUT_CHANNEL_ID_MAX; i++)
  {
    put_channel_id_string_tab[i][0] = (uint8_t)('\0');
  }
  for (uint8_t i = 0U; i < GET_CHANNEL_ID_MAX; i++)
  {
    get_channel_id_string_tab[i][0] = (uint8_t)('\0');
  }

  /* Configuration by menu or default value */
  http_client_configuration();

  /* Socket initialization */
  socket_http_client = COM_SOCKET_INVALID_ID;
  socket_state = SOCKET_INVALID;
  socket_closing = false;

  /* Error counter initialization */
  http_client_nb_error_short_limit = HTTP_CLIENT_ERROR_SHORT_LIMIT_MAX;
  http_client_nb_error_short = 0U;
  http_client_modem_reset_in_progress = false;
  http_client_modem_restart_in_progress = false;
  http_client_modem_reset = HTTP_CLIENT_DEFAULT_MODEM_RESET;

  /* NFMC initialization */
  http_client_nfmc_tempo_index = 0U;

  /* Statistic initialization */
  (void)memset((void *)&httpclient_stat, 0, sizeof(httpclient_stat));

  /* HTTP client set date/time to do */
  /* If HTTP is deativated then another Appli is in charge to set date/time */
  http_client_set_date_time = http_client_process_flag;

  /* Network state */
  network_is_on = false;

  /* Local IP Address */
  httpclient_localip_addr.addr = 0U;

  /*
    osMessageQDef(http_client_queue, 1, uint32_t);
    http_client_queue = osMessageCreate(osMessageQ(http_client_queue), NULL);

    if (http_client_queue == NULL)
    {
      ERROR_Handler(DBG_CHAN_HTTP, 1, ERROR_FATAL);
    }
  */
}

/**
  * @brief  Start
  * @note   HTTP client start
  * @param  -
  * @retval -
  */
void httpclient_start(void)
{
  static osThreadId httpClientTaskHandle;

  /* Registration to datacache - for Network On/Off and Button */
  (void)dc_com_register_gen_event_cb(&dc_com_db, http_client_notif_cb, (void *) NULL);

#if (USE_CMD_CONSOLE == 1)
  CMD_Declare((uint8_t *)"httpclient", http_client_cmd, (uint8_t *)"http client management");
#endif /* USE_CMD_CONSOLE == 1 */

  /* Create HTTP Client thread  */
  osThreadDef(httpClientTask,
              http_client_socket_thread,
              HTTPCLIENT_THREAD_PRIO, 0,
              USED_HTTPCLIENT_THREAD_STACK_SIZE);
  httpClientTaskHandle = osThreadCreate(osThread(httpClientTask), NULL);
  if (httpClientTaskHandle == NULL)
  {
    ERROR_Handler(DBG_CHAN_HTTP, 2, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(httpClientTaskHandle,
                                             USED_HTTPCLIENT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE == 1 */
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
