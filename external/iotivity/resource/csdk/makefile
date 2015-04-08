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
# override with `make BUILD=debug`
# override with `make PLATFORM=arduinomega` or `make PLATFORM=arduinodue`
# default to release build
# default to build for linux
BUILD	 := release
PLATFORM := linux

# override with `make PLATFORM=arduinomega ARDUINOWIFI=1` to enable Arduino WiFi shield
ARDUINOWIFI := 0

ifeq ($(ROOT_DIR), )
ROOT_DIR	= ./
endif

PLATFORM_TYPE = linux

ifeq ($(PLATFORM),linux)
    CXX=g++
    CC=gcc
    AR=ar
    RANLIB=ranlib
    CFLAGS_PLATFORM = -DWITH_POSIX -std=c99
    PLATFORM_TYPE=linux
else ifeq ($(PLATFORM),arduinomega)
    include local.properties
    include $(PLATFORM).properties
    CC=$(ARDUINO_TOOLS_DIR)/avr-g++
    ifeq ($(wildcard $(ARDUINO_DIR)/libraries/Time/Time/),)
        $(error Arduino Time library needs to be moved from <ARDUINO_DIR>/libraries/Time \
        to <ARDUINO_DIR>/libraries/Time/Time. You may need to create \
        <ARDUINO_DIR>/libraries/Time/Time directory. Please refer to the wiki or readme \
        for more information)
    endif
    PLATFORM_TYPE=arduino
else ifeq ($(PLATFORM),arduinodue)
    include local.properties
    include $(PLATFORM).properties
    CC=$(ARDUINO_TOOLS_DIR)/arm-none-eabi-g++
    PLATFORM_TYPE=arduino
else
    $(error Wrong value for PLATFORM !!)
endif

ifeq ($(PLATFORM_TYPE),arduino)
    ifeq ($(ARDUINOWIFI),0)
    ARDUINO_SHIELD_TYPE := "/ethernet_shield"
    else ifeq ($(ARDUINOWIFI), 1)
    ARDUINO_SHIELD_TYPE := "/wifi_shield"
    endif
    PLATFORM_SPECIFIC_BACKOUT:= ../../../../
else
    PLATFORM_SPECIFIC_BACKOUT := ../../../
endif

ifeq ($(OUT_DIR), )
OUT_DIR	= $(PLATFORM)$(ARDUINO_SHIELD_TYPE)/$(BUILD)
endif

ifeq ($(OBJ_DIR), )
OBJ_DIR	= $(OUT_DIR)/obj
endif

OCLOGGER_DIR	= logger
OC_LOG_DIR      = ../oc_logger
OCRANDOM_DIR	= ocrandom
OCSOCKET_DIR	= ocsocket
LCOAP_DIR	= libcoap-4.1.1
OCCOAP_DIR	= occoap
OCTBSTACK_DIR	= stack
OCMALLOC_DIR 	= ocmalloc
EXTLIBS_DIR	= ../../extlibs
CJSON_DIR	= $(EXTLIBS_DIR)/cjson
TINYDTLS_DIR	= $(EXTLIBS_DIR)/tinydtls

OCCOAP_SRC	= $(OCCOAP_DIR)/src
OCTBSTACK_SRC	= $(OCTBSTACK_DIR)/src
OCMALLOC_SRC 	= $(OCMALLOC_DIR)/src
CJSON_SRC	= $(CJSON_DIR)

OCLOGGER_INC	= $(OCLOGGER_DIR)/include
OC_LOG_INC	= $(OC_LOG_DIR)/include
OCRANDOM_INC	= $(OCRANDOM_DIR)/include
OCSOCKET_INC	= $(OCSOCKET_DIR)/include
LCOAP_INC	= $(LCOAP_DIR)
OCCOAP_INC	= $(OCCOAP_DIR)/include
OCTBSTACK_INC	= $(OCTBSTACK_DIR)/include
OCMALLOC_INC	= $(OCMALLOC_DIR)/include
CJSON_INC	= $(CJSON_DIR)

INC_DIRS	:= -I$(OCLOGGER_INC)
INC_DIRS	+= -I$(OC_LOG_INC)
INC_DIRS	+= -I$(OCRANDOM_INC)
INC_DIRS	+= -I$(OCSOCKET_INC)
INC_DIRS	+= -I$(LCOAP_INC)
INC_DIRS	+= -I$(OCCOAP_INC)
INC_DIRS	+= -I$(OCMALLOC_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)/internal
INC_DIRS	+= -I$(CJSON_INC)

