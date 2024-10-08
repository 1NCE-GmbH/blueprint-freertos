/**
 * @file udp_interface_zephyr.c
 * @brief Implements a UDP interface for ZEPHYR OS.
 * @author  Hatim Jamali & Mohamed Abdelmaksoud
 * @date 01 September 2022
 */


#ifndef NCE_SDK_H_
    #include <nce_iot_c_sdk.h>
#endif

#include <zephyr/kernel.h>
#include <stdio.h>
#include <modem/lte_lc.h>
#include <zephyr/net/socket.h>
#include "log_interface.h"
#include <udp_interface_zephyr.h>


LOG_MODULE_DECLARE( NCE_SDK, CONFIG_NCE_SDK_LOG_LEVEL );

/* Sample Network definitions */
struct OSNetwork xOSNetwork = { .os_socket = 0 };

int nce_os_udp_connect( OSNetwork_t osnetwork,
                        OSEndPoint_t nce_oboarding )
{
    int socket = zsock_socket( AF_INET, SOCK_DGRAM,
                               IPPROTO_UDP );
    int err;
    struct zsock_addrinfo * addr;
    struct zsock_addrinfo hints =
    {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_DGRAM
    };

    err = getaddrinfo( nce_oboarding.host,
                       NULL, &hints, &addr );

    NceOSLogDebug( "getaddrinfo status: %d\n", err );

    osnetwork->os_socket = socket;

    ( ( struct sockaddr_in * ) addr->ai_addr )->sin_port = htons( nce_oboarding.port );

    size_t peer_addr_size = sizeof( struct sockaddr_in );

    err = zsock_connect( socket, addr->ai_addr,
                         peer_addr_size );
    NceOSLogDebug( "UDP Socket Connect: %d\n", err );


    return err;
}

int nce_os_udp_send( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToSend )
{
    int flags = 0;
    int ret;

    ret = zsock_send( osnetwork->os_socket, pBuffer, bytesToSend, flags );
    NceOSLogDebug( "UDP Socket Send: %d\n", ret );

    return ret;
}


int nce_os_udp_recv( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToRecv )
{
    int flags = 0;
    int ret;

    ret = zsock_recv( osnetwork->os_socket, pBuffer, bytesToRecv, flags );
    NceOSLogDebug( "UDP Socket Receive: %d\n", ret );

    return ret;
}

int nce_os_udp_disconnect( OSNetwork_t osnetwork )
{
    int err;

    err = zsock_close( osnetwork->os_socket );
    NceOSLogDebug( "UDP Socket Disconnect: %d\n", err );

    return err;
}
