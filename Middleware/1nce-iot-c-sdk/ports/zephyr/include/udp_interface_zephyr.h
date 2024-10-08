/**
 * @file udp_interface_zephyr.h
 * @brief UDP interface definitions to send and receive data over the
 * network via UDP in ZEPHYR OS.
 */


#include "udp_interface.h"

/**
 * @typedef OSNetwork_t
 */
struct OSNetwork
{
    int os_socket;
};

int nce_os_udp_connect( OSNetwork_t osnetwork,
                        OSEndPoint_t nce_oboarding );

int nce_os_udp_send( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToSend );

int nce_os_udp_recv( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToRecv );

int nce_os_udp_disconnect( OSNetwork_t osnetwork );
