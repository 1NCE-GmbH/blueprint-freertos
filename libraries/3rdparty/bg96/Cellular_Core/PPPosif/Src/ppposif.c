/**
  ******************************************************************************
  * @file    ppposif.c
  * @author  MCD Application Team
  * @brief   This file contains pppos adatation layer
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

#include "ppposif.h"
#include "ppposif_client.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)

#include "ppposif_ipc.h"
#include "cellular_service_os.h"
#include "error_handler.h"
/* LwIP is a Third Party so MISRAC messages linked to it are ignored */
/*cstat -MISRAC2012-* */
#include "ppp.h"
#include "netif/ppp/pppos.h"
#include "lwip/sys.h"
#include "lwip/dns.h"
/*cstat +MISRAC2012-* */

/* Private defines -----------------------------------------------------------*/
#define RCV_SIZE_MAX 100

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  PPP status callback is called on PPP status change (up, down, …) from lwIP
  * @param  pcb        pcb reference
  * @param  err_code   error
  * @param  pcb        user context
  * @retval ppposif_status_cb    return status
  */

static void ppposif_err_none_mngt(ppp_pcb *pcb);

static void ppposif_err_none_mngt(ppp_pcb *pcb)
{
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
  struct netif *pppif = ppp_netif((pcb));
#endif  /* (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1) */
#if LWIP_DNS
  const ip_addr_t *ns;
#endif /* LWIP_DNS */
  /* PRINT_PPPOSIF("status_cb: Connected\n") */
#if PPP_IPV4_SUPPORT
  PRINT_PPPOSIF("\n\r")
  PRINT_PPPOSIF("   our_ipaddr  = %s", ipaddr_ntoa(&pppif->ip_addr))
  PRINT_PPPOSIF("   his_ipaddr  = %s", ipaddr_ntoa(&pppif->gw))
  PRINT_PPPOSIF("   netmask     = %s", ipaddr_ntoa(&pppif->netmask))
#if LWIP_DNS
  ns = dns_getserver(0U);
  if (ns->addr == 0U)
  {
    ip_addr_t dns_addr;
    IP_ADDR4(&dns_addr, 8, 8, 8, 8);
    dns_setserver(0, &dns_addr);
    ns = dns_getserver(0);
  }
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
  PRINT_PPPOSIF("   dns1        = %s", ipaddr_ntoa(ns))
#endif   /* (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1) */
  ns = dns_getserver(1U);
  if (ns->addr == 0U)
  {
    ip_addr_t dns_addr;
    IP_ADDR4(&dns_addr, 8, 8, 4, 4);
    dns_setserver(1, &dns_addr);
    ns = dns_getserver(1);
  }
#if (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1)
  PRINT_PPPOSIF("   dns2        = %s", ipaddr_ntoa(ns))
#endif /* (USE_TRACE_PPPOSIF == 1) || (USE_PRINTF == 1) */
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
  PRINT_PPPOSIF("   our6_ipaddr = %s", ip6addr_ntoa(netif_ip6_addr(pppif, 0)))
#endif /* PPP_IPV6_SUPPORT */
}

void ppposif_status_cb(ppp_pcb *pcb, int32_t err_code, void *ctx)
{
  UNUSED(ctx);
  switch (err_code)
  {
    case PPPERR_NONE:
    {
      ppposif_err_none_mngt(pcb);
      break;
    }
    case PPPERR_PARAM:
    {
      PRINT_PPPOSIF("status_cb: Invalid parameter\n\r")
      break;
    }
    case PPPERR_OPEN:
    {
      PRINT_PPPOSIF("status_cb: Unable to open PPP session\n\r")
      break;
    }
    case PPPERR_DEVICE:
    {
      PRINT_PPPOSIF("status_cb: Invalid I/O device for PPP\n\r")
      break;
    }
    case PPPERR_ALLOC:
    {
      PRINT_PPPOSIF("status_cb: Unable to allocate resources\n\r")
      break;
    }
    case PPPERR_USER:
    {
      PRINT_PPPOSIF("status_cb: User interrupt\n\r")
      (void)ppposif_client_close(PPPOSIF_CAUSE_CONNECTION_LOST);
      break;
    }
    case PPPERR_CONNECT:
    {
      PRINT_PPPOSIF("status_cb: Connection lost\n\r")
      break;
    }
    case PPPERR_AUTHFAIL:
    {
      PRINT_PPPOSIF("status_cb: Failed authentication challenge\n\r")
      break;
    }
    case PPPERR_PROTOCOL:
    {
      PRINT_PPPOSIF("status_cb: Failed to meet protocol\n\r")
      break;
    }
    case PPPERR_PEERDEAD:
    {
      PRINT_PPPOSIF("status_cb: Connection timeout\n\r")
      break;
    }
    case PPPERR_IDLETIMEOUT:
    {
      PRINT_PPPOSIF("status_cb: Idle Timeout\n\r")
      break;
    }
    case PPPERR_CONNECTTIME:
    {
      PRINT_PPPOSIF("status_cb: Max connect time reached\n\r")
      break;
    }
    case PPPERR_LOOPBACK:
    {
      PRINT_PPPOSIF("status_cb: Loopback detected\n\r")
      break;
    }
    default:
    {
      PRINT_PPPOSIF("status_cb: Unknown error code %ld\n\r", err_code)
      break;
    }
  }

  /*
   * This should be in the switch case, this is put outside of the switch
   * case for example readability.
   */

  if ((err_code != PPPERR_NONE) && (err_code != PPPERR_USER))
  {
    /*
     * Try to reconnect in 30 seconds, if you need a modem chatscript you have
     * to do a much better signaling here ;-)
     */
    (void)ppp_connect(pcb, 30U);
  }
}


