/**
  ******************************************************************************
  * @file    cellular_service.h
  * @author  MCD Application Team
  * @brief   Header for cellular_service.c module
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
#ifndef CELLULAR_SERVICE_H
#define CELLULAR_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "at_core.h"
#include "plf_config.h"

/* Exported constants --------------------------------------------------------*/
#define CELLULAR_MAX_SOCKETS     (6U)
#define MAX_SIZE_IMEI            ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_MANUFACT_NAME   ((uint16_t) 256U) /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_MODEL           ((uint16_t) 256U) /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_REV             ((uint16_t) 64U)  /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_SN              ((uint16_t) 64U)  /* theorical MAX = 2048 characters !!! */
#define MAX_SIZE_IMSI            ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_PHONE_NBR       ((uint16_t) 64U)  /* MAX = 32 characters */
#define MAX_SIZE_ICCID           ((uint16_t) 32U)  /* MAX = 32 characters */
#define MAX_SIZE_OPERATOR_NAME   ((uint16_t) 64U)  /* MAX = 64 characters */
#define MAX_SIZE_USERNAME        ((uint16_t) 32U)  /* MAX = 32 characters */
#define MAX_SIZE_PASSWORD        ((uint16_t) 32U)  /* MAX = 32 characters */
#define MAX_SIZE_IPADDR          ((uint16_t) 64U)  /* MAX = 64 characters */
#define MAX_DIREXT_CMD_SIZE      ((uint16_t)(ATCMD_MAX_BUF_SIZE - 10U))
//#define MAX_DIREXT_CMD_SIZE      ((uint16_t)(2000U))

#define CS_INVALID_SOCKET_HANDLE ((socket_handle_t)-1)

/* Exported types ------------------------------------------------------------*/
typedef int32_t socket_handle_t;
typedef uint8_t CS_CHAR_t;

/* enum */
typedef enum
{
  CELLULAR_FALSE = 0,
  CELLULAR_TRUE  = 1,
} CS_Bool_t;

typedef enum
{
  CELLULAR_OK = 0,                /* No error */
  CELLULAR_ERROR,                 /* Generic error */
  CELLULAR_NOT_IMPLEMENTED,       /* Function not implemented yet */
  /* - SIM errors - */
  CELLULAR_SIM_BUSY,               /* SIM error: SIM is busy */
  CELLULAR_SIM_NOT_INSERTED,       /* SIM error: SIM not inserted */
  CELLULAR_SIM_PIN_OR_PUK_LOCKED,  /* SIM error: SIM locked due to PIN, PIN2, PUK or PUK2 */
  CELLULAR_SIM_INCORRECT_PASSWORD, /* SIM error: SIM password is incorrect */
  CELLULAR_SIM_ERROR,              /* SIM error: SIM other error */

} CS_Status_t;

typedef enum
{
  CS_CMI_MINI = 0,
  CS_CMI_FULL,
  CS_CMI_SIM_ONLY,
} CS_ModemInit_t;

typedef enum
{
  CS_MODEM_SIM_SOCKET_0 = 0,    /* to select SIM card placed in SIM socket */
  CS_MODEM_SIM_ESIM_1,          /* to select integrated SIM in modem module */
  CS_STM32_SIM_2,               /* to select SIM in STM32 side (various implementations) */
} CS_SimSlot_t;

typedef enum
{
  CS_PS_DETACHED = 0,
  CS_PS_ATTACHED = 1,
} CS_PSattach_t;

typedef enum
{
  CS_NRM_AUTO = 0,
  CS_NRM_MANUAL = 1,
  CS_NRM_DEREGISTER = 2,
  CS_NRM_MANUAL_THEN_AUTO = 4,
} CS_NetworkRegMode_t;

