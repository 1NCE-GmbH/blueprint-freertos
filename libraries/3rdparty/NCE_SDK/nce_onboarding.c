/*
 * nce_onboarding.c
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include <stdbool.h>
#include "nce_onboarding.h"
#include "cellular_bg96.h"
#include "cellular_pkthandler_internal.h"
#include "cellular_common_internal.h"
/*-----------------------------------------------------------*/
static void nce_dtls_psk( uint8_t * complete_response );
static void nce_prepare_and_upload_certificates( uint8_t * complete_response );

#ifdef democonfigRANGE_SIZE

/**
 * @brief The total byte length of the original response.
 *
 * @param[in] PART: A buffer to save received data (response header).
 *
 * @return the total byte of the original response.
 */

    static int response_length( char * );
#endif
/*-----------------------------------------------------------*/

/* External variable used to indicate Onboarding Status */
bool DEVICE_ONBOARDED = false;

/*-----------------------------------------------------------*/

/* Onboarding Defines */
#if  defined( ENABLE_DTLS )
    uint8_t PSK[ 500 ];
    uint8_t psk_identity[ 20 ];
#endif
uint8_t PART[ 1700 ];
#if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED )
    char find[] = "\\n";
    char replace_with[] = "\n";
    int offset = &PART[ 0 ];
    char end_key[] = "-----END RSA PRIVATE KEY-----";
    int end_key_len = sizeof( end_key );
    char end_cert[] = "-----END CERTIFICATE-----";
    int end_cert_len = sizeof( end_cert );
    char end_identifier[] = "\"";
    char * location;
    int client_cert_size = 0;
    int client_cert_cmd_size = 0;
    int client_key_size = 0;
    int client_key_cmd_size = 0;
    uint32_t root_size = 0;
    int root_cmd_size = 0;
    char amazonRootCaUrl[ 80 ];
    char sim_iccid[ 30 ];
    char identifier[ 25 ];
    char iotCoreEndpointUrl[ 50 ];
/*-----------------------------------------------------------*/

/* MQTT Defines */
    char MQTT_TOPIC[ 30 ];
    char IOT_DEMO_MQTT_TOPIC_PREFIX[ 19 ];
    char WILL_TOPIC_NAME[ 35 ];
    char ACKNOWLEDGEMENT_TOPIC_NAME[ 40 ];
    const char * pTopics[ TOPIC_FILTER_COUNT ];
    uint16_t WILL_TOPIC_NAME_LENGTH = 0;
    uint16_t TOPIC_FILTER_LENGTH = 0;
#endif /* if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED ) */
/*-----------------------------------------------------------*/

/* Modem Defines */
CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

/*-----------------------------------------------------------*/

