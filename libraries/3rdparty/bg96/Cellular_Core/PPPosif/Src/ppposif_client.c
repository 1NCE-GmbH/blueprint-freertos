/**
  ******************************************************************************
  * @file    ppposif_client.c
  * @author  MCD Application Team
  * @brief   This file contains pppos client adatation layer
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
#include "ppposif_client.h"
#include "plf_config.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
/* LwIP is a Third Party so MISRAC messages linked to it are ignored */
/*cstat -MISRAC2012-* */
#include "ppp.h"
/*cstat +MISRAC2012-* */
#include "ppposif.h"
#include "ipc_uart.h"
#include "ppposif_ipc.h"
#include "cmsis_os_misrac2012.h"

#include "main.h"
#include "error_handler.h"
#include "trace_interface.h"
#include "dc_common.h"
#include "cellular_datacache.h"

/* Private defines -----------------------------------------------------------*/
#define IPC_DEVICE IPC_DEVICE_0
#define PPPOSIF_CONFIG_TIMEOUT_VALUE 15000U
#define PPPOSIF_CONFIG_FAIL_MAX 3

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static struct netif  gnetif_ppp_client;
static ppp_pcb      *ppp_pcb_client;
static osTimerId     ppposif_config_timeout_timer_handle;
static osSemaphoreId sem_ppp_init_client = NULL;

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void ppp_notify_phase_client_cb(ppp_pcb *pcb, u8_t phase, void *ctx);
static void ppposif_client_running(ppp_pcb *pcb);
static void ppposif_client_dead(void);
static void ppposif_reconf(void);
static void ppposif_client_thread(const void *argument);
static void ppposif_config_timeout_timer_callback(void const *argument);

/* Functions Definition ------------------------------------------------------*/

/* Private Functions Definition ------------------------------------------------------*/

/*
 Notify phase callback (PPP_NOTIFY_PHASE)
==========================================

Notify phase callback, enabled using the PPP_NOTIFY_PHASE config option, let
you configure a callback that is called on each PPP internal state change.
This is different from the status callback which only warns you about
up(running) and down(dead) events.

Notify phase callback can be used, for example, to set a LED pattern depending
on the current phase of the PPP session. Here is a callback example which
tries to mimic what we usually see on xDSL modems while they are negotiating
the link, which should be self-explanatory:
*/

static void ppp_notify_phase_client_cb(ppp_pcb *pcb, u8_t phase, void *ctx)
{
  UNUSED(ctx);
  (void)osDelay(500U); /* hack, add a delay to avoid race condition with Modem which sends an LCP Request earlier.
                   To be improved by syncing the input reading and PPP state machine */
  switch (phase)
  {

    /* Session is down (either permanently or briefly) */
    case PPP_PHASE_DEAD:
      PRINT_PPPOSIF("client ppp_notify_phase_cb: PPP_PHASE_DEAD")
      ppposif_client_dead();
      break;

    /* We are between two sessions */
    case PPP_PHASE_HOLDOFF:
      PRINT_PPPOSIF("client ppp_notify_phase_cb: PPP_PHASE_HOLDOFF")
      ppposif_reconf();

      break;

    /* Session just started */
    case PPP_PHASE_INITIALIZE:
      PRINT_PPPOSIF("client ppp_notify_phase_cb: PPP_PHASE_INITIALIZE")
      break;

    /* Session is running */
    case PPP_PHASE_RUNNING:
      PRINT_PPPOSIF("client ppp_notify_phase_cb: PPP_PHASE_RUNNING")
      ppposif_client_running(pcb);
      break;

    default:
      break;
  }
}

/* ppposif thread */
static void ppposif_client_thread(const void *argument)
{
  UNUSED(argument);
  (void)osSemaphoreWait(sem_ppp_init_client, RTOS_WAIT_FOREVER);
  while (true)
  {
    ppposif_input(&gnetif_ppp_client, ppp_pcb_client, IPC_DEVICE);
  }
}


