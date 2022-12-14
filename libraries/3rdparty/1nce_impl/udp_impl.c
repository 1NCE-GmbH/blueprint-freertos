

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
bool BIND = false;
struct OSNetwork
{
    Socket_t udpSocket;
};

/*-----------------------------------------------------------*/

int nce_os_udp_connect_impl(OSNetwork_t osnetwork,OSEndPoint_t nce_oboarding) {
		int8_t returnStatus = 0;
		BaseType_t socketStatus = 0;
		SocketsSockaddr_t ServerAddress;
	    /* Connect to Device Authenticator endpoint */
	    IotLogInfo( "connect to onboarding hostname\r\n" );
	    ServerAddress.usPort = SOCKETS_htons( NceOnboard.port );
	    ServerAddress.ulAddress = SOCKETS_GetHostByName( &(NceOnboard.host) );
	    uint32_t timeout = 30000;
		osnetwork->udpSocket = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );
        /* Set a time out so a missing reply does not cause the task to block
         * indefinitely. */
        SOCKETS_SetSockOpt( osnetwork->udpSocket, 0, SOCKETS_SO_RCVTIMEO, &timeout, sizeof( timeout ) );
        SOCKETS_SetSockOpt( osnetwork->udpSocket, 0, SOCKETS_SO_SNDTIMEO, &timeout, sizeof( timeout ) );

        if(osnetwork->udpSocket != SOCKETS_INVALID_SOCKET){
    		/* Establish a UDP connection with the server. */
        	socketStatus= SOCKETS_Connect( osnetwork->udpSocket, &ServerAddress, sizeof( ServerAddress ) );
        	if (socketStatus != 0) {
        				LogError(( "Failed to connect to %s with error %d.\r\n", NceOnboard.host, socketStatus ));
        				returnStatus = -1;
        	}
        }

		if (osnetwork->udpSocket != NULL) {
			LogInfo((  "(Network connection %p) Connection to %s established.\r\n",osnetwork, NceOnboard.host ));
			returnStatus = 0;
		}else{
			LogError(( "Failed Network connection %p.\r\n",osnetwork));
			returnStatus = -1;
		}

		return returnStatus;
	}
/*-----------------------------------------------------------*/

int nce_os_udp_disconnect_impl(OSNetwork_t osnetwork) {
		/* Close connection. */
		return SOCKETS_Close( osnetwork->udpSocket );

}
/*-----------------------------------------------------------*/

int32_t nce_os_udp_recv_impl(OSNetwork_t osnetwork, void *pBuffer,
			size_t bytesToRecv) {
		int32_t tlsStatus = 0;

		tlsStatus = (int32_t) SOCKETS_Recv( osnetwork->udpSocket, ( char * ) pBuffer,
                ( int32_t ) bytesToRecv, NULL );

	return tlsStatus;
}
/*-----------------------------------------------------------*/

int32_t nce_os_udp_send_impl(OSNetwork_t osnetwork,
			const void *pBuffer, size_t bytesToSend) {
	int32_t tlsStatus = 0;

	tlsStatus = (int32_t) SOCKETS_Send( osnetwork->udpSocket, pBuffer, bytesToSend, NULL );

	return tlsStatus;
}


