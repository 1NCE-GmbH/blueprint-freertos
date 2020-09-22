/**
  ******************************************************************************
  * @file    cellular_service_os.h
  * @author  MCD Application Team
  * @brief   Header for cellular_service_os.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CELLULAR_SERVICE_OS_H
#define CELLULAR_SERVICE_OS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "plf_config.h"
#include "cellular_service.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/* Cellular Service Library Init */
CS_Bool_t osCDS_cellular_service_init(void);

/**
  * @brief  Read the actual signal quality seen by Modem .
  * @note   Call CS_get_signal_quality with mutex acces protection
  * @param  same parameters as the CS_get_signal_quality function
  * @retval CS_Status_t
  */
CS_Status_t osCS_get_signal_quality(CS_SignalQuality_t *p_sig_qual);

/* SOCKET API */
/**
  * @brief  Allocate a socket among of the free sockets (maximum 6 sockets)
  * @param  addr_type Specifies a communication domain.
  *         This parameter can be one of the following values:
  *         IPAT_IPV4 for IPV4 (default)
  *         IPAT_IPV6  for IPV6
  * @param  protocol Specified the transport protocol to be used with the socket.
  *         TCP_PROTOCOL
  *         UDP_PROTOCOL
  * @param  cid Specifies the identity of PDN configuration to be used.
  *         CS_PDN_PREDEF_CONFIG To use default PDN configuration.
  *         CS_PDN_USER_CONFIG_1-5 To use a dedicated PDN configuration.
  * @retval Socket handle which references allocated socket
  */
socket_handle_t osCDS_socket_create(CS_IPaddrType_t addr_type,
                                    CS_TransportProtocol_t protocol,
                                    CS_PDN_conf_id_t cid);

/**
  * @brief  Set the callbacks to use when datas are received or sent.
  * @note   This function has to be called before to use a socket.
  * @param  sockHandle Handle of the socket
  * @param  data_ready_cb Pointer to the callback function to call when datas are received
  * @param  data_sent_cb Pointer to the callback function to call when datas has been sent
  *         This parameter is only used for asynchronous behavior (NOT IMPLEMENTED)
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_set_callbacks(socket_handle_t sockHandle,
                                       cellular_socket_data_ready_callback_t data_ready_cb,
                                       cellular_socket_data_sent_callback_t data_sent_cb,
                                       cellular_socket_closed_callback_t remote_close_cb);

/**
  * @brief  Define configurable options for a created socket.
  * @note   This function is called to configure one parameter at a time.
  *         If a parameter is not configured with this function, a default value will be applied.
  * @note   Call CDS_socket_set_option with mutex access protection
  * @param  same parameters as the CDS_socket_set_option function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_set_option(socket_handle_t sockHandle,
                                    CS_SocketOptionLevel_t opt_level,
                                    CS_SocketOptionName_t opt_name,
                                    void *p_opt_val);

/**
  * @brief  Retrieve configurable options for a created socket.
  * @note   This function is called for one parameter at a time.
  * @note   Function not implemented yet
  * @note   Call CDS_socket_get_option with mutex access protection
  * @param  same parameters as the CDS_socket_get_option function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_get_option(void);

/**
  * @brief  Bind the socket to a local port.
  * @note   If this function is not called, default local port value = 0 will be used.
  * @note   Call CDS_socket_bind with mutex access protection
  * @param  same parameters as the CDS_socket_bind function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_bind(socket_handle_t sockHandle,
                              uint16_t local_port);

/**
  * @brief  Connect to a remote server (for socket client mode).
  * @note   This function is blocking until the connection is setup or when the timeout to wait
  *         for socket connection expires.
  * @note   Call CDS_socket_connect with mutex access protection
  * @param  same parameters as the CDS_socket_connect function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_connect(socket_handle_t sockHandle,
                                 CS_IPaddrType_t addr_type,
                                 CS_CHAR_t *p_ip_addr_value,
                                 uint16_t remote_port); /* for socket client mode */

/**
  * @brief  Listen to clients (for socket server mode).
  * @note   Function not implemeted yet
  * @note   Call CDS_socket_listen with mutex access protection
  * @param  same parameters as the CDS_socket_listen function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_listen(socket_handle_t sockHandle);

/**
  * @brief  Send data over a socket to a remote server.
  * @note   This function is blocking until the data is transfered or when the
  *         timeout to wait for transmission expires.
  * @note   Call CDS_socket_send with mutex access protection
  * @param  same parameters as the CDS_socket_send function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_send(socket_handle_t sockHandle,
                              const CS_CHAR_t *p_buf,
                              uint32_t length);

/**
  * @brief  Receive data from the connected remote server.
  * @note   This function is blocking until expected data length is received or a receive timeout has expired.
  * @note   Call CDS_socket_receive with mutex access protection
  * @param  same parameters as the CDS_socket_receive function
  * @retval Size of received data (in bytes).
  */
