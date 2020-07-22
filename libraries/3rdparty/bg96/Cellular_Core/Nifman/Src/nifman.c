/**
  ******************************************************************************
  * @file    nifman.c
  * @author  MCD Application Team
  * @brief   This file contains the Network InterFace MANager code
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
#include <stdint.h>
#include "cmsis_os_misrac2012.h"

#include "nifman.h"
#include "plf_config.h"

#include "error_handler.h"
#include "com_sockets.h"
#include "dc_common.h"
#include "cellular_datacache.h"
#include "cellular_runtime_custom.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
#include "ppposif_client.h"
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
#include "cellular_service_task.h"
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM) */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_FORCE(format, args...) \
  TRACE_PRINT_FORCE(DBG_CHAN_NIFMAN, DBL_LVL_P0, "" format "\n\r", ## args)
#else
#include <stdio.h>
#define PRINT_FORCE(format, args...)   (void)printf("" format "\n\r", ## args);
#endif  /* (USE_PRINTF == 0U) */

/* Private variables ---------------------------------------------------------*/
static osMessageQId nifman_queue;


/* Global variables ----------------------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void   nifman_notif_cb(dc_com_event_id_t dc_event_id, const void *private_data);
static void nifman_thread(void const *argument);


/* Data cache callback ------------------------------------------------------- */
/**
  * @brief  Data Cache callback
  * @note   catch event to manage data transfert availability
  * @param  none
  * @retval none
  */
static void   nifman_notif_cb(dc_com_event_id_t dc_event_id, const void *private_data)
{
  UNUSED(private_data);
  if ((dc_event_id == DC_CELLULAR_DATA_INFO)
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
      || (dc_event_id == DC_PPP_CLIENT_INFO)
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
     )
  {
    (void)osMessagePut(nifman_queue, (uint32_t)dc_event_id, 0U);
  }
}
/**
  * @brief  nifman thread function
  * @param  argument unused
  * @retval none
  */
static void nifman_thread(void const *argument)
{
  osEvent event;
  dc_com_event_id_t          dc_event_id;
  dc_cellular_data_info_t    cellular_data_info;
  uint32_t err;
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
  dc_ppp_client_info_t       ppp_client_info;
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
  dc_nifman_info_t           nifman_info;
#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)
  uint8_t     addr[4];
  CS_IPaddrType_t ip_addr_type;
  static uint8_t nifman_ip_addr[DC_MAX_IP_ADDR_SIZE];
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM) */

  for (;;)
  {
    /* waiting for DC event */
    event = osMessageGet(nifman_queue, RTOS_WAIT_FOREVER);
    dc_event_id = (dc_com_event_id_t)event.value.v;

    if (dc_event_id == DC_CELLULAR_DATA_INFO)
    {
      /* DC_CELLULAR_DATA_INFO DC event received => read entry values */
      (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cellular_data_info, sizeof(cellular_data_info));
      switch (cellular_data_info.rt_state)
      {
        case DC_SERVICE_ON:
        {
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
          /* LWIP mode:                                      */
          /* MODEM CELLULAR DATA Ready => PPPOS to configure */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state = DC_SERVICE_STARTING;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));

          err = ppposif_client_config();
          if (err != 0U)
          {
            ERROR_Handler(DBG_CHAN_NIFMAN, 2, ERROR_FATAL);
            cellular_data_info.rt_state   =  DC_SERVICE_FAIL;
            (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cellular_data_info,
                               sizeof(cellular_data_info));
          }
#else
          /* Modem Socket mode:                                           */
          /* MODEM CELLULAR DATA Ready => MODEM CELLULAR DATA operational */
          /* Set values of nifman DC entry */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          (void)CST_get_dev_IP_address(&ip_addr_type, nifman_ip_addr);
          err = crc_get_ip_addr(&nifman_ip_addr[1], addr, NULL);
          if (err == 0U)
          {
            nifman_info.ip_addr.addr = (uint32_t)addr[0] +
                                       ((uint32_t)addr[1] << 8) + ((uint32_t)addr[2] << 16) + ((uint32_t)addr[3] << 24);
          }
          else
          {
            nifman_info.ip_addr.addr = 0;
          }

          nifman_info.network      =  DC_CELLULAR_SOCKET_MODEM;
          nifman_info.rt_state     =  cellular_data_info.rt_state;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
          break;
        }
        case DC_SERVICE_SHUTTING_DOWN:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
          if ((nifman_info.rt_state == DC_SERVICE_STARTING)
              || (nifman_info.rt_state == DC_SERVICE_ON))
          {
            (void)ppposif_client_close(PPPOSIF_CAUSE_POWER_OFF);
          }
          nifman_info.rt_state   =  DC_SERVICE_SHUTTING_DOWN;
#else
          nifman_info.rt_state   =  DC_SERVICE_OFF;
#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP */
          /* Cellular link down => update nifman DC entry */
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          break;
        }
        case DC_SERVICE_FAIL:
        {
          /* Cellular link fail => update nifman DC entry */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state   =  DC_SERVICE_FAIL;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          break;
        }
        case DC_SERVICE_OFF:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
          if ((nifman_info.rt_state == DC_SERVICE_STARTING)
              || (nifman_info.rt_state == DC_SERVICE_ON))
          {
            (void)ppposif_client_close(PPPOSIF_CAUSE_POWER_OFF);
          }
#else
          /* Cellular service off => update nifman DC entry */
          nifman_info.rt_state   =  DC_SERVICE_OFF;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
#endif  /*  (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
          break;
        }
        default:
        {
          /* In all other case => update nifman DC entry to service off */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state   =  DC_SERVICE_OFF;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          break;
        }
      }
    }
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
    /* LWIP mode => PPP management */
    else if (dc_event_id == (dc_com_event_id_t)DC_PPP_CLIENT_INFO)
    {
      /* PPP client event reveived */
      (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
      switch (ppp_client_info.rt_state)
      {
        case DC_SERVICE_ON:
        {
          /* PPP client ON => data transfert feature available */
          /* update nifman DC entry (service on) */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state   =  DC_SERVICE_ON;
          nifman_info.network    =  DC_CELLULAR_SOCKETS_LWIP;
          nifman_info.ip_addr    =  ppp_client_info.ip_addr;

          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          break;
        }
        case DC_SERVICE_OFF:
        {
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state   =  DC_SERVICE_OFF;
          nifman_info.network    =  DC_CELLULAR_SOCKETS_LWIP;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          break;
        }
        default:
        {
          /* all other event => data transfert feature no more available */
          /* update nifman DC entry (service off)                        */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));
          nifman_info.rt_state   =  DC_SERVICE_OFF;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_NIFMAN_INFO, (void *)&nifman_info, sizeof(nifman_info));

          /* update DC_CELLULAR_DATA_INFO entry to inform cellular service task automaton */
          (void)dc_com_read(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cellular_data_info, sizeof(cellular_data_info));
          cellular_data_info.rt_state   =  DC_SERVICE_FAIL;
          (void)dc_com_write(&dc_com_db, DC_CELLULAR_DATA_INFO, (void *)&cellular_data_info,
                             sizeof(cellular_data_info));

          break;
        }
      }
    }
