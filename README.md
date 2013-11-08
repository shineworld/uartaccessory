uartaccessory
=============

uart accessory emulator

This project aim to help a user to develop an open accessory android application without left adb usb debug using a<br>
linux powerd PC how android accessory device emulator.

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
  
 