int32_t osCDS_socket_receive(socket_handle_t sockHandle,
                             CS_CHAR_t *p_buf,
                             uint32_t max_buf_length);

/**
  * @brief  Send data over a socket to a remote server.
  * @note   This function is blocking until the data is transfered or when the
  *         timeout to wait for transmission expires.
  * @note   Call CDS_socket_sendto with mutex access protection
  * @param  same parameters as the CDS_socket_sendto function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_sendto(socket_handle_t sockHandle,
                                const CS_CHAR_t *p_buf,
                                uint32_t length,
                                CS_IPaddrType_t addr_type,
                                CS_CHAR_t *p_ip_addr_value,
                                uint16_t remote_port);

/**
  * @brief  Receive data from the connected remote server.
  * @note   This function is blocking until expected data length is received or a receive timeout has expired.
  * @note   Call CDS_socket_receivefrom with mutex access protection
  * @param  same parameters as the CDS_socket_receivefrom function
  * @retval Size of received data (in bytes).
  */
int32_t osCDS_socket_receivefrom(socket_handle_t sockHandle,
                                 CS_CHAR_t *p_buf,
                                 uint32_t max_buf_length,
                                 CS_IPaddrType_t *p_addr_type,
                                 CS_CHAR_t *p_ip_addr_value,
                                 uint16_t *p_remote_port);

CS_Status_t osCDS_socket_cnx_status(socket_handle_t sockHandle,
                                    CS_SocketCnxInfos_t *infos);

/**
  * @brief  Get connection status for a given socket.
  * @note   If a PDN is activated at socket creation, the socket will not be deactivated at socket closure.
  * @note   Call CDS_socket_cnx_status with mutex access protection
  * @param  same parameters as the CDS_socket_cnx_status function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_socket_close(socket_handle_t sockHandle,
                               uint8_t force);

/* =========================================================
   ===========      Mode Command services        ===========
   ========================================================= */
/**
  * @brief  Read the latest registration state to the Cellular Network.
  * @note   Call CS_get_net_status with mutex access protection
  * @param  same parameters as the CS_get_net_status function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_get_net_status(CS_RegistrationStatus_t *p_reg_status);

/**
  * @brief  Return information related to modem status.
  * @note   Call CS_get_device_info with mutex access protection
  * @param  same parameters as the CS_get_device_info function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_get_device_info(CS_DeviceInfo_t *p_devinfo);

/**
  * @brief  Register to an event change notification related to Network status.
  * @note   This function should be called for each event the user wants to start monitoring.
  * @param  same parameters as the CS_subscribe_net_event function
  * @param  urc_callback Handle on user callback that will be used to notify a
  *                      change on requested event.
  * @retval CS_Status_t
  */
CS_Status_t osCDS_subscribe_net_event(CS_UrcEvent_t event,
                                      cellular_urc_callback_t urc_callback);

/**
  * @brief  Register to specified modem events.
  * @note   This function should be called once with all requested events.
  * @note   Call CS_subscribe_modem_event with mutex access protection
  * @param  same parameters as the CS_subscribe_modem_event function
  *         change on requested event.
  * @retval CS_Status_t
  */
CS_Status_t osCDS_subscribe_modem_event(CS_ModemEvent_t events_mask,
                                        cellular_modem_event_callback_t modem_evt_cb);

/**
  * @brief  Power ON the modem
  * @note   Call CS_power_on with mutex access protection
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t osCDS_power_on(void);

/**
  * @brief  Request to reset the device.
  * @note   Call CS_reset with mutex access protection
  * @param  same parameters as the CS_reset function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_reset(CS_Reset_t rst_type);

/**
  * @brief  Initialize the service and configures the Modem FW functionalities
  * @note   Used to provide PIN code (if any) and modem function level.
  * @note   Call CS_init_modem with mutex access protection
  * @param  same parameters as the CS_init_modem function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_init_modem(CS_ModemInit_t init,
                             CS_Bool_t reset,
                             const CS_CHAR_t *pin_code);

/**
  * @brief  Request the Modem to register to the Cellular Network.
  * @note   This function is used to select the operator. It returns a detailled
  *         network registration status.
  * @note   Call CS_register_net with mutex access protection
  * @param  same parameters as the CS_register_net function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_register_net(CS_OperatorSelector_t *p_operator,
                               CS_RegistrationStatus_t *p_reg_status);

/**
  * @brief  Request for packet attach status.
  * @note   Call CDS_socket_set_callbacks with mutex access protection
  * @param  same parameters as the CDS_socket_set_callbacks function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_get_attach_status(CS_PSattach_t *p_attach);

/**
  * @brief  Request attach to packet domain.
  * @note   Call CS_attach_PS_domain with mutex access protection
  * @param  none.
  * @retval CS_Status_t
  */
