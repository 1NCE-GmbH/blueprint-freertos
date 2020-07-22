/**
  ******************************************************************************
  * @file    cellular_datacache.h
  * @author  MCD Application Team
  * @brief   Data Cache definitions for cellular components
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
#ifndef CELLULAR_DATACACHE_H
#define CELLULAR_DATACACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "dc_common.h"
#include "com_sockets.h"
#include "cellular_service_int.h"

/* Exported constants --------------------------------------------------------*/
/** @addtogroup CELLULAR_MANAGEMENT_Constants
  * @{
  */

/** @defgroup CELLULAR_MANAGEMENT_Datacache_Constants Cellular Data Cache Constants
  * @{
  */

/* Data Cache String Sizes */
#define DC_MAX_SIZE_MNO_NAME       32U
#define DC_MAX_SIZE_IMEI           32U
#define DC_MAX_SIZE_MANUFACT_NAME  32U
#define DC_MAX_SIZE_MODEL          32U
#define DC_MAX_SIZE_REV            32U
#define DC_MAX_SIZE_SN             32U
#define DC_MAX_SIZE_ICCID          32U
#define DC_MAX_SIZE_APN            32U
#define DC_MAX_IP_ADDR_SIZE        MAX_IP_ADDR_SIZE
#define DC_MAX_SIZE_IMSI           32U
#define DC_CST_USERNAME_SIZE       20U
#define DC_CST_PASSWORD_SIZE       20U


#define DC_DEFAULT_ATTACHMENT_TIMEOUT_STRING  ((uint8_t*)"180000")   /* attachment timeout 3minutes */


/** @brief Number of NFMC tempo */
#define DC_NFMC_TEMPO_NB           7U

/** @brief Number max of SIM slot  */
#define DC_SIM_SLOT_NB             3U

/** @brief Value of signal level meaning 'no attached' */
#define DC_NO_ATTACHED             0U

/**
  * @}
  */

/** @defgroup CELLULAR_MANAGEMENT_Datacache_Deprecated_Use Deprecated Use
  * @{
  */
/** @brief Old value use DC_CELLULAR_INFO
    @deprecated better to use DC_CELLULAR_INFO */
#define DC_COM_CELLULAR_INFO          DC_CELLULAR_INFO
/** @brief Old value use DC_CELLULAR_INFO
    @deprecated better to use DC_CELLULAR_INFO */
#define DC_COM_CELLULAR               DC_CELLULAR_INFO

/** @brief Old value use DC_CELLULAR_DATA_INFO
    @deprecated better to use DC_CELLULAR_DATA_INFO */
#define DC_COM_CELLULAR_DATA_INFO     DC_CELLULAR_DATA_INFO
/** @brief Old value use DC_CELLULAR_DATA_INFO
    @deprecated better to use DC_CELLULAR_DATA_INFO */
#define DC_COM_CELLULAR_DATA          DC_CELLULAR_DATA_INFO

/** @brief Old value use DC_CELLULAR_NFMC_INFO
    @deprecated better to use DC_CELLULAR_NFMC_INFO */
#define DC_COM_NFMC_TEMPO             DC_CELLULAR_NFMC_INFO
/** @brief Old value use DC_CELLULAR_NFMC_INFO
    @deprecated better to use DC_CELLULAR_NFMC_INFO */
#define DC_COM_NFMC_TEMPO_INFO        DC_CELLULAR_NFMC_INFO

/** @brief Old value use DC_CELLULAR_SIM_INFO
    @deprecated better to use DC_CELLULAR_SIM_INFO */
#define DC_COM_SIM_INFO               DC_CELLULAR_SIM_INFO

/** @brief Old value use DC_CELLULAR_NIFMAN_INFO
    @deprecated better to use DC_CELLULAR_NIFMAN_INFO */
#define DC_COM_NIFMAN_INFO            DC_CELLULAR_NIFMAN_INFO

/** @brief Old value use DC_PPP_CLIENT_INFO
    @deprecated use DC_PPP_CLIENT_INFO */