typedef enum
{
  CS_NRS_NOT_REGISTERED_NOT_SEARCHING     = 0,
  CS_NRS_REGISTERED_HOME_NETWORK          = 1,
  CS_NRS_NOT_REGISTERED_SEARCHING         = 2,
  CS_NRS_REGISTRATION_DENIED              = 3,
  CS_NRS_UNKNOWN                          = 4,
  CS_NRS_REGISTERED_ROAMING               = 5,
  CS_NRS_REGISTERED_SMS_ONLY_HOME_NETWORK = 6,
  CS_NRS_REGISTERED_SMS_ONLY_ROAMING      = 7,
  CS_NRS_EMERGENCY_ONLY                   = 8,
  CS_NRS_REGISTERED_CFSB_NP_HOME_NETWORK  = 9,
  CS_NRS_REGISTERED_CFSB_NP_ROAMING       = 10,
} CS_NetworkRegState_t;

typedef enum
{
  CS_ACT_GSM               = 0,
  CS_ACT_GSM_COMPACT       = 1,
  CS_ACT_UTRAN             = 2,
  CS_ACT_GSM_EDGE          = 3,
  CS_ACT_UTRAN_HSDPA       = 4,
  CS_ACT_UTRAN_HSUPA       = 5,
  CS_ACT_UTRAN_HSDPA_HSUPA = 6,
  CS_ACT_E_UTRAN           = 7,
  CS_ACT_EC_GSM_IOT        = 8, /* = LTE Cat.M1 */
  CS_ACT_E_UTRAN_NBS1      = 9, /* = LTE Cat.NB1 */
} CS_AccessTechno_t;

typedef enum
{
  CS_ONF_LONG = 0,    /* up to 16 chars */
  CS_ONF_SHORT = 1,   /* up to 8 chars */
  CS_ONF_NUMERIC = 2, /* LAI */
  CS_ONF_NOT_PRESENT = 9, /* Operator Name not present */
} CS_OperatorNameFormat_t;

typedef enum
{
  CS_URCEVENT_NONE                  = 0, /* none */
  CS_URCEVENT_EPS_NETWORK_REG_STAT  = 1, /* EPS registration status (CEREG) */
  CS_URCEVENT_EPS_LOCATION_INFO     = 2, /* EPS location status (CEREG) */
  CS_URCEVENT_GPRS_NETWORK_REG_STAT = 3, /* GPRS registration status (CGREG) */
  CS_URCEVENT_GPRS_LOCATION_INFO    = 4, /* GPRS registration status (CGREG) */
  CS_URCEVENT_CS_NETWORK_REG_STAT   = 5, /* Circuit-switched registration status (CREG) */
  CS_URCEVENT_CS_LOCATION_INFO      = 6, /* Circuit-switched status (CREG) */
  CS_URCEVENT_SIGNAL_QUALITY        = 7, /* signal quality registration (if supported) */
  CS_URCEVENT_PING_RSP              = 8, /* Ping response */
} CS_UrcEvent_t;

typedef uint16_t CS_ModemEvent_t;
#define CS_MDMEVENT_NONE          (CS_ModemEvent_t)(0x0000) /* none */
#define CS_MDMEVENT_BOOT          (CS_ModemEvent_t)(0x0001) /* Modem Boot indication received (if exists) */
#define CS_MDMEVENT_POWER_DOWN    (CS_ModemEvent_t)(0x0002) /* Modem Power Down indication received (if exists) */
#define CS_MDMEVENT_FOTA_START    (CS_ModemEvent_t)(0x0004) /* Modem FOTA Start indication received (if exists) */
#define CS_MDMEVENT_FOTA_END      (CS_ModemEvent_t)(0x0008) /* Modem FOTA End indication received (if exists) */
#define CS_MDMEVENT_LP_ENTER      (CS_ModemEvent_t)(0x0010) /* Modem Enter Low Power state */
#define CS_MDMEVENT_LP_LEAVE      (CS_ModemEvent_t)(0x0020) /* Modem Leave Low Power state */

typedef enum
{
  CS_TCP_PROTOCOL = 0,
  CS_UDP_PROTOCOL = 1,
} CS_TransportProtocol_t;

typedef uint16_t CS_ConnectionMode_t; /* uint16_t is used to keep API consistent */
#define CS_CM_COMMAND_MODE              (CS_ConnectionMode_t)(0x0U) /* remains in command line after each
                                                                     * packet transmission (tx or rx) - default */
