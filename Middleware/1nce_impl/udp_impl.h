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
#include "iot_config.h"
#include "logging_levels.h"

#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "SDK_IMPL"
#endif

#ifdef IOT_LOG_LEVEL_NCE_SDK
    #define LIBRARY_LOG_LEVEL        IOT_LOG_LEVEL_NCE_SDK
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_NONE
    #endif
#endif
#include "logging_stack.h"

/************ End of logging configuration ****************/

/* Transport interface include. */
#include "nce_iot_c_sdk.h"
/* Exported variable ------------------------------------------------------- */
/* The application needs to provide the cellular handle for the usage of AT Commands */
/* External variable used to indicate Device Authenticator Status */
extern char IPAdd[ 16 ];
extern char Port[ 6 ];


int nce_os_udp_connect_impl( OSNetwork_t osnetwork,
                             OSEndPoint_t nce_oboarding );


int nce_os_udp_disconnect_impl( OSNetwork_t pNetworkContext );


int32_t nce_os_udp_recv_impl( OSNetwork_t osnetwork,
                              void * pBuffer,
                              size_t bytesToRecv );


int32_t nce_os_udp_send_impl( OSNetwork_t osnetwork,
                              const void * pBuffer,
                              size_t bytesToSend );

extern DtlsKey_t nceKey;
extern OSNetwork_t xOSNetwork;
extern os_network_ops_t osNetwork;
#endif /* ifndef UDP_IMPL */
