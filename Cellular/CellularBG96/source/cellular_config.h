/*
 * Amazon FreeRTOS Cellular Preview Release
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
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file cellular_config.h
 * @brief cellular config options.
 */

#ifndef __CELLULAR_CONFIG_H__
#define __CELLULAR_CONFIG_H__

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for DEMO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for DEMO.
 */
#include "iot_config.h"
#include "logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "CellularLib"
#endif

#ifdef IOT_LOG_LEVEL_CELLULAR_LIB
    #define LIBRARY_LOG_LEVEL        IOT_LOG_LEVEL_CELLULAR_LIB
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_NONE
    #endif
#endif

#include "logging_stack.h"

/* Include the logging setup header. This enables the logs. */
#include "iot_logging_setup.h"
/************ End of logging configuration ****************/

/* This is a project specific file and is used to override config values defined
 * in cellular_config_defaults.h. */

/**
 * Cellular comm interface make use of COM port on computer to communicate with
 * cellular module on windows simulator, for example "COM5".
 * #define CELLULAR_COMM_INTERFACE_PORT    "...insert here..."
 */
/* #define CELLULAR_COMM_INTERFACE_PORT    "COM3" */

/*
 * Default APN for network registartion.
 * #define CELLULAR_APN                    "...insert here..."
 */
#define CELLULAR_APN                    "iot.1nce.net"

/*
 * PDN context id for cellular network.
 */
#define CELLULAR_PDN_CONTEXT_ID         ( CELLULAR_PDN_CONTEXT_ID_MIN )

/*
 * PDN connect timeout for network registration.
 */
#define CELLULAR_PDN_CONNECT_TIMEOUT    ( 100000UL )

/*
 * Overwrite default config for different cellular modules.
 */
#define CELLULAR_IP_ADDRESS_MAX_SIZE                        ( 64U )

/* When enable CELLULAR_CONFIG_STATIC_ALLOCATION_CONTEXT,
 * below the contexts have statically allocated,
 * and reserves 4,016 bytes.
 *   _cellularManagerContext, _cellular_Context, _iotCommIntf
 *
 * When disable CELLULAR_CONFIG_STATIC_ALLOCATION_CONTEXT,
 * the contexts will be dynamically allocated when CELLULAR service is started. MEMTEST
 */
#define CELLULAR_CONFIG_STATIC_ALLOCATION_CONTEXT           ( 0U )

/* When enable CELLULAR_CONFIG_STATIC_ALLOCATION_COMM_CONTEXT,
 * below the contexts have statically allocated,
 * and reserves 1,772 bytes.
 *  _iotCommIntfCtx, _iotFifoBuffer, _iotCommIntfPhy
 *
 * When disable CELLULAR_CONFIG_STATIC_ALLOCATION_COMM_CONTEXT,
 * the contexts will be dynamically allocated when CELLULAR module is turned on.
 */
#define CELLULAR_CONFIG_STATIC_ALLOCATION_COMM_CONTEXT      ( 0U )

/* When enable CELLULAR_CONFIG_STATIC_ALLOCATION_SOCKET_CONTEXT,
 * below the context has statically allocated,
 * and reserves 232 (116x2) bytes if CELLULAR_NUM_SOCKET_MAX is '2'.
 *  _socketData
 * Customers can adjust the value of CELLULAR_NUM_SOCKET_MAX
 * between 1 to 12 as per their use scenario.
 * When disable CELLULAR_CONFIG_STATIC_ALLOCATION_SOCKET_CONTEXT,
 * the context will be dynamically allocated when a socket is opened.
 */
#define CELLULAR_CONFIG_STATIC_ALLOCATION_SOCKET_CONTEXT    ( 0U )
#if ( CELLULAR_CONFIG_STATIC_ALLOCATION_SOCKET_CONTEXT == 1U )
    #ifdef CELLULAR_NUM_SOCKET_MAX
        #undef CELLULAR_NUM_SOCKET_MAX
        #define CELLULAR_NUM_SOCKET_MAX    ( 2U )
    #endif
#else
    #ifdef CELLULAR_NUM_SOCKET_MAX

/* In order to save the memory in memory constraint devices,
 * the maximum number of Sockets are reduced from 12 to 4.
 * Hence, the number of active sockets are undefined and new
 * maximum number of sockets is redefined. */
/* coverity[misra_c_2012_rule_20_5_violation] */
        #undef CELLULAR_NUM_SOCKET_MAX
        #define CELLULAR_NUM_SOCKET_MAX    ( 4U )
    #endif
#endif /* if ( CELLULAR_CONFIG_STATIC_ALLOCATION_SOCKET_CONTEXT == 1U ) */
#endif /* __CELLULAR_CONFIG_H__ */