Onboarding_Status_t nce_onboard_device( void )
{
    Onboarding_Status_t onboarding_status = ONBOARDING_ERROR;
    int rangeStart = 0;
    int rangeEnd = democonfigRANGE_SIZE;
    int rangeOriginalSize;

    if( rangeEnd > 1500 )
    {
        IotLogError( "Onboarding Error : Max Range for Modem is 1500 (BG96) " );
        return( onboarding_status );
    }

    pktStatus = nce_reset_modem_fs();

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        IotLogError( "Onboarding Error : Modem Reset " );
        return( onboarding_status );
    }
    else
    {
        IotLogInfo( " ************** Modem Reset Done ************** \n" );
    }

    pktStatus = nce_configure_onboarding_socket();

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        IotLogError( "Onboarding Error : Onboarding Socket Configuration " );
        return( onboarding_status );
    }
    else
    {
        IotLogInfo(
            " ************** Onboarding Socket Configuration Done ************** \n" );
    }

    /* Connect to onboarding endpoint */

    Socket_t cert_socket = SOCKETS_Socket( SOCKETS_AF_INET,
                                           SOCKETS_SOCK_STREAM, SOCKETS_IPPROTO_TCP );

    uint32_t ROOT_IP = SOCKETS_GetHostByName( &ONBOARDING_ENDPOINT );
    SocketsSockaddr_t root_address =
    {
        .ucLength       = sizeof( SocketsSockaddr_t ),
        .ucSocketDomain = SOCKETS_AF_INET,            .usPort= SOCKETS_htons( 443 ),
        .ulAddress      = ROOT_IP
    };
    uint32_t timeout = 30000;

    int32_t set_timeout = SOCKETS_SetSockOpt( cert_socket,
                                              CELLULAR_SOCKET_OPTION_LEVEL_TRANSPORT, SOCKETS_SO_RCVTIMEO,
                                              &timeout, ( int32_t ) sizeof( timeout ) );
    int32_t lRetVal = SOCKETS_Connect( cert_socket, &root_address,
                                       sizeof( SocketsSockaddr_t ) );
    char send_packet[ 124 ];
    int32_t rec;
    int32_t SendVal;

    #ifdef ENABLE_DTLS
        uint8_t * complete_response = ( uint8_t * ) pvPortMalloc(
            1000 * sizeof( uint8_t ) );
        ( void ) memset( complete_response, ( int8_t ) '\0', 1000 );
        sprintf( send_packet, "GET /device-api/onboarding/coap HTTP/1.1\r\n"
                              "Host: %s\r\n"
                              "Accept: text/csv\r\n\r\n", ONBOARDING_ENDPOINT );
        SendVal = SOCKETS_Send( cert_socket, &send_packet, strlen( send_packet ),
                                NULL );
        rec = SOCKETS_Recv( cert_socket, ( char * ) &PART[ 0 ],
                            ( int32_t ) sizeof( PART ), NULL );
        strcat( complete_response,
                strstr( PART, "Express\r\n\r\n" ) + strlen( "Express\r\n\r\n" ) );
        nce_dtls_psk( complete_response );
    #else  /* ifdef ENABLE_DTLS */
        uint8_t * complete_response = ( uint8_t * ) pvPortMalloc(
            5000 * sizeof( uint8_t ) );
        memset( complete_response, '\0', 5000 );

        do
        {
            ( void ) memset( &send_packet, ( int8_t ) '\0', sizeof( send_packet ) );
            sprintf( send_packet, "GET /device-api/onboarding HTTP/1.1\r\n"
                                  "Host: %s\r\n"
                                  "Range: bytes=%d-%d\r\n"
                                  "Accept: text/csv\r\n\r\n", ONBOARDING_ENDPOINT, rangeStart,
                     rangeEnd );
            SendVal = SOCKETS_Send( cert_socket, &send_packet, strlen( send_packet ),
                                    NULL );
            rec = SOCKETS_Recv( cert_socket, ( char * ) &PART[ 0 ],
                                ( int32_t ) sizeof( PART ), NULL );

            if( rangeEnd == democonfigRANGE_SIZE )
            {
                rangeOriginalSize = response_length( PART );
            }

            IotLogInfo(
                " ************** Raw Response ***min= %d*****RANGE=%d******   %d bytes \n",
                rangeStart, rangeEnd, strlen( PART ) );
            strcat( complete_response,
                    strstr( PART, "Express\r\n\r\n" ) + strlen( "Express\r\n\r\n" ) );

            ( void ) memset( &PART, ( int8_t ) '\0', sizeof( PART ) );
            rangeStart = rangeEnd + 1;
            rangeEnd += democonfigRANGE_SIZE;
        } while ( rangeStart < rangeOriginalSize );
        nce_prepare_and_upload_certificates( complete_response );
    #endif /* ifdef ENABLE_DTLS */

    IotLogInfo( " ************** Response Received ************** \n" );
    nce_configure_ssl_socket();

    onboarding_status = ONBOARDING_OK;
    DEVICE_ONBOARDED = true;
    return( onboarding_status );
}

/*-----------------------------------------------------------*/

