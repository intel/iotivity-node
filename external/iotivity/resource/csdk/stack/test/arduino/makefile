# //******************************************************************
# //
# // Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# //      http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
BUILD	 := release
PLATFORM := arduinomega

ROOT_DIR = ../../..
include $(ROOT_DIR)/local.properties
LIBCOAP_DIR = $(ROOT_DIR)/libcoap-4.1.1
OCSOCK_DIR = $(ROOT_DIR)/ocsocket
OCCOAP_DIR = $(ROOT_DIR)/occoap
LOGGER_DIR = $(ROOT_DIR)/logger
RANDOM_DIR = $(ROOT_DIR)/ocrandom
JSON_DIR = $(ROOT_DIR)/../../extlibs/cjson
OCSTACK_DIR = $(ROOT_DIR)/stack
BIN_DIR = $(OCSTACK_DIR)/bin/arduino/
OBJ_DIR = $(OCSTACK_DIR)/obj/arduino/

#Source directories
SDIR_ARD_CORE = $(ARDUINO_DIR)/hardware/arduino/cores/arduino
SDIR_ARD_SPI = $(ARDUINO_DIR)/libraries/SPI
SDIR_ARD_ETH = $(ARDUINO_DIR)/libraries/Ethernet
SDIR_ARD_ETH_UTIL = $(ARDUINO_DIR)/libraries/Ethernet/utility
SDIR_ARD_TIME = $(ARDUINO_DIR)/libraries/Time

VPATH = $(SDIR_ARD_CORE):$(SDIR_ARD_SPI):$(SDIR_ARD_ETH):$(SDIR_ARD_ETH_UTIL):$(SDIR_ARD_TIME):$(RANDOM_DIR)/src:$(OCSTACK_DIR)/src:$(OCCOAP_DIR)/src:$(JSON_DIR)

#include directories
IDIR_ARD_CORE = $(ARDUINO_DIR)/hardware/arduino/cores/arduino
IDIR_ARD_VARIANT = $(ARDUINO_DIR)/hardware/arduino/variants/mega
IDIR_ARD_SPI = $(ARDUINO_DIR)/libraries/SPI
IDIR_ARD_ETH = $(ARDUINO_DIR)/libraries/Ethernet
IDIR_ARD_ETH_UTIL = $(ARDUINO_DIR)/libraries/Ethernet/utility
IDIR_ARD_TIME = $(ARDUINO_DIR)/libraries/Time

INC_DIRS = -I$(IDIR_ARD_CORE) -I$(IDIR_ARD_VARIANT) -I$(IDIR_ARD_SPI) \
	-I$(IDIR_ARD_ETH) -I$(IDIR_ARD_ETH_UTIL) -I$(IDIR_ARD_TIME) \
	-I$(OCSOCK_DIR)/include -I$(LOGGER_DIR)/include -I$(RANDOM_DIR)/include \
	-I$(OCSTACK_DIR)/include -I$(OCSTACK_DIR)/include/internal \
	 -I$(LIBCOAP_DIR)/ -I$(OCCOAP_DIR)/include -I$(JSON_DIR)

#Compiler/Linker flags
COPTIONS_ARDUINO = -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=156 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -MMD -DNDEBUG
CFLAGS += -c -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -felide-constructors -std=c++0x -DATMEGA2560 -DTB_LOG -DWITH_ARDUINO

#Compilers
CXX=avr-g++
CC=avr-gcc
AR=avr-ar
RANLIB=avr-ranlib

TESTAPP = ocserver

CORE_COBJ = WInterrupts.o wiring.o wiring_digital.o
CORE_CPPOBJ = main.o Stream.o WMath.o WString.o HardwareSerial.o Print.o SPI.o IPAddress.o wiring_analog.o
ETH_CPPOBJ = Dhcp.o Dns.o Ethernet.o EthernetUdp.o
ETH_UTIL_CPPOBJ = socket.o w5100.o
OCSTACK_CPPOBJ = ocrandom.o ocresource.o occollection.o ocobserve.o ocserverrequest.o ocstack.o occoaphelper.o occoap.o occlientcb.o
OCDEPENDENT_CPPOBJ = wiring_analog.o
CJSON_COBJ = cJSON.o

all: core.a $(TESTAPP).o $(TESTAPP).elf $(TESTAPP).hex
	mkdir -p $(BIN_DIR)
	cp $(TESTAPP).hex $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	cp *.o $(OBJ_DIR)
	cp *.elf $(OBJ_DIR)

# Note for Arduino: The C files are compiled using C++ compiler since Arduino
# build includes Time.h header file which has C++ style definitions.
%.o: %.c
	$(CXX) $(CFLAGS) $(COPTIONS_ARDUINO) $(INC_DIRS) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(COPTIONS_ARDUINO) $(INC_DIRS) $< -o $@

core.a: $(CORE_COBJ) $(CORE_CPPOBJ) $(ETH_CPPOBJ) $(ETH_UTIL_CPPOBJ) $(LIBCOAP_DIR)/$(BUILD)/libcoap.a
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(TESTAPP).elf: $(TESTAPP).o core.a $(OCSTACK_CPPOBJ) $(OCDEPENDENT_CPPOBJ) $(LIBCOAP_DIR)/$(BUILD)/libcoap.a $(CJSON_COBJ)
	$(CC) -Os -Wl,--gc-sections,--relax $(COPTIONS_ARDUINO) $^ -lm -o $@

$(TESTAPP).hex: $(TESTAPP).elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $(TESTAPP).eep
	avr-objcopy -O ihex -R .eeprom $< $@

install: all
	avrdude -C$(ARDUINO_DIR)/hardware/tools/avrdude.conf -v -v -v -v -patmega2560 -cstk500v2 -P/dev/ttyACM0 -b115200 -D -Uflash:w:$(TESTAPP).hex:i

.PHONY: clean

clean:
	rm -f *.o *.d *.elf *.eep *.a *.hex *-
	rm -fr $(BIN_DIR)
	rm -fr $(OBJ_DIR)