#define DC_COM_PPP_CLIENT_INFO        DC_PPP_CLIENT_INFO
/** @brief Old value use DC_PPP_CLIENT_INFO
    @deprecated better to use DC_PPP_CLIENT_INFO */
#define DC_COM_PPP_CLIENT             DC_PPP_CLIENT_INFO

/** @brief Old value use DC_CELLULAR_CONFIG
    @deprecated better to use DC_CELLULAR_CONFIG */
#define DC_COM_CELLULAR_PARAM         DC_CELLULAR_CONFIG

/**
  * @}
  */

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @addtogroup CELLULAR_MANAGEMENT_Types
  * @{
  */

/** @defgroup CELLULAR_MANAGEMENT_Datacache_Types Cellular Data Cache Types
  * @{
  */

/** @brief Raw signal level value */
typedef uint8_t dc_cs_signal_level_t; /*!< range 0..99  0: DC_NO_ATTACHED
                                                        99: Not known or not detectable */

/** @brief list of SIM slot types */
typedef enum
{
  DC_SIM_SLOT_MODEM_SOCKET           = 0,   /*!< */
  DC_SIM_SLOT_MODEM_EMBEDDED_SIM     = 1,
  DC_SIM_SLOT_STM32_EMBEDDED_SIM     = 2
} dc_cs_sim_slot_type_t;


/** @brief eDRX act type */
typedef uint8_t dc_cellular_power_edrx_act_type_t;
#define DC_EDRX_ACT_NOT_USED             (dc_cellular_power_edrx_act_type_t)(0x0) /*!< Not used */
#define DC_EDRX_ACT_EC_GSM_IOT           (dc_cellular_power_edrx_act_type_t)(0x1) /*!< GSM IOT*/
#define DC_EDRX_ACT_GSM                  (dc_cellular_power_edrx_act_type_t)(0x2) /*!< GSM */
#define DC_EDRX_ACT_UTRAN                (dc_cellular_power_edrx_act_type_t)(0x3) /*!< UTRAN */
#define DC_EDRX_ACT_E_UTRAN_WB_S1        (dc_cellular_power_edrx_act_type_t)(0x4) /*!< = LTE */
#define DC_EDRX_ACT_E_UTRAN_NB_S1        (dc_cellular_power_edrx_act_type_t)(0x5) /*!< = LTE Cat.M1 or LTE Cat.NB1 */

/** @brief list of modem target state */
typedef enum
{
  DC_TARGET_STATE_OFF           = 0,  /*!< Modem off */
  DC_TARGET_STATE_SIM_ONLY      = 1,  /*!< SIM connected but no network management (not implemented)*/
  DC_TARGET_STATE_FULL          = 2,  /*!< Full modem features available (data transfer) */
  DC_TARGET_STATE_UNKNOWN       = 3   /*!< */
} dc_cs_target_state_t;

/** @brief list of modem states */
typedef enum
{
  DC_MODEM_STATE_OFF            = 0,     /*!< modem not started                 */
  DC_MODEM_STATE_POWERED_ON     = 1,     /*!< modem powered on                  */
  DC_MODEM_STATE_SIM_CONNECTED  = 2,     /*!< modem started with SIM connected  */
  DC_MODEM_STATE_DATA_OK        = 3,     /*!< modem started with data available */
} dc_cs_modem_state_t;

/** @brief list of SIM states */
typedef enum
{
  DC_SIM_OK                  = 0,
  DC_SIM_NOT_IMPLEMENTED     = 1,
  DC_SIM_BUSY                = 2,
  DC_SIM_NOT_INSERTED        = 3,
  DC_SIM_PIN_OR_PUK_LOCKED   = 4,
  DC_SIM_INCORRECT_PASSWORD  = 5,
  DC_SIM_ERROR               = 6,
  DC_SIM_NOT_USED            = 7,
  DC_SIM_CONNECTION_ON_GOING  = 8
} dc_cs_sim_status_t;

/** @brief list of Network modes */
typedef enum
{
  DC_NO_NETWORK              = 0,
  DC_CELLULAR_SOCKET_MODEM   = 1,
  DC_CELLULAR_SOCKETS_LWIP   = 2,
  DC_WIFI_NETWORK            = 3
} dc_nifman_network_t;

/** @brief IP address */
typedef com_ip_addr_t dc_network_addr_t;

#if (USE_LOW_POWER == 1)
/** @brief list of power states */
typedef enum
{
  DC_POWER_RUN_REAL_TIME        = 0,   /*!< Real time power      (STM32 RUN_LP      / eDRX disabled / PSM disabled) */
  DC_POWER_RUN_INTERACTIVE_0    = 1,   /*!< Interactive power    (STM32 RUN_LP      / eDRX disabled / PSM disabled) */
  DC_POWER_RUN_INTERACTIVE_1    = 2,   /*!< Interactive power    (STM32 RUN_LP      / eDRX disabled / PSM enabled)  */
  DC_POWER_RUN_INTERACTIVE_2    = 3,   /*!< Interactive power    (STM32 RUN_LP      / eDRX enabled  / PSM enabled)  */
  DC_POWER_RUN_INTERACTIVE_3    = 4,   /*!< Interactive power    (STM32 RUN_LP      / eDRX enabled  / PSM disabled) */
  DC_POWER_IDLE                 = 5,   /*!< Idle power           (STM32 STOP2       / eDRX disabled / PSM disabled) */
  DC_POWER_IDLE_LP              = 6,   /*!< Idle low power       (STM32 STOP2       / eDRX allowed  / PSM allowed)  */
  DC_POWER_LP                   = 7,   /*!< low power            (STM32 STOP2       / eDRX enabled  / PSM enabled)  */
  DC_POWER_ULP                  = 8,   /*!< Ultra Low power      (STM32 STBY-shdown / eDRX enabled  / PSM enabled)  */
  DC_POWER_STBY1                = 9,   /*!< STanby 1 Low power   (STM32 STBY w RTC  / Modem Off     / NETWORK Off)  */
  DC_POWER_STBY2                = 10,  /*!< STanby 2 Low power  (STM32 STBY w RTC   / Modem Off     / NETWORK Off)  */
  DC_POWER_OFF                  = 11   /*!< Modem OFF           (STM32 shutdown     / Modem Off     / NETWORK Off)  */
} dc_power_mode_t;


/** @brief list of power commands */
typedef enum
{
  DC_POWER_CMD_INIT         = 0,   /*!< First init of power parameters        */
  DC_POWER_CMD_SETTING      = 1    /*!< setting of power parameters           */
} dc_power_cmd_t;


/** @brief low power psm mode */
typedef enum
{
  DC_POWER_PSM_MODE_DISABLE     = 0,  /*!< Low power PSM mode disable         */
  DC_POWER_PSM_MODE_ENABLE      = 1,  /*!< Low power PSM mode enable          */
} dc_psm_mode_t;

/** @brief eDRX mode */
typedef enum
{
  DC_POWER_EDRX_MODE_DISABLE            = 0,
  DC_POWER_EDRX_MODE_ENABLE             = 1,
  DC_POWER_EDRX_MODE_ENABLE_WITH_URC    = 2,
  DC_POWER_EDRX_MODE_DISABLE_AND_RESET  = 3,
} dc_edrx_mode_t;
#endif  /* (USE_LOW_POWER == 1) */


/**
  * @}
  */

/**
  * @}
  */

/* Structures types ------------------------------------------------------------*/
/** @addtogroup CELLULAR_MANAGEMENT_Types
  * @{
  */

/** @defgroup CELLULAR_MANAGEMENT_Datacache_Structure_Entries Cellular Data Cache Structure Entries
  * @{
  */

/* =================================================== */
/* Structures definition of Data Cache entries - BEGIN */
/* =================================================== */

/**
  * @brief  Structure definition of DC_CELLULAR_INFO DC entry.
  * This DC entry contains the main cellular information received
  * from modem after the boot.
  */
typedef struct
{
  dc_service_rt_header_t header;
  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_UNAVAIL: service not initialized,
    *                       the field values of structure are not significant.
    * - DC_SERVICE_RUN: modem powered on and initialized,
    *                   the field values of structure are significant except MNO name.
    * - DC_SERVICE_ON: modem attached,
    *                  all the field values of structure are significant.
    * - Other state values not used.
    */
  dc_service_rt_state_t rt_state;

  dc_cs_modem_state_t  modem_state; /*!< current modem state */

  dc_cs_signal_level_t cs_signal_level; /*!< raw signal level range 0-99
                                             0  : Not attached
                                             99 : Not known or not detectable */
  int32_t     cs_signal_level_db;  /*!< signal level in dB */

  /* modem information */
  uint8_t imei[DC_MAX_SIZE_IMEI];
  uint8_t mno_name[DC_MAX_SIZE_MNO_NAME];
  uint8_t manufacturer_name[DC_MAX_SIZE_MANUFACT_NAME];
  uint8_t model[DC_MAX_SIZE_MODEL];
  uint8_t revision[DC_MAX_SIZE_REV];
  uint8_t serial_number[DC_MAX_SIZE_SN];
  uint8_t iccid[DC_MAX_SIZE_ICCID];
} dc_cellular_info_t;


/**
  * @brief  Structure definition of DC_CELLULAR_SIM_INFO entry.
  * This DC entry contains information of the available SIM.
  */
typedef struct
{
  dc_service_rt_header_t header;

  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_UNAVAIL: service not initialized,
    *                       the field values of structure are not significant.
    * - DC_SERVICE_ON: modem attached,
    *                  all the field values of structure are significant.
    * - Other state values not used.
    */
  dc_service_rt_state_t  rt_state;
  int8_t                 imsi[DC_MAX_SIZE_IMSI];
  uint8_t                index_slot;
  dc_cs_sim_slot_type_t  active_slot;
  dc_cs_sim_status_t     sim_status[DC_SIM_SLOT_NB];
} dc_sim_info_t;

/**
  * @brief  Structure definition of DC_CELLULAR_APN_CONFIG entry.
  * This DC entry allows to set a new APN configuration.
  * when this entry is set, the modem is reset and configured with this configuration
  * if flash configuration set-up is used, the flash configuration is updated in flash.
  */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t  rt_state;                       /*!< must be set to DC_SERVICE_ON
                                                              all the field values of structure are significant. */

  uint8_t                apn[DC_MAX_SIZE_APN];           /*!< APN (string) */
  uint8_t                cid;                            /*!< CID (1-9) */
  uint8_t                username[DC_CST_USERNAME_SIZE]; /*!< username: empty string => no username */
  uint8_t                password[DC_CST_PASSWORD_SIZE]; /*!< password (used only is username is defined) */
} dc_apn_config_t;

