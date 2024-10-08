/*
 * FreeRTOS-Cellular-Interface v1.3.0
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */

#include "cellular_config.h"
#include "cellular_config_defaults.h"

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cellular_platform.h"
#include "cellular_types.h"
#include "cellular_common.h"
#include "cellular_common_api.h"
#include "cellular_common_portable.h"
#include "cellular_bg96.h"

/*-----------------------------------------------------------*/

#define CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX            "+QIURC: \"recv\","
#define CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX_LEN        15

/* The length for the string "+QIURC: \"recv\",<socket_index:1>,<socket_size:1~4>\r\n". */
#define CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX_MAX_LEN    24

/*-----------------------------------------------------------*/

static void _Cellular_ProcessCereg( CellularContext_t * pContext,
                                    char * pInputLine );
static void _Cellular_ProcessCgreg( CellularContext_t * pContext,
                                    char * pInputLine );
static void _Cellular_ProcessCreg( CellularContext_t * pContext,
                                   char * pInputLine );
static void _Cellular_ProcessPowerDown( CellularContext_t * pContext,
                                        char * pInputLine );
static void _Cellular_ProcessPsmPowerDown( CellularContext_t * pContext,
                                           char * pInputLine );
static void _Cellular_ProcessModemRdy( CellularContext_t * pContext,
                                       char * pInputLine );
static void _Cellular_ProcessSocketOpen( CellularContext_t * pContext,
                                         char * pInputLine );
static void _Cellular_ProcessSocketurc( CellularContext_t * pContext,
                                        char * pInputLine );
static void _Cellular_ProcessSimstat( CellularContext_t * pContext,
                                      char * pInputLine );
static void _Cellular_ProcessIndication( CellularContext_t * pContext,
                                         char * pInputLine );
static CellularPktStatus_t prvParseDirectPushURCPrefix( char * pBuffer,
                                                        uint32_t bufferLength,
                                                        uint32_t * pPrefixLength,
                                                        uint32_t * pSocketIndex,
                                                        uint32_t * pDataLength );
static CellularPktStatus_t prvStoreDirectPushSocketData( CellularContext_t * pContext,
                                                         char * pBuffer,
                                                         uint32_t prefixLength,
                                                         uint32_t socketIndex,
                                                         uint32_t dataLength );

/*-----------------------------------------------------------*/

/* Try to Keep this map in Alphabetical order. */
CellularAtParseTokenMap_t CellularUrcHandlerTable[] =
{
    { "CEREG",          _Cellular_ProcessCereg        },
    { "CGREG",          _Cellular_ProcessCgreg        },
    { "CREG",           _Cellular_ProcessCreg         },
    { "POWERED DOWN",   _Cellular_ProcessPowerDown    },
    { "PSM POWER DOWN", _Cellular_ProcessPsmPowerDown },
    { "QIND",           _Cellular_ProcessIndication   },
    { "QIOPEN",         _Cellular_ProcessSocketOpen   },
    { "QIURC",          _Cellular_ProcessSocketurc    },
    { "QSIMSTAT",       _Cellular_ProcessSimstat      },
    { "RDY",            _Cellular_ProcessModemRdy     }
};

uint32_t CellularUrcHandlerTableSize = sizeof( CellularUrcHandlerTable ) / sizeof( CellularAtParseTokenMap_t );

/*-----------------------------------------------------------*/

