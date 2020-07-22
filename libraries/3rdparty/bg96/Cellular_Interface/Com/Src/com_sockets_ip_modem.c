/**
  ******************************************************************************
  * @file    com_sockets_ip_modem.c
  * @author  MCD Application Team
  * @brief   This file implements Socket IP on MODEM side
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
#include "com_sockets_ip_modem.h"
#include "plf_config.h"

#if (USE_SOCKETS_TYPE == USE_SOCKETS_MODEM)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "com_sockets_net_compat.h"
#include "com_sockets_err_compat.h"
#include "com_sockets_statistic.h"

#include "cellular_service_os.h"
#if (USE_LOW_POWER == 1)
#include "cellular_service_power.h"
#endif /* USE_LOW_POWER == 1 */

#if (USE_DATACACHE == 1)
#include "dc_common.h"
#include "dc_time.h"
#include "cellular_datacache.h"

#include "mqttdemo.h"
#endif /* USE_DATACACHE == 1 */

#if (UDP_SERVICE_SUPPORTED == 1U)
#include "rng.h" /* Random functions used for local port */
#endif /* UDP_SERVICE_SUPPORTED == 1U */

/* Private defines -----------------------------------------------------------*/
#if (USE_TRACE_COM_SOCKETS == 1U)
#if (USE_PRINTF == 0U)
#include "trace_interface.h"
#define PRINT_INFO(format, args...) TRACE_PRINT(DBG_CHAN_COM, DBL_LVL_P0, "COM: " format "\n\r", ## args)
#define PRINT_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_COM, DBL_LVL_P1, "COM: " format "\n\r", ## args)
#define PRINT_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_COM, DBL_LVL_ERR, "COM ERROR: " format "\n\r", ## args)
#else /* USE_PRINTF == 1 */
#define PRINT_INFO(format, args...)  (void)printf("COM: " format "\n\r", ## args);
/* To reduce trace PRINT_DBG is deactivated when using printf */
#define PRINT_DBG(...)               __NOP(); /* Nothing to do */
#define PRINT_ERR(format, args...)   (void)printf("COM ERROR: " format "\n\r", ## args);
#endif /* USE_PRINTF == 0U */

#else /* USE_TRACE_COM_SOCKETS == 0U */
#define PRINT_INFO(...)  __NOP(); /* Nothing to do */
#define PRINT_DBG(...)   __NOP(); /* Nothing to do */
#define PRINT_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_COM_SOCKETS == 1U */

/* Maximum data that can be passed between COM and low level */
/* Use low level define according to the modem and hardware capabilities */
#define COM_MODEM_MAX_TX_DATA_SIZE CONFIG_MODEM_MAX_SOCKET_TX_DATA_SIZE
#define COM_MODEM_MAX_RX_DATA_SIZE CONFIG_MODEM_MAX_SOCKET_RX_DATA_SIZE

#define COM_SOCKET_LOCAL_ID_NB 1U /* Socket local id number : 1 for ping */

#define COM_LOCAL_PORT_BEGIN  0xc000U /* 49152 */
#define COM_LOCAL_PORT_END    0xffffU /* 65535 */

#if (USE_LOW_POWER == 1)
#define COM_TIMER_INACTIVITY_MS 10000U /* in ms */
#endif /* USE_LOW_POWER == 1 */

/* Private typedef -----------------------------------------------------------*/
typedef char CSIP_CHAR_t; /* used in stdio.h and string.h service call */

/* Message exchange between callback and sockets */
typedef uint16_t com_socket_msg_type_t;
#define COM_SOCKET_MSG        (com_socket_msg_type_t)1    /* MSG is SOCKET type       */
#define COM_PING_MSG          (com_socket_msg_type_t)2    /* MSG is PING type         */

typedef uint16_t com_socket_msg_id_t;
#define COM_DATA_RCV          (com_socket_msg_id_t)1      /* MSG id is DATA_RCV       */
#define COM_CLOSING_RCV       (com_socket_msg_id_t)2      /* MSG id is CLOSING_RCV    */

typedef uint32_t com_socket_msg_t;



typedef struct
{
  CS_IPaddrType_t ip_type;
  /* In cellular_service socket_configure_remote()
     memcpy from char *addr to addr[] without knowing the length
     and by using strlen(char *addr) so ip_value must contain /0 */
  /* IPv4 : xxx.xxx.xxx.xxx=15+/0*/
  /* IPv6 : xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx = 39+/0*/
  uint8_t         ip_value[40];
  uint16_t        port;
} socket_addr_t;

#if (USE_LOW_POWER == 1)
/* Timer State */
typedef enum
{
  COM_TIMER_INVALID = 0,
  COM_TIMER_IDLE,
  COM_TIMER_RUN
} com_timer_state_t;
#endif /* USE_LOW_POWER == 1 */

/* Private macros ------------------------------------------------------------*/

#define COM_MIN(a,b) (((a)<(b)) ? (a) : (b))

/* Set socket error */
#define SOCKET_SET_ERROR(socket, val) do {\
                                           if ((socket) != NULL) {\
                                           (socket)->error = (val); }\
                                         } while(0)

/* Get socket error */
#define SOCKET_GET_ERROR(socket, val) do {\
                                           if ((socket) != NULL) {\
                                             (*(int32_t *)(val)) = ((((socket)->error) == COM_SOCKETS_ERR_OK) ? \
                                             COM_SOCKETS_ERR_OK : \
                                           com_sockets_err_to_errno((com_sockets_err_t)((socket)->error))); }\
                                           else {\
                                             (*(int32_t *)(val)) = \
                                           com_sockets_err_to_errno(COM_SOCKETS_ERR_DESCRIPTOR); }\
                                         } while(0)

/* Description
socket_msg_t
{
  socket_msg_type_t type;
  socket_msg_id_t   id;
} */
/* Set / Get Socket message */
#define SET_SOCKET_MSG_TYPE(msg, type) ((msg) = ((msg)&0xFFFF0000U) | (type))
#define SET_SOCKET_MSG_ID(msg, id)     ((msg) = ((msg)&0x0000FFFFU) | ((id)<<16))
#define GET_SOCKET_MSG_TYPE(msg)       ((com_socket_msg_type_t)((msg)&0x0000FFFFU))
#define GET_SOCKET_MSG_ID(msg)         ((com_socket_msg_id_t)(((msg)&0xFFFF0000U)>>16))

/* Private variables ---------------------------------------------------------*/

/* Mutex to protect access to :
   socket descriptor list,
   socket local_id array */
static osMutexId ComSocketsMutexHandle;

static socket_desc_t *socket_desc_list; /* Socket descriptor list */

/* Provide a socket local id - Used for Ping */
static bool socket_local_id[COM_SOCKET_LOCAL_ID_NB]; /* false : unused, true  : in use  */
#if (USE_COM_PING == 1)
static int32_t ping_socket_id; /* Ping socket id */
#endif /* USE_COM_PING  == 1 */

#if (USE_DATACACHE == 1)
static bool com_network_is_up; /* Network status is managed through Datacache */
#endif /* USE_DATACACHE == 1 */

#if (USE_LOW_POWER == 1)
/* Timer to check inactivity on socket and maybe to go in data idle mode */
static osTimerId ComTimerInactivityId;
/* Timer state */
static com_timer_state_t com_timer_inactivity_state;
/* If com_nb_wake_up <= 1 inactivity timer can be activated */
static uint8_t com_nb_wake_up;
/* Mutex to protect access to :
   com_timer_inactivity_state,
   com_nb_wake_up
  (several applications and datacache) */
static osMutexId ComTimerInactivityMutexHandle;
#endif /* USE_LOW_POWER == 1 */

#if (UDP_SERVICE_SUPPORTED == 1U)
/* Local port allocated - used when bind(local_port = 0U) */
static uint16_t com_local_port; /* a value in [COM_LOCAL_PORT_BEGIN, COM_LOCAL_PORT_BEGIN] */
#endif /* UDP_SERVICE_SUPPORTED == 1U */

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Callback prototype */

/* Callback called by AT when datas are received */
static void com_ip_modem_data_ready_cb(socket_handle_t sock);
/* Callback called by AT when closing is received */
static void com_ip_modem_closing_cb(socket_handle_t sock);

#if (USE_DATACACHE == 1)
/* Callback called by Datacache - used to know Network status */
static void com_socket_datacache_cb(dc_com_event_id_t dc_event_id,
                                    const void *private_gui_data);
#endif /* USE_DATACACHE == 1 */

#if (USE_COM_PING == 1)
/* Callback called by AT when Ping rsp is received */
static void com_ip_modem_ping_rsp_cb(CS_Ping_response_t ping_rsp);
#endif /* USE_COM_PING == 1 */

#if (USE_LOW_POWER == 1)
/* Callback called When Timer Inactivity is raised */
static void com_ip_modem_timer_inactivity_cb(void const *argument);
#endif /* USE_LOW_POWER == 1 */

/* Initialize a socket descriptor */
static void com_ip_modem_init_socket_desc(socket_desc_t *socket_desc);
/* Create a socket descriptor */
static socket_desc_t *com_ip_modem_create_socket_desc(void);
/* Provide a free socket descriptor */
static socket_desc_t *com_ip_modem_provide_socket_desc(bool local);
/* Delete a socket descriptor - in fact reinitialize it */
static void com_ip_modem_delete_socket_desc(int32_t sock,
                                            bool    local);

/* Conversion IP address functions */
static bool com_translate_ip_address(const com_sockaddr_t *addr,
                                     int32_t              addrlen,
                                     socket_addr_t        *socket_addr);
static bool com_convert_IPString_to_sockaddr(uint16_t       ipaddr_port,
                                             com_char_t     *ipaddr_str,
                                             com_sockaddr_t *sockaddr);
static void com_convert_ipaddr_port_to_sockaddr(const com_ip_addr_t *ip_addr,
                                                uint16_t port,
                                                com_sockaddr_in_t *sockaddr_in);
static void com_convert_sockaddr_to_ipaddr_port(const com_sockaddr_in_t *sockaddr_in,
                                                com_ip_addr_t *ip_addr,
                                                uint16_t *port);

/* Provide Network status */
static bool com_ip_modem_is_network_up(void);

#if (UDP_SERVICE_SUPPORTED == 1U)
/* Provide a free local port */
static uint16_t com_ip_modem_new_local_port(void);

/* Establish a UDP service (sendto/recvfrom) socket */
static int32_t com_ip_modem_connect_udp_service(socket_desc_t *socket_desc);
#endif /* UDP_SERVICE_SUPPORTED == 1U */

/* Request low power */
static void com_ip_modem_wakeup_request(void);
static void com_ip_modem_idlemode_request(bool immediate);
#if (USE_LOW_POWER == 1U)
static bool com_ip_modem_are_all_sockets_invalid(void);
#endif /* USE_LOW_POWER == 1U */

/* Private function Definition -----------------------------------------------*/

/**
  * @brief  Initialize a socket descriptor
  * @note   queue is reused
  * @param  socket_desc - socket descriptor to initialize
  * @retval -
  */
static void com_ip_modem_init_socket_desc(socket_desc_t *socket_desc)
{
  socket_desc->state            = COM_SOCKET_INVALID;
  socket_desc->local            = false;
  socket_desc->closing          = false;
  socket_desc->id               = COM_SOCKET_INVALID_ID;
  socket_desc->local_port       = 0U;
  socket_desc->remote_port      = 0U;
  socket_desc->remote_addr.addr = 0U;
  (void)memset((void *)&socket_desc->remote_addr, 0, sizeof(socket_desc->remote_addr));
  socket_desc->rcv_timeout      = RTOS_WAIT_FOREVER;
  socket_desc->snd_timeout      = RTOS_WAIT_FOREVER;
  socket_desc->error            = COM_SOCKETS_ERR_OK;
  /* socket_desc->next is not re-initialize - element is let in the list at its place */
  /* socket_desc->queue is not re-initialize - queue is reused */
}

/**
  * @brief  Create a socket descriptor
  * @note   Allocate a new socket_desc_t and its queue
  *         and initialize the socket to default value
  * @param  -
  * @retval socket_desc_t or NULL (if not enough memory)
  */
static socket_desc_t *com_ip_modem_create_socket_desc(void)
{
  socket_desc_t *socket_desc;

  socket_desc = (socket_desc_t *)pvPortMalloc(sizeof(socket_desc_t));
  if (socket_desc != NULL)
  {
    osMessageQDef(SOCKET_DESC_QUEUE, 4, uint32_t);
    socket_desc->queue = osMessageCreate(osMessageQ(SOCKET_DESC_QUEUE), NULL);
    if (socket_desc->queue == NULL)
    {
      /* Not enough memory - Deallocate socket_desc */
      vPortFree(socket_desc);
      socket_desc = NULL;
    }
    else
    {
      socket_desc->next = NULL;
      com_ip_modem_init_socket_desc(socket_desc);
    }
  }

  return socket_desc;
}

/**
  * @brief  Provide a socket descriptor
  * @note   Search the first empty place in the chained socket list
  *         or create a new socket and add it at the end of the chained list
  * @param  local
  * @note   true/false socket is local (used for Ping) / network one
  * @retval socket_desc_t or NULL (if not enough memory)
  */