#define CS_CM_ONLINE_MODE               (CS_ConnectionMode_t)(0x1U) /* NOT SUPPORTED - remains in online mode until
                                                                     * transfer over socket is closed */
#define CS_CM_ONLINE_AUTOMATIC_SUSPEND  (CS_ConnectionMode_t)(0x2U) /* NOT SUPPORTED - remains in online mode until
                                                                     * a suspend timeout expires */

typedef enum
{
  CS_IPAT_INVALID = 0,   /* invalid format */
  CS_IPAT_IPV4,          /* IPV4 */
  CS_IPAT_IPV6,          /* IPV6 */
} CS_IPaddrType_t;

typedef enum
{
  CS_SOL_IP        = 0,
  CS_SOL_TRANSPORT = 1,
} CS_SocketOptionLevel_t;

typedef uint16_t CS_SocketOptionName_t;
#define  CS_SON_NO_OPTION                 (CS_SocketOptionName_t)(0x00)
#define  CS_SON_IP_MAX_PACKET_SIZE        (CS_SocketOptionName_t)(0x01)  /* 0 to 1500 bytes */
#define  CS_SON_TRP_MAX_TIMEOUT           (CS_SocketOptionName_t)(0x02)  /* 0 to 65535, in second, 0 means infinite */
#define  CS_SON_TRP_CONNECT_SETUP_TIMEOUT (CS_SocketOptionName_t)(0x04)  /* 10 to 1200, in 100 of ms, 0 means infnite*/
#define  CS_SON_TRP_TRANSFER_TIMEOUT      (CS_SocketOptionName_t)(0x08)  /* 1 to 255, in ms, 0 means infinite */
#define  CS_SON_TRP_CONNECT_MODE          (CS_SocketOptionName_t)(0x10)  /**/
#define  CS_SON_TRP_SUSPEND_TIMEOUT       (CS_SocketOptionName_t)(0x20)  /* 0 to 2000, in ms , 0 means infinite */
#define  CS_SON_TRP_RX_TIMEOUT            (CS_SocketOptionName_t)(0x40)  /* 0 to 255, in ms, 0 means infinite */


typedef uint16_t CS_DeviceInfoFields_t;
#define  CS_DIF_IMEI_PRESENT               (CS_DeviceInfoFields_t)(0x01)
#define  CS_DIF_MANUF_NAME_PRESENT         (CS_DeviceInfoFields_t)(0x02)
#define  CS_DIF_MODEL_PRESENT              (CS_DeviceInfoFields_t)(0x04)
#define  CS_DIF_REV_PRESENT                (CS_DeviceInfoFields_t)(0x08)
#define  CS_DIF_SN_PRESENT                 (CS_DeviceInfoFields_t)(0x10)
#define  CS_DIF_IMSI_PRESENT               (CS_DeviceInfoFields_t)(0x20)
#define  CS_DIF_PHONE_NBR_PRESENT          (CS_DeviceInfoFields_t)(0x40)
#define  CS_DIF_ICCID_PRESENT              (CS_DeviceInfoFields_t)(0x80)

typedef enum
{
  CS_PDPTYPE_IP                  = 0x00,
  CS_PDPTYPE_IPV6                = 0x01,
  CS_PDPTYPE_IPV4V6              = 0x02,
  CS_PDPTYPE_PPP                 = 0x03,
  /*    CS_PDPTYPE_X25     */         /* NOT SUPPORTED - OBSOLETE */
  /*    CS_PDPTYPE_OSPIH   */         /* NOT SUPPORTED - OBSOLETE */
  /*    CS_PDPTYPE_NON_IP  */         /* NOT SUPPORTED */
} CS_PDPtype_t;

typedef enum
{
  CS_RESET_SW             = 0,
  CS_RESET_HW             = 1,
  CS_RESET_AUTO           = 2,
  CS_RESET_FACTORY_RESET  = 3,
} CS_Reset_t;