CC_FLAGS.debug		:= -O0 -g3 -Wall -c -fmessage-length=0 -pedantic -fpic -DTB_LOG
CC_FLAGS.release	:= -Os -Wall -c -fmessage-length=0 -fpic

CFLAGS		+= $(CC_FLAGS.$(BUILD)) $(INC_DIRS) $(CFLAGS_PLATFORM) $(INC_DIR_PLATFORM)
LDLIBS		+= -lcoap

CJSON_SOURCES		:= $(CJSON_SRC)/cJSON.c

OCCOAP_SOURCES		:= $(OCCOAP_SRC)/occoap.c
OCCOAP_SOURCES		+= $(OCCOAP_SRC)/occoaphelper.c

OCTBSTACK_SOURCES	:= $(OCTBSTACK_SRC)/ocstack.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/occlientcb.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/ocresource.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/ocobserve.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/ocserverrequest.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/occollection.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/oicgroup.c
OCTBSTACK_SOURCES	+= $(OCTBSTACK_SRC)/ocsecurity.c

SOURCES			:= $(CJSON_SOURCES)
SOURCES			+= $(OCCOAP_SOURCES)
SOURCES			+= $(OCTBSTACK_SOURCES)

all:	make_lcoap objdirs obj_build liboctbstack.a

buildScript_all: objdirs obj_build liboctbstack.a

make_lcoap:
	$(MAKE) -C $(LCOAP_DIR) "BUILD=$(BUILD)" "PLATFORM=$(PLATFORM)" "ARDUINOWIFI=$(ARDUINOWIFI)"

objdirs:
	mkdir -p $(PLATFORM)
	mkdir -p $(PLATFORM)/$(ARDUINO_SHIELD_TYPE)
	mkdir -p $(OUT_DIR)
	mkdir -p $(OBJ_DIR)

obj_build:
	@echo "Building $@"
	# Output all *.o files to $(OUT_DIR)/$(BUILD)/$(OBJ_DIR)
	$(foreach source,$(SOURCES), $(CC) $(CFLAGS) $(source) -o $(patsubst %.c, %.o, $(patsubst %, $(OBJ_DIR)/%, $(notdir $(source))));)

liboctbstack.a: obj_build
	@echo "Building $@"
	# Unpackage libcoap.a to $(OBJ_DIR)/$(BUILD). The output objects from OCStack and OCCoap are already at this location
	@cd $(OBJ_DIR) && $(AR) -x $(PLATFORM_SPECIFIC_BACKOUT)$(LCOAP_DIR)/$(PLATFORM)$(ARDUINO_SHIELD_TYPE)/$(BUILD)/libcoap.a
	# Repackage all the objects at this location into a single archive. This is OCStack, OCCoap, and LibCoap (LibCoap contains OCRandom, OCLogger, and OCSocket.).
	$(AR) -r $(OUT_DIR)/$@ $(OBJ_DIR)/*.o

    ifeq ($(PLATFORM),linux)
     ifneq ($(wildcard $(TINYDTLS_DIR)/libtinydtls.a),)
			$(info "Building liboctbstack with DTLS support")
			mkdir -p $(OBJ_DIR)/tinydtls
			@cd $(OBJ_DIR)/tinydtls && $(AR) -x ../$(PLATFORM_SPECIFIC_BACKOUT)$(TINYDTLS_DIR)/libtinydtls.a
			$(AR) -q $(OUT_DIR)/$@ $(OBJ_DIR)/tinydtls/*.o
     endif
    endif

.PHONY: clean print_vars

clean: legacy_clean
	-rm -rf linux
	-rm -rf arduinomega
	-rm -rf arduinodue

deepclean: legacy_deepclean
	-rm -rf linux
	-rm -rf arduinomega
	-rm -rf arduinodue

legacy_clean:
	@echo "Cleaning all."
	rm -f $(OBJ_DIR)/$(BUILD)/*.o
	rm -f $(ROOT_DIR)/$(BUILD)/liboctbstack.a
	rm -rf $(OBJ_DIR)
	rm -rf release
	rm -rf debug

legacy_deepclean:
	@echo "Deep-Cleaning all."
	rm -f $(OBJ_DIR)/$(BUILD)/*.o
	rm -f $(ROOT_DIR)/liboctbstack.a
	rm -rf $(OBJ_DIR)
	$(MAKE) clean -C $(LCOAP_DIR)
	rm -rf release
	rm -rf debug