/**
  * @brief  Structure definition of DC_CELLULAR_DATA_INFO entry.
  * This DC entry contains the state of cellular data transfer feature.
  */
typedef struct
{
  dc_service_rt_header_t header;

  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_OFF          : data transfer not available
    * - DC_SERVICE_ON           : data transfer available
    * - DC_SERVICE_SHUTTING_DOWN: cellular issue - data transfer no more available
    * - Other state values not used.
    */
  dc_service_rt_state_t  rt_state;
} dc_cellular_data_info_t;

/**
  * @brief  Structure definition of DC_CELLULAR_TARGET_STATE_CMD entry.
  * This DC entry allows to request a new modem target state.
  */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t  rt_state;     /*!< must be set to DC_SERVICE_ON
                                            all the field values of structure are significant. */

  dc_cs_target_state_t   target_state; /*!< modem target state to reach */
} dc_cellular_target_state_t;

/**
  * @brief  Structure definition of DC_CELLULAR_NIFMAN_INFO entry.
  * This DC entry contains the Nifman informations.
  */
typedef struct
{
  dc_service_rt_header_t header;
  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_UNAVAIL      : service not initialized,
    *                             the field values of structure are not significant
    * - DC_SERVICE_OFF          : network not available
    * - DC_SERVICE_ON           : network available,
    *                             the other field values of structure are significant
    * - DC_SERVICE_FAIL         : network stack returns error. Network is not available
    * - DC_SERVICE_SHUTTING_DOWN: network shut down. Network is not available
    * - Other state values not used.
    */
  dc_service_rt_state_t  rt_state;

  dc_nifman_network_t    network; /*!< network type used */
  dc_network_addr_t      ip_addr; /*!< IP address */
} dc_nifman_info_t;

