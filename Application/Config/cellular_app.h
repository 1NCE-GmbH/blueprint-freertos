/*
 *  cellular_app.h
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#ifndef CELLULAR_APP_H
#define CELLULAR_APP_H

#ifdef __cplusplus
    extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "iot_demo_logging.h"
#include "iot_secure_sockets.h"
/* Demo includes. */
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "iot_uart.h"
#include "logging_stack.h"
#include "udp_demo.h"
#include "coap_demo.h"
#include "lwm2m_demo.h"

/**
 * @brief Default delay for UDP demo tasks in milliseconds.
 *
 * This constant defines the delay between iterations or task execution
 * in the UDP demo. It can be used in task delays to prevent high CPU usage
 * and allow for periodic execution of tasks.
 */
#define DEFAULT_TASK_DELAY_MS                ( pdMS_TO_TICKS( 5000U ) )

/**
 * @brief Default socket receive operation timeout in ticks.
 *
 * This constant defines the maximum time (30 seconds) the system will wait for data
 * to be received on the socket. After this period, the receive operation will timeout
 * if no data is received, ensuring that the system does not block indefinitely.
 */
#define DEFAULT_SOCKET_TIMEOUT_MS            ( pdMS_TO_TICKS( 30000U ) )

/**
 * @brief Bit mask for the event that indicates data has been received on the socket.
 *
 * This bit is used in the event group to signal that data is ready to be processed.
 * When this bit is set, it triggers the corresponding event (e.g., a data received
 * callback) to be executed. In this example, the bit mask is 0x00000001U, meaning
 * it represents the least significant bit (LSB) in the event group.
 */
#define SOCKET_DATA_RECEIVED_CALLBACK_BIT    ( 0x00000001U )


/**
 * @brief Stack size for a task's thread in bytes.
 *
 * This constant defines the amount of memory allocated for the task's stack.
 * It is set to 1024 bytes, which is sufficient for tasks with moderate memory
 * requirements.
 */
#define THREAD_STACK_SIZE    ( 1024U )

/**
 * @brief Function to log the IP address in readable format
 */

    #define IP_TO_STRING( ulAddress )                                             \
            ( {                                                                      \
        static char ip_str[ 16 ];                                              \
        uint32_t ip_in_host_order = SOCKETS_ntohl( ulAddress );                \
        uint8_t bytes[ 4 ];                                                   \
        bytes[ 0 ] = ( ip_in_host_order >> 24 ) & 0xFF;                         \
        bytes[ 1 ] = ( ip_in_host_order >> 16 ) & 0xFF;                         \
        bytes[ 2 ] = ( ip_in_host_order >> 8 ) & 0xFF;                          \
        bytes[ 3 ] = ip_in_host_order & 0xFF;                                 \
        snprintf( ip_str, sizeof( ip_str ), "%u.%u.%u.%u",                     \
                  bytes[ 0 ], bytes[ 1 ], bytes[ 2 ], bytes[ 3 ] );                   \
        ip_str;                                                             \
    } )

/**
 * @brief  Start all threads needed to activate CellularApp features and call Cellular start
 * @param  -
 * @retval -
 */
void RunDemoTask( void );


#ifdef __cplusplus
    }
#endif
#endif /* ifndef CELLULAR_APP_H */
