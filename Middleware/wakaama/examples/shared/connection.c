/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Pascal Rieux - Please refer to git log
 *
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "connection.h"
#include "lwm2mclient.h"


Socket_t create_socket(const char * portStr, int addressFamily)
{
    uint8_t bind =1;
	SocketsSockaddr_t  pxAddress;
	pxAddress.usPort=atoi(portStr);
	pxAddress.ucSocketDomain=addressFamily;
	pxAddress.ulAddress=SOCKETS_inet_addr_quick( 127, 0, 0, 1 );
    Socket_t udp = SOCKETS_Socket( addressFamily, SOCKETS_SOCK_DGRAM,
                                               SOCKETS_IPPROTO_UDP );
    if( udp == SOCKETS_INVALID_SOCKET )
    {
    	IotLogError( "Failed to create new socket." );
        return SOCKETS_INVALID_SOCKET;
    }

    SOCKETS_SetSockOpt( udp, 0, SOCKETS_UDP_SERVICE, (uint8_t) bind, sizeof(uint8_t) );
    if (-1 == SOCKETS_Connect( udp,&pxAddress,sizeof( pxAddress ) ))
    {
    	SOCKETS_Close(udp);
        udp = SOCKETS_INVALID_SOCKET;
    }
    bind=0;
    SOCKETS_SetSockOpt( udp, 0, SOCKETS_UDP_SERVICE, (uint8_t) bind, sizeof(uint8_t) );
    return udp;
}

connection_t * connection_find(connection_t * connList,
                               SocketsSockaddr_t * addr,
                               size_t addrLen)
{
    connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {
        if ((connP->addrLen == addrLen)
         && (memcmp(&(connP->addr), addr->ulAddress, addrLen) == 0))
        {
            return connP;
        }
        connP = connP->next;
    }

    return connP;
}

connection_t * connection_new_incoming(connection_t * connList,
                                       Socket_t sock,
									   SocketsSockaddr_t * addr,
                                       size_t addrLen)
{
    connection_t * connP;

    connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP != NULL)
    {
        connP->sock = sock;
        memcpy(&(connP->addr), addr->ulAddress, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;
    }

    return connP;
}

connection_t * connection_create(connection_t * connList,
                                 Socket_t sock,
                                 char * host,
                                 char * port,
                                 int addressFamily)
{
	SocketsSockaddr_t *sa = NULL;
    connection_t * connP = NULL;
    Socket_t socket=SOCKETS_INVALID_SOCKET;
    SocketsSockaddr_t ServerAddress;
    int portint = atoi(port);
    ServerAddress.usPort = SOCKETS_htons( portint );
    ServerAddress.ulAddress = SOCKETS_GetHostByName( host );

    /* unused parameter */
    (void)sock;

    socket=SOCKETS_Socket( addressFamily, SOCKETS_SOCK_DGRAM,SOCKETS_IPPROTO_UDP );
	#ifdef ENABLE_DTLS

		SOCKETS_SetSockOpt( socket, 0, SOCKETS_SO_REQUIRE_TLS, NULL, ( size_t ) 0 );
    
	#endif

    if (socket >= 0)
    {
         if ( SOCKETS_Connect( socket, &ServerAddress, sizeof( ServerAddress ) ) == 0 )
         {
          	connP = connection_new_incoming(connList, socket, sa, sizeof( ServerAddress ));
         }
    }
    else{
      	SOCKETS_Close(socket);
      	return -1;
    }
    return connP;
}

void connection_free(connection_t * connList)
{
    while (connList != NULL)
    {
        connection_t * nextP;

        nextP = connList->next;
        lwm2m_free(connList);
        connList = nextP;
    }
}

int connection_send(connection_t *connP,
                    uint8_t * buffer,
                    size_t length)
{
    int nbSent;
    size_t offset;

#ifdef LWM2M_WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connP->addr.sin6_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&connP->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    else if (AF_INET6 == connP->addr.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&connP->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    }

    IotLogInfo("Sending %lu bytes to [%s]:%hu\r\n", length, s, ntohs(port));

    output_buffer(stderr, buffer, length, 0);
#endif

    offset = 0;
    while (offset != length)
    {
    	nbSent = SOCKETS_Send( connP->sock, buffer + offset, length + offset , NULL );

        if (nbSent == -1) return -1;
        offset += nbSent;
    }
    return 0;
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    connection_t * connP = (connection_t*) sessionH;

    (void)userdata; /* unused */

    if (connP == NULL)
    {
        IotLogError("#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        IotLogError("#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    (void)userData; /* unused */

    return (session1 == session2);
}
