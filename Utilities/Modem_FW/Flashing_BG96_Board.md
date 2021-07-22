# How to flash BG96 FW on STMod+ board 

### TODO: Add missing Screenshots into this document

Package Content: 
* How to flash BG96 board.pdf (this document) 
* Zip file that contains the modem FW (example file name: 
BG96MAR02A07M1G_01.010.01.010.zip) 
* STM32 binary (L496_modem_boot.bin) 


To flash the board, the following HW is needed, A Discovery L496 STM32 board, A BG96 STMod+ board, 2 micro USB cables 
Assumption STM32 USB driver is already installed (that allow to connect PC to STM32 board and update STM32 FW) 

1. Unzip Modem FW at root directory 
2. Download the Quectel BG96 Window USB driver from this url: https://jan-sulaiman-1nce-public.s3-eu-central-1.amazonaws.com/connectivity-suite/Blueprints/Quectel_LTE%265G_Windows_USB_Driver_V2.1.zip
It is a zip file (example: Quectel_BG96_Windows_USB_Driver_V1.2.zip) that contains an exe file 
3. Install the BG96 USB driver, run the executable file (only the first time) 
4. Download the modem FW flasher tool (QFlash) from this url: https://jan-sulaiman-1nce-public.s3-eu-central-1.amazonaws.com/connectivity-suite/Blueprints/QFlash_V4.12.zip There are the zip file that contains the tool (example: QFlash_V4.12) and the associated User 
Guide (example: Quectel_QFlash_User_Guide_V2.8.pdf) 
5. Unzip the Flasher tool at root directory (only the first time or if there is a new version of tool) 
6. Connect BG96 STMod+ board to Discovery L496 board 
Connect first micro USB cable between laptop and Discovery L496 
7. Flash the STM32 binary (L496_modem_boot.bin) on Discovery L496 (drag & drop the binary on 
8. Reset Discovery board (black button) and wait for 10 sec 
Optional: to be sure boot is completed open a TeraTerm with the correct parameters (on STM32 
VCP) 
and check trace ends with “MODEM ON” 
9. Connect the 2nd micro USB cable between Laptop and BG96 STMOd+ board. 
10. Verify that VCP (Virtual COM Port) for Device Management tool is setup. Please use the 
Windows Device manager to double check the port number of “Quectel USB DM Port” (COMxx). 
In the example COM59. 

11. Start the Flasher tool (QFlash). Note QFlash version used for this document is 4.8 but could be 
different for you depending of the latest version available @ Quectel url. 
12. Select the Image to be flashed by clicking to Load FW Files 

13. Select the image to be flashed: By selecting one of the file in the UPDATE directory 
(c:\BG96MAR02A07M1G_01.010.01.010\update\) 
14. Click Open 
15. Click on Start button to start the download and flashing 
16. Once flashed (after about 200 s): 

17. Reboot the device (black button) and check the actual version running on the board. 
To do this by executing the AT command AT+QGMR using the TeraTerm (a new one opened on 
Quectel USB AT Port) 
It is VCOM61 in this example 
The displayed version must be the same as the just flashed FW 