static socket_desc_t *com_ip_modem_provide_socket_desc(bool local)
{
  /* Chained list will be changed */
  (void)osMutexWait(ComSocketsMutexHandle, RTOS_WAIT_FOREVER);

  bool found;
  uint8_t i;
  socket_desc_t *socket_desc;
  socket_desc_t *socket_desc_previous;

  i = 0U;
  found = false;
  socket_desc = socket_desc_list;

  /* If socket is local first check an id is still available in the table */
  if (local == true)
  {
    while ((i < COM_SOCKET_LOCAL_ID_NB)
           && (found == false))
    {
      if (socket_local_id[i] == false)
      {
        found = true; /* an unused local id has been found */
        /* com_socket_local_id book is done only if socket is really created */
      }
    }
  }
  else
  {
    found = true; /* if local == false no specific treatment to do */
  }

  if (found == true)
  {
    /* Need to create a new socket_desc ? */
    while ((socket_desc->state != COM_SOCKET_INVALID)
           && (socket_desc->next != NULL))
    {
      socket_desc = socket_desc->next; /* Check next descriptor */
    }
    /* Find an empty socket ? */
    if (socket_desc->state != COM_SOCKET_INVALID)
    {
      /* No empty socket, save the last socket descriptor to attach the new one */
      socket_desc_previous = socket_desc;
      /* Create new socket descriptor */
      socket_desc = com_ip_modem_create_socket_desc();
      if (socket_desc != NULL)
      {
        PRINT_DBG("socket desc created %ld queue %p", socket_desc->id, socket_desc->queue)
        /* Before to attach this new socket to the descriptor list finalize its intialization */
        socket_desc->state = COM_SOCKET_CREATING;
        socket_desc->local = local;
        if (local == true)
        {
          /* even if an application create two sockets one local - one distant
             no issue if id is same because it will be stored
             in two variables at application level */
          /* Don't need an OFFSET to not overlap Modem id */
          socket_desc->id = (int32_t)i;
          /*  Socket is really created - book com_socket_local_id */
          socket_local_id[i] = true;
        }
        /* Initialization is finalized, socket can be attached to the list
           and so visible to other functions
           even the one accessing in read mode to socket descriptor list */
        socket_desc_previous->next = socket_desc;
      }
    }
    else
    {
      /* Find an empty place */
      socket_desc->state = COM_SOCKET_CREATING;
      socket_desc->local = local;
      if (local == true)
      {
        /* even if an application create two sockets one local - one distant
           no issue if id is same because it will be stored
           in two variable s at application level */
        /* Don't need an OFFSET to not overlap Modem id */
        socket_desc->id = (int32_t)i;
        /*  Socket is really created - book com_socket_local_id */
        socket_local_id[i] = true;
      }
    }
  }

  (void)osMutexRelease(ComSocketsMutexHandle);

  /* If provide == false then socket_desc = NULL */
  return socket_desc;
}

/**
  * @brief  Delete a socket descriptor
  * @note   Search the socket descriptor and reinitialze it to unused
  * @param  sock
  * @note   socket id
  * @param  local
  * @note   true/false
  * @retval -
  */
static void com_ip_modem_delete_socket_desc(int32_t sock,
                                            bool    local)
{
  /* Chained list will be changed */
  (void)osMutexWait(ComSocketsMutexHandle, RTOS_WAIT_FOREVER);

  bool found;
  socket_desc_t *socket_desc;

  found = false;

  socket_desc = socket_desc_list;

  /* Search the socket descriptor */
  while ((socket_desc != NULL)
         && (found != true))
  {
    if ((socket_desc->id == sock)
        && (socket_desc->local == local))
    {
      /* Socket descriptor is found */
      found = true;
    }
    else
    {
      /* Not the searched one ... Next */
      socket_desc = socket_desc->next;
    }
  }
  if (found == true)
  {
    /* Always keep a created socket */
    com_ip_modem_init_socket_desc(socket_desc);
    if (local == true)
    {
      /* Free com_socket_local_id */
      socket_local_id[sock] = false;
    }
  }

  (void)osMutexRelease(ComSocketsMutexHandle);
}

/**
  * @brief  Find a socket descriptor
  * @note   Search the socket descriptor
  * @param  sock
  * @note   socket id
  * @param  local
  * @note   true/false
  * @retval socket_desc_t or NULL
  */
 socket_desc_t *com_ip_modem_find_socket(int32_t sock,
                                               bool    local)
{
  socket_desc_t *socket_desc;

  if (sock >= 0)
  {
    bool found;

    socket_desc = socket_desc_list;
    found = false;

    /* Search the socket descriptor */
    while ((socket_desc != NULL)
           && (found != true))
    {
      if ((socket_desc->id == sock)
          && (socket_desc->local == local))
      {
        /* Socket descriptor is found */
        found = true;
      }
      else
      {
        /* Not the searched one ... Next */
        socket_desc = socket_desc->next;
      }
    }
  }
  else
  {
    socket_desc = NULL;
  }

  /* If found == false then socket_desc = NULL */
  return socket_desc;
}

#if (USE_LOW_POWER == 1)
/**
  * @brief  Are all sockets invalid
  * @note   Check if all sockets are invalid
  * @param  -
  * @retval bool : false at least one socket is opened
  *                true  no socket are opened
  */
static bool com_ip_modem_are_all_sockets_invalid(void)
{
  socket_desc_t *socket_desc;
  bool result; /* false : at leat one socket is still open
                  true : all sockets are Invalid */

  socket_desc = socket_desc_list;
  result = true;

  /* Search the socket descriptor */
  while ((socket_desc != NULL)
         && (result != false))
  {
    if (socket_desc->id > COM_SOCKET_INVALID_ID)
    {
      result = false;
    }
    else
    {
      /* Socket is Invalid, check the next one */
      socket_desc = socket_desc->next;
    }
  }

  return result;
}
#endif /* USE_LOW_POWER == 1 */

/**
  * @brief  Translate a com_sockaddr_t to a socket_addr_t
  * @note   -
  * @param  addr - address in com socket format
  * @note   address is uint32_t 0xDDCCBBAA and port is COM_HTONS
  * @param  addrlen - com socket address length
  * @note   must be equal sizeof(com_sockaddr_in_t)
  * @param  socket_addr - socket address AT format
  * @note   address is AAA.BBB.CCC.DDD and port is COM_NTOHS
  * @retval bool - false/true conversion ok/nok
  */
static bool com_translate_ip_address(const com_sockaddr_t *addr,
                                     int32_t              addrlen,
                                     socket_addr_t        *socket_addr)
{
  bool result;
  const com_sockaddr_in_t *p_sockaddr_in;

  result = false;

  if (addrlen == (int32_t)sizeof(com_sockaddr_in_t))
  {
    p_sockaddr_in = (const com_sockaddr_in_t *)addr;

    if ((addr != NULL)
        && (socket_addr != NULL))
    {
      if (addr->sa_family == (uint8_t)COM_AF_INET)
      {
        socket_addr->ip_type = CS_IPAT_IPV4;
        if (p_sockaddr_in->sin_addr.s_addr == COM_INADDR_ANY)
        {
          (void)memcpy(&socket_addr->ip_value[0], "0.0.0.0", strlen("0.0.0.0"));
        }
        else
        {
          com_ip_addr_t com_ip_addr;
          com_ip_addr.addr = ((const com_sockaddr_in_t *)addr)->sin_addr.s_addr;
          (void)sprintf((CSIP_CHAR_t *)socket_addr->ip_value,
                        "%hhu.%hhu.%hhu.%hhu",
                        COM_IP4_ADDR1(&com_ip_addr),
                        COM_IP4_ADDR2(&com_ip_addr),
                        COM_IP4_ADDR3(&com_ip_addr),
                        COM_IP4_ADDR4(&com_ip_addr));
        }
        socket_addr->port = COM_NTOHS(((const com_sockaddr_in_t *)addr)->sin_port);
        result = true;
      }
      /* else if (addr->sa_family == COM_AF_INET6) */
      /* or any other value */
      /* not supported */
    }
  }

  return result;
}

/**
  * @brief  Translate a port/address string to com_sockaddr_t
  * @note   -
  * @param  ipaddr_port
  * @note   port
  * @param  ipaddr_str - address in string format
  * @note   "xxx.xxx.xxx.xxx" or xxx.xxx.xxx.xxx
  * @param  sockaddr - address in com socket format
  * @note   address is uint32_t 0xDDCCBBAA and port is COM_HTONS
  * @retval bool - false/true conversion ok/nok
  */
static bool com_convert_IPString_to_sockaddr(uint16_t       ipaddr_port,
                                             com_char_t     *ipaddr_str,
                                             com_sockaddr_t *sockaddr)
{
  bool result;
  int32_t  count;
  uint8_t  begin;
  uint32_t  ip_addr[4];
  com_ip4_addr_t ip4_addr;

  result = false;

  if (sockaddr != NULL)
  {
    begin = (ipaddr_str[0] == ((uint8_t)'"')) ? 1U : 0U;

    (void)memset(sockaddr, 0, sizeof(com_sockaddr_t));

    count = sscanf((CSIP_CHAR_t *)(&ipaddr_str[begin]),
                   "%03lu.%03lu.%03lu.%03lu",
                   &ip_addr[0], &ip_addr[1],
                   &ip_addr[2], &ip_addr[3]);

    if (count == 4)
    {
      if ((ip_addr[0] <= 255U)
          && (ip_addr[1] <= 255U)
          && (ip_addr[2] <= 255U)
          && (ip_addr[3] <= 255U))
      {
        sockaddr->sa_family = (uint8_t)COM_AF_INET;
        sockaddr->sa_len    = (uint8_t)sizeof(com_sockaddr_in_t);
        ((com_sockaddr_in_t *)sockaddr)->sin_port = COM_HTONS(ipaddr_port);
        COM_IP4_ADDR(&ip4_addr,
                     ip_addr[0], ip_addr[1],
                     ip_addr[2], ip_addr[3]);
        ((com_sockaddr_in_t *)sockaddr)->sin_addr.s_addr = ip4_addr.addr;
        result = true;
      }
    }
  }

  return (result);
}

/**
  * @brief  Translate a port/address uint32_t to com_sockaddr_in_t
  * @note   -
  * @param  ip_addr - address in uint32_t format
  * @note   0xDDCCBBAA
  * @param  port - port
  * @note   -
  * @param  sockaddr_in - address in com socket format
  * @note   address is uint32_t 0xDDCCBBCCAA and port is COM_HTONS
  * @retval -
  */
static void com_convert_ipaddr_port_to_sockaddr(const com_ip_addr_t *ip_addr,
                                                uint16_t port,
                                                com_sockaddr_in_t *sockaddr_in)
{
  sockaddr_in->sin_len         = (uint8_t)sizeof(com_sockaddr_in_t);
  sockaddr_in->sin_family      = COM_AF_INET;
  sockaddr_in->sin_addr.s_addr = ip_addr->addr;
  sockaddr_in->sin_port        = COM_HTONS(port);
  (void) memset(sockaddr_in->sin_zero, 0, COM_SIN_ZERO_LEN);
}

/**
  * @brief  Translate com_sockaddr_in_t to a port/address uint32_t
  * @note   -
  * @param  sockaddr_in - address in com socket format
  * @note   address is uint32_t 0xDDCCDDAA and port is COM_HTONS
  * @param  ip_addr - address in uint32_t format
  * @note   0xDDCCBBAA
  * @param  port - port
  * @note   port is COM_NTOHS
  * @retval -
  */
static void com_convert_sockaddr_to_ipaddr_port(const com_sockaddr_in_t *sockaddr_in,
                                                com_ip_addr_t *ip_addr,
                                                uint16_t *port)
{
  ip_addr->addr = sockaddr_in->sin_addr.s_addr;
  *port = COM_NTOHS(sockaddr_in->sin_port);
}

/**
  * @brief  Provide Network status
  * @note   -
  * @param  -
  * @retval true/false network is up/down
  */
static bool com_ip_modem_is_network_up(void)
{
  bool result;

#if (USE_DATACACHE == 1)
  result = com_network_is_up;
#else /* USE_DATACACHE == 0 */
  /* Feature not supported without Datacache
     Do not block -=> consider network is up */
  result = true;
#endif /* USE_DATACACHE == 1 */

  return result;
}

#if (UDP_SERVICE_SUPPORTED == 1U)
/**
  * @brief  Allocate a new local port value
  * @note   -
  * @retval new local port value in [COM_LOCAL_PORT_BEGIN, COM_LOCAL_PORT_END]
  *         or 0U if impossible to find a free port
  */
static uint16_t com_ip_modem_new_local_port(void)
{
  bool local_port_ok;
  bool found;
  uint16_t iter;
  uint16_t result;
  socket_desc_t *socket_desc;

  local_port_ok = false;
  iter = 0U;

  while ((local_port_ok != true)
         && (iter < (COM_LOCAL_PORT_END - COM_LOCAL_PORT_BEGIN)))
  {
    /* Test the next local port value */
    com_local_port++;
    iter++;
    if (com_local_port == COM_LOCAL_PORT_END)
    {
      com_local_port = COM_LOCAL_PORT_BEGIN;
    }

    socket_desc = socket_desc_list;
    found = false;

    /* See if a socket already created is not using this port */
    while ((socket_desc != NULL)
           && (found != true))
    {
      if (socket_desc->local_port == com_local_port)
      {
        /* Local port already used */
        found = true;
      }
      else
      {
        socket_desc = socket_desc->next;
      }
    }

    if (found == false)
    {
      /* Local port is unused */
      local_port_ok = true;
    }
    /* Continue to search a free value */
  }
  if (local_port_ok != true)
  {
    result = 0U;
  }
  else
  {
    result = com_local_port;
  }

  return result;
}

