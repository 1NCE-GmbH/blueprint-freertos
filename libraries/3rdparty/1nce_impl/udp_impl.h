/*
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file tls_freertos.h
 * @brief TLS transport interface header.
 */

#ifndef UDP_IMPL
#define UDP_IMPL

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging related header files are required to be included in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define LIBRARY_LOG_NAME and  LIBRARY_LOG_LEVEL.
 * 3. Include the header file "logging_stack.h".
 */

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Logging configuration for the Sockets. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "SDK_IMPL"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/

/* Amazon FreeRTOS network include. */
//#include "iot_secure_sockets.h"

/* Transport interface include. */
#include "nce_iot_c_sdk.h"
#include "cellular_types.h"
/* Exported variable ------------------------------------------------------- */
/* The application needs to provide the cellular handle for the usage of AT Commands */
extern CellularHandle_t CellularHandle;
/* External variable used to indicate Device Authenticator Status */
extern bool BIND;


int nce_os_udp_connect_impl(OSNetwork_t osnetwork,OSEndPoint_t nce_oboarding);


int nce_os_udp_disconnect_impl( OSNetwork_t pNetworkContext );


int32_t nce_os_udp_recv_impl( OSNetwork_t osnetwork,
                           void * pBuffer,
                           size_t bytesToRecv );


int32_t nce_os_udp_send_impl( OSNetwork_t osnetwork,
                           const void * pBuffer,
                           size_t bytesToSend );


CellularPktStatus_t nce_send_sms( char * smsTroubleshooting );

OSNetwork_t xOSNetwork= { 0 };
os_network_ops_t osNetwork={
		.os_socket=&xOSNetwork,
		.nce_os_udp_connect = nce_os_udp_connect_impl,
		.nce_os_udp_send=nce_os_udp_send_impl,
		.nce_os_udp_recv=nce_os_udp_recv_impl,
		.nce_os_udp_disconnect=nce_os_udp_disconnect_impl };
#endif /* ifndef UDP_IMPL */