typedef enum
{
  CS_PDN_PREDEF_CONFIG  = 0,     /* pre-defined configuration, cannot be modified by user */
  CS_PDN_USER_CONFIG_1  = 1,     /* user defined configuration */
  CS_PDN_USER_CONFIG_2  = 2,     /* user defined configuration */
  CS_PDN_USER_CONFIG_3  = 3,     /* user defined configuration */
  CS_PDN_USER_CONFIG_4  = 4,     /* user defined configuration */
  CS_PDN_USER_CONFIG_5  = 5,     /* user defined configuration */
  CS_PDN_CONFIG_MAX     = CS_PDN_USER_CONFIG_5,
  CS_PDN_CONFIG_DEFAULT = 11,  /* use one of previous config. (if not set, will use PDN_PREDEF_CONFIG by default) */
  CS_PDN_NOT_DEFINED    = 12,     /* for internal use only */
  CS_PDN_ALL            = 13,     /* for internal use only: all PDN are concerned */
} CS_PDN_conf_id_t;

typedef enum
{
  CS_PDN_EVENT_OTHER, /*none of the event described below */
  CS_PDN_EVENT_NW_DETACH,
  CS_PDN_EVENT_NW_DEACT,
  CS_PDN_EVENT_NW_PDN_DEACT,
} CS_PDN_event_t;

/* structures */
typedef struct
{
  /* Not implemented yet */
  /* this configuration structure will be used to configure modem at boot time
     also when reset triggered ? */
  CS_Bool_t test_variable;
} CS_ModemConfig_t;

typedef struct
{
  CS_DeviceInfoFields_t field_requested; /* device info field request below */
  union
  {
    CS_CHAR_t imei[MAX_SIZE_IMEI];
    CS_CHAR_t manufacturer_name[MAX_SIZE_MANUFACT_NAME];
    CS_CHAR_t model[MAX_SIZE_MODEL];
    CS_CHAR_t revision[MAX_SIZE_REV];
    CS_CHAR_t serial_number[MAX_SIZE_SN];
    CS_CHAR_t imsi[MAX_SIZE_IMSI];
    CS_CHAR_t phone_number[MAX_SIZE_PHONE_NBR];
    CS_CHAR_t iccid[MAX_SIZE_ICCID];
  } u;

} CS_DeviceInfo_t;

typedef struct
{
  CS_NetworkRegMode_t      mode;
  CS_OperatorNameFormat_t  format;
  CS_CHAR_t                operator_name[MAX_SIZE_OPERATOR_NAME];

} CS_OperatorSelector_t;

typedef uint16_t CS_RegistrationStatusFields_t;
#define CS_RSF_NONE                    (CS_RegistrationStatusFields_t)(0x00U)
#define CS_RSF_FORMAT_PRESENT          (CS_RegistrationStatusFields_t)(0x01U)
#define CS_RSF_OPERATOR_NAME_PRESENT   (CS_RegistrationStatusFields_t)(0x02U)
#define CS_RSF_ACT_PRESENT             (CS_RegistrationStatusFields_t)(0x04U)

typedef struct
{
  CS_NetworkRegMode_t               mode;                                   /* mandatory field */
  CS_NetworkRegState_t              EPS_NetworkRegState;                    /* mandatory field */
  CS_NetworkRegState_t              GPRS_NetworkRegState;                   /* mandatory field */
  CS_NetworkRegState_t              CS_NetworkRegState;                     /* mandatory field */

  CS_RegistrationStatusFields_t     optional_fields_presence;               /* which fields below are present */
  CS_OperatorNameFormat_t           format;                                 /* optional field */
  CS_CHAR_t                         operator_name[MAX_SIZE_OPERATOR_NAME];  /* optional field */
  CS_AccessTechno_t                 AcT;                                    /* optional field */

} CS_RegistrationStatus_t;

typedef struct
{
  uint8_t rssi;
  uint8_t ber;
} CS_SignalQuality_t;

typedef struct
{
  CS_PDPtype_t  pdp_type;  /* if NULL, applies default value */
  CS_CHAR_t     username[MAX_SIZE_USERNAME];
  CS_CHAR_t     password[MAX_SIZE_PASSWORD];
} CS_PDN_configuration_t;

typedef struct
{
  /* CS_SocketState_t    state; */ /* to add ? */
  CS_CHAR_t           loc_ip_addr_value[MAX_SIZE_IPADDR];
  uint16_t            loc_port;
  CS_CHAR_t           rem_ip_addr_value[MAX_SIZE_IPADDR];
  uint16_t            rem_port;
} CS_SocketCnxInfos_t;