/**
  * @brief  Establish a UDP service(sendto/recvfrom) socket
  * @note   Regarding the socket state and its paramaters
  *         Allocate a local port
  *         Send bind
  *         Send modem connect to used sendto/recvfrom services
  * @param  socket_desc
  * @note   socket descriptor (local port, state)
  * @retval result
  */
static int32_t com_ip_modem_connect_udp_service(socket_desc_t *socket_desc)
{
  int32_t result;

  result = COM_SOCKETS_ERR_STATE;

  if (socket_desc->state == COM_SOCKET_CREATED)
  {
    /* Bind and Connect udp service must be done */
    result = COM_SOCKETS_ERR_OK;

    /* Find a local port ? */
    if (socket_desc->local_port == 0U)
    {
      socket_desc->local_port = com_ip_modem_new_local_port();
      if (socket_desc->local_port == 0U)
      {
        /* No local port available */
        result = COM_SOCKETS_ERR_LOCKED;
      }
    }

    /* Connect must be done with specific parameter*/
    if (result == COM_SOCKETS_ERR_OK)
    {
      result = COM_SOCKETS_ERR_GENERAL;

      com_ip_modem_wakeup_request();

      /* Bind */
      if (osCDS_socket_bind(socket_desc->id,
                            socket_desc->local_port)
          == CELLULAR_OK)
      {
        PRINT_INFO("socket internal bind ok")
        /* Connect UDP service */
        if (osCDS_socket_connect(socket_desc->id,
                                 CS_IPAT_IPV4,
                                 CONFIG_MODEM_UDP_SERVICE_CONNECT_IP,
                                 0)
            == CELLULAR_OK)
        {
          result = COM_SOCKETS_ERR_OK;
          PRINT_INFO("socket internal connect ok")
          socket_desc->state = COM_SOCKET_CONNECTED;
        }
        else
        {
          PRINT_ERR("socket internal connect NOK at low level")
        }
      }
      else
      {
        PRINT_ERR("socket internal bind NOK at low level")
      }

      com_ip_modem_idlemode_request(false);

    }
  }
  else if (socket_desc->state == COM_SOCKET_CONNECTED)
  {
    /* Already connected - nothing to do */
    result = COM_SOCKETS_ERR_OK;
  }
  else
  {
    PRINT_ERR("socket internal connect - err state")
  }

  return (result);
}
#endif /* UDP_SERVICE_SUPPORTED == 1U */

/**
  * @brief  Request Idle mode
  * @note   -
  * @param  immediate - false/true
  * @retval -
  */
static void com_ip_modem_idlemode_request(bool immediate)
{
#if (USE_LOW_POWER == 1)
  (void)osMutexWait(ComTimerInactivityMutexHandle, RTOS_WAIT_FOREVER);

  if (com_nb_wake_up <= 1U) /* only one socket in progress */
  {
    /* Are all sockets closed ?
       If so, don't arm the timer, immediate request to go in idle */
    if ((immediate == true)
        && (com_ip_modem_are_all_sockets_invalid() == true)) /* Should be always true */
    {
      com_timer_inactivity_state = COM_TIMER_IDLE;
      (void)osTimerStop(ComTimerInactivityId);
      if (com_network_is_up == true) /* If network is up IdleMode can be requested */
      {
        PRINT_INFO("Inactivity: All sockets closed: Timer stopped and IdleMode requested because network is up")
        if (CSP_DataIdle() == CELLULAR_OK)
        {
          PRINT_INFO("Inactivity: IdleMode request OK")
        }
        else
        {
          /* CSP_DataIdle may be NOK because CSP already in Idle */
          PRINT_INFO("Inactivity: IdleMode request NOK")
        }
      }
      else
      {
        PRINT_INFO("Inactivity: All sockets closed: Timer stopped but IdleMode NOT requested because network is down")
      }
    }
    else
    {
      com_timer_inactivity_state = COM_TIMER_RUN;
      /* Start or Restart timer */
      (void)osTimerStart(ComTimerInactivityId, COM_TIMER_INACTIVITY_MS);
      PRINT_INFO("Inactivity: last command finished - Timer re/started")
    }
    com_nb_wake_up = 0U;
  }
  else /* at least one socket in transaction ping/send/recv... and another finished its action */
  {
    /* Improvement : do next treatement only if all sockets in INVALID/CREATING/CREATED state ? */
    com_timer_inactivity_state = COM_TIMER_RUN;
    /* Start or Restart timer */
    (void)osTimerStart(ComTimerInactivityId, COM_TIMER_INACTIVITY_MS);
    PRINT_INFO("Inactivity: one command finished - Timer re/started")
    com_nb_wake_up --;
  }
  (void)osMutexRelease(ComTimerInactivityMutexHandle);

#else /* USE_LOW_POWER == 0 */
  UNUSED(immediate);
  /* Nothing to do */
#endif /* USE_LOW_POWER == 1 */
}

/**
  * @brief  Request Wake-Up
  * @note   -
  * @param  -
  * @retval -
  */
static void com_ip_modem_wakeup_request(void)
{
#if (USE_LOW_POWER == 1)
  (void)osMutexWait(ComTimerInactivityMutexHandle, RTOS_WAIT_FOREVER);

  com_timer_inactivity_state = COM_TIMER_IDLE;
  (void)osTimerStop(ComTimerInactivityId);
  com_nb_wake_up++;
  PRINT_INFO("Inactivity: WakeUp requested - Timer stopped")
  if (CSP_DataWakeup() == CELLULAR_OK)
  {
    PRINT_INFO("Inactivity: WakeUp request OK")
  }
  else
  {
    /* CSP_DataWakeUp may be NOK because CSP already WakeUp */
    PRINT_INFO("Inactivity: WakeUp request NOK")
  }

  (void)osMutexRelease(ComTimerInactivityMutexHandle);
#else /* USE_LOW_POWER == 0 */
  __NOP();
#endif /* USE_LOW_POWER == 1 */
}

/**
  * @brief  Callback called when URC data ready raised
  * @note   Managed URC data ready
  * @param  sock - socket handle
  * @note   -
  * @retval -
  */
static void com_ip_modem_data_ready_cb(socket_handle_t sock)
{
  com_socket_msg_t   msg;
  socket_desc_t *socket_desc;

  socket_desc = com_ip_modem_find_socket(sock,
                                         false);
  msg = 0U;
// (void)osMessagePut(socket_desc->queue, msg, 0U);

 while (socket_desc->state != COM_SOCKET_WAITING_RSP)
 {
	  (void) osDelay(100U);
      PRINT_ERR("********************************* com_data_ready WAITING RESPONSE, %d *******************************",socket_desc->state)
	  socket_desc = com_ip_modem_find_socket(sock,
	                                         false);

 }

  if (socket_desc != NULL)
  {
    if (socket_desc->closing != true)
    {
      if (socket_desc->state == COM_SOCKET_WAITING_RSP)
      {
        PRINT_INFO("cb socket %ld data ready called: waiting rsp", socket_desc->id)
        SET_SOCKET_MSG_TYPE(msg, COM_SOCKET_MSG);
        SET_SOCKET_MSG_ID(msg, COM_DATA_RCV);
        PRINT_DBG("cb socket %ld MSGput %lu queue %p", socket_desc->id, msg, socket_desc->queue)
       (void)osMessagePut(socket_desc->queue, msg, 0U);
      }
      else if (socket_desc->state == COM_SOCKET_WAITING_FROM)
      {
        PRINT_INFO("cb socket %ld data ready called: waiting from", socket_desc->id)
        SET_SOCKET_MSG_TYPE(msg, COM_SOCKET_MSG);
        SET_SOCKET_MSG_ID(msg, COM_DATA_RCV);
        PRINT_DBG("cb socket %ld MSGput %lu queue %p", socket_desc->id, msg, socket_desc->queue)
        (void)osMessagePut(socket_desc->queue, msg, 0U);
      }
      else
      {
        PRINT_INFO("cb socket data ready called: socket_state:%i NOK",
                   socket_desc->state)
      }
    }
    else
    {
      PRINT_ERR("cb socket data ready called: socket is closing")
    }
  }
  else
  {
    PRINT_ERR("cb socket data ready called: unknown socket")
  }
}

/**
  * @brief  Callback called when URC socket closing raised
  * @note   Managed URC socket closing
  * @param  sock - socket handle
  * @note   -
  * @retval -
  */
static void com_ip_modem_closing_cb(socket_handle_t sock)
{
  PRINT_DBG("callback socket closing called")

  com_socket_msg_t   msg;
  socket_desc_t *socket_desc;

  msg = 0U;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);
  if (socket_desc != NULL)
  {
    PRINT_INFO("cb socket closing called: close rqt")
    if (socket_desc->closing == false)
    {
      socket_desc->closing = true;
      PRINT_INFO("cb socket closing: close rqt")
    }
    if ((socket_desc->state == COM_SOCKET_WAITING_RSP)
        || (socket_desc->state == COM_SOCKET_WAITING_FROM))
    {
      PRINT_ERR("!!! cb socket %ld closing called: data_expected !!!", socket_desc->id)
      SET_SOCKET_MSG_TYPE(msg, COM_SOCKET_MSG);
      SET_SOCKET_MSG_ID(msg, COM_CLOSING_RCV);
      PRINT_DBG("cb socket %ld MSGput %lu queue %p", socket_desc->id, msg, socket_desc->queue)
      (void)osMessagePut(socket_desc->queue, msg, 0U);
    }
  }
  else
  {
    PRINT_ERR("cb socket closing called: unknown socket")
  }
}

#if (USE_DATACACHE == 1)
/**
  * @brief  Callback called when a value in datacache changed
  * @note   Managed datacache value changed
  * @param  dc_event_id - value changed
  * @note   -
  * @param  private_gui_data - value provided at service subscription
  * @note   Unused
  * @retval -
  */
static void com_socket_datacache_cb(dc_com_event_id_t dc_event_id,
                                    const void *private_gui_data)
{
  UNUSED(private_gui_data);

  /* Used to know Network status */
  if (dc_event_id == DC_COM_NIFMAN_INFO)
  {
    dc_nifman_info_t  dc_nifman_rt_info;

    if (dc_com_read(&dc_com_db, DC_COM_NIFMAN_INFO,
                    (void *)&dc_nifman_rt_info,
                    sizeof(dc_nifman_rt_info))
        == DC_COM_OK)
    {
      if (dc_nifman_rt_info.rt_state == DC_SERVICE_ON)
      {
        /* Filtering multiple same notification */
        if (com_network_is_up == false)
        {
          com_network_is_up = true;
//          com_sockets_statistic_update(COM_SOCKET_STAT_NWK_UP);
#if (USE_LOW_POWER == 1)
          (void)osMutexWait(ComTimerInactivityMutexHandle, RTOS_WAIT_FOREVER);
          com_timer_inactivity_state = COM_TIMER_RUN;
          /* Start or Restart timer */
          (void)osTimerStart(ComTimerInactivityId, COM_TIMER_INACTIVITY_MS);
          PRINT_INFO("Inactivity: Network on - Timer started")
          (void)osMutexRelease(ComTimerInactivityMutexHandle);
#endif /* USE_LOW_POWER == 1 */
        }
      }
      else
      {
        /* Filtering multiple same notification */
        if (com_network_is_up == true)
        {
          com_network_is_up = false;
//          com_sockets_statistic_update(COM_SOCKET_STAT_NWK_DWN);
#if (USE_LOW_POWER == 1)
          (void)osMutexWait(ComTimerInactivityMutexHandle, RTOS_WAIT_FOREVER);
          com_timer_inactivity_state = COM_TIMER_IDLE;
          (void)osTimerStop(ComTimerInactivityId);
          PRINT_INFO("Inactivity: Network off - Timer stopped")
          (void)osMutexRelease(ComTimerInactivityMutexHandle);
#endif /* USE_LOW_POWER == 1 */
        }
      }
    }
  }
  else
  {
    /* Nothing to do */
  }
}
#endif /* USE_DATACACHE == 1 */

#if (USE_COM_PING == 1)
/**
  * @brief Ping response callback
  * @param  ping_rsp
  * @note   ping data response
  * @retval -
  */
