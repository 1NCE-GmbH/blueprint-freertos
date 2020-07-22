/**
  ******************************************************************************
  * @file    com_sockets.h
  * @author  MCD Application Team
  * @brief   Header for com_sockets.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COM_SOCKETS_H
#define COM_SOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "plf_config.h"

#include "com_sockets_addr_compat.h"
#include "com_sockets_net_compat.h"
#include "com_sockets_err_compat.h"

/** @addtogroup CELLULAR_INTERFACE
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                      ##### How to use COM Module #####
  ==============================================================================

  COM module allows :
  1) communication with a remote host using IP protocol
  2) ping a remote host

  IP stack can be at two different exclusive places:
  1) on MCU side : LwIP (when USE_SOCKETS_TYPE = USE_SOCKETS_LWIP)
  2) on Modem side : Modem socket (when USE_SOCKETS_TYPE = USE_SOCKETS_MODEM)

  com_socket interface offers a generic interface whatever the IP configuration selection.
  For each com_sockets services there is a declination in :
  - com_socket_lwip_mcu module when LwIP is on MCU side
  - com_socket_ip_mcu module when IP is on Modem side

  It is recommended for Application to call com_sockets interface services in order to be
  independent of the IP selection.

  @endverbatim
  */

/** @defgroup COM_SOCKETS COM module
  * @{
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup COM_SOCKETS_Constants Constants
  * @{
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup COM_SOCKETS_Types Types
  * @{
  */

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup COM_SOCKETS_Variables Variables
  * @{
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup COM_SOCKETS_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup COM_SOCKETS_Functions Functions
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup COM_SOCKETS_Functions
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Com Sockets interface #####
  ==============================================================================
  IP stack can be at two different places:
  1) on MCU side : LwIP (when USE_SOCKETS_TYPE = USE_SOCKETS_LWIP)
  2) on Modem side : Modem socket (when USE_SOCKETS_TYPE = USE_SOCKETS_MODEM))
  com_socket modules offers a generic interface whatever the configuration selected

  in Application call:
  generic com_socket services (e.g com_socket()) rather than
  com_socket_ip_modem() or com_socket_lwip_mcu() to be independent of the USE_SOCKETS_TYPE value
  @endverbatim

  */

/** @defgroup COM_SOCKETS_Functions_Socket Socket services
  * @brief    BSD Socket functions and communication flow
  * @{
  */


/** @defgroup COM_SOCKETS_Functions_Socket_Generic Generic Socket services
  * @{
  */

/*** Socket management ********************************************************/

/**
  * @brief  Socket handle creation
  * @note   Create a communication endpoint called socket
  * @param  family   - address family
  * @param  type     - connection type
  * @param  protocol - protocol type
  * @retval int32_t  - socket handle or error value
  */
int32_t com_socket(int32_t family, int32_t type, int32_t protocol);

/**
  * @brief  Socket option set
  * @note   Set option for the socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which the option is defined
  * @param  optname   - option name for which the value is to be set
  * @param  optval    - pointer to the buffer containing the option value
  * @param  optlen    - size of the buffer containing the option value
  * @retval int32_t   - ok or error value
  */
int32_t com_setsockopt(int32_t sock, int32_t level, int32_t optname,
                       const void *optval, int32_t optlen);

/**
  * @brief  Socket option get
  * @note   Get option for a socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  level     - level at which option is defined
  * @param  optname   - option name for which the value is requested
  * @param  optval    - pointer to the buffer that will contain the option value
  * @param  optlen    - size of the buffer that will contain the option value
  * @retval int32_t   - ok or error value
  */
int32_t com_getsockopt(int32_t sock, int32_t level, int32_t optname,
                       void *optval, int32_t *optlen);

/**
  * @brief  Socket bind
  * @note   Assign a local address and port to a socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - local IP address and port
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_bind(int32_t sock,
                 const com_sockaddr_t *addr, int32_t addrlen);

/**
  * @brief  Socket listen
  * @note   Set socket in listening mode
  * @param  sock      - socket handle obtained with com_socket
  * @param  backlog   - number of connection requests that can be queued
  * @retval int32_t   - ok or error value
  */
int32_t com_listen(int32_t sock,
                   int32_t backlog);

/**
  * @brief  Socket accept
  * @note   Accept a connect request for a listening socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - IP address and port number of the accepted connection
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_accept(int32_t sock,
                   com_sockaddr_t *addr, int32_t *addrlen);

/**
  * @brief  Socket connect
  * @note   Connect socket to a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  addr      - remote IP address and port
  * @param  addrlen   - addr length
  * @retval int32_t   - ok or error value
  */
int32_t com_connect(int32_t sock,
                    const com_sockaddr_t *addr, int32_t addrlen);

/**
  * @brief  Socket send data
  * @note   Send data on already connected socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @param  len       - length of the data to send (in bytes)
  * @param  flags     - options
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_send(int32_t sock,
                 const com_char_t *buf, int32_t len,
                 int32_t flags);

/**
  * @brief  Socket send to data
  * @note   Send data to a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to send
  * @param  len       - length of the data to send (in bytes)
  * @param  flags     - options
  * @param  to        - remote IP address and port number
  * @param  tolen     - remote IP length
  * @retval int32_t   - number of bytes sent or error value
  */
