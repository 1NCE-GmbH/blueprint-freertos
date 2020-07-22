/**
  ******************************************************************************
  * @file    cellular_service_int.c
  * @author  MCD Application Team
  * @brief   This file defines internal functions for Cellular Service
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

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
csint_socket_infos_t cs_ctxt_sockets_info[CELLULAR_MAX_SOCKETS]; /* socket infos (array index = socket handle) */

/* Functions Definition ------------------------------------------------------*/

void csint_modem_reset_update_socket_state(void)
{
  /* When a Modem RESET is requested, socket state will be update as follow:
  *
  * - if SOCKETSTATE_NOT_ALLOC => no modification
  * - if SOCKETSTATE_CREATED => keep socket context, still valid from user (OPEN not requested to modem yet)
  * - if SOCKETSTATE_CONNECTED => socket is lost, invalid state
  * - if SOCKETSTATE_ALLOC_BUT_INVALID => socket already in an invalid state
  */
  uint8_t cpt;

  for (cpt = 0U; cpt < CELLULAR_MAX_SOCKETS; cpt++)
  {
    switch (cs_ctxt_sockets_info[cpt].state)
    {
      case SOCKETSTATE_NOT_ALLOC:
      case SOCKETSTATE_ALLOC_BUT_INVALID:
        /* nothing to do */
        break;

      case SOCKETSTATE_CREATED:
        /* nothing to do - keep user context */
        break;

      case SOCKETSTATE_CONNECTED:
        /* modem reset occured, context no more valid
        *  waiting for a close from client
        */
        cs_ctxt_sockets_info[cpt].state = SOCKETSTATE_ALLOC_BUT_INVALID;
        break;

      default:
        PRINT_ERR("unknown socket state, Should not happen")
        break;
    }
  }
}

void csint_socket_init(socket_handle_t index)
{
  PRINT_API("<Cellular_Service> SOCKET_init (index=%ld)", index)

  cs_ctxt_sockets_info[index].socket_handle = index;
  cs_ctxt_sockets_info[index].state = SOCKETSTATE_NOT_ALLOC;
  cs_ctxt_sockets_info[index].config = CS_SON_NO_OPTION;

  cs_ctxt_sockets_info[index].addr_type = CS_IPAT_IPV4;
  cs_ctxt_sockets_info[index].protocol = CS_TCP_PROTOCOL;
  cs_ctxt_sockets_info[index].local_port = 0U;
  cs_ctxt_sockets_info[index].conf_id = CS_PDN_NOT_DEFINED;

  cs_ctxt_sockets_info[index].ip_max_packet_size = DEFAULT_IP_MAX_PACKET_SIZE;
  cs_ctxt_sockets_info[index].trp_max_timeout = DEFAULT_TRP_MAX_TIMEOUT;
  cs_ctxt_sockets_info[index].trp_conn_setup_timeout = DEFAULT_TRP_CONN_SETUP_TIMEOUT;
  cs_ctxt_sockets_info[index].trp_transfer_timeout = DEFAULT_TRP_TRANSFER_TIMEOUT;
  cs_ctxt_sockets_info[index].trp_connect_mode = CS_CM_COMMAND_MODE;
  cs_ctxt_sockets_info[index].trp_suspend_timeout = DEFAULT_TRP_SUSPEND_TIMEOUT;
  cs_ctxt_sockets_info[index].trp_rx_timeout = DEFAULT_TRP_RX_TIMEOUT;

  /* socket callback functions pointers */
  cs_ctxt_sockets_info[index].socket_data_ready_callback = NULL;
  cs_ctxt_sockets_info[index].socket_data_sent_callback = NULL;
  cs_ctxt_sockets_info[index].socket_remote_close_callback = NULL;
}

socket_handle_t csint_socket_allocateHandle(void)
{
  socket_handle_t socket_handle = CS_INVALID_SOCKET_HANDLE;
  uint8_t cpt;

  for (cpt = 0U; cpt < CELLULAR_MAX_SOCKETS; cpt++)
  {
    if (cs_ctxt_sockets_info[cpt].state == SOCKETSTATE_NOT_ALLOC)
    {
      /* free socket handle found */
      socket_handle = (socket_handle_t)cpt;
      break;
    }
  }

  return (socket_handle);
}