static void _Cellular_ProcessCereg( CellularContext_t * pContext,
                                    char * pInputLine )
{
    CellularPktStatus_t pktStatus;

    pktStatus = Cellular_CommonUrcProcessCereg( pContext, pInputLine );

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogError( "_Cellular_ProcessCereg: process CEREG failed %d", pktStatus );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessCgreg( CellularContext_t * pContext,
                                    char * pInputLine )
{
    CellularPktStatus_t pktStatus;

    pktStatus = Cellular_CommonUrcProcessCgreg( pContext, pInputLine );

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogError( "_Cellular_ProcessCgreg: process CGREG failed %d", pktStatus );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessCreg( CellularContext_t * pContext,
                                   char * pInputLine )
{
    CellularPktStatus_t pktStatus;

    pktStatus = Cellular_CommonUrcProcessCreg( pContext, pInputLine );

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogError( "_Cellular_ProcessCreg: process CREG failed %d", pktStatus );
    }
}

/*-----------------------------------------------------------*/

/* internal function of _parseSocketOpen to reduce complexity. */
static CellularPktStatus_t _parseSocketOpenNextTok( const char * pToken,
                                                    uint32_t sockIndex,
                                                    CellularSocketContext_t * pSocketData )
{
    int32_t sockStatus = 0;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    atCoreStatus = Cellular_ATStrtoi( pToken, 10, &sockStatus );

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        if( sockStatus != 0 )
        {
            pSocketData->socketState = SOCKETSTATE_DISCONNECTED;
            LogError( "_parseSocketOpen: Socket open failed, conn %d, status %d", sockIndex, sockStatus );
        }
        else
        {
            pSocketData->socketState = SOCKETSTATE_CONNECTED;
            LogDebug( "_parseSocketOpen: Socket open success, conn %d", sockIndex );
        }

        /* Indicate the upper layer about the socket open status. */
        if( pSocketData->openCallback != NULL )
        {
            if( sockStatus != 0 )
            {
                pSocketData->openCallback( CELLULAR_URC_SOCKET_OPEN_FAILED,
                                           pSocketData, pSocketData->pOpenCallbackContext );
            }
            else
            {
                pSocketData->openCallback( CELLULAR_URC_SOCKET_OPENED,
                                           pSocketData, pSocketData->pOpenCallbackContext );
            }
        }
        else
        {
            LogError( "_parseSocketOpen: Socket open callback for conn %d is not set!!", sockIndex );
        }
    }

    pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
    return pktStatus;
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessSocketOpen( CellularContext_t * pContext,
                                         char * pInputLine )
{
    char * pUrcStr = NULL, * pToken = NULL;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    uint32_t sockIndex = 0;
    int32_t tempValue = 0;
    CellularSocketContext_t * pSocketData = NULL;

    if( pContext == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_FAILURE;
    }
    else if( pInputLine == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
    }
    else
    {
        pUrcStr = pInputLine;
        atCoreStatus = Cellular_ATRemoveAllWhiteSpaces( pUrcStr );

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATGetNextTok( &pUrcStr, &pToken );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            if( ( tempValue >= 0 ) &&
                ( tempValue < ( int32_t ) CELLULAR_NUM_SOCKET_MAX ) )
            {
                sockIndex = ( uint32_t ) tempValue;
            }
            else
            {
                LogError( "Error processing in Socket index. token %s", pToken );
                atCoreStatus = CELLULAR_AT_ERROR;
            }
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            pSocketData = _Cellular_GetSocketData( pContext, sockIndex );

            if( pSocketData != NULL )
            {
                atCoreStatus = Cellular_ATGetNextTok( &pUrcStr, &pToken );

                if( atCoreStatus == CELLULAR_AT_SUCCESS )
                {
                    pktStatus = _parseSocketOpenNextTok( pToken, sockIndex, pSocketData );
                }
            }
            else
            {
                pktStatus = CELLULAR_PKT_STATUS_FAILURE;
            }
        }

        if( atCoreStatus != CELLULAR_AT_SUCCESS )
        {
            pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
        }
    }

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogDebug( "Socket Open URC Parse failure" );
    }
}

/*-----------------------------------------------------------*/

