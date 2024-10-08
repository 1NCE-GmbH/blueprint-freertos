# 1NCE FreeRTOS BluePrint

1NCE FreeRTOS BluePrint demonstrates the usage of various IoT protocols inculuding CoAP, LwM2M, and UDP with cellular connectivity. This repository showcases examples integrating the 1NCE SDK to leverage 1NCE OS tools, such as device authentication and energy-saving features, using the ([Wakaama LWM2M Library](https://www.eclipse.org/wakaama/)).

## Overview

This repository provides examples of the following protocols:

* UDP  
* CoAP  (with/without DTLS)
* LwM2M with Bootstrap (with/without DTLS)

Each demo includes an optional **Energy Saver** feature that can be enabled for testing.

## Quick Start Guide

The [Binaries](./Binaries) folder contains pre-built binaries for UDP and CoAP demo applications.

1. **Configure The Energy saver Feature in 1NCE OS**  
   Using this [template](./Tools/energy_saver_template.json).

2. **Connect the [P-L496G-CELL02](https://www.st.com/en/evaluation-tools/p-l496g-cell02.html) Board**  
   When connected via USB, the board should appear as a storage drive on your computer.

3. **Flash the Binary**  
   Simply drag and drop the desired binary file from the `Binaries` folder onto the storage drive. The board will automatically flash the binary. 
   
   **Note:** If flashing fails, refer to [Flashing using STM32CubeProgrammer](#flashing-using-stm32cubeprogrammer).


4. **View Demo Logs**  
   Use the [Serial Monitor](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor) in Visual Studio Code to view the demo logs.

## Getting Started (Local Build)
### Prerequisites
* [P-L496G-CELL02 LTE Cellular to Cloud Pack with STM32L496AG MCU](https://www.st.com/en/evaluation-tools/p-l496g-cell02.html)
*  [1NCE SIM Card](https://shop.1nce.com/portal/shop/) 
*  [VSCode](https://code.visualstudio.com/)
*  [STM32 VS Code Extension v2.0.0](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension) with the mentioned prerequisites.
* Ensure that CMake version 3.22 or higher is used.

#### STLink firmware upgrade
An STLink firmware upgrade is required. The STM32 VSCode plugin includes a button for this, but if it doesn’t work, the upgrade can be manually initiated by running the `.bat` file from the installed STM32 folder: `ST/STM32CubeCLTxx/STLinUpgrade.bat`

#### Modem Firmware Update (Optional)
 Please ensure that your BG96 modem has the latest firmware version. You can download the firmware update package and instructions from ([ST's X-Cube Cellular page](https://www.st.com/en/embedded-software/x-cube-cellular.html )) (v6.0.0 recommended).

For modem flashing, the QFlash tool can now be downloaded from the official Quectel website:
([QFlash Download (V7.1)](https://www.quectel.com/download/qflash_v7-1_en/))

### Building Sample

To configure the demo you wish to use, modify the `nce_demo_config.h` file located in `Application/Config/`: (by default `CONFIG_COAP_DEMO_ENABLED`)

```
CONFIG_COAP_DEMO_ENABLED
CONFIG_UDP_DEMO_ENABLED
CONFIG_LwM2M_DEMO_ENABLED
```

### Available Demos

#### UDP Demo

1NCE FreeRTOS BluePrint allows customers to communicate with 1NCE endpoints via UDP and use all features as part of the 1NCE OS.
* Setup the Demo runner in file `Application/Config/nce_demo_config.h`
```c
#define CONFIG_UDP_DEMO_ENABLED
```
* The following parameters can be configured:
```c
#define CONFIG_UDP_DATA_UPLOAD_FREQUENCY_SECONDS    60
```

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


* Setup the Demo runner in file `Application/Config/nce_demo_config.h`
```
#define CONFIG_COAP_DEMO_ENABLED
```
* The following parameters can be configured:
```c
    #define CONFIG_COAP_URI_QUERY                        "t=test"
    #define CONFIG_COAP_DATA_UPLOAD_FREQUENCY_SECONDS    60
    #define CONFIG_NCE_ENERGY_SAVER
```
#### CoAPs with DTLS 

For the DTLS Support the default Port is 5684 and automatically defines the `ENABLE_DTLS` as an additional define

The CoAP DTLS performs 3 main tasks from the [1NCE IoT C SDK](https://github.com/1NCE-GmbH/1nce-iot-c-sdk) :

* Send the Device Authenticator Request
* Get the Response
* Process the Response and give the DTLS identity and PSK to the application code.


#### LwM2M Demo 

The LWM2M support is provided using Eclipse Wakaama library communicating with a Leshan LWM2M server. This mode allows the device to act as an LwM2M client, facilitating communication with an LwM2M server for use cases such as device management, firmware updates, and sensor data collection.  By default, the client registers with the 1NCE LwM2M server, and secure communication is ensured via optional DTLS support.

* Setup the Demo runner in file (Application/Config/nce_demo_config.h)
```c
#define CONFIG_LwM2M_DEMO_ENABLED
```

##### Key LwM2M Configuration Parameters

The following parameters are critical for enabling and customizing the LwM2M client mode in the blueprint:

* **LwM2M Server Endpoint:** The device communicates with the 1NCE LwM2M server by default.

```c
#define LWM2M_ENDPOINT "lwm2m.os.1nce.com"
```

* **LwM2M Client Mode:** it will act as a client that registers with and communicates with an LwM2M server. The client mode is responsible for sending requests (e.g., device data or sensor readings) to the server and handling commands sent by the server.

```c
#define LWM2M_CLIENT_MODE
```

* **Bootstrap Mode:** Allows the device to retrieve its configuration, security keys, and server information from a bootstrap server if necessary. This feature is useful when the device does not have the configuration needed to directly register with the LwM2M server.

```c
#define LWM2M_BOOTSTRAP

```

* **SenML JSON Support:** This enables the device to send sensor data in SenML (Sensor Markup Language) JSON format. SenML is a standardized way to represent sensor measurements and device events in JSON format, making it easier to interpret and process on the server side.

```c
#define LWM2M_SUPPORT_SENML_JSON
```
* **JSON Data Format:** This enables JSON formatting for general data communication, ensuring compatibility with servers that use JSON for sending and receiving messages.
```c
#define LWM2M_SUPPORT_JSON
```
* **Little Endian Format:** Configures the device to use Little Endian byte order. This is common for many embedded systems and microcontrollers, where the least significant byte is stored or transmitted first.
```c
#define LWM2M_LITTLE_ENDIAN
```
* **TLV (Type-Length-Value) Support:** Enables the device to use TLV (Type-Length-Value) encoding, which is a compact binary format used to represent objects and values in LwM2M communication. TLV is particularly useful for efficiently transferring data between constrained devices and the server.
```c
#define LWM2M_SUPPORT_TLV
```
* **CoAP Default Block Size:** Sets the default block size to 1024 bytes for CoAP communication. When transferring large messages, block-wise transfers are used, and this configuration determines the size of each block.
```c
#define LWM2M_COAP_DEFAULT_BLOCK_SIZE 1024
```
* **Single Server Registration:** This configuration ensures that the device registers with only one LwM2M server. For most cases, registering with multiple servers is unnecessary, and a single server suffices for device management and data exchange.
```c
#define LWM2M_SINGLE_SERVER_REGISTERATION
```
* **LwM2M Object Send:** This defines the LwM2M Object URI that will be sent to the server. The default URI "/3/0" corresponds to the LwM2M Device Object, which provides essential device information, such as the manufacturer, model number, and firmware version.
```c
#define LWM2M_OBJECT_SEND "/3/0"
```
* Please add **the ICCID** in the configurtion file. If DTLS is enabled, the bootstrap psk should also be defined. The PSK can be set during LwM2M integration testing via the Device Integrator or through [1NCE API](https://help.1nce.com/dev-hub/reference/post_v1-integrate-devices-deviceid-presharedkey).
```c
    #define CONFIG_NCE_ICCID          ""
    #define CONFIG_LWM2M_BOOTSTRAP_PSK    ""
```

## Energy Saver Feature

The **Energy Saver** feature is available for both UDP and CoAP demos. It allows users to optimize device power consumption when communicating with 1NCE endpoints. 

### Configuration

To enable the Energy Saver feature, add the following flag in `nce_demo_config.h`:

```c
#define CONFIG_NCE_ENERGY_SAVER
```

**Note:** To use the Energy Saver feature for UDP and CoAP demos, ensure that the correct translation template is applied in the 1NCE OS. and the right protocol selected and the [used template](./Tools/energy_saver_template.json).

## Device Controller

The Device Controller is an API that allows you to interact with devices integrated into the 1NCE API. You can use this API to send requests to devices, and the devices will respond accordingly. For more details you can visit our [DevHub](https://help.1nce.com/dev-hub/docs/1nce-os-device-controller)

### Sending a Request
To send a request to a specific device, you can refer to our documentation in [1NCE DevHub](https://help.1nce.com/dev-hub/docs/1nce-os-device-controller)


#### FreeRTOS Configuration 
To handle the incoming request from the 1NCE API, the configuration of certain parameters needed `Application/Config/nce_demo_config.h`

```c
/* C2D Parameters */
/* This port is used for both UDP and CoAP communication. */
#define NCE_RECV_PORT 3000
#define NCE_RECEIVE_BUFFER_SIZE_BYTES 200

```
* `NCE_RECV_PORT`: This is the port number where your device will listen for incoming requests. It should match the port parameter used in the request.
* `NCE_RECEIVE_BUFFER_SIZE_BYTES` : This is the size of the buffer that will be used to receive the incoming data from the 1NCE API.

**Note:** C2D (Cloud to Device) is supported for all three protocols: UDP, CoAP, and LwM2M. The LwM2M client is tightly integrated with C2D requests, and for UDP and CoAP also open background port for C2D communication. 

## Troubleshooting

### Modem Firmware and Band Configuration 
If the device connects to only 2G networks or is unable to connect in some regions, you may need to adjust the RAT (Radio Access Technology) and band settings in `Application/Config/nce_demo_config.h`:

```c
#define CELLULAR_CONFIG_DEFAULT_RAT 8  // Example for CAT M1
#define CELLULAR_CONFIG_DEFAULT_RAT_2 0 // Example for GSM
#define CELLULAR_CONFIG_DEFAULT_RAT_3 9 // Example for NBIOT
#define CUSTOM_BAND_BG96 "AT+QCFG=\"band\",F,80004,80008"  // Example for Germany CATM1

// Values 
/**
*  The GSM RATs network      0
*  The CAT M1 RATs network   8
*  The NBIOT RATs network    9
**/
```
For more details on band settings, refer to the [BG96 AT Commands Manual](https://www.quectel.com/download/quectel_bg96_at_commands_manual_v2-3/).

### Flashing using STM32CubeProgrammer
* Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).
* Connect the board and press "Connect" to enable ST-Link.
* Select the "Erasing & Programming" tab on the left.
* Add the binary file and press "Start Programming".

## Configuring Log Level
The verbosity of logs can be set by defining the `LIBRARY_LOG_LEVEL` macro in the `Core/Inc/iot_config.h` file.
This setting controls the level of logging details for debugging and troubleshooting purposes.

The available log levels for `LIBRARY_LOG_LEVEL` are:

`IOT_LOG_NONE`: Disables all logging.
`IOT_LOG_ERROR`: Enables error messages only.
`IOT_LOG_WARN`: Enables warnings and errors.
`IOT_LOG_INFO`: Enables informational messages, warnings, and errors.
`IOT_LOG_DEBUG`: Enables detailed debug information, warnings, errors, and informational messages.

Example Configuration in `iot_config.h`:
```c
#define LIBRARY_LOG_LEVEL IOT_LOG_DEBUG
```
This configuration outputs all debug information, which is useful during development or troubleshooting.

## Asking for Help

The most effective communication with our team is through GitHub. Simply create a [new issue](https://github.com/1NCE-GmbH/blueprint-freertos/issues/new/choose) and select from a range of templates covering bug reports, feature requests, documentation issue, or Gerneral Question.