void csint_socket_deallocateHandle(socket_handle_t sockhandle)
{
  PRINT_INFO("socket_deallocateHandle %ld", sockhandle)
  csint_socket_init(sockhandle);
}

CS_Status_t csint_socket_create(socket_handle_t sockhandle,
                                CS_IPaddrType_t addr_type,
                                CS_TransportProtocol_t protocol,
                                uint16_t local_port,
                                CS_PDN_conf_id_t cid)
{
  CS_Status_t retval;

  cs_ctxt_sockets_info[sockhandle].addr_type = addr_type;
  cs_ctxt_sockets_info[sockhandle].protocol = protocol;
  cs_ctxt_sockets_info[sockhandle].local_port = local_port;
  cs_ctxt_sockets_info[sockhandle].conf_id = cid;

  if (cs_ctxt_sockets_info[sockhandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    /* update socket state */
    cs_ctxt_sockets_info[sockhandle].state = SOCKETSTATE_CREATED;
    retval = CELLULAR_OK;
  }
  else
  {
    PRINT_ERR("<Cellular_Service> socket handle %ld not available", sockhandle)
    retval = CELLULAR_ERROR;
  }

  return (retval);
}

CS_Status_t csint_socket_bind(socket_handle_t sockhandle,
                              uint16_t local_port)
{
  CS_Status_t retval;

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockhandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    PRINT_ERR("<Cellular_Service> invalid socket handle %ld (bind)", sockhandle)
    retval = CELLULAR_ERROR;
  }
  else
  {
    /* set the local port */
    cs_ctxt_sockets_info[sockhandle].local_port = local_port;
    retval = CELLULAR_OK;
  }

  return (retval);
}

CS_Status_t csint_socket_configure(socket_handle_t sockhandle,
                                   CS_SocketOptionLevel_t opt_level,
                                   CS_SocketOptionName_t opt_name,
                                   void *p_opt_val)
{
  CS_Status_t retval = CELLULAR_OK;

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockhandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    PRINT_ERR("<Cellular_Service> invalid socket handle %ld (cfg)", sockhandle)
    retval = CELLULAR_ERROR;
  }

  if (p_opt_val == NULL)
  {
    PRINT_ERR("<Cellular_Service> NULL ptr")
    retval = CELLULAR_ERROR;
  }

  if (retval == CELLULAR_OK)
  {
    uint16_t *p_uint16;
    p_uint16 = (uint16_t *)p_opt_val;

    if (opt_level == CS_SOL_IP)
    {
      switch (opt_name)
      {
        case CS_SON_IP_MAX_PACKET_SIZE:
          if (*p_uint16 <= DEFAULT_IP_MAX_PACKET_SIZE)
          {
            if (*p_uint16 == 0U)
            {
              cs_ctxt_sockets_info[sockhandle].ip_max_packet_size = DEFAULT_IP_MAX_PACKET_SIZE;
            }
            else
            {
              cs_ctxt_sockets_info[sockhandle].ip_max_packet_size = *p_uint16;
            }
            PRINT_DBG("DBG: trp_conn_setup_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_conn_setup_timeout)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> max_packet_size value out of range ")
            retval = CELLULAR_ERROR;
          }
          break;

        default:
          PRINT_ERR("<Cellular_Service> invalid option name for IP")
          retval = CELLULAR_ERROR;
          break;
      }
    }
    else if (opt_level == CS_SOL_TRANSPORT)
    {
      switch (opt_name)
      {
        case CS_SON_TRP_MAX_TIMEOUT:
        {
          /* max value = 65535, no check needed */
          cs_ctxt_sockets_info[sockhandle].trp_max_timeout = *p_uint16;
          PRINT_DBG("DBG: trp_max_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_max_timeout)
          break;
        }

        case CS_SON_TRP_CONNECT_SETUP_TIMEOUT:
        {
          if (*p_uint16 <= 1200U)
          {
            cs_ctxt_sockets_info[sockhandle].trp_conn_setup_timeout = *p_uint16;
            PRINT_DBG("DBG: trp_conn_setup_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_conn_setup_timeout)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> parameter value out of range ")
            retval = CELLULAR_ERROR;
          }
          break;
        }

        case CS_SON_TRP_TRANSFER_TIMEOUT:
        {
          if (*p_uint16 <= 255U)
          {
            cs_ctxt_sockets_info[sockhandle].trp_transfer_timeout = *p_uint16;
            PRINT_DBG("DBG: trp_transfer_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_transfer_timeout)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> parameter value out of range ")
            retval = CELLULAR_ERROR;
          }
          break;
        }

        case CS_SON_TRP_CONNECT_MODE:
        {
          if (*p_uint16 == CS_CM_COMMAND_MODE)
          {
            cs_ctxt_sockets_info[sockhandle].trp_connect_mode = CS_CM_COMMAND_MODE;
            PRINT_DBG("DBG: trp_connect_mode = %d", cs_ctxt_sockets_info[sockhandle].trp_connect_mode)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> Connection mode not supported")
            retval = CELLULAR_ERROR;
          }
          break;
        }

        case CS_SON_TRP_SUSPEND_TIMEOUT:
        {
          if (*p_uint16 <= 2000U)
          {
            cs_ctxt_sockets_info[sockhandle].trp_suspend_timeout =  *p_uint16;
            PRINT_DBG("DBG: trp_suspend_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_suspend_timeout)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> parameter value out of range ")
            retval = CELLULAR_ERROR;
          }
          break;
        }

        case CS_SON_TRP_RX_TIMEOUT:
        {
          if (*p_uint16 <= 255U)
          {
            cs_ctxt_sockets_info[sockhandle].trp_rx_timeout = *p_uint16;
            PRINT_DBG("DBG: trp_rx_timeout = %d", cs_ctxt_sockets_info[sockhandle].trp_rx_timeout)
          }
          else
          {
            PRINT_ERR("<Cellular_Service> parameter value out of range ")
            retval = CELLULAR_ERROR;
          }
          break;
        }

        default:
          PRINT_ERR("<Cellular_Service> invalid option name for TRANSPORT")
          retval = CELLULAR_ERROR;
          break;
      }
    }
    else
    {
      PRINT_ERR("<Cellular_Service> invalid socket option level ")
      retval = CELLULAR_ERROR;
    }
  }

  if (retval == CELLULAR_OK)
  {
    /* set a flag to indicate that this parameter has been set */
    cs_ctxt_sockets_info[sockhandle].config |= opt_name;
  }

  return (retval);
}