#endif   /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
    else
    {
      /* Nothing to do */
    }
  }
}

/* Exported functions ------------------------------------------------------- */
/**
  * @brief  component init
  * @param  none
  * @retval bool  false/true: init nok/ok
  */
bool nifman_init(void)
{
  bool result;
  osMessageQDef(nifman_queue, 1, uint32_t);
  nifman_queue = osMessageCreate(osMessageQ(nifman_queue), NULL);
  if (nifman_queue == NULL)
  {
    result = false;
    ERROR_Handler(DBG_CHAN_NIFMAN, 2, ERROR_FATAL);
  }
  else
  {
    result = true;
  }

  return result;
}

/**
  * @brief  component start
  * @param  none
  * @retval bool  false/true: start nok/ok
  */
bool nifman_start(void)
{
  bool result;
  static osThreadId  nifman_ThreadId = NULL;

  /* registers callback to Data Cache service */
  (void)dc_com_register_gen_event_cb(&dc_com_db, nifman_notif_cb, (void *) NULL);

  /* creates and starts NIFMAN task managment  */
  osThreadDef(NIFMAN, nifman_thread, NIFMAN_THREAD_PRIO, 0, NIFMAN_THREAD_STACK_SIZE);
  nifman_ThreadId = osThreadCreate(osThread(NIFMAN), NULL);
  if (nifman_ThreadId == NULL)
  {
    result =  false;
    ERROR_Handler(DBG_CHAN_NIFMAN, 2, ERROR_FATAL);
  }
  else
  {
    result = true;
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(nifman_ThreadId, NIFMAN_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }

  return result;
}

/***************************** (C) COPYRIGHT STMicroelectronics *******END OF FILE ************/