/**
  * @brief  Structure definition of DC_PPP_CLIENT_INFO entry.
  * This DC entry contains the PPP client informations.
  */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t  rt_state; /*!< To document */

  dc_network_addr_t      ip_addr;
  dc_network_addr_t      netmask;
  dc_network_addr_t      gw;
} dc_ppp_client_info_t;

/**
  * @brief  Structure definition of DC_CELLULAR_NFMC_INFO entry.
  * This DC entry contains the NFMC informations.
  */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t rt_state;   /*!< To document */

  uint32_t activate;                /*!< 0: feature not activated - 1: feature activated */
  uint32_t tempo[DC_NFMC_TEMPO_NB]; /*!< NFMC tempo values (significant only if activate==1) */
} dc_nfmc_info_t;


/**
  * @brief dc_sim_slot_t - SIM configuration structure.
  */
typedef struct
{
  dc_cs_sim_slot_type_t sim_slot_type;                  /*!< type of SIM slot */
  uint8_t               apn[DC_MAX_SIZE_APN];           /*!<  APN Value (string) */
  uint8_t               cid;                            /*!< CID value (1-9) */
  uint8_t               username[DC_CST_USERNAME_SIZE]; /*!< username: empty string => no username */
  uint8_t               password[DC_CST_PASSWORD_SIZE]; /*!< password (used only is username is defined) */
} dc_sim_slot_t;

