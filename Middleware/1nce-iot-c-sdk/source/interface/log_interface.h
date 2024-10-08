#ifndef LOG_INTERFACE_H_
#define LOG_INTERFACE_H_

/**
 *  @brief implementation of logging macros for FreeRTOS.
 */
#define FREERTOS
#ifdef FREERTOS
    #include "FreeRTOSConfig.h"
void vLoggingPrintf( const char * pcFormat,
                     ... );

    #define NceOSLogInfo( format, ... )     vLoggingPrintf( format, ## __VA_ARGS__ )
    #define NceOSLogDebug( format, ... )    vLoggingPrintf( format, ## __VA_ARGS__ )
    #define NceOSLogError( format, ... )    vLoggingPrintf( format, ## __VA_ARGS__ )
    #define NceOSLogWarn( format, ... )     vLoggingPrintf( format, ## __VA_ARGS__ )

#elif defined( __ZEPHYR__ )
    #include <zephyr/logging/log.h>

/**
 *  @brief implementation of logging macros for ZEPHYR OS.
 */
    #define NceOSLogInfo( ... )     LOG_INF( __VA_ARGS__ )
    #define NceOSLogDebug( ... )    LOG_DBG( __VA_ARGS__ )
    #define NceOSLogError( ... )    LOG_ERR( __VA_ARGS__ )
    #define NceOSLogWarn( ... )     LOG_WRN( __VA_ARGS__ )

#else /* ifdef FREERTOS */

/**
 *  @brief Define the abbreviated logging macros.
 */
    #define NceOSLogInfo( format, ... )
    #define NceOSLogDebug( format, ... )
    #define NceOSLogError( format, ... )
    #define NceOSLogWarn( format, ... )

#endif /* ifdef FREERTOS */
#endif /* ifndef LOG_INTERFACE_H_ */