static char * str_replace( char * orig,
                           char * rep,
                           char * with )
{
    char * result;
    char * ins;
    char * tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if( !orig || !rep )
    {
        return NULL;
    }

    len_rep = strlen( rep );

    if( len_rep == 0 )
    {
        return NULL;
    }

    if( !with )
    {
        with = "";
    }

    len_with = strlen( with );
    ins = orig;

    for( count = 0; tmp = strstr( ins, rep ); ++count )
    {
        ins = tmp + len_rep;
    }

    tmp = result = pvPortMalloc(
        strlen( orig ) + ( len_with - len_rep ) * count + 1 );

    if( !result )
    {
        return NULL;
    }

    while( count-- )
    {
        ins = strstr( orig, rep );
        len_front = ins - orig;
        tmp = strncpy( tmp, orig, len_front ) + len_front;
        tmp = strcpy( tmp, with ) + len_with;
        orig += len_front + len_rep;
    }

    strcpy( tmp, orig );
    return result;
}

/*-----------------------------------------------------------*/

#if defined( ENABLE_DTLS )
    static void nce_dtls_psk( uint8_t * complete_response )
    {
        /* get the first token */
        char * token = strtok( complete_response, "," );

        memcpy( PSK, token, strlen( token ) );
        IotLogInfo( " *****PSK********* %s ************** \n", PSK );
        token = strtok( NULL, "," );
        memcpy( psk_identity, token, strlen( token ) );
        IotLogInfo( " ******ID******** %s ************** \n", psk_identity );
    }
