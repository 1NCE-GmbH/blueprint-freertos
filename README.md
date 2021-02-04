# amazon_freertos_cellular

 The Internet of Things (IoT) application has attracted a lot of attention due to its powerfulness to build a digital cyber world with meaningful information.Mainly, IoT’s devices are characterized by an (i)small size, (ii) short memory, and (iii)low consumption energy.  Towards this end, reduced-memory and efficient embedded real-time operating systems (RTOS) are required to process data acquired by such devices. Primarily, RTOS is designed to support IoT devices in diverse applications andoperational requirements on time.
FreeRTOS is known as one of the most utilized RTOS in practice. This is becauseit supports numerous processor architectures and its possession by Amazon has driven increased investment in engineering. Nonetheless, FreeRTOS connectivity is limited to Wi-Fi, Ethernet, or Amazon Web Services (AWS)-IoT Core via a mobile device using Bluetooth Low Energy connectivity. This rends the communication’s coverage very short, especially when connecting to the application and network remotely is needed. Motivating by the above, we aim to add new functionality in FreeRTOS AWS allow-ing to support a cellular Service in module Quectel BG96 (e.g. Narrowband (NB)-IoT,2G) able to connect the device to the internet. 
# User Guide: 
#### Required Hardware: 
To install our project you will need:
  - B-L475E-IOT01A2 STM32 Discovery kit IoT node connected to BG96 (LTE Cat M1/Cat NB1/EGPRS modem) through X-NUCLEO-STMODA1 expansion board.
 ![Required Hardware](/images/material.png) 
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
   ![imoport project](/images/import1.png) 
  - Choose the Existing project into Workspace and click Next.
  ![imoport project](/images/import2.png)
  - Click browse and select the folder FreeRTOS cellular, a project with the title “aws_demos“ should be listed, then click Finish.
    ![imoport project](/images/import3.png)
  -  Edit the following parameter in config_files/demo_config.h : 

     -  clientcredentialMQTT_BROKER_ENDPOINT[] = "url of end point";

     -  PUBLISH_PAYLOAD_FORMAT "your message"

     -  clientcredentialMQTT_BROKER_PORT “MQTT Port”
     -  SUB_TOPIC "name of your topic iccid/sub_topic"
  -  debug the code. 
  ![debug project](/images/debug1.png)
  - Open a console on MPU Serial Device.
# Thecnical Guide :
#### System overview:
 ![Architecture](/images/architecture.png)
  -  **Cellular library** with SSL offload uses the certificates stored in the modem file system to establish a secure connection between the board and AWS Cloud.

  - **Secure socket** library is a standard from Amazon FreeRTOS and only supports TCP.

  -  **COM_Socket Library** provides a collection of socket functions to open, configure, and send or receive the application to remote TCP or UDP applications.
#### Secure Socket:
  -  SOCKETS_Close: Closes the socket and frees the related resources. A socket cannot be used after it has been closed.

  -  SOCKETS_Shutdown: Closes all or part of a full-duplex connection on the socket. Disable reads and writes on a connected TCP socket.

  -  SOCKETS_Send: Transmit data to the remote socket.

  -  SOCKETS_Recv: Receive data from a TCP socket.

  -  SOCKETS_Connect: Connects the socket to the specified IP address and port.

  -  SOCKETS_Socket: Creates a TCP socket.

  -  SOCKETS_Init: To initialize the Secure Sockets library.

  -  SOCKETS_SetSockOpt: Manipulates the options for the socket.

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
In the BG96 we receive the response splitted in partitions with size of 1500 bytes  :
```c
rec = SOCKETS_Recv(cert_socket, (com_char_t*) &PART[0],
    (int32_t) sizeof(PART),
    COM_MSG_WAIT);
```
##### Prepare the certificate: 

for the preparation the certificate, we need to merge all the response in one variable because the response comes in multiple partitions. 

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
 ![mqtt scenario](/images/mqtt.png)

#### Resources needed :
| RAM           | Flash Memory  | 
| ------------- |:-------------:|
| 7.1 KB  ( 2.1 KB Static + 5 KB Dynamic)      | 44.7 KB (14.5 KB SDK + 30.2 KB  Secure Socket) |

**Note:** The code is based on SSL offloading so the SSL related functionality is done on the BG96 modem,  without this option the RAM requirements will increase by around 32 KB. 


#### Dependencies :
The blueprint uses the following : 

* Amazon FreeRTOS MQTT Library : integrated using the Secure socket layer

* STMicroelectronics BG96 Cellular Driver: customized to support modem file transfer functionality and SSL offloading. 

## UDP connection with our SDK:

1NCE’s AWS FreeRTOS Version allows customers to communicate with the IoT core with MQTT (by default ) or UDP and use of all features as part of the 1NCE IoT Connectivity Suite.

#### Configuration:
The onboarding script configuration can be found in blueprint-freertos\vendors\st\boards\stm32l475_discovery\aws_demos\config_files\demo_config.h in the root folder of the blueprint or /aws_demos/config_files/demo_config.h in IDE.

#### UDP Configuration:

UDP_ENDPOINT: URL used to perform the onboarding call

UDP_PORT: port to connect to the UDP endpoint. 

IoT Core Configuration: 

PUBLISH_PAYLOAD_FORMAT: the message you want to publish I'm IoT Core

Active/Deactivate UDP:

 #define USE_UDP our SDK work by Default with MQTT if you want to use it with UDP communication you need to define USE_UDP.


#### Example:
The bluePrint consists of an example to publish a message in IoT Core with UDP.



#### Demo: 
In IoT Core we subscribe in <ICCID>/messages and we run our solution : 
![UDP Demo](/images/udp.png)
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
#define USE_UDP
#define COAP
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5683
#define configCOAP_URI_PATH    "t=<quiry_path>"
#define PUBLISH_PAYLOAD_FORMAT                   "your text"
```
Example: 
![demo_config.h](/images/CoAP1.PNG)


The Message show in AWS :
 
![aws after Sending a message](/images/CoAP2.PNG)
### COAP with DTLS Support: 

For the DTLS Support the default Port is 5684 and define the DTLS_Demo as aditional define 
```c
#define USE_UDP
#define COAP
#define DTLS_Demo
#define COAP_ENDPOINT "coap.connectivity-suite.cloud"
#define configCOAP_PORT    5684
#define configCOAP_URI_PATH    "t=<quiry_path>"
#define PUBLISH_PAYLOAD_FORMAT                   "your text"
```
The CoAP DTLS performs 3 main tasks :
* Send the Onboarding Request
* Get the Response
* Process the Response and give the DTLS identity and PSK to the application code.
![DTLS Flow](/images/DTLS.png)
#### Conclusion:
When connecting to AWS the setup process is tricky and the policies, rules and certificates need to be setup correctly in order to successfully connect to your thing we automatize all this thing for you to connect fast and performed safety and secure connection.
