#==============================================================================
# NOTICE - Transition to SCONS
#==============================================================================

The IoTivity build system is transitioning to SCONS. Although the 
makefiles are still available (until v1.0) and some developers are 
still using them, they are currently no longer supported. To learn more 
about building using SCONS see Readme.scons.txt in the repository root 
directory. The build steps used in continuous integration can be found
in auto_build.sh which is also in the the repository root directory.

#==============================================================================
# Build instructions for Linux
#==============================================================================

In ./occoap directory
clean ==> make clean
build ==> make

#==============================================================================
# Unit test instructions for Linux
#==============================================================================

After building (make), run
./occoap/bin/debug/occoaptest