#endif /* if defined( ENABLE_DTLS ) */
/*-----------------------------------------------------------*/
#if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED )
    static void nce_prepare_and_upload_certificates( uint8_t * complete_response )
    {
        /* TODO */
        /* get the first token */
        CellularContext_t * pContext = ( CellularContext_t * ) CellularHandle;

        char * token = strtok( complete_response, "," );

        memcpy( sim_iccid, token, strlen( token ) );
        int i = 0;

        offset = &sim_iccid[ 0 ];
        location = strstr( sim_iccid, end_identifier );
        int identifier_size = location - offset;

        memcpy( identifier, sim_iccid + 1, strlen( sim_iccid ) - 2 );
        memcpy( IOT_DEMO_MQTT_TOPIC_PREFIX, identifier, strlen( identifier ) );

        /* walk through other tokens */
        while( token != NULL )
        {
            token = strtok( NULL, "," );

            if( i == 0 )
            {
                ( void ) memset( &iotCoreEndpointUrl, ( int8_t ) '\0',
                                 sizeof( iotCoreEndpointUrl ) );
                memcpy( iotCoreEndpointUrl, token + 1, strlen( token ) - 2 );
            }

            if( i == 1 )
            {
                memcpy( amazonRootCaUrl, token, strlen( token ) );
            }

            if( i == 2 )
            {
                /*Process root.pem*/
                memcpy( PART, token + 1, strlen( token ) - 1 );
                memcpy( PART, str_replace( PART, find, replace_with ), strlen( PART ) );
                offset = &PART[ 0 ];
                location = strstr( PART, end_cert );
                root_size = location + end_cert_len - offset;
                PART[ root_size ] = '\n';

                char root_cmd[ 30 ] = "AT+QFUPL=\"root.pem\",";
                sprintf( root_cmd, "%s%d,60", root_cmd, root_size );
                root_cmd_size = strlen( root_cmd );

                char cmdBuf[ CELLULAR_AT_CMD_MAX_SIZE ] = { '\0' };
                CellularAtReq_t atReq =
                {
                    cmdBuf, CELLULAR_AT_NO_RESULT,
                    NULL,
                    NULL,
                    NULL,   0,
                };
                ( void ) snprintf( cmdBuf, CELLULAR_AT_CMD_MAX_SIZE, root_cmd );
                CellularAtDataReq_t dataReq =
                {
                    &PART[ 0 ], root_size, &root_size,
                    NULL,       0
                };

                /*Upload root.pem*/
                _Cellular_TimeoutAtcmdDataSendRequestWithCallback( pContext, atReq,
                                                                   dataReq, PACKET_REQ_TIMEOUT_MS, DATA_SEND_TIMEOUT_MS );
                ( void ) memset( &PART, ( int8_t ) '\0', sizeof( PART ) );
            }

            if( i == 3 )
            {
                /* Process client_cert.pem */
                memcpy( PART, token + 1, strlen( token ) - 1 );
                memcpy( PART, str_replace( PART, find, replace_with ), strlen( PART ) );
                offset = &PART[ 0 ];
                location = strstr( PART, end_cert );
                client_cert_size = location + end_cert_len - offset;
                PART[ client_cert_size ] = '\n';
                char client_cert_cmd[ 40 ] = "AT+QFUPL=\"clientcert.pem\",";
                sprintf( client_cert_cmd, "%s%d,60", client_cert_cmd,
                         client_cert_size );
                client_cert_cmd_size = strlen( client_cert_cmd );
                /* Upload client_cert.pem */
                char cmdBuf[ CELLULAR_AT_CMD_MAX_SIZE ] = { '\0' };
                CellularAtReq_t atReq =
                {
                    cmdBuf, CELLULAR_AT_NO_RESULT,
                    NULL,
                    NULL,
                    NULL,   0,
                };
                ( void ) snprintf( cmdBuf, CELLULAR_AT_CMD_MAX_SIZE, client_cert_cmd );
                CellularAtDataReq_t dataReq =
                {
                    &PART[ 0 ],        client_cert_size,
                    &client_cert_size,
                    NULL,              0
                };

                /*Upload root.pem*/
                _Cellular_TimeoutAtcmdDataSendRequestWithCallback( pContext, atReq,
                                                                   dataReq, PACKET_REQ_TIMEOUT_MS, DATA_SEND_TIMEOUT_MS );
                ( void ) memset( &PART, ( int8_t ) '\0', sizeof( PART ) );
            }

            if( i == 4 )
            {
                /* Process client_key.pem */
                memcpy( PART, token + 1, strlen( token ) - 1 );
                memcpy( PART, str_replace( PART, find, replace_with ), strlen( PART ) );
                offset = &PART[ 0 ];
                location = strstr( PART, end_key );
                client_key_size = location + end_key_len - offset;
                PART[ client_key_size ] = '\n';
                char client_key_cmd[ 40 ] = "AT+QFUPL=\"clientkey.pem\",";
                sprintf( client_key_cmd, "%s%d,60", client_key_cmd, client_key_size );
                client_key_cmd_size = strlen( client_key_cmd );
                /* Upload client_key.pem */
                char cmdBuf[ CELLULAR_AT_CMD_MAX_SIZE ] = { '\0' };
                CellularAtReq_t atReq =
                {
                    cmdBuf, CELLULAR_AT_NO_RESULT,
                    NULL,
                    NULL,
                    NULL,   0,
                };
                ( void ) snprintf( cmdBuf, CELLULAR_AT_CMD_MAX_SIZE, client_key_cmd );
                CellularAtDataReq_t dataReq =
                {
                    &PART[ 0 ],       client_key_size,
                    &client_key_size,
                    NULL,             0
                };

                /*Upload root.pem*/
                _Cellular_TimeoutAtcmdDataSendRequestWithCallback( pContext, atReq,
                                                                   dataReq, PACKET_REQ_TIMEOUT_MS, DATA_SEND_TIMEOUT_MS );
                ( void ) memset( &PART, ( int8_t ) '\0', sizeof( PART ) );
                vPortFree( complete_response );
            }

            i++;
        }

        memcpy( MQTT_TOPIC, IOT_DEMO_MQTT_TOPIC_PREFIX,
                strlen( IOT_DEMO_MQTT_TOPIC_PREFIX ) );
        memcpy( MQTT_TOPIC + strlen( MQTT_TOPIC ), SUB_TOPIC, strlen( SUB_TOPIC ) + 1 );
        IotLogInfo(
            " ************** Certificates Uploaded Successfully  ************** \n" );
    }
    static int response_length( char * PART )
    {
        int pch = strstr( PART, "bytes" );

        int rangeStart, rangeEnd, rangeOriginalSize;

        if( 3
            == sscanf( pch,
                       "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                       &rangeStart, &rangeEnd, &rangeOriginalSize ) )
        {
        }

        return rangeOriginalSize;
    }
#endif /* if  defined( CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED ) */
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
    atReqGetWithResult.pAtCmd = "AT+CMGS=\"882285101073139\"";
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