typedef struct
{
  CS_CHAR_t           primary_dns_addr[MAX_SIZE_IPADDR];
} CS_DnsConf_t;

typedef struct
{
  CS_CHAR_t           host_name[MAX_SIZE_IPADDR];
} CS_DnsReq_t;

typedef struct
{
  CS_CHAR_t           host_addr[MAX_SIZE_IPADDR];
} CS_DnsResp_t;

typedef struct
{
  CS_CHAR_t  host_addr[MAX_SIZE_IPADDR];
  uint8_t  timeout;
  uint8_t  pingnum;
} CS_Ping_params_t;

typedef struct
{
  uint8_t      index;
  CS_Status_t  ping_status;
  CS_Bool_t    is_final_report;

  /* Intermediate ping reponse parameters (if is_final_report = false) */
  CS_CHAR_t  ping_addr[MAX_SIZE_IPADDR];
  uint16_t   ping_size; /* size in bytes */
  uint32_t   time;
  uint8_t    ttl;

  /* Final ping reponse parameters (if is_final_report = true)
    *  no parameters reported in this case
    */

} CS_Ping_response_t;

typedef struct
{
  CS_CHAR_t cmd_str[MAX_DIREXT_CMD_SIZE]; /* the command string to send to the modem (without termination characters)*/
  uint16_t  cmd_size;                     /* size of cmd_str */
  uint32_t  cmd_timeout;                  /* timeout (in ms) to apply. If set to 0, will use default timer value */
} CS_direct_cmd_tx_t;

typedef struct
{
  CS_CHAR_t cmd_str[MAX_DIREXT_CMD_SIZE]; /* the command string to send to the modem (without termination characters)*/
  uint16_t  cmd_size;
  CS_Bool_t cmd_final;
} CS_direct_cmd_rx_t;

/* callbacks */
#if (RTOS_USED == 0)
typedef void (* cellular_event_callback_t)(uint32_t event_callback);
#endif /* RTOS_USED == 0 */
typedef void (* cellular_urc_callback_t)(void);
typedef void (* cellular_modem_event_callback_t)(CS_ModemEvent_t modem_event_received);
typedef void (* cellular_pdn_event_callback_t)(CS_PDN_conf_id_t cid, CS_PDN_event_t pdn_event);
typedef void (* cellular_socket_data_ready_callback_t)(socket_handle_t sockHandle);
typedef void (* cellular_socket_data_sent_callback_t)(socket_handle_t sockHandle);
typedef void (* cellular_socket_closed_callback_t)(socket_handle_t sockHandle);
typedef void (* cellular_ping_response_callback_t)(CS_Ping_response_t ping_response);
typedef void (* cellular_direct_cmd_callback_t)(CS_direct_cmd_rx_t direct_cmd_rx);

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
#if (RTOS_USED == 0)
void CS_check_idle_events(void); /* only used for bare mode */
CS_Status_t CS_init_bare(cellular_event_callback_t event_callback); /* only used for bare mode */
#else /* RTOS_USED */
CS_Status_t CS_init(void);
#endif /* RTOS_USED */
CS_Status_t CS_power_on(void);
CS_Status_t CS_power_off(void);
CS_Status_t CS_check_connection(void);
CS_Status_t CS_sim_select(CS_SimSlot_t simSelected);
CS_Status_t CS_init_modem(CS_ModemInit_t init, CS_Bool_t reset, const CS_CHAR_t *pin_code);
CS_Status_t CS_get_device_info(CS_DeviceInfo_t *p_devinfo);
CS_Status_t CS_register_net(CS_OperatorSelector_t *p_operator,
                            CS_RegistrationStatus_t *p_reg_status);