static void com_ip_modem_ping_rsp_cb(CS_Ping_response_t ping_rsp)
{
  PRINT_DBG("callback ping response")

  bool treated;
  com_socket_msg_t  msg;
  socket_desc_t *socket_desc;

  PRINT_DBG("Ping rsp status:%d index:%d final:%d time:%ld",
            ping_rsp.ping_status,
            ping_rsp.index, ping_rsp.is_final_report,
            ping_rsp.time)

  treated = false;
  socket_desc = NULL;

  /* Avoid treatment if Ping Id is unknown - Should not happen */
  if (ping_socket_id != COM_SOCKET_INVALID_ID)
  {
    msg = 0U;
    socket_desc = com_ip_modem_find_socket(ping_socket_id,
                                           true);

    if ((socket_desc != NULL)
        && (socket_desc->closing == false)
        && (socket_desc->state   == COM_SOCKET_WAITING_RSP))
    {
      treated = true;

      if (ping_rsp.ping_status != CELLULAR_OK)
      {
        socket_desc->rsp->status = COM_SOCKETS_ERR_GENERAL;
        socket_desc->rsp->time   = 0U;
        socket_desc->rsp->size   = 0U;
        socket_desc->rsp->ttl    = 0U;
        PRINT_INFO("callback ping data ready: error rcv - exit")
        SET_SOCKET_MSG_TYPE(msg, COM_PING_MSG);
        SET_SOCKET_MSG_ID(msg, COM_DATA_RCV);
        (void)osMessagePut(socket_desc->queue, msg, 0U);
      }
      else
      {
        if (ping_rsp.index == 1U)
        {
          if (ping_rsp.is_final_report == CELLULAR_FALSE)
          {
            /* Save the data wait final report to send event */
            PRINT_INFO("callback ping data ready: rsp rcv - wait final report")
            socket_desc->rsp->status = COM_SOCKETS_ERR_OK;
            socket_desc->rsp->time   = ping_rsp.time;
            socket_desc->rsp->size   = ping_rsp.ping_size;
            socket_desc->rsp->ttl    = ping_rsp.ttl;
            SET_SOCKET_MSG_TYPE(msg, COM_PING_MSG);
            SET_SOCKET_MSG_ID(msg, COM_DATA_RCV);
            (void)osMessagePut(socket_desc->queue, msg, 0U);
          }
          else
          {
            /* index == 1U and final report == true => error */
            PRINT_INFO("callback ping data ready: index=1 and final report=true - exit")
            SET_SOCKET_MSG_TYPE(msg, COM_PING_MSG);
            SET_SOCKET_MSG_ID(msg, COM_CLOSING_RCV);
            (void)osMessagePut(socket_desc->queue, msg, 0U);
          }
        }
        else
        {
          /* Must wait final report */
          if (ping_rsp.is_final_report == CELLULAR_TRUE)
          {
            PRINT_INFO("callback ping data ready: final report rcv")
            SET_SOCKET_MSG_TYPE(msg, COM_PING_MSG);
            SET_SOCKET_MSG_ID(msg, COM_CLOSING_RCV);
            (void)osMessagePut(socket_desc->queue, msg, 0U);
          }
          else
          {
            /* we receive more than one response */
            SET_SOCKET_MSG_TYPE(msg, COM_PING_MSG);
            SET_SOCKET_MSG_ID(msg, COM_DATA_RCV);
            (void)osMessagePut(socket_desc->queue, msg, 0U);
          }
        }
      }
    }
  }

  if (treated == false)
  {
    PRINT_INFO("!!! PURGE callback ping data ready - index %d !!!",
               ping_rsp.index)
    if (socket_desc == NULL)
    {
      PRINT_ERR("Ping Id unknown or no Ping in progress")
    }
    else
    {
      if (socket_desc->closing == true)
      {
        PRINT_ERR("callback ping data ready: ping is closing")
      }
      if (socket_desc->state != COM_SOCKET_WAITING_RSP)
      {
        PRINT_DBG("callback ping data ready: ping state:%d index:%d final report:%u NOK",
                  socket_desc->state, ping_rsp.index, ping_rsp.is_final_report)
      }
    }
  }
}

#endif /* USE_COM_PING == 1 */

#if (USE_LOW_POWER == 1)
/**
  * @brief  Callback called when Inactivity Timer raised
  * @note   Managed Inactivity Timer
  * @param  sock - socket handle
  * @note   -
  * @retval -
  */
static void com_ip_modem_timer_inactivity_cb(void const *argument)
{
  PRINT_DBG("callback socket inactitvity timer called")
  UNUSED(argument);
  PRINT_INFO("Inactivity: Inactivity Timer: Timer cb interaction %d", com_nb_wake_up)

  /* Improvement : do next treatement also if a Ping session is opened and not closed ? */
  if ((com_timer_inactivity_state == COM_TIMER_RUN)
      && (com_nb_wake_up == 0U))
  {
    com_timer_inactivity_state = COM_TIMER_IDLE;
    if (com_network_is_up == true) /* If network is up IdleMode can be requested */
    {
      PRINT_INFO("Inactivity: Inactivity Timer: IdleMode requested because network is up")

      if (CSP_DataIdle() == CELLULAR_OK)
      {
        PRINT_INFO("Inactivity: Inactivity Timer: IdleMode request OK")
      }
      else
      {
        /* CSP_DataIdle may be NOK because CSP already in Idle */
        PRINT_INFO("Inactivity: Inactivity Timer: IdleMode request NOK")
      }
    }
    else
    {
      PRINT_INFO("Inactivity: Inactivity Timer: IdleMode NOT requested because network is down")
    }
  }
  else
  {
    PRINT_INFO("Inactivity: Inactivity Timer: Timer cb called but timer state or interaction: %d NOK", com_nb_wake_up)
  }
}
#endif /* USE_LOW_POWER == 1 */

/* Functions Definition ------------------------------------------------------*/

/*** Socket management ********************************************************/

/**
  * @brief  Socket handle creation
  * @note   Create a communication endpoint called socket
  *         only TCP/UDP IPv4 client mode supported
  * @param  family   - address family
  * @note   only AF_INET supported
  * @param  type     - connection type
  * @note   only SOCK_STREAM or SOCK_DGRAM supported
  * @param  protocol - protocol type
  * @note   only IPPROTO_TCP or IPPROTO_UDP supported
  * @retval int32_t  - socket handle or error value
  */
int32_t com_socket_ip_modem(int32_t family, int32_t type, int32_t protocol)
{
  int32_t sock;
  int32_t result;

  CS_IPaddrType_t IPaddrType;
  CS_TransportProtocol_t TransportProtocol;
  CS_PDN_conf_id_t PDN_conf_id;

  result = COM_SOCKETS_ERR_OK;
  sock = COM_SOCKET_INVALID_ID;

  PDN_conf_id = CS_PDN_CONFIG_DEFAULT;

  if (family == COM_AF_INET)
  {
    /* address family IPv4 */
    IPaddrType = CS_IPAT_IPV4;
  }
  else if (family == COM_AF_INET6)
  {
    /* address family IPv6 */
    IPaddrType = CS_IPAT_IPV6; /* To avoid a warning */
    result = COM_SOCKETS_ERR_UNSUPPORTED;
  }
  else
  {
    IPaddrType = CS_IPAT_INVALID; /* To avoid a warning */
    result = COM_SOCKETS_ERR_PARAMETER;
  }

  if ((type == COM_SOCK_STREAM)
      && ((protocol == COM_IPPROTO_TCP)
          || (protocol == COM_IPPROTO_IP)))
  {
    /* IPPROTO_TCP = must be used with SOCK_STREAM */
    TransportProtocol = CS_TCP_PROTOCOL;
  }
  else if ((type == COM_SOCK_DGRAM)
           && ((protocol == COM_IPPROTO_UDP)
               || (protocol == COM_IPPROTO_IP)))
  {
    /* IPPROTO_UDP = must be used with SOCK_DGRAM */
    TransportProtocol = CS_UDP_PROTOCOL;
  }
  else
  {
    TransportProtocol = CS_TCP_PROTOCOL; /* To avoid a warning */
    result = COM_SOCKETS_ERR_UNSUPPORTED;
  }

  if (result == COM_SOCKETS_ERR_OK)
  {
    result = COM_SOCKETS_ERR_GENERAL;
    PRINT_DBG("socket create request")
    com_ip_modem_wakeup_request();
    sock = osCDS_socket_create(IPaddrType,
                               TransportProtocol,
                               PDN_conf_id);

    if (sock != CS_INVALID_SOCKET_HANDLE)
    {
      socket_desc_t *socket_desc;
      PRINT_INFO("create socket ok low level")

      /* Need to create a new socket_desc ? */
      socket_desc = com_ip_modem_provide_socket_desc(false);
      if (socket_desc == NULL)
      {
        result = COM_SOCKETS_ERR_NOMEMORY;
        PRINT_ERR("create socket NOK no memory")
        /* Socket descriptor is not existing in COM
          must close directly the socket and not call com_close */
        if (osCDS_socket_close(sock, 0U)
            == CELLULAR_OK)
        {
          PRINT_INFO("close socket ok low level")
        }
        else
        {
          PRINT_ERR("close socket NOK low level")
        }
      }
      else
      {
        /* Update socket descriptor */
        socket_desc->id    = sock;
        socket_desc->type  = (uint8_t)type;
        socket_desc->state = COM_SOCKET_CREATED;

        if (osCDS_socket_set_callbacks(sock,
                                       com_ip_modem_data_ready_cb,
                                       NULL,
                                       com_ip_modem_closing_cb)
            == CELLULAR_OK)
        {
          result = COM_SOCKETS_ERR_OK;
        }
        else
        {
          PRINT_ERR("rqt close socket issue at creation")
          if (com_closesocket_ip_modem(sock)
              == COM_SOCKETS_ERR_OK)
          {
            PRINT_INFO("close socket ok low level")
          }
          else
          {
            PRINT_ERR("close socket NOK low level")
          }
        }
      }
    }
    else
    {
      PRINT_ERR("create socket NOK low level")
    }
    com_ip_modem_idlemode_request(false);
    /* Stat only socket whose parameters are supported */
//    com_sockets_statistic_update((result == COM_SOCKETS_ERR_OK) ? \
//                                 COM_SOCKET_STAT_CRE_OK : COM_SOCKET_STAT_CRE_NOK);
  }
  else
  {
    PRINT_ERR("create socket NOK parameter NOK")
  }

  /* result == COM_SOCKETS_ERR_OK return socket handle */
  /* result != COM_SOCKETS_ERR_OK socket not created,
     no need to call SOCKET_SET_ERROR */
  return ((result == COM_SOCKETS_ERR_OK) ? sock : result);
}


/**
  * @brief  Socket option set
  * @note   Set option for the socket
  * @note   only send or receive timeout supported
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which the option is defined
  * @note   only COM_SOL_SOCKET supported
  * @param  optname   - option name for which the value is to be set
  * @note
  *         - COM_SO_SNDTIMEO : OK but value not used because there is already
  *                             a tempo at low level - risk of conflict
  *         - COM_SO_RCVTIMEO : OK
  *         - any other value is rejected
  * @param  optval    - pointer to the buffer containing the option value
  * @note   COM_SO_SNDTIMEO and COM_SO_RCVTIMEO : unit is ms
  * @param  optlen    - size of the buffer containing the option value
  * @retval int32_t   - ok or error value
  */
int32_t com_setsockopt_ip_modem(int32_t sock, int32_t level, int32_t optname,
                                const void *optval, int32_t optlen)
{
  int32_t result;
  socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  if (socket_desc != NULL)
  {
    if ((optval != NULL)
        && (optlen > 0))
    {
      com_ip_modem_wakeup_request();
      if (level == COM_SOL_SOCKET)
      {
        switch (optname)
        {
          /* Send Timeout */
          case COM_SO_SNDTIMEO :
          {
            if ((uint32_t)optlen <= sizeof(socket_desc->rcv_timeout))
            {
              /* A tempo already exists at low level and cannot be redefined */
              /* Ok to accept value setting but :
                 value will not be used due to conflict risk
                 if tempo differ from low level tempo value */
              socket_desc->snd_timeout = *(const uint32_t *)optval;
              result = COM_SOCKETS_ERR_OK;
            }
            break;
          }
          /* Receive Timeout */
          case COM_SO_RCVTIMEO :
          {
            if ((uint32_t)optlen <= sizeof(socket_desc->rcv_timeout))
            {
              /* A tempo already exists at low level and cannot be redefined */
              /* Ok to accept value setting but :
                 if tempo value is shorter and data are received after
                 then if socket is not closing data still available in the modem
                 and can still be read if modem manage this feature
                 if tempo value is bigger and data are received before
                 then data will be send to application */
              socket_desc->rcv_timeout = *(const uint32_t *)optval;
              result = COM_SOCKETS_ERR_OK;
            }
            break;
          }
          /* Error */
          case COM_SO_ERROR :
          {
            /* Set for this option NOK */
            break;
          }
          default :
          {
            /* Other options NOT YET SUPPORTED */
            break;
          }
        }
      }
      else
      {
        /* Other level than SOL_SOCKET NOT YET SUPPORTED */
      }
      com_ip_modem_idlemode_request(false);
    }
  }
  else
  {
    result = COM_SOCKETS_ERR_DESCRIPTOR;
  }

  SOCKET_SET_ERROR(socket_desc, result);
  return result;
}


/**
  * @brief  Socket option get
  * @note   Get option for a socket
  * @note   only send timeout, receive timeout, last error supported
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which option is defined
  * @note   only COM_SOL_SOCKET supported
  * @param  optname   - option name for which the value is requested
  * @note
  *         - COM_SO_SNDTIMEO, COM_SO_RCVTIMEO, COM_SO_ERROR supported
  *         - any other value is rejected
  * @param  optval    - pointer to the buffer that will contain the option value
  * @note   COM_SO_SNDTIMEO, COM_SO_RCVTIMEO: in ms for timeout (uint32_t)
  *         COM_SO_ERROR : result of last operation (int32_t)
  * @param  optlen    - size of the buffer that will contain the option value
  * @note   must be sizeof(x32_t)
  * @retval int32_t   - ok or error value
  */
