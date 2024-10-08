# FreeRTOS Cellular Interface Reference Quectel BG96

## Introduction

This repository provides the reference implementation as cellular module ports for [Quectel BG96](https://www.quectel.com/product/lte-bg96-cat-m1-nb1-egprs/). This repository should be used with [FreeRTOS-Cellular-Interface](https://github.com/FreeRTOS/FreeRTOS-Cellular-Interface). See [FreeRTOS-Cellular-Interface README](https://github.com/FreeRTOS/FreeRTOS-Cellular-Interface/blob/main/README.md) for more details.

The BG96 port implemenation follows [Quectel_BG96_AT_Commands_Manual_V2.3](https://www.quectel.com/download/quectel_bg96_at_commands_manual_v2-3).
The firmware version of BG96 can be acquired with the following AT commands:
* ATI
* AT+QGMR

If the firmware version is not supported in the AT command document, you may contact
Quectel for a firmware update.

## Use Case

There is also an use case at [MQTT_Mutual_Auth_Demo_with_BG96](https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS-Plus/Demo/FreeRTOS_Cellular_Interface_Windows_Simulator/MQTT_Mutual_Auth_Demo_with_BG96) as a demonstration to run BG96 with FreeRTOS Windows Simulator.

## How Does Community Members to Contribute a Cellular Module Port

We provide a repository, [FreeRTOS-Cellular-Interface-Community-Supported-Ports](https://github.com/FreeRTOS/FreeRTOS-Cellular-Interface-Community-Supported-Ports), for community members to contribute their modules. See [How to Contribute a Cellular Module Port](https://github.com/FreeRTOS/FreeRTOS-Cellular-Interface-Community-Supported-Ports#how-to-contribute-a-cellular-module-port) for more details.

## Security

See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

The FreeRTOS Cellular Interface library is distributed under MIT open source license. The code in this repository is licensed under the MIT License. See the [LICENSE](LICENSE) file.