void nce_troubleshooting( char ** smsTroubleshooting )
{
	    CellularServiceStatus_t pServiceStatus;
	    CellularSignalInfo_t pSignalInfo;
	    CellularPlmnInfo_t pNetworkInfo;
	    CellularHandle_t pContext = CellularHandle;

	    Cellular_GetSignalInfo( pContext, &pSignalInfo );
	    Cellular_GetRegisteredNetwork( pContext, &pNetworkInfo );
	    Cellular_CommonGetServiceStatus( pContext, &pServiceStatus );

	    switch( pServiceStatus.rat )
	    {
	        case CELLULAR_RAT_GSM:
	            memcpy( smsTroubleshooting, "GSM", strlen( "GSM" ) );
	            break;

	        case CELLULAR_RAT_LTE:
	            memcpy( smsTroubleshooting, "LTE", strlen( "LTE" ) );
	            break;

	        case CELLULAR_RAT_CATM1:
	            memcpy( smsTroubleshooting, "CATM1", strlen( "CATM1" ) );
	            break;

	        case CELLULAR_RAT_NBIOT:
	            memcpy( smsTroubleshooting, "NBIOT", strlen( "NBIOT" ) );
	            break;

	        default:
	            memcpy( smsTroubleshooting, "NULL", strlen( "NULL" ) );
	            break;
	    }

	    sprintf( smsTroubleshooting, "%s mcc=%s mnc=%s", smsTroubleshooting,
	             pServiceStatus.plmnInfo.mcc, pServiceStatus.plmnInfo.mnc );
	    /* Registered network operator cell Id. */
	    /* Registered network operator Location Area Code. */
	    /* Registered network operator Routing Area Code. */
	    /* Registered network operator Tracking Area Code. */
	    /* RN registred network. *//* id CellId. */

	    sprintf( smsTroubleshooting, "%s RN Id=%d LAC=%d RAC=%d,TAC=%d",
	             smsTroubleshooting, pContext->libAtData.cellId,
	             pContext->libAtData.lac, pContext->libAtData.rac,
	             pContext->libAtData.tac );
	    sprintf( smsTroubleshooting, "%s Reject CS %d %d %d RS %d %d %d",
	             smsTroubleshooting, pContext->libAtData.csRegStatus,
	             pContext->libAtData.csRejectType, pContext->libAtData.csRejCause,
	             pContext->libAtData.psRegStatus, pContext->libAtData.psRejectType,
	             pContext->libAtData.psRejCause );
	    sprintf( smsTroubleshooting,
	             "%s sigInf rssi=%d rsrp=%d rsrq=%d sinr=%d ber=%d bars=%d%c",
	             smsTroubleshooting, pSignalInfo.rssi, pSignalInfo.rsrp,
	             pSignalInfo.rsrq, pSignalInfo.sinr, pSignalInfo.ber,
	             pSignalInfo.bars, 26 );
}
#ifdef TROUBLESHOOTING
	CellularPktStatus_t nce_send_sms( char * smsTroubleshooting )
	{
	    CellularError_t cellularStatus = CELLULAR_SUCCESS;
	    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
	    CellularHandle_t pContext = CellularHandle;
	    CellularAtReq_t atReqGetWithResult =
	    {
	        NULL, CELLULAR_AT_MULTI_WO_PREFIX,
	        NULL,
	        NULL,
	        NULL, 0
	    };

	    atReqGetWithResult.pAtCmd = "AT+CMGF=1";
	    cellularStatus = _Cellular_AtcmdRequestWithCallback( pContext,
	                                                         atReqGetWithResult );
	    sprintf(atReqGetWithResult.pAtCmd,"AT+CMGS=\"%s\"",democonfigCLIENT_ICCID );
	    cellularStatus = _Cellular_PktioSendAtCmd( pContext,
	                                               atReqGetWithResult.pAtCmd, atReqGetWithResult.atCmdType,
	                                               atReqGetWithResult.pAtRspPrefix );
	    IotLogDebug( ">>>>>Start sending Data <<<<<" );

	    int pSentDataLength = _Cellular_PktioSendData( pContext, smsTroubleshooting,
	                                                   strlen( smsTroubleshooting ) );

	    if( pSentDataLength != strlen( smsTroubleshooting ) )
	    {
	        IotLogError(
	            "_Cellular_DataSendWithTimeoutDelayRaw, incomplete data transfer" );
	        pktStatus = CELLULAR_PKT_STATUS_SEND_ERROR;
	    }

	    return( pktStatus );
	}
#endif
	/*-----------------------------------------------------------*/