CS_Status_t CS_get_net_status(CS_RegistrationStatus_t *p_reg_status);
CS_Status_t CS_subscribe_net_event(CS_UrcEvent_t event, cellular_urc_callback_t urc_callback);
CS_Status_t CS_unsubscribe_net_event(CS_UrcEvent_t event);
CS_Status_t CS_attach_PS_domain(void);
CS_Status_t CS_detach_PS_domain(void);
CS_Status_t CS_get_attach_status(CS_PSattach_t *p_attach);
CS_Status_t CS_get_signal_quality(CS_SignalQuality_t *p_sig_qual);
CS_Status_t CS_activate_pdn(CS_PDN_conf_id_t cid);
CS_Status_t CS_deactivate_pdn(CS_PDN_conf_id_t cid);
CS_Status_t CS_define_pdn(CS_PDN_conf_id_t cid, const CS_CHAR_t *apn, CS_PDN_configuration_t *pdn_conf);
CS_Status_t CS_set_default_pdn(CS_PDN_conf_id_t cid);
CS_Status_t CS_get_dev_IP_address(CS_PDN_conf_id_t cid, CS_IPaddrType_t *ip_addr_type, CS_CHAR_t *p_ip_addr_value);
CS_Status_t CS_suspend_data(void);
CS_Status_t CS_resume_data(void);
CS_Status_t CS_subscribe_modem_event(CS_ModemEvent_t events_mask, cellular_modem_event_callback_t modem_evt_cb);
CS_Status_t CS_register_pdn_event(CS_PDN_conf_id_t cid, cellular_pdn_event_callback_t pdn_event_callback);
CS_Status_t CS_deregister_pdn_event(CS_PDN_conf_id_t cid);
CS_Status_t CS_reset(CS_Reset_t rst_type);
CS_Status_t CS_dns_config(CS_PDN_conf_id_t cid, CS_DnsConf_t *dns_conf); /* NOT IMPLEMENTED YET */
CS_Status_t CS_dns_request(CS_PDN_conf_id_t cid, CS_DnsReq_t *dns_req, CS_DnsResp_t *dns_resp);
CS_Status_t CDS_ping(CS_PDN_conf_id_t cid, CS_Ping_params_t *ping_params,
                     cellular_ping_response_callback_t cs_ping_rsp_cb);
CS_Status_t CS_direct_cmd(CS_direct_cmd_tx_t *direct_cmd_tx, cellular_direct_cmd_callback_t direct_cmd_callback);

/* SOCKET API */
socket_handle_t CDS_socket_create(CS_IPaddrType_t addr_type,
                                  CS_TransportProtocol_t protocol,
                                  CS_PDN_conf_id_t cid);
CS_Status_t CDS_socket_bind(socket_handle_t sockHandle,
                            uint16_t local_port);
CS_Status_t CDS_socket_set_callbacks(socket_handle_t sockHandle,
                                     cellular_socket_data_ready_callback_t data_ready_cb,
                                     cellular_socket_data_sent_callback_t data_sent_cb,
                                     cellular_socket_closed_callback_t remote_close_cb);
CS_Status_t CDS_socket_set_option(socket_handle_t sockHandle,
                                  CS_SocketOptionLevel_t opt_level,
                                  CS_SocketOptionName_t opt_name,
                                  void *p_opt_val);
CS_Status_t CDS_socket_connect(socket_handle_t sockHandle,
                               CS_IPaddrType_t ip_addr_type,
                               CS_CHAR_t *p_ip_addr_value,
                               uint16_t remote_port); /* for socket client mode */
CS_Status_t CDS_socket_send(socket_handle_t sockHandle,
                            const CS_CHAR_t *p_buf,
                            uint32_t length);
CS_Status_t CDS_socket_sendto(socket_handle_t sockHandle,
                              const CS_CHAR_t *p_buf,
                              uint32_t length,
                              CS_IPaddrType_t ip_addr_type,
                              CS_CHAR_t *p_ip_addr_value,
                              uint16_t remote_port);
int32_t CDS_socket_receive(socket_handle_t sockHandle,
                           CS_CHAR_t *p_buf,
                           uint32_t max_buf_length);
int32_t CDS_socket_receivefrom(socket_handle_t sockHandle,
                               CS_CHAR_t *p_buf,
                               uint32_t max_buf_length,
                               CS_IPaddrType_t *p_ip_addr_type,
                               CS_CHAR_t *p_ip_addr_value,
                               uint16_t *p_remote_port);
