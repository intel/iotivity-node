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
BUILD := release
PLATFORM := arduinomega
ARDUINO_PORT := /dev/ttyACM0

# override with `make PLATFORM=arduinomega ARDUINOWIFI=1` to enable Arduino WiFi shield
ARDUINOWIFI := 0

ifeq ($(ARDUINOWIFI),0)
	APP_NAME := ocsocket_test
	OCSOCKET_CPPOBJ = ocsocket_arduino.o
else
	APP_NAME := ocsocket_wifi_test
	OCSOCKET_CPPOBJ = ocsocket_arduino_wifi.o
endif

OBJ_DIR := ./bin

ROOT_DIR = ../../..

include $(ROOT_DIR)/local.properties
include $(ROOT_DIR)/$(PLATFORM).properties


#include directories
OCSOCK_DIR = $(ROOT_DIR)/ocsocket
LOGGER_DIR = $(ROOT_DIR)/logger
INC_DIRS = -I$(OCSOCK_DIR)/include/ -I$(LOGGER_DIR)/include

VPATH := $(SDIR_ARD_PLATFORM):$(LOGGER_DIR)/src:$(OCSOCK_DIR)/src 

CFLAGS	:= -Os -Wall -c -DTB_LOG

all: core.a $(APP_NAME).o $(APP_NAME).elf $(APP_NAME).hex

core.a: $(PLATFORM_OBJS)
	$(AR) rcs $@ $^
	$(RANLIB) $@

#logger needs to be compiled using C++ compiler
logger.o: logger.c
	$(CXX) $(CFLAGS) $(CFLAGS_PLATFORM) $(INC_DIRS) $(INC_DIR_PLATFORM) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_PLATFORM) $(INC_DIRS) $(INC_DIR_PLATFORM) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_PLATFORM) $(INC_DIRS) $(INC_DIR_PLATFORM) $< -o $@

$(APP_NAME).elf: $(APP_NAME).o $(OCSOCKET_CPPOBJ) logger.o core.a
	$(CC) -Os -Wl,--gc-sections,--relax $(CFLAGS_PLATFORM) $^ -lm -o $@

$(APP_NAME).hex: $(APP_NAME).elf
	$(AVR_OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $(APP_NAME).eep
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

install: all
	$(AVR_PROGRAMMER) -C$(ARDUINO_DIR)/hardware/tools/avrdude.conf -v -v -v -v -patmega2560 -cstk500v2 -P$(ARDUINO_PORT) -b115200 -D -Uflash:w:$(APP_NAME).hex:i

.PHONY: clean

clean:
	@rm -f *.o *.d *.elf *.eep *.a *.hex *.bin *.map *-
	@rm -rf $(OBJ_DIR)





