int32_t com_getsockopt_ip_modem(int32_t sock, int32_t level, int32_t optname,
                                void *optval, int32_t *optlen)
{
  int32_t result;
  socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  if (socket_desc != NULL)
  {
    if ((optval != NULL)
        && (optlen != NULL))
    {
      com_ip_modem_wakeup_request();
      if (level == COM_SOL_SOCKET)
      {
        switch (optname)
        {
          /* Send Timeout */
          case COM_SO_SNDTIMEO :
          {
            /* Force optval to be on uint32_t to be compliant with lwip */
            if ((uint32_t)*optlen == sizeof(uint32_t))
            {
              *(uint32_t *)optval = socket_desc->snd_timeout;
              result = COM_SOCKETS_ERR_OK;
            }
            break;
          }
          /* Receive Timeout */
          case COM_SO_RCVTIMEO :
          {
            /* Force optval to be on uint32_t to be compliant with lwip */
            if ((uint32_t)*optlen == sizeof(uint32_t))
            {
              *(uint32_t *)optval = socket_desc->rcv_timeout;
              result = COM_SOCKETS_ERR_OK;
            }
            break;
          }
          /* Error */
          case COM_SO_ERROR :
          {
            /* Force optval to be on int32_t to be compliant with lwip */
            if ((uint32_t)*optlen == sizeof(int32_t))
            {
              SOCKET_GET_ERROR(socket_desc, optval);
              socket_desc->error = COM_SOCKETS_ERR_OK;
              result = COM_SOCKETS_ERR_OK;
            }
            break;
          }
          default :
          {
            /* Other options NOT YET SUPPORTED */
            break;
          }
        }
      }
      else
      {
        /* Other level than SOL_SOCKET NOT YET SUPPORTED */
      }
      com_ip_modem_idlemode_request(false);
    }
  }
  else
  {
    result = COM_SOCKETS_ERR_DESCRIPTOR;
  }

  SOCKET_SET_ERROR(socket_desc, result);
  return result;
}


/**
  * @brief  Socket bind
  * @note   Assign a local address and port to a socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - local IP address and port
  * @note   only port value field is used as local port,
  *         but whole addr parameter must be "valid"
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_bind_ip_modem(int32_t sock,
                          const com_sockaddr_t *addr, int32_t addrlen)
{
  int32_t result;

  socket_addr_t socket_addr;
  socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  if (socket_desc != NULL)
  {
    com_ip_modem_wakeup_request();
    /* Bind supported only in Created state */
    if (socket_desc->state == COM_SOCKET_CREATED)
    {
      if (com_translate_ip_address(addr, addrlen,
                                   &socket_addr)
          == true)
      {
        result = COM_SOCKETS_ERR_GENERAL;
        PRINT_DBG("socket bind request")

        if (osCDS_socket_bind(sock,
                              socket_addr.port)
            == CELLULAR_OK)
        {
          PRINT_INFO("socket bind ok low level")
          result = COM_SOCKETS_ERR_OK;
          socket_desc->local_port = socket_addr.port;
        }
      }
      else
      {
        PRINT_ERR("socket bind NOK translate IP NOK")
      }
    }
    else
    {
      result = COM_SOCKETS_ERR_STATE;
      PRINT_ERR("socket bind NOK state invalid")
    }
    com_ip_modem_idlemode_request(false);
  }

  SOCKET_SET_ERROR(socket_desc, result);
  return result;
}


/**
  * @brief  Socket listen
  * @note   Set socket in listening mode
  *         NOT YET SUPPORTED
  * @param  sock      - socket handle obtained with com_socket
  * @param  backlog   - number of connection requests that can be queued
  * @retval int32_t   - COM_SOCKETS_ERR_UNSUPPORTED
  */
int32_t com_listen_ip_modem(int32_t sock,
                            int32_t backlog)
{
  UNUSED(sock);
  UNUSED(backlog);
  return COM_SOCKETS_ERR_UNSUPPORTED;
}


/**
  * @brief  Socket accept
  * @note   Accept a connect request for a listening socket
  *         NOT YET SUPPORTED
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - IP address and port number of the accepted connection
  * @param  addrlen   - addr length
  * @retval int32_t   - COM_SOCKETS_ERR_UNSUPPORTED
  */
int32_t com_accept_ip_modem(int32_t sock,
                            com_sockaddr_t *addr, int32_t *addrlen)
{
  UNUSED(sock);
  UNUSED(addr);
  UNUSED(addrlen);
  return COM_SOCKETS_ERR_UNSUPPORTED;
}


/**
  * @brief  Socket connect
  * @note   Connect socket to a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - remote IP address and port
  * @note   only an IPv4 address is supported
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_connect_ip_modem(int32_t sock,
                             const com_sockaddr_t *addr, int32_t addrlen)
{
  int32_t result;
  socket_addr_t socket_addr;
  socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;

  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  /* Check parameters validity */
  if (socket_desc != NULL)
  {
    if (com_translate_ip_address(addr, addrlen,
                                 &socket_addr)
        == true)
    {
      /* Parameters are valid */
      result = COM_SOCKETS_ERR_OK;
    }
  }

  /* If parameters are valid continue the treatment */
  if (result == COM_SOCKETS_ERR_OK)
  {
    if (socket_desc->type == (uint8_t)COM_SOCK_STREAM)
    {
      if (socket_desc->state == COM_SOCKET_CREATED)
      {
        /* Check Network status */
        if (com_ip_modem_is_network_up() == true)
        {
          com_ip_modem_wakeup_request();
          if (osCDS_socket_connect(socket_desc->id,
                                   socket_addr.ip_type,
                                   &socket_addr.ip_value[0],
                                   socket_addr.port)
              == CELLULAR_OK)
          {
            result = COM_SOCKETS_ERR_OK;
            PRINT_INFO("socket connect ok")
            socket_desc->state = COM_SOCKET_CONNECTED;
          }
          else
          {
            result = COM_SOCKETS_ERR_GENERAL;
            PRINT_ERR("socket connect NOK at low level")
          }
          com_ip_modem_idlemode_request(false);
        }
        else
        {
          result = COM_SOCKETS_ERR_NONETWORK;
          PRINT_ERR("socket connect NOK no network")
        }
      }
      else
      {
        result = COM_SOCKETS_ERR_STATE;
        PRINT_ERR("socket connect NOK err state")
      }
    }
    else /* socket_desc->type == (uint8_t)COM_SOCK_DGRAM */
    {
#if (UDP_SERVICE_SUPPORTED == 0U)
      /* even if CONNECTED let MODEM decide if it is supported
         to update internal configuration */
      /* for DGRAM no need to check network status
         because connection is internal */
      if ((socket_desc->state == COM_SOCKET_CREATED)
          || (socket_desc->state == COM_SOCKET_CONNECTED))
      {
        com_ip_modem_wakeup_request();
        if (osCDS_socket_connect(socket_desc->id,
                                 socket_addr.ip_type,
                                 &socket_addr.ip_value[0],
                                 socket_addr.port)
            == CELLULAR_OK)
        {
          result = COM_SOCKETS_ERR_OK;
          PRINT_INFO("socket connect ok")
          socket_desc->state = COM_SOCKET_CONNECTED;
        }
        else
        {
          result = COM_SOCKETS_ERR_GENERAL;
          PRINT_ERR("socket connect NOK at low level")
        }
        com_ip_modem_idlemode_request(false);
      }
      else
      {
        result = COM_SOCKETS_ERR_STATE;
        PRINT_ERR("socket connect NOK err state")
      }
#else /* UDP_SERVICES_SUPPORTED == 1U */
      /* A specific udp service connection is done
         in order to be able to use sendto/recvfrom services */
      result = com_ip_modem_connect_udp_service(socket_desc);
#endif /* UDP_SERVICES_SUPPORTED == 0U */
    }
  }
  if (socket_desc != NULL)
  {
    /* if com_translate_ip_address == FALSE, result already set to COM_SOCKETS_ERR_PARAMETER */
//    com_sockets_statistic_update((result == COM_SOCKETS_ERR_OK) ? \
//                                 COM_SOCKET_STAT_CNT_OK : COM_SOCKET_STAT_CNT_NOK);
    SOCKET_SET_ERROR(socket_desc, result);
  }

  if (result == COM_SOCKETS_ERR_OK)
  {
    /* Save remote addr - port */
    com_ip_addr_t remote_addr;
    uint16_t remote_port;

    com_convert_sockaddr_to_ipaddr_port((const com_sockaddr_in_t *)addr,
                                        &remote_addr,
                                        &remote_port);
    socket_desc->remote_addr.addr = remote_addr.addr;
    socket_desc->remote_port = remote_port;
  }

  return result;
}


/**
  * @brief  Socket send data
  * @note   Send data on already connected socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @note   see below
  * @param  len       - length of the data to send (in bytes)
  * @note   see below
  * @param  flags     - options
  * @note
  *         - if flags = COM_MSG_DONTWAIT, application request to not wait
  *         if len of buffer to send > interface between COM and low level.
  *          The maximum of interface will be send (only one send)
  *         - if flags = COM_MSG_WAIT, application accept to wait
  *         if len of buffer to send > interface between COM and low level.
  *          COM will fragment the buffer according to the interface (multiple sends)
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_send_ip_modem(int32_t sock,
                          const com_char_t *buf, int32_t len,
                          int32_t flags)
{
  bool is_network_up;
  socket_desc_t *socket_desc;
  int32_t result;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  while (socket_desc->state == COM_SOCKET_WAITING_RSP)
  {
	  (void) osDelay(100U);
	  socket_desc = com_ip_modem_find_socket(sock,
	                                         false);
      PRINT_ERR("********************************* com_send_ip_modem WAITING RESPONSE, %d *******************************",socket_desc->state)
  }

  if ((socket_desc != NULL)
      && (buf != NULL)
      && (len > 0))
  {
    if (socket_desc->state == COM_SOCKET_CONNECTED)
    {
      /* closing maybe received, refuse to send data */
      if (socket_desc->closing == false)
      {
        /* network maybe down, refuse to send data */
        if (com_ip_modem_is_network_up() == false)
        {
          result = COM_SOCKETS_ERR_NONETWORK;
          PRINT_ERR("snd data NOK no network")
        }
        else
        {
          com_ip_modem_wakeup_request();

          /* if UDP_SERVICE supported,
             Connect already done by Appli => send may be changed to sendto
             or by COM to use sendto/recvfrom services => send must be changed to sendto */
          if ((socket_desc->type == (uint8_t)COM_SOCK_DGRAM)
              && (UDP_SERVICE_SUPPORTED == 1U))
          {
            result = com_sendto_ip_modem(sock, buf, len, flags, NULL, 0);
          }
          else
          {
            uint32_t length_to_send;
            uint32_t length_send;

            result = COM_SOCKETS_ERR_GENERAL;
            length_send = 0U;
            socket_desc->state = COM_SOCKET_SENDING;

            if (flags == COM_MSG_DONTWAIT)
            {
              length_to_send = COM_MIN((uint32_t)len, COM_MODEM_MAX_TX_DATA_SIZE);
              if (osCDS_socket_send(socket_desc->id,
                                    buf, length_to_send)
                  == CELLULAR_OK)
              {
                length_send = length_to_send;
                result = (int32_t)length_send;
                PRINT_INFO("snd data DONTWAIT ok")
              }
              else
              {
                PRINT_ERR("snd data DONTWAIT NOK at low level")
              }
              socket_desc->state = COM_SOCKET_CONNECTED;
            }
            else
            {
              is_network_up = com_ip_modem_is_network_up();
              /* Send all data of a big buffer - Whatever the size */
              while ((length_send != (uint32_t)len)
                     && (socket_desc->closing == false)
                     && (is_network_up == true)
                     && (socket_desc->state == COM_SOCKET_SENDING))
              {
                length_to_send = COM_MIN((((uint32_t)len) - length_send),
                                         COM_MODEM_MAX_TX_DATA_SIZE);
                com_ip_modem_wakeup_request();
                /* A tempo is already managed at low-level */
                if (osCDS_socket_send(socket_desc->id,
                                      buf + length_send,
                                      length_to_send)
                    == CELLULAR_OK)
                {
                  length_send += length_to_send;
                  PRINT_INFO("snd data ok")
                  /* Update Network status */
                  is_network_up = com_ip_modem_is_network_up();
                }
                else
                {
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  PRINT_ERR("snd data NOK at low level")
                }
                com_ip_modem_idlemode_request(false);
              }
              socket_desc->state = COM_SOCKET_CONNECTED;
              result = (int32_t)length_send;
            }
          }
          com_ip_modem_idlemode_request(false);
        }
      }
      else
      {
        PRINT_ERR("snd data NOK socket closing")
        result = COM_SOCKETS_ERR_CLOSING;
      }
    }
    else
    {
      PRINT_ERR("snd data NOK err state")
      if (socket_desc->state < COM_SOCKET_CONNECTED)
      {
        result = COM_SOCKETS_ERR_STATE;
      }
      else
      {
        result = (socket_desc->state == COM_SOCKET_CLOSING) ? \
                 COM_SOCKETS_ERR_CLOSING : COM_SOCKETS_ERR_INPROGRESS;
      }
    }

    /* Do not count twice with sendto */
    if ((socket_desc->type == (uint8_t)COM_SOCK_STREAM)
        || (UDP_SERVICE_SUPPORTED == 0U))
    {
//      com_sockets_statistic_update((result >= 0) ? \
//                                   COM_SOCKET_STAT_SND_OK : COM_SOCKET_STAT_SND_NOK);
    }
    /* else statitic updated by sendto function */
  }

  if (result >= 0)
  {
    SOCKET_SET_ERROR(socket_desc, COM_SOCKETS_ERR_OK);
  }
  else
  {
    SOCKET_SET_ERROR(socket_desc, result);
  }

  return (result);
}


