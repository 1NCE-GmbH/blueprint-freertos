/*
 * nce_demo_config.h
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

/* ===================================================
 *                GENERAL CONFIGURATION
 * =================================================== */

/* Selected Demo, choose from:
 * CONFIG_UDP_DEMO_ENABLED
 * CONFIG_COAP_DEMO_ENABLED
 * CONFIG_LwM2M_DEMO_ENABLED
 */
#define CONFIG_COAP_DEMO_ENABLED


/* e.g. Band for Germany CATM1
 * https://www.quectel.com/download/quectel_bg96_at_commands_manual_v2-3/
 */
#define CUSTOM_BAND_BG96    "AT+QCFG=\"band\",F,80004,80"

/* Default Radio Access Technology (RAT) configuration.
 * Options:
 *   CELLULAR_RAT_GSM    = 0  -> GSM network.
 *   CELLULAR_RAT_CATM1  = 8  -> CAT M1 (LTE-M, default).
 *   CELLULAR_RAT_NBIOT  = 9  -> NB-IoT (default).
 *
 * NOTE: CAT M1 is set as the default for LTE-M communication.
 * Change these values to 0 or 9 if your region requires GSM or NB-IoT.
 */
#define CELLULAR_CONFIG_DEFAULT_RAT      8  /* Primary RAT: CAT M1. */
#define CELLULAR_CONFIG_DEFAULT_RAT_2    8  /* Secondary RAT: CAT M1. */
#define CELLULAR_CONFIG_DEFAULT_RAT_3    8  /* Tertiary RAT: CAT M1. */

/* Define this flag to enable secure communication:
 * (For CoAP/LwM2M)
 */
#define ENABLE_DTLS

#define PUBLISH_PAYLOAD_FORMAT    "Welcome to 1NCE's Solution"



/* C2D Parameters */
/* This port is used for both UDP and CoAP communication. */
#define NCE_RECV_PORT                    3000
#define NCE_RECEIVE_BUFFER_SIZE_BYTES    200


/* ===================================================
 *                DEMO CONFIGURATION
 * =================================================== */

/* UDP Configuration */
#if defined( CONFIG_UDP_DEMO_ENABLED )
    #define CONFIG_UDP_SERVER_ADDRESS                   "udp.os.1nce.com"
    #define CONFIG_UDP_SERVER_PORT                      4445
    #define CONFIG_NCE_ENERGY_SAVER
    #define CONFIG_UDP_DATA_UPLOAD_FREQUENCY_SECONDS    60

#endif

/* COAP Configuration with/without DTLS */
#if defined( CONFIG_COAP_DEMO_ENABLED )
    #define CONFIG_COAP_SERVER_ADDRESS                   "coap.os.1nce.com"
    #define CONFIG_COAP_URI_QUERY                        "t=test"
    #define CONFIG_COAP_DATA_UPLOAD_FREQUENCY_SECONDS    60
    #define CONFIG_NCE_ENERGY_SAVER
    #if defined( ENABLE_DTLS )
        #define CONFIG_COAP_SERVER_PORT                  5684
    #else
        #define CONFIG_COAP_SERVER_PORT                  5683
    #endif
#endif /* if defined( CONFIG_COAP_DEMO_ENABLED ) */

/* LWM2M Configuration */
#if defined( CONFIG_LwM2M_DEMO_ENABLED )
    #define LWM2M_ENDPOINT      "lwm2m.os.1nce.com"
    #define LWM2M_CLIENT_MODE
    #define LWM2M_BOOTSTRAP
    /* #define CONFIG_NCE_ICCID    "<ICCID>" */
    #ifndef CONFIG_NCE_ICCID
        #error "CONFIG_NCE_ICCID is not defined. Please define CONFIG_NCE_ICCID with a valid ICCID value."
    #endif
    #ifdef ENABLE_DTLS
        extern char lwm2m_psk[ 30 ];
        extern char lwm2m_psk_id[ 30 ];

        /**
         * @brief Configures the Pre-Shared Key (PSK) for LwM2M bootstrapping.
         * The PSK can be set during LwM2M integration testing via the Device Integrator or through the API.
         * For more details, refer to: https://help.1nce.com/dev-hub/reference/post_v1-integrate-devices-deviceid-presharedkey.
         */
        /* # define CONFIG_LWM2M_BOOTSTRAP_PSK    "<PSK>" */
        #ifndef CONFIG_LWM2M_BOOTSTRAP_PSK
            #error "CONFIG_LWM2M_BOOTSTRAP_PSK is not defined. Please define CONFIG_LWM2M_BOOTSTRAP_PSK with a valid value."
        #endif
    #endif /* ifdef ENABLE_DTLS */
    #define LWM2M_SUPPORT_SENML_JSON
    #define LWM2M_SUPPORT_JSON
    #define LWM2M_LITTLE_ENDIAN
    #define LWM2M_SUPPORT_TLV
    #define LWM2M_COAP_DEFAULT_BLOCK_SIZE          1024
    #define LWM2M_SINGLE_SERVER_REGISTERATION
    #define LWM2M_OBJECT_SEND                      "/3/0"
    #define CONFIG_LWM2M_SEND_FREQUENCY_SECONDS    60

#endif /* if defined( CONFIG_LwM2M_DEMO_ENABLED ) */
