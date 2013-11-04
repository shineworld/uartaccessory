uartaccessory
=============

uart accessory emulator


FAQ
===

- I get a "Permission denied" error when I try to execute the program. Is mandatory to run it with root user priviledges ?

  Usually to access to /dev/ttyUSBx you need root rights but how you can see with "ls -l /dev/ttyUSBx" you can solve
  adding your user to "dialout" group with following commands:  

  <i><b>this add your user to dialout group permitting access to /dev/ttyUSB0 without root rights</i></b><br>
  sudo adduser <user_name> dialout<br>
  sudo reboot
  
 