/**
  * @brief  Structure definition of DC_CELLULAR_CONFIG entry.
  * This DC entry contains the cellular parameters used to configure the modem.
  * At boot time this entry is set by default value
  * or if it exists the cellular configuration set-up present in flash.
  * If the application needs to set its own modem configuration,
  * it must set the DC_CELLULAR_CONFIG Data Cache entry at boot time between the calls
  * to cellular_init() and cellular_start().
  */
typedef struct
{
  dc_service_rt_header_t header;

  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_UNAVAIL      : service not initialized,
    *                             the field values of structure are not significant
    * - DC_SERVICE_ON           : service started,
    *                             the other field values of structure are significant
    * - Other state values not used.
    */
  dc_service_rt_state_t  rt_state;

  /**
    *!<
    * set_pdn_mode: used for selecting the APN and CID definition modes.
    * - 0: Only the CID value is used and transmitted to the modem.
    *      This option can be used only if an APN/CID association has previously been
    *      stored in the modem.
    * - 1: The APN and CID defined (refer to APN and CID) are associated and sent to
    *      the modem.
    */
  uint8_t                set_pdn_mode;

  /*!<
    * sim_slot_nb: number of SIM slots used (max 2)
    * For each SIM slot used, set the associated parameters in the sim_slot table.
    */
  uint8_t                sim_slot_nb;

  /*!< sim_slot: table of SIM slot parameters */
  dc_sim_slot_t          sim_slot[DC_SIM_SLOT_NB];

  /*!<  target_state: target state for the modem at boot time. */
  dc_cs_target_state_t   target_state;

  /*!< maximum network attachment duration (default value 25000ms) */
  uint32_t               attachment_timeout;

  /*!<
    *  nfmc_active: this flag (0:inactive - 1: active)
    *  specifies if NFMC feature must be activated.
    *  If yes, the nfmc_value table is used.
    */
  uint8_t                nfmc_active;

  /*!<
    *   nfmc_value: table of NFMC values allowing the calculation of NFMC tempos.
    *   This field is used only if nfmc_active==1.
    */
  uint32_t               nfmc_value[DC_NFMC_TEMPO_NB];
} dc_cellular_params_t;



#if (USE_LOW_POWER == 1)
/**
  * @brief dc_cellular_power_psm_config_t - psm lower power configuration .
  */
