uartaccessory
=============

uart accessory emulator

This project aim to help a user to develop an open accessory android application without left adb usb debug using a linux powerd PC how android accessory device emulator.

The scheme is simple:<br>
  Android Device -> AOA ready application -> USB cable -> Linux PC -> <b>this software</b> ->/dev/ttyUSBx -> target device 


FAQ
===

- To build on console use: gcc -g -o accessory $(pkg-config --cflags libusb-1.0) *.c $(pkg-config --libs libusb-1.0)
  
  That compiles and links everything in one go, only one -g is needed, and pkg-config is used to get the correct compiler and linker flags for finding libusb files.<br>
  To debug it use: dbg ./accessory<br>
  To run it use: run [options]<br>
  To catch stack after a segfault use: "fb"<br>

- I get a "Permission denied" error when I try to execute the program. Is mandatory to run it with root user priviledges ?

  Usually to access to /dev/ttyUSBx you need root rights but how you can see with "ls -l /dev/ttyUSBx" you can solve
  adding your user to "dialout" group with following commands:  

  <i><b>this add your user to dialout group permitting access to /dev/ttyUSB0 without root rights</i></b><br>
  sudo adduser user_name dialout<br>
  sudo reboot
  
- How can I enable libusb debug and catch eventual interface error logs ?

  To enable libusb log message is only necessary to define the environment variable <b>LIBUSB_DEBUG</b> to desired value:
  
  Level 0: no messages ever printed by the library (default)<br>
  Level 1: error messages are printed to stderr<br>
  Level 2: warning and error messages are printed to stderr<br>
  Level 3: informational messages are printed to stdout, warning and error messages are printed to stderr<br>

- Sometime I get error running like user.

  This depends by fact that for some at me unknowed reason (at moment) the USB interface is already claimed by kernel and there is necessity to free it. To do that the program must be run in root rights.