static void ppposif_client_running(ppp_pcb *pcb)
{
  dc_ppp_client_info_t ppp_client_info;
  struct netif *pppif;
  pppif = ppp_netif((pcb));

  (void)osTimerStop(ppposif_config_timeout_timer_handle);


  (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  ppp_client_info.rt_state = DC_SERVICE_ON;
  ppp_client_info.ip_addr = pppif->ip_addr;
  ppp_client_info.gw      = pppif->gw;
  ppp_client_info.netmask = pppif->netmask;

  (void)dc_com_write(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
}

static void ppposif_client_dead(void)
{
  dc_ppp_client_info_t ppp_client_info;

  (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  if (ppp_client_info.rt_state == DC_SERVICE_SHUTTING_DOWN)
  {
    PRINT_PPPOSIF("ppposif_client_dead: DC_SERVICE_OFF\r\n")
    ppp_client_info.rt_state = DC_SERVICE_OFF;
    (void)dc_com_write(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  }
}

static void ppposif_reconf(void)
{
  dc_ppp_client_info_t ppp_client_info;
  (void)ppp_close(ppp_pcb_client, 0U);
  PRINT_PPPOSIF("ppposif_config_timeout_timer_callback")
  (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  ppp_client_info.rt_state = DC_SERVICE_FAIL;
  (void)dc_com_write(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
}

static void ppposif_config_timeout_timer_callback(void const *argument)
{
  UNUSED(argument);
  ppposif_reconf();
}

/*  Exported functions Definition ------------------------------------------------------*/

/**
  * @brief  component init
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_init(void)
{
  ppposif_ipc_init(IPC_DEVICE);
  return PPPOSIF_OK;
}

/**
  * @brief  component start
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_start(void)
{
  static osThreadId pppClientThreadId = NULL;

  ppposif_status_t ret = PPPOSIF_OK;

  PRINT_PPPOSIF("ppposif_client_config")

  osSemaphoreDef(SEM_PPP_CLIENT_INIT);
  sem_ppp_init_client = osSemaphoreCreate(osSemaphore(SEM_PPP_CLIENT_INIT), 1);
  (void)osSemaphoreWait(sem_ppp_init_client, RTOS_WAIT_FOREVER);

  osTimerDef(PPPOSIF_CONFIG_TIMEOUT_timer, ppposif_config_timeout_timer_callback);
  ppposif_config_timeout_timer_handle = osTimerCreate(osTimer(PPPOSIF_CONFIG_TIMEOUT_timer), osTimerOnce, NULL);

  osThreadDef(PPPOS_CLIENT, ppposif_client_thread, PPPOSIF_CLIENT_THREAD_PRIO, 0, PPPOSIF_CLIENT_THREAD_STACK_SIZE);
  pppClientThreadId = osThreadCreate(osThread(PPPOS_CLIENT), NULL);
  if (pppClientThreadId == NULL)
  {
    ERROR_Handler(DBG_CHAN_PPPOSIF, 1, ERROR_FATAL);
  }
  else
  {
#if (STACK_ANALYSIS_TRACE == 1)
    (void)stackAnalysis_addStackSizeByHandle(pppClientThreadId,
                                             PPPOSIF_CLIENT_THREAD_STACK_SIZE);
#endif /* STACK_ANALYSIS_TRACE */
  }

  return ret;
}

/**
  * @brief  Create a new PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_config(void)
{
  static uint32_t ppposif_create_done = 0U;
  err_t ppp_err;
  ppposif_status_t ret = PPPOSIF_OK;

  PRINT_PPPOSIF("ppposif_client_config")
  ppposif_ipc_select(IPC_DEVICE);

  if (ppposif_create_done == 0U)
  {
    ppp_pcb_client = pppos_create(&gnetif_ppp_client, ppposif_output_cb, ppposif_status_cb, (void *)IPC_DEVICE);
    if (ppp_pcb_client == NULL)
    {
      ERROR_Handler(DBG_CHAN_PPPOSIF, 3, ERROR_FATAL);
      ret =  PPPOSIF_ERROR;
    }
    else
    {
      netif_set_default(&gnetif_ppp_client);
      ppposif_create_done = 1U;
    }
  }

  if (ret ==  PPPOSIF_OK)
  {
    ppp_set_default((ppp_pcb_client));
    ppp_set_notify_phase_callback(ppp_pcb_client,  ppp_notify_phase_client_cb);
    ppp_set_usepeerdns((ppp_pcb_client), (1));

    /*  ppp_set_auth(ppp_pcb_client, PPPAUTHTYPE_PAP, "USER", "PASS"); */

    (void)osTimerStart(ppposif_config_timeout_timer_handle, PPPOSIF_CONFIG_TIMEOUT_VALUE);
    ppp_err = ppp_connect(ppp_pcb_client, 0U);
    (void)osSemaphoreRelease(sem_ppp_init_client);
    if (ppp_err != (err_t)ERR_OK)
    {
      ret = PPPOSIF_ERROR;
    }
  }
  return ret;
}


/**
  * @brief  close a PPPoS client interface
  * @param  none
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_client_close(uint8_t cause)
{
  dc_ppp_client_info_t ppp_client_info;
  PRINT_PPPOSIF("ppposif_client_close")

  (void)osTimerStop(ppposif_config_timeout_timer_handle);

  if (cause == PPPOSIF_CAUSE_POWER_OFF)
  {
    PRINT_PPPOSIF("ppposif_client_close : Closing PPP for POWER OFF")
    (void) ppposif_close(ppp_pcb_client);
    (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
    ppp_client_info.rt_state = DC_SERVICE_SHUTTING_DOWN;
    (void)dc_com_write(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  }
  else
  {
    (void)dc_com_read(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
    ppp_client_info.rt_state = DC_SERVICE_FAIL;
    (void)dc_com_write(&dc_com_db, DC_PPP_CLIENT_INFO, (void *)&ppp_client_info, sizeof(ppp_client_info));
  }



  return PPPOSIF_OK;
}


#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