CS_Status_t CDS_socket_close(socket_handle_t sockHandle,
                             uint8_t force);
CS_Status_t CDS_socket_cnx_status(socket_handle_t sockHandle,
                                  CS_SocketCnxInfos_t *p_infos);

/* LOW POWER API */

/* <<< LTE-M (cat.M1) deployment guide >>>
  *
  * PSM STANDALONE TIMERS
  * =====================
  * PSM turns off monitoring of Paging instances on the device and increases time periods of devices
  * sending Periodic Tracking Area Updates (pTAUs) to extended intervals to inform the network of its
  * current registration.
  * PSM has 2 timers:
  *   T3324 Active Timer: time the "IOT module" stays in ACTIVE/IDLE mode following a wake-up periodic TAU or
  *                       initiate a Mobile Origination event.
  *   T3412 Timer: this is the value the device informs the network (with periodic TAU) that it is still registered.
  *                T3412 resets after Mobile Origination events.
  *
  *   Recommanded ratio of T3324/T3412 should be > 90%
  *   T3324: no recommended value but minimum value = 16 sec
  *   T3412: minimum recommended value = 4 hours, maximum value = 413 days
  *
  * eDRX STANDALONE
  * ===============
  * eDRX can be used without PSM or in conjunction with PSM.
  * During DRX, the device switch off network reception for a brief moment.
  * eDRX allows the time interval during which a device is not listening to the network to be greatly extended.
  * Mobile Origination events can be triggered at any time.
  *
  *  Recommanded LTE-M values: minimum of 5.12 sec, maximum of 43.69 min
  *
  * PSM and eDRX COMBINED IMPLEMENTATION
  * ====================================
  * Combinent implementation is recommended.
  * Careful alignment is needed between different configuration parameters.
  *
  */


typedef uint8_t CS_PSM_mode_t;
#define PSM_MODE_DISABLE       (CS_PSM_mode_t)(0x0U)
#define PSM_MODE_ENABLE        (CS_PSM_mode_t)(0x1U)

typedef uint8_t CS_EDRX_mode_t;
#define EDRX_MODE_DISABLE            (CS_EDRX_mode_t)(0x0)
#define EDRX_MODE_ENABLE             (CS_EDRX_mode_t)(0x1)
#define EDRX_MODE_ENABLE_WITH_URC    (CS_EDRX_mode_t)(0x2)
#define EDRX_MODE_DISABLE_AND_RESET  (CS_EDRX_mode_t)(0x3)

typedef uint8_t CS_EDRX_AcT_type_t;
#define EDRX_ACT_NOT_USED          (CS_EDRX_AcT_type_t)(0x0) /* */
#define EDRX_ACT_EC_GSM_IOT        (CS_EDRX_AcT_type_t)(0x1) /* */
#define EDRX_ACT_GSM               (CS_EDRX_AcT_type_t)(0x2) /* */
#define EDRX_ACT_UTRAN             (CS_EDRX_AcT_type_t)(0x3) /* */
#define EDRX_ACT_E_UTRAN_WB_S1     (CS_EDRX_AcT_type_t)(0x4) /* = LTE or LTE Cat.M1 */
#define EDRX_ACT_E_UTRAN_NB_S1     (CS_EDRX_AcT_type_t)(0x5) /* = LTE Cat.NB1 */

/* Predefined PSM T3312 values (GERAN/UTRAN) */
#define PSM_T3312_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */

/* Predefined PSM T3314 values (GERAN/UTRAN) */
#define PSM_T3314_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */

/* Predefined PSM T3412 values (E-UTRAN) */
#define PSM_T3412_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */
#define PSM_T3412_1_MIN       (uint8_t)(0xA1) /* "101.00001" = 0xA1 - for test only */
#define PSM_T3412_4_HOURS     (uint8_t)(0x24) /* "001.00100" = 0x24 */
#define PSM_T3412_24_HOURS    (uint8_t)(0x38) /* "001.11000" = 0x38 */
#define PSM_T3412_170_HOURS   (uint8_t)(0x51) /* "010.10001" = 0x51 */
#define PSM_T3412_40_DAYS     (uint8_t)(0xD3) /* "110.00011" = 0xD3 */

