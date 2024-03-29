/*
 * FreeRTOS V1.4.7
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

#ifndef _AWS_DEMO_CONFIG_H_
#define _AWS_DEMO_CONFIG_H_

/* To run a particular demo you need to define one of these.
 * Only one demo can be configured at a time
 *
 *			CONFIG_COAP_DEMO_ENABLED
 *			CONFIG_UDP_DEMO_ENABLED
 *			CONFIG_LwM2M_DEMO_ENABLED
 *
 *  These defines are used in iot_demo_runner.h for demo selection */
#define CONFIG_COAP_DEMO_ENABLED


/* Default configuration for all demos. Individual demos can override these below */
#define democonfigDEMO_STACKSIZE                                     ( configMINIMAL_STACK_SIZE * 8 )
#define democonfigDEMO_PRIORITY                                      ( tskIDLE_PRIORITY + 5 )
#define democonfigNETWORK_TYPES                                      ( AWSIOT_NETWORK_TYPE_CELLULAR )

#define democonfigMEMORY_ANALYSIS

#ifdef democonfigMEMORY_ANALYSIS
    #define democonfigMEMORY_ANALYSIS_STACK_DEPTH_TYPE    UBaseType_t
    #define democonfigMEMORY_ANALYSIS_MIN_EVER_HEAP_SIZE()        xPortGetMinimumEverFreeHeapSize()
    #if ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )
        /* Convert from stack words to bytes */
        #define democonfigMEMORY_ANALYSIS_STACK_WATERMARK( x )    uxTaskGetStackHighWaterMark( x ) * ( uint32_t ) sizeof( StackType_t ); /*lint !e961 Casting is not redundant on smaller architectures. */
    #else
        #define democonfigMEMORY_ANALYSIS_STACK_WATERMARK( x )    NULL
    #endif /* if( INCLUDE_uxTaskGetStackHighWaterMark == 1 ) */
#endif /* democonfigMEMORY_ANALYSIS */

#endif /* _AWS_DEMO_CONFIG_H_ */
