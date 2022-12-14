# 1NCE FreeRTOS BluePrint

1NCE FreeRTOS BluePrint demonstrates the usage of various IoT protocols inculuding CoAP, LwM2M, and UDP with cellular connectivity. In combination with 1NCE SDK for the integration of 1NCE OS tools.

## Overview

1NCE FreeRTOS BluePrint release integrates 1NCE SDK to benefit from different 1NCE OS tools including device Authentication and Energy Saver, with the addition of a static library for CoAP Protocol ([Lobaro CoAP](https://www.lobaro.com/portfolio/lobaro-coap/)) and LwM2M ([Wakaama](https://www.eclipse.org/wakaama/)).

This Repository present examples of simple code:

* CoAP protocol
* CoAPs protocol (with DTLS security using Pre-shared key)
* UDP Demo 
* LwM2M with Bootstrap
* LwM2M without Bootstrap

Additionally, All the Demos have the Energy Saver feature as a Flag that can be enabled to test this feature.


## Getting started
### Prerequisites
*  B-L475E-IOT01A2 STM32 Discovery kit IoT node connected to BG96 (LTE Cat M1/Cat NB1/EGPRS modem) through X-NUCLEO-STMODA1 expansion board.
*  [1NCE SIM Card.](https://shop.1nce.com/portal/shop/) 
*  STM32CubeIDE from https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-ides/stm32cubeide.html
* STM32 ST-LINK utility from https://www.st.com/en/development-tools/stsw-link004.html
* Upgrade the modem BG96 to the latest firmware. (https://github.com/1NCE-GmbH/blueprint-freertos/tree/master/Utilities/Modem_FW)
 **Note:**  Download the modem FW flasher tool (QFlash) from this url:  https://github.com/1NCE-GmbH/blueprint-freertos/tree/master/Utilities/Modem_FW this tools taked from quectel from the web site listed in the official documentation.


### Cloning the Repository
After navigating to your workspace Clone the repository using HTTPS:
```
$ git clone https://github.com/1NCE-GmbH/blueprint-freertos.git --recurse-submodules

```
Using SSH:
```
$ git clone git@github.com:1NCE-GmbH/blueprint-freertos.git --recurse-submodules
```
If you have downloaded the repo without using the --recurse-submodules argument, you need to run:
```
git submodule update --init --recursive
```

* Import the project in STM32Cube.
### Building Sample

Setup your demo want to use by going to config_files/aws_demo_config.h define one of three demos exist (by default `CONFIG_COAP_DEMO_ENABLED`)
```
CONFIG_COAP_DEMO_ENABLED
CONFIG_UDP_DEMO_ENABLED
CONFIG_LwM2M_DEMO_ENABLED
```
### Sample Demos

#### COAP Demo without DTLS 
1NCE FreeRTOS BluePrint allows customers to communicate with 1NCE endpoints via CoAP and use of all features as part of the 1NCE OS.

COAP POST request:
In this Section, the following steps are executed:

* Register to the Network.
* Perform a DNS Resolution.
* Create a socket and connect to Server
* Create Confirmable CoAP POST with Query option
* Create Client Interaction and analyze the response (ACK)
* Validate the response.


* Setup the Demo runner in file (config_files/aws_demo_config.h)
```
#define CONFIG_COAP_DEMO_ENABLED
```
* The onboarding script configuration can be found in blueprint-freertos\vendors\st\boards\stm32l475_discovery\aws_demos\config_files\nce_demo_config.h in the root folder of the blueprint or /aws_demos/config_files/nce_demo_config.h in IDE.
```
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define democonfigCLIENT_ICCID "<ICCID>"
#define COAP_ENDPOINT           "coap.os.1nce.com"
#define configCOAP_PORT         5683
#define democonfigCLIENT_IDENTIFIER    "t=test"
#if ( configCOAP_PORT == 5684 )
  #define ENABLE_DTLS
#endif
/* Enable send the Information to 1NCE's client support */
#if  defined( TROUBLESHOOTING ) && ( configCOAP_PORT == 5684 )
    #ifndef ENABLE_DTLS
        #define ENABLE_DTLS
    #endif
#endif
```
#### CoAPs with DTLS 

For the DTLS Support the default Port is 5684 and automatically defines the `ENABLE_DTLS` as an additional define

The CoAP DTLS performs 3 main tasks from the [1NCE IoT C SDK](https://github.com/1NCE-GmbH/1nce-iot-c-sdk) :

* Send the Device Authenticator Request
* Get the Response
* Process the Response and give the DTLS identity and PSK to the application code.

#### UDP Demo

1NCE FreeRTOS BluePrint allows customers to communicate with 1NCE endpoints via UDP and use all features as part of the 1NCE OS.
* Setup the Demo runner in file (config_files/aws_demo_config.h)
```
#define CONFIG_UDP_DEMO_ENABLED
```
* The onboarding script configuration can be found in blueprint-freertos\vendors\st\boards\stm32l475_discovery\aws_demos\config_files\nce_demo_config.h in the root folder of the blueprint or /aws_demos/config_files/nce_demo_config.h in IDE.
```
#define UDP_ENDPOINT "udp.os.1nce.com"
#define UDP_PORT 4445
#define CONFIG_NCE_ENERGY_SAVER
//the message you want to publish in IoT Core
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define democonfigCLIENT_ICCID "<ICCID>"
```

#### LwM2M Demo 
The LWM2M support is provided using Eclipse Wakaama library communicating with a Leshan LWM2M server
* Setup the Demo runner in file (config_files/aws_demo_config.h)
```
#define CONFIG_LwM2M_DEMO_ENABLED
```
* The client that has registered on the LwM2M server, can send data by doing the Send operation. More specifically, it is used by the Client to report values for Resources and Resource Instances of known LwM2M Object Instance(s) to the LwM2M Server.
To use this feature in our Blueprint: remove/ comment #define LWM2M_PASSIVE_REPORTING and define sending object (e.g. /4/0 here). The LWM2M endpoint and the client name can be configured in config_files/nce_demo_config.h as follows:
```
#define LWM2M_ENDPOINT    "lwm2m.os.1nce.com"
#define ENABLE_DTLS
#define LWM2M_CLIENT_MODE
#define LWM2M_BOOTSTRAP
#ifdef ENABLE_DTLS
char lwm2m_psk[30];
char lwm2m_psk_id[30];
#endif
#define LWM2M_SUPPORT_SENML_JSON
#define LWM2M_LITTLE_ENDIAN
#define LWM2M_SUPPORT_TLV
#define LWM2M_COAP_DEFAULT_BLOCK_SIZE 1024
#define LWM2M_VERSION_1_1
#define LWM2M_SINGLE_SERVER_REGISTERATION
//#define LWM2M_PASSIVE_REPORTING
#if defined(LWM2M_PASSIVE_REPORTING)
#define LWM2M_1NCE_LIFETIME   30000
#else
  #define LWM2M_OBJECT_SEND "/4/0"
#endif
```
### Troubleshooting Demo:

> This feature is limited to Users and Accounts who have already accepted our 1NCEOS Addendum to the 1NCE T&Cs. It is a one-time action per 1NCE Customer Account. Please log into the 1NCE Customer Portal as Owner or Admin, navigate to the 1NCEOS, and accept the Terms. If you don't see anything to accept and get directly to the Dashboard of the 1NCEOS, you are ready to go!

> N.B: The SMS and Data Consumed for the Troubleshooting are counted against the Customers own Volume of the SIM Card.

This initial version's main target is to allow customers to connect their Things and automate the network debugging faster and more reliably. This will also reduce the workload on our Customer facing support teams and will also allow us to focus on further common issues.
* Go to config_files/nce_demo_config.h --> enable the flag TROUBLESHOOTING (Troubleshooting Example with/without DTLS in primary Flow and SMS as an alternative)
```
#define COAP_ENDPOINT           "coap.os.1nce.com"
#define configCOAP_PORT         5684
#define democonfigCLIENT_IDENTIFIER    "t=test"
#if ( configCOAP_PORT == 5684 )
  #define ENABLE_DTLS
#endif
/* Enable send the Information to 1NCE's client support */
#define TROUBLESHOOTING
#if  defined( TROUBLESHOOTING ) && ( configCOAP_PORT == 5684 )
    #ifndef ENABLE_DTLS
        #define ENABLE_DTLS
    #endif
#endif
```
* run the demo : the demo will send the information to the coap endpoint as a first step if No ACK comes then an SMS to 1NCE portal with the required pieces of information.

**** Primary case ****
<p align="center"><img src="/images/troubleshootingcoap.png" width="70%"><br>
Figure 16. Troubleshooting from the coap endpoint</p>

**** Fallback ****
<p align="center"><img src="/images/troubleshooting.png" width="70%"><br>
Figure 17. Troubleshooting from Portal</p>

for more information on the troubleshooting
| Parameter   |      description      |
|:------------------------:|:---------------------|
| <RAT> Radio Access Technology |  - GSM <br> - LTE <br> - CATM1 <br> - NBIOT <br> - Otherwise: NULL |
| Public Land Mobile Network (PLMN) information |     - <mcc> Mobile Country Code <br> - <mnc> Mobile Network Code <br>   |
| Registred Network (RN) |- <id CellId> Registered network operator cell Id. <br> - <LAC> Registered network operator Location Area Code. <br> - <RAC> Registered network operator Routing Area Code. <br> <TAC> Registered network operator Tracking Area Code.|
|Reject CS ((Circuit Switched) registration status)| - <Status>: Table 2. <br> - <type>: 0: 3GPP specific Reject Cause. Manufacture specific. <br> <cause>: Circuit Switch Reject cause.|
|Reject PS ((Packet Switched) registration status)| - <Status>: Table 2. <br> - <type>: 0: 3GPP specific Reject Cause. Manufacture specific. <br> <cause>: Circuit Switch Reject cause.|
|Signal Information|- <RSSI> Received Signal Strength Indicator (RSSI) in dBm. <br> - <RSRP> LTE Reference Signal Received Power (RSRP) in dBm <br> - <RSRQ> LTE Reference Signal Received Quality (RSRQ) in dB. <br> - <SINR> LTE Signal to Interference-Noise Ratio in dB. <br> - <BER> Bit Error Rate (BER) in 0.01%. <br> -<BARS> A number between 0 to 5 (both inclusive) indicating signal strength.|
<p>Table 1. Key Feature of Troubleshooting Message </p>

| Number   |      description      |
|:--------:|:----------------------|
|0|CELLULAR NETWORK REGISTRATION STATUS NOT REGISTERED NOT SEARCHING|
|1|CELLULAR NETWORK REGISTRATION STATUS REGISTERED HOME|
|2|CELLULAR NETWORK REGISTRATION STATUS NOT REGISTERED SEARCHING|
|3|CELLULAR NETWORK REGISTRATION STATUS REGISTRATION DENIED|
|4|CELLULAR NETWORK REGISTRATION STATUS UNKNOWN|
|5|CELLULAR NETWORK REGISTRATION STATUS REGISTERED ROAMING|
|6|CELLULAR NETWORK REGISTRATION STATUS REGISTERED HOME SMS ONLY|
|7|CELLULAR NETWORK REGISTRATION STATUS REGISTERED ROAMING SMS ONLY|
|8|CELLULAR NETWORK REGISTRATION STATUS ATTACHED EMERG SERVICES ONLY|
|9|CELLULAR NETWORK REGISTRATION STATUS MAX|
<p>Table 2. Network Registration Status </p>

## Asking for Help

You can ask for help on our email [embedded@1nce.com](mailto:embedded@1nce.com). Please send bug reports and feature requests to GitHub.