/* Predefined T3324 values, PSM for active timer (GERAN/UTRAN or E-UTRAN)*/
#define PSM_T3324_DEACTIVATED (uint8_t)(0xE0) /* "111 00000" = 0xE0 */
#define PSM_T3324_10_SEC      (uint8_t)(0x05) /* "000.00101" = 0x05  - for test only */
#define PSM_T3324_16_SEC      (uint8_t)(0x08) /* "000.01000" = 0x08 */
#define PSM_T3324_24_MIN      (uint8_t)(0x38) /* "001.11000" = 0x38 */
#define PSM_T3324_10_HOURS    (uint8_t)(0x41) /* "010.00001" = 0x41 */
#define PSM_T3324_30_HOURS    (uint8_t)(0x43) /* "010.00011" = 0x43 */
#define PSM_T3324_90_HOURS    (uint8_t)(0x49) /* "010.01001" = 0x49 */

/* Predefined EDRX values */
#define EDRX_WB_S1_PTW_1S_DRX_40S  (uint8_t)(0x03) /* "0000.0011" = 0x49 : WB-S1 mode PTW=1.28 sec, EDRX=40.96 sec */

typedef struct
{
  uint8_t             req_periodic_RAU;     /* GERAN/UTRAN networks, (T3312), cf Table 10.5.163a from TS 24.008 */
  uint8_t             req_GPRS_READY_timer; /* GERAN/UTRAN networks, T3314), cf Table 10.5.172 from TS 24.008 */
  uint8_t             req_periodic_TAU;     /* E-UTRAN networks, (T3412), cf Table 10.5.163a from TS 24.008 */
  uint8_t             req_active_time;      /* GERAN/UTRAN and E-UTRAN networks, (T3324),
                                             *    cf Table 10.5.163 from TS 24.008 */
} CS_PSM_params_t;

typedef struct
{
  CS_EDRX_AcT_type_t  act_type;
  uint8_t             req_value;
} CS_EDRX_params_t;

typedef struct
{
  CS_Bool_t  low_power_enable;    /* if false: ignore psm and edrx structures below
                                   * if true:  take into account psm and edrx structures
                                   */
  /* PSM and EDRX structures */
  CS_PSM_params_t  psm;
  CS_EDRX_params_t edrx;

} CS_init_power_config_t;

typedef struct
{
  CS_Bool_t  psm_present;              /* indicates if PSM parameters below are present */
  CS_Bool_t  edrx_present;             /* indicates if eDRX parameters below are present */

  /* PSM and EDRX structures */
  CS_PSM_mode_t    psm_mode;           /* requested PSM mode */
  CS_PSM_params_t  psm;
  CS_EDRX_mode_t   edrx_mode;          /* requested eDRX mode */
  CS_EDRX_params_t edrx;

} CS_set_power_config_t;

CS_Status_t CS_InitPowerConfig(CS_init_power_config_t *p_power_config);
CS_Status_t CS_SetPowerConfig(CS_set_power_config_t *p_power_config);
CS_Status_t CS_SleepRequest(void);
CS_Status_t CS_SleepComplete(void);
CS_Status_t CS_SleepCancel(void);
CS_Status_t CS_PowerWakeup(void);

/* API functions not implemented yet  */
CS_Status_t CDS_socket_get_option(void);
CS_Status_t CDS_socket_listen(socket_handle_t sockHandle); /* for socket server mode, parameters to clarify */

/* API functions not available yet
  * CS_Status_t CS_modem_config(CS_ModemConfig_t *p_config);
  *   => to configure modem persistant parameters
  * CS_Status_t CS_dns_config(CS_PDN_conf_id_t cid, CS_DnsConf_t *dns_conf);
  *   => to offer the possibilty to user to configure the DNS primary address.
  *      Actually, it is hard-coded in this file.
  *      Will need to update CS_DnsReq_t to remove primary_dns_addr parameter.
  * CS_Status_t CS_autoactivate_pdn(void);
  */

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_SERVICE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

