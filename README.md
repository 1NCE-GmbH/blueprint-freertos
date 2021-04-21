# amazon_freertos_cellular

 The Internet of Things (IoT) application has attracted a lot of attention due to its powerfulness to build a digital cyber world with meaningful information.Mainly, IoT’s devices are characterized by an (i)small size, (ii) short memory, and (iii)low consumption energy.  Towards this end, reduced-memory and efficient embedded real-time operating systems (RTOS) are required to process data acquired by such devices. Primarily, RTOS is designed to support IoT devices in diverse applications andoperational requirements on time.
FreeRTOS is known as one of the most utilized RTOS in practice. This is becauseit supports numerous processor architectures and its possession by Amazon has driven increased investment in engineering. Nonetheless, FreeRTOS connectivity is limited to Wi-Fi, Ethernet, or Amazon Web Services (AWS)-IoT Core via a mobile device using Bluetooth Low Energy connectivity. This rends the communication’s coverage very short, especially when connecting to the application and network remotely is needed. Motivating by the above, we aim to add new functionality in FreeRTOS AWS allow-ing to support a cellular Service in module Quectel BG96 (e.g. Narrowband (NB)-IoT,2G) able to connect the device to the internet. 
# User Guide: 
#### Required Hardware: 
To install our project you will need:
  - B-L475E-IOT01A2 STM32 Discovery kit IoT node connected to BG96 (LTE Cat M1/Cat NB1/EGPRS modem) through X-NUCLEO-STMODA1 expansion board.
<p align="center"><img src="/images/material.png" width="70%"><br>
Figure 1. Material Used</p>
#### Required Software:
   - STM32CubeIDE from https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-ides/stm32cubeide.html
  - You will need to register/ login to access downloads.

  - STM32 ST-LINK utility from https://www.st.com/en/development-tools/stsw-link004.html

  - Clone this repository
   