static CellularPktStatus_t _parseUrcIndicationCsq( const CellularContext_t * pContext,
                                                   char * pUrcStr )
{
    char * pToken = NULL;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularError_t cellularStatus = CELLULAR_SUCCESS;
    int32_t retStrtoi = 0;
    int16_t csqRssi = CELLULAR_INVALID_SIGNAL_VALUE, csqBer = CELLULAR_INVALID_SIGNAL_VALUE;
    CellularSignalInfo_t signalInfo = { 0 };
    char * pLocalUrcStr = pUrcStr;

    if( ( pContext == NULL ) || ( pUrcStr == NULL ) )
    {
        atCoreStatus = CELLULAR_AT_BAD_PARAMETER;
    }
    else
    {
        /* Parse the RSSI index from string and convert it. */
        atCoreStatus = Cellular_ATGetNextTok( &pLocalUrcStr, &pToken );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATStrtoi( pToken, 10, &retStrtoi );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        if( ( retStrtoi >= INT16_MIN ) && ( retStrtoi <= ( int32_t ) INT16_MAX ) )
        {
            cellularStatus = _Cellular_ConvertCsqSignalRssi( ( int16_t ) retStrtoi, &csqRssi );

            if( cellularStatus != CELLULAR_SUCCESS )
            {
                atCoreStatus = CELLULAR_AT_BAD_PARAMETER;
            }
        }
        else
        {
            atCoreStatus = CELLULAR_AT_ERROR;
        }
    }

    /* Parse the BER index from string and convert it. */
    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATGetNextTok( &pLocalUrcStr, &pToken );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATStrtoi( pToken, 10, &retStrtoi );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        if( ( retStrtoi >= INT16_MIN ) &&
            ( retStrtoi <= ( int32_t ) INT16_MAX ) )
        {
            cellularStatus = _Cellular_ConvertCsqSignalBer( ( int16_t ) retStrtoi, &csqBer );

            if( cellularStatus != CELLULAR_SUCCESS )
            {
                atCoreStatus = CELLULAR_AT_BAD_PARAMETER;
            }
        }
        else
        {
            atCoreStatus = CELLULAR_AT_ERROR;
        }
    }

    /* Handle the callback function. */
    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        signalInfo.rssi = csqRssi;
        signalInfo.rsrp = CELLULAR_INVALID_SIGNAL_VALUE;
        signalInfo.rsrq = CELLULAR_INVALID_SIGNAL_VALUE;
        signalInfo.ber = csqBer;
        signalInfo.bars = CELLULAR_INVALID_SIGNAL_BAR_VALUE;
        _Cellular_SignalStrengthChangedCallback( pContext, CELLULAR_URC_EVENT_SIGNAL_CHANGED, &signalInfo );
    }

    if( atCoreStatus != CELLULAR_AT_SUCCESS )
    {
        pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
    }

    return pktStatus;
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessIndication( CellularContext_t * pContext,
                                         char * pInputLine )
{
    char * pUrcStr = NULL, * pToken = NULL;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;

    /* Check context status. */
    if( pContext == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_FAILURE;
    }
    else if( pInputLine == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
    }
    else
    {
        pUrcStr = pInputLine;
        atCoreStatus = Cellular_ATRemoveAllDoubleQuote( pUrcStr );

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATRemoveLeadingWhiteSpaces( &pUrcStr );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATGetNextTok( &pUrcStr, &pToken );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            if( strstr( pToken, "csq" ) != NULL )
            {
                pktStatus = _parseUrcIndicationCsq( ( const CellularContext_t * ) pContext, pUrcStr );
            }
        }

        if( atCoreStatus != CELLULAR_AT_SUCCESS )
        {
            pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
        }
    }

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogDebug( "UrcIndication Parse failure" );
    }
}

/*-----------------------------------------------------------*/

static void _informDataReadyToUpperLayer( CellularSocketContext_t * pSocketData )
{
    /* Indicate the upper layer about the data reception. */
    if( ( pSocketData != NULL ) && ( pSocketData->dataReadyCallback != NULL ) )
    {
        pSocketData->dataReadyCallback( pSocketData, pSocketData->pDataReadyCallbackContext );
    }
    else
    {
        LogError( "_parseSocketUrc: Data ready callback not set!!" );
    }
}

/*-----------------------------------------------------------*/

