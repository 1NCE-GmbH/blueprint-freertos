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
 *
 *******************************************************************************/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <liblwm2m.h>
#include "udp_impl.h"
// #include "nce_demo_config.h"
#include "iot_secure_sockets.h"
#include "cellular_types.h"
#define LWM2M_STANDARD_PORT_STR "5683"
#define LWM2M_STANDARD_PORT      5683
#define LWM2M_DTLS_PORT_STR     "5684"
#define LWM2M_DTLS_PORT          5684
#define LWM2M_BSSERVER_PORT_STR "5683"
#define LWM2M_BSSERVER_PORT      5683 

typedef struct _connection_t
{
    struct _connection_t *  next;
    Socket_t                sock;
    CellularIPAddress_t		addr;
    size_t                  addrLen;
} connection_t;

Socket_t create_socket(const char * portStr, int ai_family);

connection_t * connection_find(connection_t * connList, SocketsSockaddr_t * addr, size_t addrLen);
connection_t * connection_new_incoming(connection_t * connList, Socket_t sock, SocketsSockaddr_t * addr, size_t addrLen);
connection_t * connection_create(connection_t * connList, Socket_t sock, char * host, char * port, int addressFamily);

void connection_free(connection_t * connList);

int connection_send(connection_t *connP, uint8_t * buffer, size_t length);

#endif
