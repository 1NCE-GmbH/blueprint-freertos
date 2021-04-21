/*
 * nce_bg96_configuration.c
 *
 *  Created on: Sep 7, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "nce_bg96_configuration.h"

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

CellularPktStatus_t nce_send_modem_command( char * at_command )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;
    CellularContext_t * pContext = ( CellularContext_t * ) CellularHandle;

    char cmdBuf[ CELLULAR_AT_CMD_MAX_SIZE ] = { '\0' };
    CellularAtReq_t atReq =
    {
        cmdBuf,
        CELLULAR_AT_NO_RESULT,
        NULL,
        NULL,
        NULL,
        0,
    };

/*    ( void ) snprintf( cmdBuf, CELLULAR_AT_CMD_MAX_SIZE, "%s%d,\"%s\"", "AT+QIDNSCFG=", contextId, pDnsServerAddress ); */
    ( void ) snprintf( cmdBuf, CELLULAR_AT_CMD_MAX_SIZE, at_command );
    pktStatus = _Cellular_AtcmdRequestWithCallback( pContext, atReq );
    return( pktStatus );
}

/*-----------------------------------------------------------*/

CellularPktStatus_t nce_configure_onboarding_socket( void )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"seclevel\",0,0" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"sslversion\",0,4" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"ciphersuite\",0,0xFFFF" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"ignorelocaltime\",0,1" );

    pktStatus = nce_send_modem_command( "AT+QSSLCLOSE=1" );

    return( pktStatus );
}

CellularPktStatus_t nce_reset_modem_fs( void )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    /*Cleaning Memory*/
    pktStatus = nce_send_modem_command( "AT+QFDEL=\"*\"" );

    return( pktStatus );
}

/*-----------------------------------------------------------*/

CellularPktStatus_t nce_configure_ssl_socket( void )
{
    CellularPktStatus_t pktStatus = CELLULAR_PKT_STATUS_OK;

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"cacert\",1,\"root.pem\"" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"clientcert\",1,\"clientcert.pem\"" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"clientkey\",1,\"clientkey.pem\"" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"seclevel\",1,2" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"sslversion\",1,4" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"ciphersuite\",1,0xFFFF" );

    pktStatus = nce_send_modem_command( "AT+QSSLCFG=\"ignorelocaltime\",1,1" );

    pktStatus = nce_send_modem_command( "AT+QSSLCLOSE=1,10" );

    pktStatus = nce_send_modem_command( "AT+QSSLCLOSE=2,10" );

    return( pktStatus );
}