/**
  * @brief  Socket send to data
  * @note   Send data to a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @note   see below
  * @param  len       - length of the data to send (in bytes)
  * @note   see below
  * @param  flags     - options
  * @note
  *         - if flags = COM_MSG_DONTWAIT, application request to not wait
  *         if len of buffer to send > interface between COM and low level.
  *          The maximum of interface will be send (only one send)
  *         - if flags = COM_MSG_WAIT, application accept to wait
  *         if len of buffer to send > interface between COM and low level.
  *          COM will fragment the buffer according to the interface (multiple sends)
  * @param  to        - remote IP address and port
  * @note   only an IPv4 address is supported
  * @param  tolen     - remote IP length
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_sendto_ip_modem(int32_t sock,
                            const com_char_t *buf, int32_t len,
                            int32_t flags,
                            const com_sockaddr_t *to, int32_t tolen)
{
  socket_desc_t *socket_desc;
  int32_t result;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  if ((socket_desc != NULL)
      && (buf != NULL)
      && (len > 0))
  {
    if (socket_desc->type == (uint8_t)COM_SOCK_STREAM)
    {
      /* sendto service may be called and it is changed to send service */
      result = com_send_ip_modem(sock, buf, len, flags);
    }
    else /* socket_desc->type == (uint8_t)COM_SOCK_DGRAM */
    {
      /* If Modem doesn't support sendto rather than to test:
         if connect already done by appli
         and if IPaddress in sendto equal to IPaddress in connect
         decision is to return an error
      */
#if (UDP_SERVICE_SUPPORTED == 0U)
      {
        result = COM_SOCKETS_ERR_UNSUPPORTED;
      }
#else /* UDP_SERVICE_SUPPORTED == 1U */
      {
        bool is_network_up;
        socket_addr_t socket_addr;

        /* Check remote addr is valid */
        if ((to != NULL) && (tolen != 0))
        {
          if (com_translate_ip_address(to, tolen,
                                       &socket_addr)
              == true)
          {
            result = COM_SOCKETS_ERR_OK;
          }
          /* else result = COM_SOCKETS_ERR_PARAMETER */
        }
        /* No address provided by connect previsouly done */
        /* a send translate to sendto */
        /* Use IPaddress of connect */
        else if ((to == NULL) && (tolen == 0)
                 && (socket_desc->remote_addr.addr != 0U))
        {
          com_sockaddr_in_t sockaddr_in;
          com_ip_addr_t remote_addr;
          uint16_t remote_port;

          remote_addr.addr = socket_desc->remote_addr.addr;
          remote_port = socket_desc->remote_port;
          com_convert_ipaddr_port_to_sockaddr(&remote_addr,
                                              remote_port,
                                              &sockaddr_in);

          if (com_translate_ip_address((com_sockaddr_t *)&sockaddr_in,
                                       (int32_t)sizeof(sockaddr_in),
                                       &socket_addr)
              == true)
          {
            result = COM_SOCKETS_ERR_OK;
          }
          else
          {
            /* else result = COM_SOCKETS_ERR_PARAMETER */
          }
        }
        else
        {
          /* else result = COM_SOCKETS_ERR_PARAMETER */
        }

        if (result == COM_SOCKETS_ERR_OK)
        {
          /* If socket state == CREATED implicit bind and connect UDP service must be done */
          /* Without updating internal parameters
             => com_ip_modem_connect must not be called */
          result = com_ip_modem_connect_udp_service(socket_desc);

          /* closing maybe received, refuse to send data */
          if ((result == COM_SOCKETS_ERR_OK)
              && (socket_desc->closing == false)
              && (socket_desc->state == COM_SOCKET_CONNECTED))
          {
            /* network maybe down, refuse to send data */
            if (com_ip_modem_is_network_up() == false)
            {
              result = COM_SOCKETS_ERR_NONETWORK;
              PRINT_ERR("sndto data NOK no network")
            }
            else
            {
              uint32_t length_to_send;
              uint32_t length_send;

              result = COM_SOCKETS_ERR_GENERAL;
              length_send = 0U;
              socket_desc->state = COM_SOCKET_SENDING;

              com_ip_modem_wakeup_request();

              if (flags == COM_MSG_DONTWAIT)
              {
                length_to_send = COM_MIN((uint32_t)len, COM_MODEM_MAX_TX_DATA_SIZE);

                if (osCDS_socket_sendto(socket_desc->id,
                                        buf, length_to_send,
                                        socket_addr.ip_type,
                                        socket_addr.ip_value,
                                        socket_addr.port)
                    == CELLULAR_OK)
                {
                  length_send = length_to_send;
                  result = (int32_t)length_send;
                  PRINT_INFO("sndto data DONTWAIT ok")
                }
                else
                {
                  PRINT_ERR("sndto data DONTWAIT NOK at low level")
                }
                socket_desc->state = COM_SOCKET_CONNECTED;
              }
              else
              {
                is_network_up = com_ip_modem_is_network_up();
                /* Send all data of a big buffer - Whatever the size */
                while ((length_send != (uint32_t)len)
                       && (socket_desc->closing == false)
                       && (is_network_up == true)
                       && (socket_desc->state == COM_SOCKET_SENDING))
                {
                  length_to_send = COM_MIN((((uint32_t)len) - length_send),
                                           COM_MODEM_MAX_TX_DATA_SIZE);
                  com_ip_modem_wakeup_request();
                  /* A tempo is already managed at low-level */
                  if (osCDS_socket_sendto(socket_desc->id,
                                          buf + length_send,
                                          length_to_send,
                                          socket_addr.ip_type,
                                          socket_addr.ip_value,
                                          socket_addr.port)
                      == CELLULAR_OK)
                  {
                    length_send += length_to_send;
                    PRINT_INFO("sndto data ok")
                    /* Update Network status */
                    is_network_up = com_ip_modem_is_network_up();
                  }
                  else
                  {
                    socket_desc->state = COM_SOCKET_CONNECTED;
                    PRINT_ERR("sndto data NOK at low level")
                  }
                  com_ip_modem_idlemode_request(false);
                }
                socket_desc->state = COM_SOCKET_CONNECTED;
                result = (int32_t)length_send;
              }
              com_ip_modem_idlemode_request(false);
            }
          }
          else
          {
            if (socket_desc->closing == true)
            {
              PRINT_ERR("sndto data NOK socket closing")
              result = COM_SOCKETS_ERR_CLOSING;
            }
            else
            {
              /* else result already updated com_ip_modem_connect_udp_service */
            }
          }

//          com_sockets_statistic_update((result >= 0) ? \
//                                       COM_SOCKET_STAT_SND_OK : COM_SOCKET_STAT_SND_NOK);
        }
        else
        {
          /* result = COM_SOCKETS_ERR_PARAMETER */
        }
      }
#endif /* UDP_SERVICE_SUPPORTED == 0U */
    }
  }

  if (result >= 0)
  {
    SOCKET_SET_ERROR(socket_desc, COM_SOCKETS_ERR_OK);
  }
  else
  {
    SOCKET_SET_ERROR(socket_desc, result);
  }

  return (result);
}


/**
  * @brief  Socket receive data
  * @note   Receive data on already connected socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @note   see below
  * @param  len       - size of application data buffer (in bytes)
  * @note   even if len > interface between COM and low level
  *         a maximum of the interface capacity can be received
  *         at each function call
  * @param  flags     - options
  * @note   - if flags = COM_MSG_DONTWAIT, application request to not wait
  *         until data are available at low level
  *         - if flags = COM_MSG_WAIT, application accept to wait
  *         until data are available at low level with respect of potential
  *         timeout COM_SO_RCVTIMEO setting
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recv_ip_modem(int32_t sock,
                          com_char_t *buf, int32_t len,
                          int32_t flags)
{
  int32_t result;
  int32_t len_rcv;
  osEvent event;
  socket_desc_t *socket_desc;
  com_socket_msg_t   msg;

  result = COM_SOCKETS_ERR_PARAMETER;
  len_rcv = 0;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  while (socket_desc->state == COM_SOCKET_SENDING)
  {
	  (void) osDelay(10U);
	  socket_desc = com_ip_modem_find_socket(sock,
	                                         false);

  }
  if ((socket_desc != NULL)
      && (buf != NULL)
      && (len > 0))
  {
    /* Closing maybe received or Network maybe done
       but still some data to read */
    if (socket_desc->state == COM_SOCKET_CONNECTED)
    {
      uint32_t length_to_read;
      length_to_read = COM_MIN((uint32_t)len, COM_MODEM_MAX_RX_DATA_SIZE);
      socket_desc->state = COM_SOCKET_WAITING_RSP;

      com_ip_modem_wakeup_request();

      do
      {
        event = osMessageGet(socket_desc->queue, 0U);
        if (event.status == osEventMessage)
        {
          PRINT_DBG("rcv cleanup MSGqueue")
        }
      } while (event.status == osEventMessage);

      if (flags == COM_MSG_DONTWAIT)
      {

        /* Application don't want to wait if there is no data available */
        len_rcv = osCDS_socket_receive(socket_desc->id,
                                       buf, length_to_read);
        result = (len_rcv < 0) ? COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
        socket_desc->state = COM_SOCKET_CONNECTED;
        PRINT_INFO("rcv data DONTWAIT")
      }
      else
      {
        /* Maybe still some data available
           because application don't read all data with previous calls */
        PRINT_DBG("rcv data waiting")
        (void) osDelay(5000U);
        PRINT_DBG("d-rcv data waiting")
        len_rcv = osCDS_socket_receive(socket_desc->id,
                                       buf, length_to_read);
        PRINT_DBG("rcv data waiting exit")

        if (len_rcv == 0)
        {
          /* Waiting for Distant response or Closure Socket or Timeout */
          event = osMessageGet(socket_desc->queue,
                               socket_desc->rcv_timeout);
          if (event.status == osEventTimeout)
          {
            result = COM_SOCKETS_ERR_TIMEOUT;
            socket_desc->state = COM_SOCKET_CONNECTED;
            PRINT_INFO("rcv data exit timeout")
          }
          else
          {
            msg = event.value.v;

            if (GET_SOCKET_MSG_TYPE(msg) == COM_SOCKET_MSG)
            {
              switch (GET_SOCKET_MSG_ID(msg))
              {
                case COM_DATA_RCV :
                {
                  len_rcv = osCDS_socket_receive(socket_desc->id,
                                                 buf, length_to_read);
                  result = (len_rcv < 0) ? \
                           COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  if (len_rcv == 0)
                  {
                    PRINT_DBG("rcv data exit with no data")
                  }
                  PRINT_INFO("rcv data exit with data")
                  break;
                }
                case COM_CLOSING_RCV :
                {
                  result = COM_SOCKETS_ERR_CLOSING;
                  socket_desc->state = COM_SOCKET_CLOSING;
                  PRINT_INFO("rcv data exit socket closing")
                  break;
                }
                default :
                {
                  /* Impossible case */
                  result = COM_SOCKETS_ERR_GENERAL;
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  PRINT_ERR("rcv data exit NOK impossible case")
                  break;
                }
              }
            }
            else
            {
              /* Impossible case */
              result = COM_SOCKETS_ERR_GENERAL;
              socket_desc->state = COM_SOCKET_CONNECTED;
              PRINT_ERR("rcv data msg NOK impossible case")
            }
          }
        }
        else
        {
          result = (len_rcv < 0) ? COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
          socket_desc->state = COM_SOCKET_CONNECTED;
          PRINT_INFO("rcv data exit data available or err low level")
        }
      }

      do
      {
        event = osMessageGet(socket_desc->queue, 0U);
        if (event.status == osEventMessage)
        {
          PRINT_DBG("rcv data exit cleanup MSGqueue")
        }
      } while (event.status == osEventMessage);

      com_ip_modem_idlemode_request(false);
    }
    else
    {
      PRINT_ERR("rcv data NOK err state")
      if (socket_desc->state < COM_SOCKET_CONNECTED)
      {
        result = COM_SOCKETS_ERR_STATE;
      }
      else
      {
        result = (socket_desc->state == COM_SOCKET_CLOSING) ? \
                 COM_SOCKETS_ERR_CLOSING : COM_SOCKETS_ERR_INPROGRESS;
      }
    }

