# diyMotorController

![DRV8871 PCB v1.0](/doc/drv8871PCB.png "PCB V1.0 2024.")

This repository is used for the development of a DC motor controller board. It has a lot of common functions needed and by cloning this project you will have the following capabilities baked into your project:

1. Simple logging syntax that enabes you to direct output to the local serial terminal as well as to an MQTT broker. 
2. Unique device name based on DEVICE_TYPE specified in platformio.ini combined with the MAC address of this device.
3. Scanning and connecting to the Access Point (AP) with the best Recieved Signal Strength Indicator (RSSI) using a table of known SSID and password pairs kept in the apSecrets.h file that is locally maintained on  yur dev machine to preserve network secrecy.
4. Ability to recieve incoming MQTT messages and process them as commands by coding your own custom call back function all nicely templated out and easy to update fr yur application need.
5. Ability to publish MQTT messages to an MQTT broker allowing you to transmit telemtry and other data you wish to provide to a central monitoring system.

## Libraries
Use the PlatformIO library manager to install the following libraries that are required for this code to work:

1. PubSubClient by Nick O'Leary (knolleary/PubSubClient@^2.8) for MQTT support.
2. ArduinoJson by Benoit Blanchon (bblanchon/ArduinoJson@^7.2.1) for JSON support.
3. TaskScheduler by Richard Lowe (arkhipenko/TaskScheduler@^3.3.0) for task scheduling.
  
# Directory Structure
Please follow this convention:

1. Put .h files in ./include.
2. Put .cpp files in ./src.
3. Put Unity unit tests in /test.
4. Put reference files used during intial setu of your local repo in /ref.
5. Doxygen files end up in /html though this will chnage as we ultimately not want to store the Doxygen output in Github Pages.
6. Put Github automation scripts in /.github/workflows.
7. Use the PlatformIO Library Manager when possible for third party libraries. Do not forget to update these README instrucitons to include any changes in what liabraries your code uses.
8. If you want to use a third party library that is not listed in the PlatformIO Library manager database then put the library into the /lib directory.
9. Put any class files (.cpp and associated .h files) in their own named subdiretory in /lib directory (i.e. /lib/myClass/myClass.cpp & /lib/myClass/myClass.h).   

# Comment Standards
Make the source files Doxygen compliant following the guidelines outlined 
[here](https://www.woolseyworkshop.com/2020/03/20/documenting-arduino-sketches-with-doxygen/). 

# Unit Testing
Unit tests are loaded to the SOC via the PlatformIO, Advanced, test tool. Results appear in a seperate terminal output session that you can access once the tests complete. A future release  of the template repository will contain test cases and instructions on how to use them.

# OTA firmware updates
This feature is in the code but is not presently working. This is on the "TODO" list to address. THis is the error we get today when compiling and attempting an OTA update.

```
Sending invitation to 192.168.2.163 failed
12:36:24 [ERROR]: Host 192.168.2.163 Not Found
*** [upload] Error 1
========================================== [FAILED] Took 16.33 seconds ==========================================

Environment       Status    Duration
----------------  --------  ------------
featheresp32      SUCCESS   00:00:29.355
featheresp32_ota  FAILED    00:00:16.328
===================================== 1 failed, 1 succeeded in 00:00:45.683 =====================================
```

Instructions on how to do OTA updates with PlatformIO can be found [here](https://lonelybinary.com/en-us/blogs/learn/esp32-ota-using-platformio).

# Documentation
Doxygen comments are embedded in all source files and a Doxyfile has been added to the root folder of this respository. These are the steps to follow to set up Doxygen. Note that inorder to have your documentation avaibale via Github Pages the Repo cannot be private. Since at this time tthis repo is provate these steps are not all fully impemented.

"How to" instructions for setting up Doxygen are [here](https://kylerobots.github.io/tutorials/Automatic_Documentation/).

# Circuit
This application has been tested on a [generic 12VDC motor](https://www.amazon.ca/gp/product/B07Z55VQ83/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) driven by [Adafruit Huzzah32]() development board using PWM though a DRV8871 Dual H-Bridge IC, along with an LM286N HBridge develoment board used as a reference. The motor has a built in A/B phased encoder that runs of 3.3VDC so this circuit has both 3.3VDC and 12VDC power in it. No line level conversion is needed between the GPIO out pins on the SOC and the input pins on the low voltage side of the motor driver. The intended application will prove out the design and manufacturing steps for the PCB. Here is a Fritzing diagram that shows the ciruit used to test our DRV8871 development board.

![DRV8871 Development board](/doc/diyDrv8871_bb.png "demo circuit used to develop the code.")

The setup we are using includes a reference ccircuit based around an LM286N HBridge IC. THis reference circuit was added becuase the CD motor performance we were seeing seemed off. A line level converter was added to our DRV8871 circuit which seems to have addressed the issue of motor RMPs seeming to be too low. Here is a Fritzing diagram of the circuit used for the reference LM286N connected DC motors:

![LM286N Development board](/doc/dcMotorLN298RefCircuit_bb.png "reference circuit used to compare board performance.")

## PCB 
We have created blog posts that outline how we go about designing, manufactuing, stuffing and testing our PCBs [here](http://theagingapprentice.com). **NOTE This article needs  to be  written and this link needs updating!**

The deigns for this PCB were created in Fusion360 and can be found in this repository in the **/electronicsCAM/drv8871DualH-Bridge** directory.