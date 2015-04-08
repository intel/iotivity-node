-------------------------------------------------------------------------------
  NOTICE - Transition to SCONS
-------------------------------------------------------------------------------

The IoTivity build system is transitioning to SCONS. Although the 
makefiles are still available (until v1.0) and some developers are 
still using them, they are currently no longer supported. To learn more 
about building using SCONS see Readme.scons.txt in the repository root 
directory. The build steps used in continuous integration can be found
in auto_build.sh which is also in the the repository root directory.

-------------------------------------------------------------------------------

To compile for Linux or Android, include ocsocket.c and ocsocket.h in your project.
Look at the Makefile in ocsocket/test/linux and ocsocket/test/android for compilation dependencies.

To compile for Arduino, include ocsocket_arduino.cpp and ocsocket.h in your project.
Look at the Makefile in ocsocket/test/arduino for compilation dependencies.
Note: Patch your Arduino Ethernet Shield library for arduino/libraries/Ethernet/utility/socket.cpp
to fix recvfrom issue.

Place appropriate local.properties file under test directory to provide local path for \
GTEST libraries/header files to compile tests.

#Sample local.properties file
GTEST_LINUX_DIR = /home/gtest-1.7.0
GTEST_ANDROID_DIR = /home/gtest-1.7.0_android/gtest-1.7.0
ARDUINO_DIR = /usr/share/arduino
