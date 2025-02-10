# Meccano Crane

![CranePicture](/doc/craneExample.jpg)

This repository contains all of the information about the Meccano crane project that was undertaken in 2025. This is what happens when you remember the dreams of your youth and you now have the skills and means to make those long ago dreams a present day reality. For this project we had some rules that we tried to adhere to as much as possible. Those rues are:

1. Use Meccano parts as much as possible.
2. Have the crane be controllable by a remote control.
3. Interesting mechanisms take precidence over practical efficiency. 


## Libraries
Use the Arduino library manager to install the following libraries that are required for this code to work:

1. ESP32Servoby Kevin Harrington
2. etc.
  
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

# Documentation
A [manual](/doc/MeccanoCrane-Manual.pdf) is being created for this project. 

For software developers, Doxygen comments are embedded in all source files and a Doxyfile has been added to the root folder of this respository. These are the steps to follow to set up Doxygen. Note that inorder to have your documentation avaibale via Github Pages the Repo cannot be private. Since at this time tthis repo is provate these steps are not all fully impemented.

"How to" instructions for setting up Doxygen are [here](https://kylerobots.github.io/tutorials/Automatic_Documentation/).