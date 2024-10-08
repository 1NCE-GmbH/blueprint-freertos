
from textwrap import dedent            
from west.commands import WestCommand  
from west import log                   
import subprocess

class QuickStart(WestCommand):

    def __init__(self):
        super().__init__(
            'quickstart',
            'Build a 1NCE OS Demo and flash it to a connected nrf9160 DK',
            dedent(''' Build and flash CoAP (With  DTLS), LwM2M or UDP Demos, the payload reduction (using 1NCE Energy saver) can also be enabled.
            '''))

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name,
                                         help=self.help,
                                         description=self.description)
        parser.add_argument('-p', '--protocol', help='Select the protocol: coap (Default), lwm2m or udp')
        parser.add_argument('-e', '--energysaving', help='Set to "y" to enable energy saver')

        return parser

    def do_run(self, args, unknown_args):
        cmd = "west build -b nrf9160dk_nrf9160_ns "
        if (args.protocol is None or (args.protocol is not None and args.protocol.lower() == "coap") ):
            log.inf('Building 1NCE CoAP Demo')
            cmd = cmd + "nce-blueprint/nce_coap_demo  --build-dir build_coap_demo "
            if(  (args.energysaving is not None and args.energysaving.lower() == "y") ):
                cmd = cmd + "-DCONFIG_NCE_ENERGY_SAVER=y"
                log.inf('1NCE Energy Saver is Enabled')
            subprocess.run(cmd,shell=True) 
            subprocess.run("west flash --build-dir build_coap_demo",shell=True)
        elif ( (args.protocol is not None and args.protocol.lower() == "udp") ):
            log.inf('Building 1NCE UDP Demo')
            cmd = cmd + "nce-blueprint/nce_udp_demo --build-dir build_udp_demo  "
            if((args.energysaving is not None and args.energysaving.lower() == "y")):
                cmd = cmd + "-DCONFIG_NCE_ENERGY_SAVER=y"
                log.inf('1NCE Energy Saver is Enabled')
            subprocess.run(cmd,shell=True) 
            subprocess.run("west flash --build-dir build_udp_demo",shell=True)
        elif ( (args.protocol is not None and args.protocol.lower() == "lwm2m") ):
            log.inf('Building 1NCE LwM2M Demo')
            cmd = cmd + "nce-blueprint/nce_lwm2m_demo --build-dir build_lwm2m_demo  "
            subprocess.run(cmd,shell=True) 
            subprocess.run("west flash --build-dir build_lwm2m_demo",shell=True)
        else :
            log.err('Unsupported argument')