static CellularPktStatus_t _parseSocketUrcRecv( const CellularContext_t * pContext,
                                                char * pUrcStr )
{
    char * pToken = NULL;
    char * pLocalUrcStr = pUrcStr;
    int32_t tempValue = 0;
    uint32_t sockIndex = 0;
    CellularSocketContext_t * pSocketData = NULL;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    atCoreStatus = Cellular_ATGetNextTok( &pLocalUrcStr, &pToken );

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            if( ( tempValue >= 0 ) && ( tempValue < ( int32_t ) CELLULAR_NUM_SOCKET_MAX ) )
            {
                sockIndex = ( uint32_t ) tempValue;
            }
            else
            {
                LogError( "Error in processing SockIndex. Token %s", pToken );
                atCoreStatus = CELLULAR_AT_ERROR;
            }
        }
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        pSocketData = _Cellular_GetSocketData( pContext, sockIndex );

        if( pSocketData != NULL )
        {
            if( pSocketData->dataMode == CELLULAR_ACCESSMODE_BUFFER )
            {
                /* Data received indication in buffer mode, need to fetch the data. */
                LogDebug( "Data Received on socket Conn Id %d", sockIndex );
                _informDataReadyToUpperLayer( pSocketData );
            }
        }
        else
        {
            atCoreStatus = CELLULAR_AT_ERROR;
        }
    }

    if( atCoreStatus != CELLULAR_AT_SUCCESS )
    {
        pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
    }

    return pktStatus;
}

/*-----------------------------------------------------------*/

static CellularPktStatus_t _parseSocketUrcClosed( const CellularContext_t * pContext,
                                                  char * pUrcStr )
{
    char * pToken = NULL;
    char * pLocalUrcStr = pUrcStr;
    int32_t tempValue = 0;
    uint32_t sockIndex = 0;
    CellularSocketContext_t * pSocketData = NULL;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    atCoreStatus = Cellular_ATGetNextTok( &pLocalUrcStr, &pToken );

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        if( tempValue < ( int32_t ) CELLULAR_NUM_SOCKET_MAX )
        {
            sockIndex = ( uint32_t ) tempValue;
        }
        else
        {
            LogError( "Error in processing Socket Index. Token %s", pToken );
            atCoreStatus = CELLULAR_AT_ERROR;
        }
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        pSocketData = _Cellular_GetSocketData( pContext, sockIndex );

        if( pSocketData != NULL )
        {
            pSocketData->socketState = SOCKETSTATE_DISCONNECTED;
            LogDebug( "Socket closed. Conn Id %d", sockIndex );

            /* Indicate the upper layer about the socket close. */
            if( pSocketData->closedCallback != NULL )
            {
                pSocketData->closedCallback( pSocketData, pSocketData->pClosedCallbackContext );
            }
            else
            {
                LogInfo( "_parseSocketUrc: Socket close callback not set!!" );
            }
        }
        else
        {
            atCoreStatus = CELLULAR_AT_ERROR;
        }
    }

    if( atCoreStatus != CELLULAR_AT_SUCCESS )
    {
        pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
    }

    return pktStatus;
}

/*-----------------------------------------------------------*/

static CellularPktStatus_t _parseSocketUrcAct( const CellularContext_t * pContext,
                                               char * pUrcStr )
{
    int32_t tempValue = 0;
    char * pToken = NULL;
    char * pLocalUrcStr = pUrcStr;
    uint8_t contextId = 0;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    atCoreStatus = Cellular_ATGetNextTok( &pLocalUrcStr, &pToken );

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
    }

    if( atCoreStatus == CELLULAR_AT_SUCCESS )
    {
        if( ( ( tempValue >= ( int32_t ) CELLULAR_PDN_CONTEXT_ID_MIN ) &&
              ( tempValue <= ( int32_t ) CELLULAR_PDN_CONTEXT_ID_MAX ) ) )
        {
            contextId = ( uint8_t ) tempValue;

            if( _Cellular_IsValidPdn( contextId ) == CELLULAR_SUCCESS )
            {
                LogDebug( "PDN deactivated. Context Id %d", contextId );
                /* Indicate the upper layer about the PDN deactivate. */
                _Cellular_PdnEventCallback( pContext, CELLULAR_URC_EVENT_PDN_DEACTIVATED, contextId );
            }
            else
            {
                atCoreStatus = CELLULAR_AT_ERROR;
            }
        }
        else
        {
            atCoreStatus = CELLULAR_AT_ERROR;
            LogError( "Error in processing Context Id. Token %s", pToken );
        }
    }

    pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );

    return pktStatus;
}

/*-----------------------------------------------------------*/