####  Prerequisites:
  - Upgrade the modem BG96 to the latest firmware. (https://github.com/1NCE-GmbH/blueprint-freertos/tree/master/Utilities/Modem_FW)
   **Note:**  Download the modem FW flasher tool (QFlash) from this url:  https://github.com/1NCE-GmbH/blueprint-freertos/tree/master/Utilities/Modem_FW this tools taked from quectel from the web site listed in the official documentation.

#### STM32 Setup

  - Import the project.
<p align="center"><img src="/images/import1.png" width="70%"><br>
Screenshot 1. Import project</p> 
  - Choose the Existing project into Workspace and click Next.
 <p align="center"><img src="/images/impot2.png" width="70%"><br>
Screenshot 2. Help</p>
  - Click browse and select the folder FreeRTOS cellular, a project with the title “aws_demos“ should be listed, then click Finish.
<p align="center"><img src="/images/import3.png" width="70%"><br>
Screenshot 3. Help2</p>
  - Go to config_files/aws_demo_config.h --> choose the Demo you want to unable (MQTT Example)
  
  ```
//  #define CONFIG_TROUBLESHOOTING_DEMO_ENABLED
//#define CONFIG_UDP_DEMO_ENABLED
//#define CONFIG_COAP_DEMO_ENABLED
#define CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED
  ```

  -  Edit the following parameter in config_files/nce_demo_config.h : 

```c
/* MQTT Configuration */ 
#define ONBOARDING_ENDPOINT  "device.connectivity-suite.cloud"
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
#define SUB_TOPIC "/1nce_test"
#define clientcredentialMQTT_BROKER_PORT 8883
#define democonfigRANGE_SIZE 500 //(you can add the range request or comment this line )
```

  -  debug the code.

  <p align="center"><img src="/images/debug1.png" width="70%"><br>
Screenshot 4. Debug</p>

  - Open a console on MPU Serial Device.

#### Onboarding Service : 
Our goal is to automate device onboarding and make the connection more simple, easy and fast to establish within Amazon FreeRTOS.

 To automatize the process of establishing a connection with the AWS cloud we need to: 

  -  Get the certificate from 1NCE API. 

  -  Prepare the certificate. 

  -  Send the certificate to the modem. 
 

 ##### Get the certificate from 1NCE API: 
We send an HTTPS request to our API to get the response : 

```c
char send_packet[] = "GET /device-api/onboarding HTTP/1.1\r\n"
          "Host: device.connectivity-suite.cloud\r\n"
          "Accept: text/csv\r\n\r\n";
int32_t SendVal = SOCKETS_Send(cert_socket, &send_packet,strlen(send_packet), NULL);
```
In the BG96 we receive the response splitted in partitions with size of 1500 bytes (in case you did not precise the Range in nce_demo_config)  :
```c
rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
    (int32_t) sizeof(PART),
    NULL);
```
##### Prepare the certificate: 

For the preparation of the certificate, we need to merge all the response in one variable because the response comes in multiple partitions. 

for the memory optimisation, we use just one variable when we receive the response we add to the variable.

##### Separate the certificate based on “,“.

```c
char *token = strtok(all, ",");
// in the token we have the first element which is the iccid
while (token != NULL) {

    token = strtok(NULL, ",");
    //we have the second element
}
```
Replace \n in response with the breakline. (before send it to the modem)

```c
memcpy(PART, str_replace(PART, rep, w), strlen(PART));
```
##### Upload the certificate to the modem: 

```c
char root_cmd[30] = "AT+QFUPL=\"root.pem\",";
sprintf(root_cmd, "%s%d,60", root_cmd, root_size);
root_cmd_size = strlen(root_cmd);
cs_status = osCDS_direct_cmd(&crt_command_root,CST_cellular_direct_cmd_callback);
retval = sendToIPC(0, (uint8_t*) &PART[0], root_size);
```
#### SSL Configuration:
open a Secure Socket for BG96 we need to send some AT commands : 

In order to set up the MQTT client to use a secure connection.

  -  AT+QSSLCFG= "clientkey",<sslctxID>,<client_key_path>

  -  AT+QSSLCFG= "seclevel",<sslctxID>,<seclevel> :Configure the authentication mode for the specified SSL context.

  -  AT+QSSLCFG="sslversion",,<sslctxID>,<sslversion> :Configure the SSL version for the specified SSL context

  -  AT+QSSLCFG="ciphersuite",<sslctxID>,<ciphersuites> :Configure the SSL cipher suites for the specified SSL context.

  -  AT+QSSLCLOSE=<sslctxID> to make sure the socket which we need to open is close.

#### MQTT configuration: 
To facilitate the understanding of our solution we try to give you some scenario.

<p align="center"><img src="/images/mqtt.png" width="70%"><br>
Figure 2. Mqtt Scenario</p>

## UDP connection with our SDK:

1NCE’s AWS FreeRTOS Version allows customers to communicate with the IoT core with MQTT (by default ) or UDP and use of all features as part of the 1NCE IoT Connectivity Suite.

#### Configuration:
1.   - Go to config_files/aws_demo_config.h --> choose the Demo you want to unable (UDP Example)
  ```
//  #define CONFIG_TROUBLESHOOTING_DEMO_ENABLED
#define CONFIG_UDP_DEMO_ENABLED
//#define CONFIG_COAP_DEMO_ENABLED
//#define CONFIG_CORE_MQTT_MUTUAL_AUTH_DEMO_ENABLED
  ```
2. The onboarding script configuration can be found in blueprint-freertos\vendors\st\boards\stm32l475_discovery\aws_demos\config_files\nce_demo_config.h in the root folder of the blueprint or /aws_demos/config_files/nce_demo_config.h in IDE.

#### UDP Configuration:
```
/* UDP Configuration */ 
//URL used to perform the onboarding call
#define UDP_ENDPOINT "udp.connectivity-suite.cloud"
//port to connect to the UDP endpoint.
#define UDP_PORT 4445
//the message you want to publish in IoT Core
#define PUBLISH_PAYLOAD_FORMAT                   "Welcome to 1NCE's Solution"
```


#### Example:
The bluePrint consists of an example to publish a message in IoT Core with UDP.
In IoT Core we subscribe in <ICCID>/messages and we run our solution : 

<p align="center"><img src="/images/udp.png" width="70%"><br>
Figure 3. UDP Demo</p>

### CoAP connection with our SDK: 
#### COAP POST request sending over UDP:
In this Section our SDK following steps are executed:
* Register to the Network. 
* Perform a DNS Resolution.
* Create a socket and connect to Server
* Create Confirmable CoAP POST with Quiry option
* Create Client Interaction and analyse the response (ACK)
* Valid the response.

##### User Guide:

The code by default work with MQTT if you want to use our CoAP Solution you need to add : 
/aws_demos/config_files/demo_config.h
```c
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5683
#define configCOAP_URI_PATH    "t=<quiry_path>"
#define PUBLISH_PAYLOAD_FORMAT                   "your text"
```

#### Example:

<p align="center"><img src="/images/coap.png" width="70%"><br>
Figure 4. COAP Demo</p>

### COAP with DTLS Support: 

For the DTLS Support the default Port is 5684 and define the DTLS_Demo as aditional define 
```c
#define ENABLE_DTLS
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5684
#define configCOAP_URI_PATH    "t=<quiry_path>"
#define PUBLISH_PAYLOAD_FORMAT                   "your text"
```
The CoAP DTLS performs 3 main tasks :
* Send the Onboarding Request
* Get the Response
* Process the Response and give the DTLS identity and PSK to the application code.

<p align="center"><img src="/images/DTLS.png" width="70%"><br>
Figure 5. DTLS Flow</p>

#### Example:

<p align="center"><img src="/images/DTLSdemo.png" width="70%"><br>
Figure 5. DTLS Demo</p>

### Troubleshoting Demo :
> This feature is limited to Users and Accounts who have already accepted our Connectivity Suite Addendum to the 1NCE T&Cs. It is a one-time action per 1NCE Customer Account. Please log into the 1NCE Customer Portal as Owner or Admin, navigate to the Connectivity Suite, and accept the Terms. If you don't see anything to accept and get directly to the Dashboard of the Connectivity Suite, you are ready to go!

> N.B: The SMS and Data Consumed for the Troubleshooting is counted against the Customers own Volume of the SIM Card.

This initial version's main target is to allow customers to connect their Things and automate the network debugging faster and more reliably. This will also reduce the workload on our Customer facing support teams and will also allow us to focus on further common issues.
1. Go to config_files/nce_demo_config.h --> enable the flag TROUBLESHOOTING (Troubleshooting Example with/without DTLS in primary Flow and SMS as alternative)
  ```
/* COAP Configuration with/without DTLS */
#define ENABLE_DTLS //if you want to use the DTLS support
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5684 // Prot with DTLS if you want to use Coap without DTLS use the port 5683
#define configCOAP_URI_QUERY    "t=test"

/* Enable send the Information to 1NCE's client support */
#define TROUBLESHOOTING
  ```
2. run the demo : the demo will send the information to coap endpoint as a firt step if No ACK come then an SMS to 1nce portal with required informations.

**** Primary case ****
<p align="center"><img src="/images/troubleshootingcoap.png" width="70%"><br>
Figure 6. Troubleshooting from the coap endpoint</p>

**** Fallback ****
<p align="center"><img src="/images/troubleshooting.png" width="70%"><br>
Figure 7. Troubleshooting from Portal</p>

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

#### Conclusion:

When connecting to AWS the setup process is tricky and the policies, rules and certificates need to be setup correctly in order to successfully connect to your thing we automatize all this thing for you to connect fast and performed safety and secure connection and support diffrent type of protocols (UDP/COAP with/without DTLS).
