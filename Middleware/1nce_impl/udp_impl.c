/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* TLS transport header. */
#include "udp_impl.h"
#include "nce_demo_config.h"
#include "cellular_pkthandler_internal.h"
#include "cellular_common_internal.h"
#include "udp_interface.h"
#include "nce_iot_c_sdk.h"
#include "cellular_bg96.h"
#include "cellular_pkthandler_internal.h"
/* Secure Sockets Include */
#include "iot_secure_sockets.h"
/* Modem Defines */
CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
char IPAdd[ 16 ];
char Port[ 6 ];
DtlsKey_t nceKey = { 0 };
OSNetwork_t xOSNetwork = { 0 };
os_network_ops_t osNetwork =
{
    .os_socket             = &xOSNetwork,
    .nce_os_udp_connect    = nce_os_udp_connect_impl,
    .nce_os_udp_send       = nce_os_udp_send_impl,
    .nce_os_udp_recv       = nce_os_udp_recv_impl,
    .nce_os_udp_disconnect = nce_os_udp_disconnect_impl
};

struct OSNetwork
{
    Socket_t udpSocket;
};

/*-----------------------------------------------------------*/

int nce_os_udp_connect_impl( OSNetwork_t osnetwork,
                             OSEndPoint_t nce_oboarding )
{
    int8_t returnStatus = 0;
    BaseType_t socketStatus = 0;
    SocketsSockaddr_t ServerAddress;

    /* Connect to Device Authenticator endpoint */
    IotLogInfo( "connect to onboarding hostname\r\n" );
    ServerAddress.usPort = SOCKETS_htons( NceOnboard.port );
    ServerAddress.ulAddress = SOCKETS_GetHostByName( &( NceOnboard.host ) );
    uint32_t timeout = 30000;
    osnetwork->udpSocket = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );

    /* Set a time out so a missing reply does not cause the task to block
     * indefinitely. */
    SOCKETS_SetSockOpt( osnetwork->udpSocket, 0, SOCKETS_SO_RCVTIMEO, &timeout, sizeof( timeout ) );
    SOCKETS_SetSockOpt( osnetwork->udpSocket, 0, SOCKETS_SO_SNDTIMEO, &timeout, sizeof( timeout ) );

    if( osnetwork->udpSocket != SOCKETS_INVALID_SOCKET )
    {
        /* Establish a UDP connection with the server. */
        socketStatus = SOCKETS_Connect( osnetwork->udpSocket, &ServerAddress, sizeof( ServerAddress ) );

        if( socketStatus != 0 )
        {
            IotLogError( "Failed to connect to %s with error %d.\r\n", NceOnboard.host, socketStatus );
            returnStatus = -1;
        }
    }

    if( osnetwork->udpSocket != NULL )
    {
        LogInfo( "(Network connection %p) Connection to %s established.\r\n", osnetwork, NceOnboard.host );
        returnStatus = 0;
    }
    else
    {
        IotLogError( "Failed Network connection %p.\r\n", osnetwork );
        returnStatus = -1;
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

int nce_os_udp_disconnect_impl( OSNetwork_t osnetwork )
{
    /* Close connection. */
    return SOCKETS_Close( osnetwork->udpSocket );
}
/*-----------------------------------------------------------*/

int32_t nce_os_udp_recv_impl( OSNetwork_t osnetwork,
                              void * pBuffer,
                              size_t bytesToRecv )
{
    int32_t tlsStatus = 0;

    tlsStatus = ( int32_t ) SOCKETS_Recv( osnetwork->udpSocket, ( char * ) pBuffer,
                                          ( int32_t ) bytesToRecv, NULL );

    return tlsStatus;
}
/*-----------------------------------------------------------*/

int32_t nce_os_udp_send_impl( OSNetwork_t osnetwork,
                              const void * pBuffer,
                              size_t bytesToSend )
{
    int32_t tlsStatus = 0;

    tlsStatus = ( int32_t ) SOCKETS_Send( osnetwork->udpSocket, pBuffer, bytesToSend, NULL );

    return tlsStatus;
}
/*-----------------------------------------------------------*/
