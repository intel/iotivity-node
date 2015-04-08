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
ARDUINO_PORT := ttyACM0

# override with `make PLATFORM=arduinomega ARDUINOWIFI=1` to enable Arduino WiFi shield
ARDUINOWIFI := 0

APP_NAME := ocserver

ROOT_DIR = ../../../../..
LOGGER_DIR = $(ROOT_DIR)/logger
OC_LOG_DIR = $(ROOT_DIR)/../oc_logger
TBSTACK_DIR = $(ROOT_DIR)/stack
TBSOCKET_DIR = $(ROOT_DIR)/ocsocket

include $(ROOT_DIR)/local.properties
include $(ROOT_DIR)/$(PLATFORM).properties

VPATH := $(SDIR_ARD_PLATFORM)

#include directories
OCSOCK_DIR = $(ROOT_DIR)/ocsocket
LOGGER_DIR = $(ROOT_DIR)/logger
STACK_DIR  = $(ROOT_DIR)/stack
INC_DIRS = -I$(OCSOCK_DIR)/include/ -I$(OC_LOG_DIR)/include -I$(LOGGER_DIR)/include -I$(STACK_DIR)/include

CC_FLAGS.debug		:= -O1 -g3 -Wall -c -fmessage-length=0 -pedantic -fpic
CC_FLAGS.release	:= -Os -Wall -c -fmessage-length=0 -fpic

CFLAGS := $(CC_FLAGS.$(BUILD)) -DTB_LOG

ifeq ($(ARDUINOWIFI),1)
	CFLAGS += -DARDUINOWIFI
	ARDUINO_SHIELD_TYPE := "/wifi_shield"
	TRANSPORT_OBJS = $(WIFI_COBJ)
else
	ARDUINO_SHIELD_TYPE := "/ethernet_shield"
	TRANSPORT_OBJS = $(ETH_CPPOBJ) $(ETH_UTIL_CPPOBJ)
endif

OUT_DIR := $(PLATFORM)$(ARDUINO_SHIELD_TYPE)/$(BUILD)

OBJ_DIR := $(OUT_DIR)/bin

all: prep_dirs core.a $(APP_NAME).o $(APP_NAME).elf $(APP_NAME).hex

core.a: $(PLATFORM_OBJS)
	@cd $(OBJ_DIR) && $(AR) -x ../../../../$(ROOT_DIR)/$(PLATFORM)$(ARDUINO_SHIELD_TYPE)/$(BUILD)/liboctbstack.a
	$(AR) rcs $(OBJ_DIR)/$@ $(foreach obj, $^, $(OBJ_DIR)/$(obj)) $(OBJ_DIR)/*.o
	@cd $(OBJ_DIR) && $(RANLIB) $@

prep_dirs:
	-mkdir $(PLATFORM)
	-mkdir $(PLATFORM)/$(ARDUINO_SHIELD_TYPE)
	-mkdir $(OUT_DIR)
	-mkdir $(OBJ_DIR)

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_PLATFORM) $(INC_DIRS) $(INC_DIR_PLATFORM) $< -o $(OBJ_DIR)/$@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_PLATFORM) $(INC_DIRS) $(INC_DIR_PLATFORM) $< -o $(OBJ_DIR)/$@

$(APP_NAME).elf: $(APP_NAME).o core.a
ifeq ($(PLATFORM),arduinomega)
	$(CC) -Os -Wl,--gc-sections,--relax $(CFLAGS_PLATFORM) $(foreach obj, $^, $(OBJ_DIR)/$(obj)) -lm -o $(OBJ_DIR)/$@
else ifeq ($(PLATFORM),arduinodue)
	$(CXX) -Os -Wl,--gc-sections -mcpu=cortex-m3 -T/$(SDIR_ARD_CORE_3)/linker_scripts/gcc/flash.ld -Wl,-Map,$(APP_NAME).map -o $(OBJ_DIR)/$@ -lm -lgcc -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols -Wl,--start-group $(foreach obj, $(APP_NAME).o $(SYSCALLS_SAM3_OBJ) $(SPI_OBJ) $(TRANSPORT_OBJS) $(VARIANT_OBJ) core.a, $(OBJ_DIR)/$(obj)) $(SDIR_ARD_CORE_3)/libsam_sam3x8e_gcc_rel.a -Wl,--end-group
else
	$(error Wrong value for PLATFORM !!)
endif

$(APP_NAME).hex: $(APP_NAME).elf
ifeq ($(PLATFORM),arduinomega)
	$(AVR_OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $(OBJ_DIR)/$< $(OBJ_DIR)/$(APP_NAME).eep
	$(AVR_OBJCOPY) -O ihex -R .eeprom $(OBJ_DIR)/$< $(OUT_DIR)/$@
else ifeq ($(PLATFORM),arduinodue)
	$(ARDUINO_TOOLS_DIR)/arm-none-eabi-objcopy -O binary $(OBJ_DIR)/$< $(OUT_DIR)/$@
else
	$(error Wrong value for PLATFORM !!)
endif

install: all
ifeq ($(PLATFORM),arduinomega)
	$(AVR_PROGRAMMER) -C$(ARDUINO_DIR)/hardware/tools/avrdude.conf -v -v -v -v -patmega2560 -cstk500v2 -P/dev/$(ARDUINO_PORT) -b115200 -D -Uflash:w:$(OUT_DIR)/$(APP_NAME).hex:i
else ifeq ($(PLATFORM),arduinodue)
	stty -F /dev/$(ARDUINO_PORT) speed 1200 cs8 -cstopb -parenb
	$(ARDUINO_DIR)/hardware/tools/bossac -i -d --port=$(ARDUINO_PORT) -U false -e -w -v -b $(OUT_DIR)/$(APP_NAME).hex -R
else
	$(error Wrong value for PLATFORM !!)
endif

.PHONY: clean

clean: legacy_clean
	rm -rf arduinomega
	rm -rf arduinodue

legacy_clean:
	@rm -rf bin
	@rm -f *.o *.d *.elf *.eep *.a *.hex *.bin *.map *-