int32_t com_sendto(int32_t sock,
                   const com_char_t *buf, int32_t len,
                   int32_t flags,
                   const com_sockaddr_t *to, int32_t tolen);

/**
  * @brief  Socket receive data
  * @note   Receive data on already connected socket
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @param  len       - size of application data buffer (in bytes)
  * @param  flags     - options
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recv(int32_t sock,
                 com_char_t *buf, int32_t len,
                 int32_t flags);

/**
  * @brief  Socket receive from data
  * @note   Receive data from a remote host
  * @param  sock      - socket handle obtained with com_socket
  * @param  buf       - pointer to application data buffer to store the data to
  * @param  len       - size of application data buffer (in bytes)
  * @param  flags     - options
  * @param  from      - remote IP address and port number
  * @param  fromlen   - remote IP length
  * @retval int32_t   - number of bytes received or error value
  */
int32_t com_recvfrom(int32_t sock,
                     com_char_t *buf, int32_t len,
                     int32_t flags,
                     com_sockaddr_t *from, int32_t *fromlen);


/**
  * @brief  Socket close
  * @note   Close a socket and release socket handle
  * @param  sock      - socket handle obtained with com_socket
  * @retval int32_t   - ok or error value
  */
int32_t com_closesocket(int32_t sock);


/*** Other functionalities ****************************************************/

/**
  * @brief  Get host IP from host name
  * @note   Retrieve host IP address from host name
  * @param  name      - host name
  * @param  addr      - host IP corresponding to host name
  * @retval int32_t   - ok or error value
  */
int32_t com_gethostbyname(const com_char_t *name,
                          com_sockaddr_t   *addr);

/**
  * @brief  Get peer name
  * @note   Retrieve IP address and port number
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number of the peer
  * @param  namelen   - name length
  * @retval int32_t   - ok or error value
  */
int32_t com_getpeername(int32_t sock,
                        com_sockaddr_t *name, int32_t *namelen);

/**
  * @brief  Get sock name
  * @note   Retrieve local IP address and port number
  * @param  sock      - socket handle obtained with com_socket
  * @param  name      - IP address and port number
  * @param  namelen   - name length
  * @retval int32_t   - ok or error value
  */
int32_t com_getsockname(int32_t sock,
                        com_sockaddr_t *name, int32_t *namelen);
/**
  * @}
  */

/**
  * @}
  */


#if (USE_COM_PING == 1)

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Com Ping interface #####
  ==============================================================================
  IP stack can be at two different places:
  1) on MCU side : LwIP (when USE_SOCKETS_TYPE = USE_SOCKETS_LWIP)
  2) on Modem side : Modem socket (when USE_SOCKETS_TYPE = USE_SOCKETS_MODEM))
  com_socket modules offers a generic interface whatever the configuration selected

  in Application call:
  generic com_ping services (e.g com_ping()) rather than
  com_ping_ip_modem() or com_ping_lwip_mcu() to be independent of the USE_SOCKETS_TYPE value

  @endverbatim

  */

/** @defgroup COM_SOCKETS_Functions_Ping Ping services
  * @brief    PING functionalities (Create, Process and Close session)
  *           define USE_COM_PING must be set to 1
  * @{
  */


/** @defgroup COM_SOCKETS_Functions_Ping_Generic Generic Ping services
  * @{
  */

/*** PING functionalities *****************************************************/

/**
  * @brief  Ping handle creation
  * @note   Create a ping session
  * @param  -
  * @retval int32_t  - ping handle or error value
  */
int32_t com_ping(void);

/**
  * @brief  Ping process request
  * @note   Process a Ping
  * @param  ping     - ping handle obtained with com_ping
  * @param  addr     - remote IP address and port
  * @param  addrlen  - addr length
  * @param  timeout  - timeout for ping response (in sec)
  * @param  rsp      - ping response
  * @retval int32_t  - ok or error value
  */
int32_t com_ping_process(int32_t ping,
                         const com_sockaddr_t *addr, int32_t addrlen,
                         uint8_t timeout, com_ping_rsp_t *rsp);
/**
  * @brief  Ping close
  * @note   Close a ping session and release ping handle
  * @param  ping      - ping handle obtained with com_ping
  * @retval int32_t   - ok or error value
  */
int32_t com_closeping(int32_t ping);

/**
  * @}
  */

/**
  * @}
  */

#endif /* USE_COM_PING == 1 */

/** @defgroup COM_SOCKETS_Functions_Other Other services
  * @{
  */

/**
  * @}
  */


/**
  * @}
  */


/*** Component Initialization/Start *******************************************/
/*** Used by Cellular-Service - Not an User Interface *************************/

/**
  * @brief  Component initialization
  * @note   must be called only one time :
  *         - before using any other functions of com_*
  * @param  -
  * @retval bool      - true/false init ok/nok
  */
bool com_init(void);

/**
  * @brief  Component start
  * @note   must be called only one time but
            after com_init and dc_start
            and before using any other functions of com_*
  * @param  -
  * @retval -
  */
void com_start(void);



#ifdef __cplusplus
}
#endif

#endif /* COM_SOCKETS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