/**
  * @brief  read data from serial and send it to PPP
  * @param  ppp_netif      (in) netif reference
  * @param  p_ppp_pcb      (in) pcb reference
  * @param  pDevice        (in) serial device id
  * @retval none
  */

void ppposif_input(const struct netif *ppp_netif, ppp_pcb  *p_ppp_pcb, IPC_Device_t pDevice)
{
  UNUSED(ppp_netif);
  int32_t rcv_size;
  uint8_t rcvChar[RCV_SIZE_MAX];

  rcv_size = ppposif_ipc_read(pDevice, rcvChar, RCV_SIZE_MAX);
  if (rcv_size != 0)
  {
    /* traceIF_hexPrint(DBG_CHAN_PPPOSIF, DBL_LVL_P0, rcvChar, rcv_size) */
    /* Pass received data to PPPoS to be decoded through lwIP TCPIP thread */
    (void)pppos_input_tcpip(p_ppp_pcb, rcvChar, rcv_size);
  }
}

/**
  * @brief  PPPoS serial output callback
  * @param  pcb            (in) PPP control block
  * @param  data           (in) data, buffer to write to serial port
  * @param  len            (in) length of the data buffer
  * @param  ctx            (in) user context : contains serial device id
  * @retval number of char sent if write succeed - 0 otherwise
  */

u32_t ppposif_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
  UNUSED(pcb);
  IPC_Device_t device;
  u32_t ret;

  if ((int32_t)ctx == (int32_t)IPC_DEVICE_0)
  {
    device = IPC_DEVICE_0;
  }
  else if ((int32_t)ctx == (int32_t)IPC_DEVICE_1)
  {
    device = IPC_DEVICE_1;
  }
  else
  {
    device = IPC_DEVICE_0;
    ERROR_Handler(DBG_CHAN_PPPOSIF, __LINE__, ERROR_FATAL);
  }

  osCCS_get_wait_cs_resource();
  ret = (u32_t)ppposif_ipc_write(device, data, (int16_t)len);
  osCCS_get_release_cs_resource();
  return ret;
}


/**
  * @brief      Closing PPP connection
  * @note       Initiate the end of the PPP session, without carrier lost signal
  *             (nocarrier=0), meaning a clean shutdown of PPP protocols.
  *             You can call this function at anytime.
  * @param  ppp_pcb_struct            (in) pcb reference
  * @retval ppposif_status_t    return status
  */
ppposif_status_t ppposif_close(ppp_pcb *ppp_pcb_struct)
{
  u8_t nocarrier = 0U;
  (void)ppp_close(ppp_pcb_struct, nocarrier);

  /*
   * Then you must wait your status_cb() to be called, it may takes from a few
   * seconds to several tens of seconds depending on the current PPP state.
   */

  /*
   * Freeing a PPP connection
   * ========================
   */

  /*
   * Free the PPP control block, can only be called if PPP session is in the
   * dead state (i.e. disconnected). You need to call ppp_close() before.
   */
  (void)ppp_free(ppp_pcb_struct);
  return PPPOSIF_OK;
}

/* sys_now needed by pppos.c module */
u32_t sys_now(void)
{
  return HAL_GetTick();
}

/* sys_jiffies needed by magic.c module */
u32_t sys_jiffies(void)
{
  static u32_t jiffies_loc = 0U;
  jiffies_loc += 10U;
  return jiffies_loc;
}


#else
/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* sys_now needed by pppos.c module */
uint32_t sys_now(void)
{
  return HAL_GetTick();
}

/* sys_jiffies needed by magic.c module */
uint32_t sys_jiffies(void)
{
  static uint32_t jiffies_loc = 0U;
  jiffies_loc += 10U;
  return jiffies_loc;
}

#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_LWIP  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