CS_Status_t csint_socket_configure_remote(socket_handle_t sockhandle,
                                          CS_IPaddrType_t ip_addr_type,
                                          CS_CHAR_t *p_ip_addr_value,
                                          uint16_t remote_port)
{
  CS_Status_t retval = CELLULAR_ERROR;

  size_t ip_addr_length;

  /* check that socket has been allocated */
  if (cs_ctxt_sockets_info[sockhandle].state == SOCKETSTATE_NOT_ALLOC)
  {
    PRINT_ERR("<Cellular_Service> invalid socket handle %ld (cfg remote)", sockhandle)
  }
  /* check p_ip_addr_value ptr */
  else if (p_ip_addr_value == NULL)
  {
    PRINT_ERR("<Cellular_Service> NULL ptr")
  }
  else
  {
    ip_addr_length = strlen((const CRC_CHAR_t *)p_ip_addr_value);
    if (ip_addr_length > MAX_IP_ADDR_SIZE)
    {
      PRINT_ERR("<Cellular_Service> IP address too long")
    }
    else
    {
      cs_ctxt_sockets_info[sockhandle].remote_port = remote_port;
      cs_ctxt_sockets_info[sockhandle].ip_addr_type = ip_addr_type;
      (void) memset((void *) &cs_ctxt_sockets_info[sockhandle].ip_addr_value, 0, MAX_IP_ADDR_SIZE);
      (void) memcpy((void *) &cs_ctxt_sockets_info[sockhandle].ip_addr_value, (void *)p_ip_addr_value,
                    ip_addr_length);
      retval = CELLULAR_OK;

      PRINT_DBG("DBG: remote_port=%d", cs_ctxt_sockets_info[sockhandle].remote_port)
      PRINT_DBG("DBG: ip_addr_type=%d", cs_ctxt_sockets_info[sockhandle].ip_addr_type)
      PRINT_DBG("DBG: ip_addr cb=%s", cs_ctxt_sockets_info[sockhandle].ip_addr_value)
    }
  }

  return (retval);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