//    com_sockets_statistic_update((result == COM_SOCKETS_ERR_OK) ? \
//                                 COM_SOCKET_STAT_RCV_OK : COM_SOCKET_STAT_RCV_NOK);
  }

  SOCKET_SET_ERROR(socket_desc, result);
  return ((result == COM_SOCKETS_ERR_OK) ? len_rcv : result);
}


/**
  * @brief  Socket receive from data
  * @note   Receive data from a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @note   see below
  * @param  len       - size of application data buffer (in bytes)
  * @note   even if len > interface between COM and low level
  *         a maximum of the interface capacity can be received
  *         at each function call
  * @param  flags     - options
  * @note   - if flags = COM_MSG_DONTWAIT, application request to not wait
  *         until data are available at low level
  *         - if flags = COM_MSG_WAIT, application accept to wait
  *         until data are available at low level with respect of potential
  *         timeout COM_SO_RCVTIMEO setting
  * @param  from      - remote IP address and port
  * @note   only an IPv4 address is supported
  *         if information is reported by the modem
  *         elsif value 0 is returned as remote IP address and port
  * @param  fromlen   - remote IP length
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recvfrom_ip_modem(int32_t sock,
                              com_char_t *buf, int32_t len,
                              int32_t flags,
                              com_sockaddr_t *from, int32_t *fromlen)
{
  int32_t result;
  int32_t len_rcv;
  socket_desc_t *socket_desc;
  CS_CHAR_t     ip_addr_value[40];
  uint16_t      ip_remote_port;

  result = COM_SOCKETS_ERR_PARAMETER;
  len_rcv = 0;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  ip_remote_port = 0U;
  (void)strcpy((CSIP_CHAR_t *)&ip_addr_value[0],
               (const CSIP_CHAR_t *)"0.0.0.0");

  if ((socket_desc != NULL)
      && (buf != NULL)
      && (len > 0))
  {
    if (socket_desc->type == (uint8_t)COM_SOCK_STREAM)
    {
      /* recvfrom service may be called and it is changed to recv service */
      result = com_recv_ip_modem(sock, buf, len, flags);
      /* If data received set remote addr parameter to the connected addr */
      if ((result > 0)
          && (from != NULL)
          && (fromlen != NULL)
          && (*fromlen >= (int32_t)sizeof(com_sockaddr_in_t)))
      {
        com_ip_addr_t remote_addr;
        uint16_t remote_port;

        remote_addr.addr = socket_desc->remote_addr.addr;
        remote_port = socket_desc->remote_port;

        com_convert_ipaddr_port_to_sockaddr(&remote_addr,
                                            remote_port,
                                            (com_sockaddr_in_t *)from);

        *fromlen = (int32_t)sizeof(com_sockaddr_in_t);
      }
    }
    else /* socket_desc->type == (uint8_t)COM_SOCK_DGRAM */
    {
      /* If Modem doesn't support recvfrom rather than to test:
         if connect already done by appli
         decision is to return an error
      */
#if (UDP_SERVICE_SUPPORTED == 0U)
      {
        result = COM_SOCKETS_ERR_UNSUPPORTED;
      }
#else /* UDP_SERVICE_SUPPORTED == 1U */
      {
        osEvent event;
        com_socket_msg_t   msg;
        CS_IPaddrType_t ip_addr_type;

        ip_addr_type = CS_IPAT_INVALID;

        com_ip_modem_wakeup_request();

        /* If socket state == CREATED implicit bind and connect must be done */
        /* Without updating internal parameters
           => com_ip_modem_connect must not be called */
        result = com_ip_modem_connect_udp_service(socket_desc);

        /* closing maybe received, refuse to send data */
        if ((result == COM_SOCKETS_ERR_OK)
            && (socket_desc->state == COM_SOCKET_CONNECTED))
        {
          uint32_t length_to_read;
          length_to_read = COM_MIN((uint32_t)len, COM_MODEM_MAX_RX_DATA_SIZE);
          socket_desc->state = COM_SOCKET_WAITING_FROM;

          do
          {
            event = osMessageGet(socket_desc->queue, 0U);
            if (event.status == osEventMessage)
            {
              PRINT_DBG("rcvfrom cleanup MSGqueue ")
            }
          } while (event.status == osEventMessage);

          if (flags == COM_MSG_DONTWAIT)
          {
            /* Application don't want to wait if there is no data available */
            len_rcv = osCDS_socket_receivefrom(socket_desc->id,
                                               buf, length_to_read,
                                               &ip_addr_type,
                                               &ip_addr_value[0],
                                               &ip_remote_port);
            result = (len_rcv < 0) ? COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
            socket_desc->state = COM_SOCKET_CONNECTED;
            PRINT_INFO("rcvfrom data DONTWAIT")
          }
          else
          {
            /* Maybe still some data available
               because application don't read all data with previous calls */
            PRINT_DBG("rcvfrom data waiting")
            len_rcv = osCDS_socket_receivefrom(socket_desc->id,
                                               buf, length_to_read,
                                               &ip_addr_type,
                                               &ip_addr_value[0],
                                               &ip_remote_port);
            PRINT_DBG("rcvfrom data waiting exit")

            if (len_rcv == 0)
            {
              /* Waiting for Distant response or Closure Socket or Timeout */
              PRINT_DBG("rcvfrom data waiting on MSGqueue")
              event = osMessageGet(socket_desc->queue,
                                   socket_desc->rcv_timeout);
              PRINT_DBG("rcvfrom data exit from MSGqueue")
              if (event.status == osEventTimeout)
              {
                result = COM_SOCKETS_ERR_TIMEOUT;
                socket_desc->state = COM_SOCKET_CONNECTED;
                PRINT_INFO("rcvfrom data exit timeout")
              }
              else
              {
                msg = event.value.v;

                if (GET_SOCKET_MSG_TYPE(msg) == COM_SOCKET_MSG)
                {
                  switch (GET_SOCKET_MSG_ID(msg))
                  {
                    case COM_DATA_RCV :
                    {
                      len_rcv = osCDS_socket_receivefrom(socket_desc->id,
                                                         buf, (uint32_t)len,
                                                         &ip_addr_type,
                                                         &ip_addr_value[0],
                                                         &ip_remote_port);
                      result = (len_rcv < 0) ? \
                               COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
                      socket_desc->state = COM_SOCKET_CONNECTED;
                      if (len_rcv == 0)
                      {
                        PRINT_DBG("rcvfrom data exit with no data")
                      }
                      PRINT_INFO("rcvfrom data exit with data")
                      break;
                    }
                    case COM_CLOSING_RCV :
                    {
                      result = COM_SOCKETS_ERR_CLOSING;
                      /* socket_desc->state = COM_SOCKET_CLOSING; */
                      socket_desc->state = COM_SOCKET_CONNECTED;
                      PRINT_INFO("rcvfrom data exit socket closing")
                      break;
                    }
                    default :
                    {
                      /* Impossible case */
                      result = COM_SOCKETS_ERR_GENERAL;
                      socket_desc->state = COM_SOCKET_CONNECTED;
                      PRINT_ERR("rcvfrom data exit NOK impossible case")
                      break;
                    }
                  }
                }
                else
                {
                  /* Impossible case */
                  result = COM_SOCKETS_ERR_GENERAL;
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  PRINT_ERR("rcvfrom data msg NOK impossible case")
                }
              }
            }
            else
            {
              result = (len_rcv < 0) ? COM_SOCKETS_ERR_GENERAL : COM_SOCKETS_ERR_OK;
              socket_desc->state = COM_SOCKET_CONNECTED;
              PRINT_INFO("rcvfrom data exit data available or err low level")
            }
          }

          do
          {
            event = osMessageGet(socket_desc->queue, 0U);
            if (event.status == osEventMessage)
            {
              PRINT_DBG("rcvfrom data exit cleanup MSGqueue")
            }
          } while (event.status == osEventMessage);
        }
        else
        {
          result = (socket_desc->state == COM_SOCKET_CLOSING) ? \
                   COM_SOCKETS_ERR_CLOSING : COM_SOCKETS_ERR_INPROGRESS;
        }
        com_ip_modem_idlemode_request(false);

//        com_sockets_statistic_update((result == COM_SOCKETS_ERR_OK) ? \
//                                     COM_SOCKET_STAT_RCV_OK : COM_SOCKET_STAT_RCV_NOK);
      }
#endif /* UDP_SERVICE_SUPPORTED == 0U */
    }

    /* Update output from and fromlen parameters */
    if ((len_rcv > 0)
        && (from != NULL)
        && (fromlen != NULL)
        && (*fromlen >= (int32_t)sizeof(com_sockaddr_in_t)))
    {
      if (true
          == com_convert_IPString_to_sockaddr(ip_remote_port,
                                              (com_char_t *)(&ip_addr_value[0]),
                                              from))
      {
        *fromlen = (int32_t)sizeof(com_sockaddr_in_t);
      }
      else
      {
        *fromlen = 0;
      }
    }
  }

  SOCKET_SET_ERROR(socket_desc, result);
  return ((result == COM_SOCKETS_ERR_OK) ? len_rcv : result);
}


/**
  * @brief  Socket close
  * @note   Close a socket and release socket handle
  *         For an opened socket as long as socket close is in error value
  *         socket must be considered as not closed and handle as not released
  * @param  sock      - socket handle obtained with com_socket
  * @retval int32_t   - ok or error value
  */
int32_t com_closesocket_ip_modem(int32_t sock)
{
  int32_t result;
  socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(sock,
                                         false);

  if (socket_desc != NULL)
  {
    /* If socket is currently under process refused to close it */
    if ((socket_desc->state == COM_SOCKET_SENDING)
        || (socket_desc->state == COM_SOCKET_WAITING_RSP))
    {
      PRINT_ERR("close socket NOK err state")
      result = COM_SOCKETS_ERR_INPROGRESS;
    }
    else
    {
      result = COM_SOCKETS_ERR_GENERAL;
      com_ip_modem_wakeup_request();
      if (osCDS_socket_close(sock, 0U)
          == CELLULAR_OK)
      {
        com_ip_modem_delete_socket_desc(sock, false);
        result = COM_SOCKETS_ERR_OK;
        PRINT_INFO("close socket ok")
      }
      else
      {
        PRINT_INFO("close socket NOK low level")
      }
      com_ip_modem_idlemode_request(true);
    }
//    com_sockets_statistic_update((result == COM_SOCKETS_ERR_OK) ? \
//                                 COM_SOCKET_STAT_CLS_OK : COM_SOCKET_STAT_CLS_NOK);
  }


  return (result);
}


/*** Other functionalities ****************************************************/

/**
  * @brief  Get host IP from host name
  * @note   Retrieve host IP address from host name
  *         DNS resolver is a fix value in the module
  *         only a primary DNS is used
  * @param  name      - host name
  * @param  addr      - host IP corresponding to host name
  * @note   only IPv4 address is managed
  * @retval int32_t   - ok or error value
  */
int32_t com_gethostbyname_ip_modem(const com_char_t *name,
                                   com_sockaddr_t   *addr)
{
  int32_t result;
  CS_PDN_conf_id_t PDN_conf_id;
  CS_DnsReq_t  dns_req;
  CS_DnsResp_t dns_resp;

  PDN_conf_id = CS_PDN_CONFIG_DEFAULT;
  result = COM_SOCKETS_ERR_PARAMETER;

  if ((name != NULL)
      && (addr != NULL))
  {
    if (strlen((const CSIP_CHAR_t *)name) <= sizeof(dns_req.host_name))
    {
      (void)strcpy((CSIP_CHAR_t *)&dns_req.host_name[0],
                   (const CSIP_CHAR_t *)name);

      result = COM_SOCKETS_ERR_GENERAL;
      com_ip_modem_wakeup_request();
      if (osCDS_dns_request(PDN_conf_id,
                            &dns_req,
                            &dns_resp)
          == CELLULAR_OK)
      {
        PRINT_INFO("DNS resolution OK - Remote: %s IP: %s", name, dns_resp.host_addr)
        if (com_convert_IPString_to_sockaddr(0U,
                                             (com_char_t *)&dns_resp.host_addr[0],
                                             addr)
            == true)
        {
          PRINT_DBG("DNS conversion OK")
          result = COM_SOCKETS_ERR_OK;
        }
        else
        {
          PRINT_ERR("DNS conversion NOK")
        }
      }
      else
      {
        PRINT_ERR("DNS resolution NOK for %s", name)
      }
      com_ip_modem_idlemode_request(false);
    }
  }

  return (result);
}


/**
  * @brief  Get peer name
  * @note   Retrieve IP address and port number
  *         NOT YET SUPPORTED
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number of the peer
  * @param  namelen   - name length
  * @retval int32_t   - COM_SOCKETS_ERR_UNSUPPORTED
  */
int32_t com_getpeername_ip_modem(int32_t sock,
                                 com_sockaddr_t *name, int32_t *namelen)
{
  UNUSED(sock);
  UNUSED(name);
  UNUSED(namelen);
  return COM_SOCKETS_ERR_UNSUPPORTED;
}


/**
  * @brief  Get sock name
  * @note   Retrieve local IP address and port number
  *         NOT YET SUPPORTED
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number
  * @param  namelen   - name length
  * @retval int32_t   - COM_SOCKETS_ERR_UNSUPPORTED
  */