typedef struct
{
  uint8_t             req_periodic_RAU;     /*!<  GERAN/UTRAN networks, (T3312), cf Table 10.5.163a from TS 24.008 */
  uint8_t             req_GPRS_READY_timer; /*!<  GERAN/UTRAN networks, T3314), cf Table 10.5.172 from TS 24.008 */
  uint8_t             req_periodic_TAU;     /*!<  E-UTRAN networks, (T3412), cf Table 10.5.163a from TS 24.008 */
  uint8_t             req_active_time;      /*!<  GERAN/UTRAN and E-UTRAN networks, (T3324), cf Table 10.5.163 from TS 24.008 */
} dc_cellular_power_psm_config_t;

/**
  * @brief dc_cellular_power_edrx_config_t - edrx lower power configuration .
  */
typedef struct
{
  dc_cellular_power_edrx_act_type_t  act_type;
  uint8_t             req_value;
} dc_cellular_power_edrx_config_t;

/**
  * @brief dc_cellular_power_config_t - low power configuration .
  */
typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t  rt_state;

  /** @brief rt_state: entry state.
    *!<
    * - DC_SERVICE_UNAVAIL: service not initialized,
    *                       the field values of structure are not significant.
    * - DC_SERVICE_ON: all the field values of structure are significant.
    */
  dc_power_cmd_t      power_cmd;  /*!< power command to apply (init or setting) */
  dc_power_mode_t     power_mode; /*!< target power mode */
  bool  psm_present;              /*!< indicates if PSM parameters below are present */
  bool  edrx_present;             /*!< indicates if eDRX parameters below are present */

  dc_psm_mode_t      psm_mode;               /* !< requested PSM mode  */
  uint32_t           sleep_request_timeout;  /* !< sleep request timeout  */
  dc_cellular_power_psm_config_t   psm;      /* !< psm config          */
  dc_edrx_mode_t     edrx_mode;              /* !< requested eDRX mode */
  dc_cellular_power_edrx_config_t  edrx;     /* !< eDRX config         */
} dc_cellular_power_config_t;
#endif  /* (USE_LOW_POWER == 1) */


/* ===================================================== */
/* Structures definition of Data Cache entries - END     */
/* ===================================================== */

/**
  * @}
  */

/**
  * @}
  */

/* External variables --------------------------------------------------------*/

/** @addtogroup CELLULAR_MANAGEMENT_Variables
  * @{
  */

/** @defgroup CELLULAR_MANAGEMENT_Datacache_Entries Cellular Data Cache Entries Identifiers
  * @{
  */
/* =============================================== */
/* List of Cellular Data Cache entries - BEGIN     */
/* =============================================== */
/** @brief contains informations returned by the modem */
extern dc_com_res_id_t    DC_CELLULAR_INFO;

/** @brief contains the state of data transfert feature */
extern dc_com_res_id_t    DC_CELLULAR_DATA_INFO;

/** @brief contains NIFMAN informations */
extern dc_com_res_id_t    DC_CELLULAR_NIFMAN_INFO;

/** @brief contains NFMC informations */
extern dc_com_res_id_t    DC_CELLULAR_NFMC_INFO;

/** @brief contains SIM slot informations */
extern dc_com_res_id_t    DC_CELLULAR_SIM_INFO;

/** @brief contains Cellular configuration parameters */
extern dc_com_res_id_t    DC_CELLULAR_CONFIG; /*<! see dc_cellular_params_t */

/** @brief new modem target state request */
extern dc_com_res_id_t    DC_CELLULAR_TARGET_STATE_CMD;

/** @brief new apn configuration request */
extern dc_com_res_id_t    DC_CELLULAR_APN_CONFIG;

/** @brief set power configuration */
extern dc_com_res_id_t    DC_CELLULAR_POWER_CONFIG;

#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
/** @brief contains PPP state (only available if USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */
extern dc_com_res_id_t    DC_PPP_CLIENT_INFO;
#endif  /* (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP) */

/* =============================================== */
/* List of Cellular Data Cache entries - END       */
/* =============================================== */

/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @addtogroup CELLULAR_MANAGEMENT_Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @addtogroup CELLULAR_MANAGEMENT_Functions
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* CELLULAR_DATACACHE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