static CellularPktStatus_t _parseSocketUrcDns( const CellularContext_t * pContext,
                                               char * pUrcStr )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    cellularModuleContext_t * pModuleContext = NULL;
    CellularError_t cellularStatus = CELLULAR_SUCCESS;

    if( pContext == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_INVALID_HANDLE;
    }
    else if( pUrcStr == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
    }
    else
    {
        cellularStatus = _Cellular_GetModuleContext( pContext, ( void ** ) &pModuleContext );

        if( cellularStatus != CELLULAR_SUCCESS )
        {
            pktStatus = CELLULAR_PKT_STATUS_INVALID_HANDLE;
        }
    }

    if( pktStatus == CELLULAR_PKT_STATUS_OK )
    {
        if( pModuleContext->dnsEventCallback != NULL )
        {
            pModuleContext->dnsEventCallback( pModuleContext, pUrcStr, pModuleContext->pDnsUsrData );
        }
        else
        {
            LogDebug( "_parseSocketUrcDns: spurious DNS response!!" );
            pktStatus = CELLULAR_PKT_STATUS_INVALID_DATA;
        }
    }

    return pktStatus;
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessSocketurc( CellularContext_t * pContext,
                                        char * pInputLine )
{
    char * pUrcStr = NULL, * pToken = NULL;
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;

    if( pContext == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_INVALID_HANDLE;
    }
    else if( pInputLine == NULL )
    {
        pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
    }
    else
    {
        pUrcStr = pInputLine;
        atCoreStatus = Cellular_ATRemoveAllDoubleQuote( pUrcStr );

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATRemoveLeadingWhiteSpaces( &pUrcStr );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            atCoreStatus = Cellular_ATGetNextTok( &pUrcStr, &pToken );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            /* Check if this is a data receive indication. */

            /* this whole if as a function and return pktstatus
             * take iotat_getnexttok inside
             * convert atcore status to pktstatus. */
            if( strstr( pToken, "recv" ) != NULL )
            {
                pktStatus = _parseSocketUrcRecv( pContext, pUrcStr );
            }
            else if( strcmp( pToken, "closed" ) == 0 )
            {
                pktStatus = _parseSocketUrcClosed( pContext, pUrcStr );
            }
            else if( strcmp( pToken, "pdpdeact" ) == 0 )
            {
                pktStatus = _parseSocketUrcAct( pContext, pUrcStr );
            }
            else if( strcmp( pToken, "dnsgip" ) == 0 )
            {
                pktStatus = _parseSocketUrcDns( pContext, pUrcStr );
            }
            else
            {
                /* Empty else MISRA 15.7 */
            }
        }

        if( atCoreStatus != CELLULAR_AT_SUCCESS )
        {
            pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
        }
    }

    if( pktStatus != CELLULAR_PKT_STATUS_OK )
    {
        LogDebug( "Socketurc Parse failure" );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessSimstat( CellularContext_t * pContext,
                                      char * pInputLine )
{
    CellularSimCardState_t simCardState = CELLULAR_SIM_CARD_UNKNOWN;

    if( pContext != NULL )
    {
        ( void ) _Cellular_ParseSimstat( pInputLine, &simCardState );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessPowerDown( CellularContext_t * pContext,
                                        char * pInputLine )
{
    /* The token is the pInputLine. No need to process the pInputLine. */
    ( void ) pInputLine;

    if( pContext == NULL )
    {
        LogError( "_Cellular_ProcessPowerDown: Context not set" );
    }
    else
    {
        LogDebug( "_Cellular_ProcessPowerDown: Modem Power down event received" );
        _Cellular_ModemEventCallback( pContext, CELLULAR_MODEM_EVENT_POWERED_DOWN );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessPsmPowerDown( CellularContext_t * pContext,
                                           char * pInputLine )
{
    /* The token is the pInputLine. No need to process the pInputLine. */
    ( void ) pInputLine;

    if( pContext == NULL )
    {
        LogError( "_Cellular_ProcessPowerDown: Context not set" );
    }
    else
    {
        LogDebug( "_Cellular_ProcessPsmPowerDown: Modem PSM power down event received" );
        _Cellular_ModemEventCallback( pContext, CELLULAR_MODEM_EVENT_PSM_ENTER );
    }
}

/*-----------------------------------------------------------*/

static void _Cellular_ProcessModemRdy( CellularContext_t * pContext,
                                       char * pInputLine )
{
    /* The token is the pInputLine. No need to process the pInputLine. */
    ( void ) pInputLine;

    if( pContext == NULL )
    {
        LogWarn( "_Cellular_ProcessModemRdy: Context not set" );
    }
    else
    {
        LogDebug( "_Cellular_ProcessModemRdy: Modem Ready event received" );
        _Cellular_ModemEventCallback( pContext, CELLULAR_MODEM_EVENT_BOOTUP_OR_REBOOT );
    }
}

/*-----------------------------------------------------------*/

#if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 )

/**
 * @brief Extract information from direct push socket URC.
 * In the following example, prefix length is 20, socket index is 0 and data length is 4.
 * +QIURC: "recv",0,4\r\n
 * test\r\n
 */
    static CellularPktStatus_t prvParseDirectPushURCPrefix( char * pBuffer,
                                                            uint32_t bufferLength,
                                                            uint32_t * pPrefixLength,
                                                            uint32_t * pSocketIndex,
                                                            uint32_t * pDataLength )
    {
        char pLocaLine[ CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX_MAX_LEN ];
        char * pLocalLinePtr = pLocaLine;
        char * pToken;
        CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
        CellularATError_t atCoreStatus;
        int32_t tempValue;
        uint32_t i;

        /* Find the first complete line in the buffer. */
        for( i = 0; i < bufferLength; i++ )
        {
            if( ( pBuffer[ i ] == '\r' ) || ( pBuffer[ i ] == '\n' ) )
            {
                break;
            }
        }

        /* A complete line is received in the buffer. */
        if( i > CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX_MAX_LEN )
        {
            /* The line length is longer than expected. */
            pktStatus = CELLULAR_PKT_STATUS_INVALID_DATA;
        }
        else if( i >= bufferLength )
        {
            /* New line is not found. */
            pktStatus = CELLULAR_PKT_STATUS_SIZE_MISMATCH;
        }
        else
        {
            strncpy( pLocalLinePtr, pBuffer, i );
            pLocalLinePtr[ i ] = '\0'; /* Replace the change line '\r' with '\0'. */
            *pPrefixLength = i + 2;    /* Add 2 to the length to include "\r\n". */

            /* Get the socket index. Socket index is the second token. */
            atCoreStatus = Cellular_ATGetNextTok( &pLocalLinePtr, &pToken );

            if( atCoreStatus == CELLULAR_AT_SUCCESS )
            {
                atCoreStatus = Cellular_ATGetNextTok( &pLocalLinePtr, &pToken );
            }

            if( atCoreStatus == CELLULAR_AT_SUCCESS )
            {
                atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
            }

            if( atCoreStatus == CELLULAR_AT_SUCCESS )
            {
                if( ( tempValue >= 0 ) &&
                    ( tempValue < ( int32_t ) CELLULAR_NUM_SOCKET_MAX ) )
                {
                    *pSocketIndex = ( uint32_t ) tempValue;
                }
                else
                {
                    LogError( "Cellular_BG96InputBufferCallback : Error processing in Socket index. token %s.", pToken );
                    atCoreStatus = CELLULAR_AT_ERROR;
                }
            }

            if( pLocalLinePtr[ 0 ] == '\0' )
            {
                /* The third token is empty. This is a buffer access mode URC. */
                pktStatus = CELLULAR_PKT_STATUS_PREFIX_MISMATCH;
            }
            else
            {
                /* Get the data length. Data length is the third token. */
                if( atCoreStatus == CELLULAR_AT_SUCCESS )
                {
                    atCoreStatus = Cellular_ATGetNextTok( &pLocalLinePtr, &pToken );
                }

                if( atCoreStatus == CELLULAR_AT_SUCCESS )
                {
                    atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
                }

                if( atCoreStatus == CELLULAR_AT_SUCCESS )
                {
                    if( tempValue >= 0 )
                    {
                        *pDataLength = ( uint32_t ) tempValue;
                    }
                    else
                    {
                        LogError( "Cellular_BG96InputBufferCallback : Error processing in dataLength. token %s.", pToken );
                        atCoreStatus = CELLULAR_AT_ERROR;
                    }
                }

                /* Translate atCoreStatus to packet status to indicate error. */
                pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
            }
        }

        return pktStatus;
    }
#endif /* if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 ) */
/*-----------------------------------------------------------*/

#if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 )

/**
 * @brief Copy socket data in URC to the socekt buffer in module context.
 */
    static CellularPktStatus_t prvStoreDirectPushSocketData( CellularContext_t * pContext,
                                                             char * pBuffer,
                                                             uint32_t prefixLength,
                                                             uint32_t socketIndex,
                                                             uint32_t dataLength )
    {
        uint8_t * pDataPtr = NULL;
        uint32_t socketDataSize;
        CellularSocketContext_t * pSocketData;
        cellularModuleContext_t * pModuleContext = NULL;
        CellularError_t cellularStatus;
        CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

        pSocketData = _Cellular_GetSocketData( pContext, socketIndex );

        if( pSocketData == NULL )
        {
            /* Invalid socket index. */
            LogError( "Cellular_BG96InputBufferCallback : Invalid socket index %u.", socketIndex );
            pktStatus = CELLULAR_PKT_STATUS_FAILURE;
        }
        else if( pSocketData->socketState != SOCKETSTATE_CONNECTED )
        {
            /* Invalid socket state. This could be socket is not closed before
             * the cellular interface is inited. Return handled to pktio and discard the data. */
            LogWarn( "Cellular_BG96InputBufferCallback : Invalid socket state %u. Discard packet.", socketIndex );
        }
        else
        {
            cellularStatus = _Cellular_GetModuleContext( pContext, ( void ** ) &pModuleContext );

            if( cellularStatus == CELLULAR_SUCCESS )
            {
                /* Copy the data to the socket buffer. */
                PlatformMutex_Lock( &pModuleContext->contextMutex );
                pDataPtr = pModuleContext->pSocketBuffer[ socketIndex ];
                socketDataSize = pModuleContext->pSocketDataSize[ socketIndex ];

                /* Check empty socket buffer left. */
                if( ( CELLULAR_BG96_DIRECT_PUSH_SOCKET_BUFFER_SIZE - socketDataSize ) > dataLength )
                {
                    memcpy( &pDataPtr[ socketDataSize ], &pBuffer[ prefixLength ], dataLength );
                    pModuleContext->pSocketDataSize[ socketIndex ] += dataLength;

                    PlatformMutex_Unlock( &pModuleContext->contextMutex );

                    /* Notify upper layer about data received. */
                    _informDataReadyToUpperLayer( pSocketData );
                }
                else
                {
                    LogError( "Cellular_BG96InputBufferCallback : drop socket %u packet. buffer left %u is not enough for %u.",
                              socketIndex, ( CELLULAR_BG96_DIRECT_PUSH_SOCKET_BUFFER_SIZE - socketDataSize ), dataLength );
                    pktStatus = CELLULAR_PKT_STATUS_FAILURE;
                }
            }
            else
            {
                /* Get the module context error. */
                LogError( "Cellular_BG96InputBufferCallback : get module context failed." );
                pktStatus = CELLULAR_PKT_STATUS_FAILURE;
            }
        }

        return pktStatus;
    }
#endif /* if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 ) */
/*-----------------------------------------------------------*/

CellularPktStatus_t _Cellular_ParseSimstat( char * pInputStr,
                                            CellularSimCardState_t * pSimState )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularATError_t atCoreStatus = CELLULAR_AT_SUCCESS;
    char * pToken = NULL;
    char * pLocalInputStr = pInputStr;
    int32_t tempValue = 0;

    if( ( pInputStr == NULL ) || ( strlen( pInputStr ) == 0U ) ||
        ( strlen( pInputStr ) < 2U ) || ( pSimState == NULL ) )
    {
        LogError( "_Cellular_ProcessQsimstat Input data is invalid %s", pInputStr );
        pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
    }
    else
    {
        atCoreStatus = Cellular_ATGetNextTok( &pLocalInputStr, &pToken );

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            LogDebug( "QSIMSTAT URC Enable: %s", pToken );
            atCoreStatus = Cellular_ATGetNextTok( &pLocalInputStr, &pToken );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            LogDebug( " Sim insert status: %s", pToken );
            atCoreStatus = Cellular_ATStrtoi( pToken, 10, &tempValue );
        }

        if( atCoreStatus == CELLULAR_AT_SUCCESS )
        {
            if( ( tempValue >= 0 ) &&
                ( tempValue < ( int32_t ) CELLULAR_SIM_CARD_STATUS_MAX ) )
            {
                /* Variable "tempValue" is ensured that it is valid and within
                 * a valid range. Hence, assigning the value at the  pointer of
                 * type cellular_SimCardState_t with an enum cast. */
                *pSimState = ( CellularSimCardState_t ) tempValue;
            }
            else
            {
                LogError( "Error in processing SIM state. token %s", pToken );
                atCoreStatus = CELLULAR_AT_ERROR;
            }
        }

        pktStatus = _Cellular_TranslateAtCoreStatus( atCoreStatus );
    }

    return pktStatus;
}
/*-----------------------------------------------------------*/

#if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 )
    CellularPktStatus_t Cellular_BG96InputBufferCallback( void * pInputBufferCallbackContext,
                                                          char * pBuffer,
                                                          uint32_t bufferLength,
                                                          uint32_t * pBufferLengthHandled )
    {
        CellularContext_t * pContext = ( CellularContext_t * ) pInputBufferCallbackContext;
        uint32_t socketIndex;
        uint32_t dataLength;
        uint32_t prefixLength;
        const uint32_t suffixLength = 2; /* The "\r\n" after the data stream. */
        CellularPktStatus_t pktStatus;

        if( pInputBufferCallbackContext == NULL )
        {
            LogError( "Cellular_BG96InputBufferCallback : pInputBufferCallbackContext is NULL." );
            pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
        }
        else if( pBuffer == NULL )
        {
            LogError( "Cellular_BG96InputBufferCallback : pBuffer is NULL." );
            pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
        }
        else if( pBufferLengthHandled == NULL )
        {
            LogError( "Cellular_BG96InputBufferCallback : pBufferLengthHandled is NULL." );
            pktStatus = CELLULAR_PKT_STATUS_BAD_PARAM;
        }
        else if( bufferLength < CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX_LEN )
        {
            /* Return CELLULAR_PKT_STATUS_PREFIX_MISMATCH if there is not enough information.
             * pktio thread will continue to process the buffer. */
            pktStatus = CELLULAR_PKT_STATUS_PREFIX_MISMATCH;
        }
        else if( strstr( pBuffer, CELLULAR_BG96_DIRECT_PUSH_SOCKET_URC_PFREFIX ) == NULL )
        {
            /* Return CELLULAR_PKT_STATUS_PREFIX_MISMATCH as the prefix is not match. */
            pktStatus = CELLULAR_PKT_STATUS_PREFIX_MISMATCH;
        }
        else
        {
            pktStatus = prvParseDirectPushURCPrefix( pBuffer, bufferLength, &prefixLength, &socketIndex, &dataLength );

            if( pktStatus != CELLULAR_PKT_STATUS_OK )
            {
                /* Error during parse the direct push URC. */
            }
            else if( ( prefixLength + dataLength + suffixLength ) > bufferLength )
            {
                /* Check if the complete data is received. If not, returns CELLULAR_PKT_STATUS_SIZE_MISMATCH
                 * to stop pktio from further process the data. This function will be called
                 * again with more data. */
                pktStatus = CELLULAR_PKT_STATUS_SIZE_MISMATCH;
            }
            else
            {
                /* Store the socket URC to a buffer in module context. */
                pktStatus = prvStoreDirectPushSocketData( pContext, pBuffer, prefixLength, socketIndex, dataLength );

                if( pktStatus == CELLULAR_PKT_STATUS_OK )
                {
                    /* Returns the complenet URC data length. Pktio thread will process
                     * the data after. */
                    *pBufferLengthHandled = prefixLength + dataLength + suffixLength;
                }
            }
        }

        return pktStatus;
    }
#endif /* if ( CELLULAR_BG96_SUPPPORT_DIRECT_PUSH_SOCKET == 1 ) */
/*-----------------------------------------------------------*/