int32_t com_getsockname_ip_modem(int32_t sock,
                                 com_sockaddr_t *name, int32_t *namelen)
{
  UNUSED(sock);
  UNUSED(name);
  UNUSED(namelen);
  return COM_SOCKETS_ERR_UNSUPPORTED;
}


/*** Ping functionalities *****************************************************/

#if (USE_COM_PING == 1)
/**
  * @brief  Ping handle creation
  * @note   Create a ping session
  * @param  -
  * @retval int32_t  - ping handle or error value
  */
int32_t com_ping_ip_modem(void)
{
  int32_t result;
  socket_desc_t *socket_desc;

  /* Need to create a new socket_desc ? */
  socket_desc = com_ip_modem_provide_socket_desc(true);
  if (socket_desc == NULL)
  {
    result = COM_SOCKETS_ERR_NOMEMORY;
    PRINT_ERR("create ping NOK no memory")
    /* Socket descriptor is not existing in COM
       and nothing to do at low level */
  }
  else
  {
    /* Socket state is set directly to CREATED
       because nothing else as to be done */
    result = COM_SOCKETS_ERR_OK;
    socket_desc->state = COM_SOCKET_CREATED;
    com_ip_modem_wakeup_request(); /* to avoid to be stopped by a close socket */
  }

  return ((result == COM_SOCKETS_ERR_OK) ? socket_desc->id : result);
}


/**
  * @brief  Ping process request
  * @note   Process a ping
  * @param  ping     - ping handle obtained with com_ping
  * @param  addr     - remote IP address and port
  * @param  addrlen  - addr length
  * @param  timeout  - timeout for ping response (in sec)
  * @param  rsp      - ping response
  * @retval int32_t  - ok or error value
  */
int32_t com_ping_process_ip_modem(int32_t ping,
                                  const com_sockaddr_t *addr, int32_t addrlen,
                                  uint8_t timeout, com_ping_rsp_t *rsp)
{
  int32_t  result;
  uint32_t timeout_ms;
  socket_desc_t *socket_desc;
  socket_addr_t  socket_addr;
  CS_Ping_params_t ping_params;
  CS_PDN_conf_id_t PDN_conf_id;
  osEvent event;
  com_socket_msg_t  msg;

  result = COM_SOCKETS_ERR_PARAMETER;

  socket_desc = com_ip_modem_find_socket(ping,
                                         true);

  /* Check parameters validity and context */
  if (socket_desc != NULL)
  {
    /* No need each time to close the connection */
    if ((socket_desc->state == COM_SOCKET_CREATED)
        || (socket_desc->state == COM_SOCKET_CONNECTED))
    {
      if ((timeout != 0U)
          && (rsp != NULL)
          && (addr != NULL))
      {
        if (com_translate_ip_address(addr, addrlen,
                                     &socket_addr)
            == true)
        {
          /* Parameters are valid */
          result = COM_SOCKETS_ERR_OK;
        }
      }
    }
    else
    {
      result = COM_SOCKETS_ERR_STATE;
      PRINT_ERR("ping send NOK state invalid")
    }
  }

  /* If parameters are valid continue the treatment */
  if (result == COM_SOCKETS_ERR_OK)
  {
    /* Check Network status */
    if (com_ip_modem_is_network_up() == true)
    {
      /* Because URC can be received before Reply and
         in URC not possible to distinguish several Ping in parallel
         authorized only one Ping at a time */
      (void)osMutexWait(ComSocketsMutexHandle, RTOS_WAIT_FOREVER);
      if (ping_socket_id != COM_SOCKET_INVALID_ID)
      {
        /* a Ping is already in progress */
        result = COM_SOCKETS_ERR_INPROGRESS;
      }
      else
      {
        ping_socket_id = socket_desc->id;
        result = COM_SOCKETS_ERR_OK;
      }
      (void)osMutexRelease(ComSocketsMutexHandle);

      if (result == COM_SOCKETS_ERR_OK)
      {
        PDN_conf_id = CS_PDN_CONFIG_DEFAULT;
        ping_params.timeout = timeout;
        ping_params.pingnum = 1U;
        (void)strcpy((CSIP_CHAR_t *)&ping_params.host_addr[0],
                     (const CSIP_CHAR_t *)&socket_addr.ip_value[0]);
        socket_desc->rsp   = rsp;

        com_ip_modem_wakeup_request();
        /* Clean-Up Ping queue */
        do
        {
          event = osMessageGet(socket_desc->queue, 0U);
          if (event.status == osEventMessage)
          {
            PRINT_DBG("Ping cleanup MSGqueue before osCDS_ping")
          }
        } while (event.status == osEventMessage);

        /* In order to receive response whatever the moment URC is received
           do not go under SENDING then WAITING_RSP state
           set the state in WAITING_RSP directly */
        socket_desc->state = COM_SOCKET_WAITING_RSP;

        /* Case 1) URC received before Reply
           Case 2) Reply received before URC */
        if (osCDS_ping(PDN_conf_id,
                       &ping_params,
                       com_ip_modem_ping_rsp_cb)
            == CELLULAR_OK)
        {
          /* Case 1) URC already available in the queue -> timeout not apply */
          /* Case 2) URC are still expected -> timeout apply */
#if (PING_URC_RECEIVED_AFTER_REPLY == 1U)
          timeout_ms = (uint32_t)timeout * 1000U;
#else /* PING_URC_RECEIVED_AFTER_REPLY == 0U */
          timeout_ms = 0U;
#endif /* PING_URC_RECEIVED_AFTER_REPLY == 1U */

          event = osMessageGet(socket_desc->queue,
                               timeout_ms);

          if (event.status == osEventTimeout)
          {
            /* Case 1) Impossible case or URC lost */
            /* Case 2) No URC available */
            result = COM_SOCKETS_ERR_TIMEOUT;
            socket_desc->state = COM_SOCKET_CONNECTED;
            PRINT_INFO("ping exit timeout")
          }
          else
          {
            msg = event.value.v;

            if (GET_SOCKET_MSG_TYPE(msg) == COM_PING_MSG)
            {
              switch (GET_SOCKET_MSG_ID(msg))
              {
                case COM_DATA_RCV :
                {
                  bool wait;
                  result = COM_SOCKETS_ERR_OK;
                  /* Sometimes rather than to receive only one report, 255 reports are received
                     Wait final report - in order to not reject next ping */
#if (PING_URC_RECEIVED_AFTER_REPLY == 1U)
                  timeout_ms = (uint32_t)COM_MIN(timeout, 5U) * 1000U;
#else /* PING_URC_RECEIVED_AFTER_REPLY == 0U */
                  timeout_ms = 0U; /* reply is send so final report must be available */
#endif /* PING_URC_RECEIVED_AFTER_REPLY == 1U */
                  wait = true;
                  while (wait == true)
                  {
                    event = osMessageGet(socket_desc->queue,
                                         timeout_ms);
                    if (event.status == osEventTimeout)
                    {
                      wait = false;
                    }
                    else
                    {
                      msg = event.value.v;
                      if (GET_SOCKET_MSG_ID(msg) == COM_CLOSING_RCV)
                      {
                        wait = false;
                      }
                    }
                  }
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  break;
                }
                case COM_CLOSING_RCV :
                {
                  /* Impossible case */
                  result = COM_SOCKETS_ERR_GENERAL;
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  PRINT_ERR("rcv data exit NOK closing case")
                  break;
                }
                default :
                {
                  /* Impossible case */
                  result = COM_SOCKETS_ERR_GENERAL;
                  socket_desc->state = COM_SOCKET_CONNECTED;
                  PRINT_ERR("rcv data exit NOK impossible case")
                  break;
                }
              }
            }
            else
            {
              /* Impossible case */
              result = COM_SOCKETS_ERR_GENERAL;
              socket_desc->state = COM_SOCKET_CONNECTED;
              PRINT_ERR("rcv socket msg NOK impossible case")
            }
          }
        }
        else
        {
          result = COM_SOCKETS_ERR_GENERAL;
          socket_desc->state = COM_SOCKET_CONNECTED;
          PRINT_ERR("ping send NOK at low level")
        }
        com_ip_modem_idlemode_request(false);
        /* Clean-Up Ping queue */
        do
        {
          event = osMessageGet(socket_desc->queue, 0U);
          if (event.status == osEventMessage)
          {
            PRINT_DBG("Ping cleanup MSGqueue after reply NOK at low level")
          }
        } while (event.status == osEventMessage);

        /* Release Ping */
        ping_socket_id = COM_SOCKET_INVALID_ID;
      }
    }
    else
    {
      result = COM_SOCKETS_ERR_NONETWORK;
      PRINT_ERR("ping send NOK no network")
    }
  }

  return result;
}


/**
  * @brief  Ping close
  * @note   Close a ping session and release ping handle
  * @param  ping      - ping handle obtained with com_ping
  * @retval int32_t   - ok or error value
  */
int32_t com_closeping_ip_modem(int32_t ping)
{
  int32_t result;
  const socket_desc_t *socket_desc;

  result = COM_SOCKETS_ERR_PARAMETER;
  socket_desc = com_ip_modem_find_socket(ping,
                                         true);

  if (socket_desc != NULL)
  {
    /* If socket is currently under process refused to close it */
    if ((socket_desc->state == COM_SOCKET_SENDING)
        || (socket_desc->state == COM_SOCKET_WAITING_RSP))
    {
      PRINT_ERR("close ping NOK err state")
      result = COM_SOCKETS_ERR_INPROGRESS;
    }
    else
    {
      com_ip_modem_delete_socket_desc(ping, true);
      result = COM_SOCKETS_ERR_OK;
      PRINT_INFO("close ping ok")
      com_ip_modem_idlemode_request(true); /* same behavior than all sockets closed */
    }
  }

  return result;
}

#endif /* USE_COM_PING == 1 */


/*** Component Initialization/Start *******************************************/
/*** Used by com_sockets module - Not an User Interface ***********************/

/**
  * @brief  Component initialization
  * @note   must be called only one time and
  *         before using any other functions of com_*
  * @param  -
  * @retval bool      - true/false init ok/nok
  */
bool com_init_ip_modem(void)
{
  bool result;

  result = false;

#if (USE_COM_PING == 1)
  ping_socket_id = COM_SOCKET_INVALID_ID;
#endif /* USE_COM_PING == 1 */

  for (uint8_t i = 0U; i < COM_SOCKET_LOCAL_ID_NB; i++)
  {
    socket_local_id[i] = false; /* set socket local id to unused */
  }

  /* Initialize Mutex to protect socket descriptor list access */
  osMutexDef(ComSocketsMutex);
  ComSocketsMutexHandle = osMutexCreate(osMutex(ComSocketsMutex));
  if (ComSocketsMutexHandle != NULL)
  {
    /* Create always the first element of the list */
    socket_desc_list = com_ip_modem_create_socket_desc();
    if (socket_desc_list != NULL)
    {
      result = true;
    }
  }

#if (USE_LOW_POWER == 1)
  /* Initialize Timer inactivity and its Mutex to check inactivity on socket */
  osTimerDef(ComTimerInactivity, com_ip_modem_timer_inactivity_cb);
  ComTimerInactivityId = osTimerCreate(osTimer(ComTimerInactivity), osTimerOnce, NULL);
  osMutexDef(ComTimerInactivityMutex);
  ComTimerInactivityMutexHandle = osMutexCreate(osMutex(ComTimerInactivityMutex));
  if ((ComTimerInactivityId == NULL)
      || (ComTimerInactivityMutexHandle == NULL))
  {
    com_timer_inactivity_state = COM_TIMER_INVALID;
    result = false;
  }
  else
  {
    com_timer_inactivity_state = COM_TIMER_IDLE;
  }
  com_nb_wake_up = 0U;
#endif /* USE_LOW_POWER == 1 */

#if (USE_DATACACHE == 1)
  com_network_is_up = false;
#else
  com_network_is_up = true;
#endif /* USE_DATACACHE == 1 */

#if (UDP_SERVICE_SUPPORTED == 1U)
  com_local_port = 0U; /* com_start_ip in charge to initialize it to a random value */
#endif /* UDP_SERVICE_SUPPORTED == 1U */

  return result;
}


/**
  * @brief  Component start
  * @note   must be called only one time but
  *         after com_init and dc_start
  *         and before using any other functions of com_*
  * @param  -
  * @retval -
  */
void com_start_ip_modem(void)
{
#if (USE_DATACACHE == 1)
  /* Datacache registration for netwok on/off status */
  (void)dc_com_register_gen_event_cb(&dc_com_db,
                                     com_socket_datacache_cb,
                                     (void *) NULL);
#endif /* USE_DATACACHE == 1 */

#if (UDP_SERVICE_SUPPORTED == 1U)
  uint32_t random;

  /* Initialize local port to a random value */
  if (HAL_OK != HAL_RNG_GenerateRandomNumber(&xHrng, &random))
  {
    random = (uint32_t)rand();
  }
  random = random & ~COM_LOCAL_PORT_BEGIN;
  random = random + COM_LOCAL_PORT_BEGIN;
  com_local_port = (uint16_t)(random);
#endif /* UDP_SERVICE_SUPPORTED == 1U */
}

#endif /* USE_SOCKETS_TYPE == USE_SOCKETS_MODEM */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
