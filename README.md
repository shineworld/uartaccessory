uartaccessory
=============

uart accessory emulator

This project aim to help a user to develop an open accessory android application without left adb usb debug using a linux powerd PC how android accessory device emulator.

Actually it simulate the FDTI FT312D behaviour detecting first packet after AOA switch in which are contained all info for correctly set UART device.
Also AOA fingerstamp sent to Android is for FTDI FT312D but can be fastly changed to anyother in code.

The scheme is simple:<br>
  Android Device -> AOA ready application -> USB cable -> Linux PC -> <b>this software<b> ->/dev/ttyUSBx -> target device 


FAQ
===

- I get a "Permission denied" error when I try to execute the program. Is mandatory to run it with root user priviledges ?

  Usually to access to /dev/ttyUSBx you need root rights but how you can see with "ls -l /dev/ttyUSBx" you can solve
  adding your user to "dialout" group with following commands:  

  <i><b>this add your user to dialout group permitting access to /dev/ttyUSB0 without root rights</i></b><br>
  sudo adduser user_name dialout<br>
  sudo reboot
  
- How can I enable libusb debug and catch eventual interface error logs ?

  To enable libusb log message is only necessary to define the environment variable <b>LIBUSB_DEBUG</b> to desired value:
  
  Level 0: no messages ever printed by the library (default)
  Level 1: error messages are printed to stderr
  Level 2: warning and error messages are printed to stderr
  Level 3: informational messages are printed to stdout, warning and error messages are printed to stderr   
