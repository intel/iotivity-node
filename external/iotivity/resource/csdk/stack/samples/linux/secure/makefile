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
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# override with `make BUILD=debug`
# default to release build
BUILD	 := release
PLATFORM := linux
CC	 := g++
OUT_DIR	  := $(BUILD)
OBJ_DIR	  := $(OUT_DIR)/obj

ifeq ($(ROOT_DIR), )
ROOT_DIR	= ../../../..
endif

OUT_DIR	= .

OCLOGGER_DIR	= $(ROOT_DIR)/logger
OC_LOG_DIR = $(ROOT_DIR)/../oc_logger
OCRANDOM_DIR	= $(ROOT_DIR)/ocrandom
OCSOCKET_DIR	= $(ROOT_DIR)/ocsocket
LCOAP_DIR	= $(ROOT_DIR)/libcoap-4.1.1
OCCOAP_DIR	= $(ROOT_DIR)/occoap
OCTBSTACK_DIR	= $(ROOT_DIR)/stack
EXTLIBS_DIR	= $(ROOT_DIR)/../../extlibs
CJSON_DIR	= $(EXTLIBS_DIR)/cjson

CJSON_SRC	= $(CJSON_DIR)

OCLOGGER_INC	= $(OCLOGGER_DIR)/include
OC_LOG_INC	= $(OC_LOG_DIR)/include
OCRANDOM_INC	= $(OCRANDOM_DIR)/include
OCSOCKET_INC	= $(OCSOCKET_DIR)/include
LCOAP_INC	= $(LCOAP_DIR)
OCCOAP_INC	= $(OCCOAP_DIR)/include
OCTBSTACK_INC	= $(OCTBSTACK_DIR)/include
CJSON_INC	= $(CJSON_DIR)

INC_DIRS	:= -I$(OCLOGGER_INC)
INC_DIRS	+= -I$(OC_LOG_INC)
INC_DIRS	+= -I$(OCRANDOM_INC)
INC_DIRS	+= -I$(OCSOCKET_INC)
INC_DIRS	+= -I$(LCOAP_INC)
INC_DIRS	+= -I$(OCCOAP_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)
INC_DIRS	+= -I$(CJSON_INC)

CC_FLAGS.debug   := -O0 -g3 -Wall -ffunction-sections -fdata-sections -fno-exceptions \
                        -std=c++0x -pedantic $(INC_DIRS) -L$(ROOT_DIR)/linux/$(BUILD) -DTB_LOG
CC_FLAGS.release := -Os -Wall -fdata-sections -Wl,--gc-sections -Wl,-s -fno-exceptions \
                        -std=c++0x $(INC_DIRS) -L$(ROOT_DIR)/linux/$(BUILD) -DTB_LOG

LDLIBS		+= -loctbstack -lpthread
CPPFLAGS	+= $(CC_FLAGS.$(BUILD)) $(LDLIBS)

CJSON_SOURCES		:= $(CJSON_SRC)/cJSON.c

SOURCES		:= $(CJSON_SOURCES)
SOURCES     += common.cpp ocserverbasicops.cpp occlientbasicops.cpp gen_sec_bin.cpp

OBJECTS:= $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

PROGRAMS	+= ocserverbasicops
PROGRAMS	+= occlientbasicops
PROGRAMS	+= gen_sec_bin

all:	c_sdk prep_dirs $(OBJECTS) $(PROGRAMS)

prep_dirs:
	-mkdir -p $(OUT_DIR)
	-mkdir -p $(OBJ_DIR)

c_sdk:
	cd $(ROOT_DIR) && $(MAKE) BUILD=$(BUILD) PLATFORM=$(PLATFORM)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

ocserverbasicops: $(OBJ_DIR)/ocserverbasicops.o $(OBJ_DIR)/common.o
	$(CC) $^ $(CPPFLAGS) -o $(OUT_DIR)/$(BUILD)/$@

occlientbasicops: $(OBJ_DIR)/occlientbasicops.o $(OBJ_DIR)/common.o
	$(CC) $^ $(CPPFLAGS) -o $(OUT_DIR)/$(BUILD)/$@

gen_sec_bin: $(OBJ_DIR)/gen_sec_bin.o
	$(CC) $^ $(CPPFLAGS) -o $(OUT_DIR)/$(BUILD)/$@

.PHONY: clean

clean: legacy_clean
	-rm -rf release
	-rm -rf debug
	cd $(ROOT_DIR) && $(MAKE) clean
	cd $(ROOT_DIR) && $(MAKE) deepclean

legacy_clean:
	rm -f *.o $(PROGRAMS)