CS_Status_t osCDS_attach_PS_domain(void);

CS_Status_t osCDS_define_pdn(CS_PDN_conf_id_t cid,
                             const CS_CHAR_t *apn,
                             CS_PDN_configuration_t *pdn_conf);

/**
  * @brief  Define internet data profile for a configuration identifier
  * @note   Call CS_define_pdn with mutex access protection
  * @param  same parameters as the CS_define_pdn function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_register_pdn_event(CS_PDN_conf_id_t cid,
                                     cellular_pdn_event_callback_t pdn_event_callback);

/**
  * @brief  Select a PDN among of defined configuration identifier(s) as the default.
  * @note   By default, PDN_PREDEF_CONFIG is considered as the default PDN.
  * @note   Call CS_set_default_pdn with mutex access protection
  * @param  same parameters as the CS_set_default_pdn function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_set_default_pdn(CS_PDN_conf_id_t cid);

/**
  * @brief  Activates a PDN (Packet Data Network Gateway) allowing communication with internet.
  * @note   This function triggers the allocation of IP public WAN to the device.
  * @note   Only one PDN can be activated at a time.
  * @note   Call CS_activate_pdn with mutex access protection
  * @param  same parameters as the CS_activate_pdn function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_activate_pdn(CS_PDN_conf_id_t cid);

/**
  * @brief  Request to suspend DATA mode.
  * @note   Call CS_suspend_data with mutex access protection
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t osCDS_suspend_data(void);

/**
  * @brief  Request to resume DATA mode.
  * @note   Call CS_resume_data with mutex access protection
  * @param  none
  * @retval CS_Status_t
  */
CS_Status_t osCDS_resume_data(void);

/**
  * @brief  DNS request
  * @note   Get IP address of the specifed hostname
  * @note   Call CS_dns_request with mutex access protection
  * @param  same parameters as the CS_dns_request function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_dns_request(CS_PDN_conf_id_t cid,
                              CS_DnsReq_t  *dns_req,
                              CS_DnsResp_t *dns_resp);

/**
  * @brief  Ping an IP address on the network
  * @note   Usually, the command AT is sent and OK is expected as response
  * @note   Call CDS_ping with mutex access protection
  * @param  same parameters as the CDS_ping function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_ping(CS_PDN_conf_id_t cid,
                       CS_Ping_params_t *ping_params,
                       cellular_ping_response_callback_t cs_ping_rsp_cb);

/**
  * @brief  Send a string will which be sended as it is to the modem (termination char will be added automatically)
  * @note   The termination char will be automatically added by the lower layer
  * @note   Call CS_direct_cmd with mutex access protection
  * @param  same parameters as the CS_direct_cmd function
  * @retval CS_Status_t
  */
CS_Status_t osCDS_direct_cmd(CS_direct_cmd_tx_t *direct_cmd_tx,
                             cellular_direct_cmd_callback_t direct_cmd_callback);

CS_Status_t osCDS_get_dev_IP_address(CS_PDN_conf_id_t cid,
                                     CS_IPaddrType_t *ip_addr_type,
                                     CS_CHAR_t *p_ip_addr_value);

/**
  * @brief  Get the IP address allocated to the device for a given PDN.
  * @note   Call osCDS_get_dev_IP_address with mutex access protection
  * @param  same parameters as the osCDS_get_dev_IP_address function
  * @retval CS_Status_t
  */
CS_Status_t osCS_sim_select(CS_SimSlot_t simSelected);

extern  osMutexId CellularServiceMutexHandle;

/* shared resource allocation */
/**
  * @brief get resource
  * @note cellular service automation  protection against concurrent access
  * @param  none
  * @retval none
  */
void osCCS_get_wait_cs_resource(void);

/**
  * @brief release resource
  * @brief cellular service automation  protection against concurrent access
  * @param  none
  * @retval none
  */
void osCCS_get_release_cs_resource(void);

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_SERVICE_OS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
