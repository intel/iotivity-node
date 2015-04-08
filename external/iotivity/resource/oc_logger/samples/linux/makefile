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
# default to release build
BUILD	 := release
PLATFORM := linux
CC	 := g++
OUT_DIR	  := $(BUILD)
OBJ_DIR	  := $(OUT_DIR)/obj

ifeq ($(ROOT_DIR), )
ROOT_DIR	= ../../../csdk
endif

OUT_DIR	= ./

OCLOGGER_DIR	= $(ROOT_DIR)/logger
OC_LOG_DIR		= $(ROOT_DIR)/../oc_logger
OCRANDOM_DIR	= $(ROOT_DIR)/ocrandom
OCSOCKET_DIR	= $(ROOT_DIR)/ocsocket
LCOAP_DIR		= $(ROOT_DIR)/libcoap-4.1.1
OCCOAP_DIR		= $(ROOT_DIR)/occoap
OCTBSTACK_DIR	= $(ROOT_DIR)/stack

OC_LOG_INC		= $(OC_LOG_DIR)/include
OC_LOG_LIB		= $(OC_LOG_DIR)/lib/oc_logger.a  

OCLOGGER_INC	= $(OCLOGGER_DIR)/include
OCRANDOM_INC	= $(OCRANDOM_DIR)/include
OCSOCKET_INC	= $(OCSOCKET_DIR)/include
LCOAP_INC		= $(LCOAP_DIR)
OCCOAP_INC		= $(OCCOAP_DIR)/include
OCTBSTACK_INC	= $(OCTBSTACK_DIR)/include

INC_DIRS		:= -I$(OCLOGGER_INC)
INC_DIRS		+= -I$(OC_LOG_INC)
INC_DIRS		+= -I$(OCRANDOM_INC)
INC_DIRS		+= -I$(OCSOCKET_INC)
INC_DIRS		+= -I$(LCOAP_INC)
INC_DIRS		+= -I$(OCCOAP_INC)
INC_DIRS		+= -I$(OCTBSTACK_INC)

CC_FLAGS.debug      := -O0 -g3 -Wall -ffunction-sections -fdata-sections \
                        -std=c99 $(INC_DIRS) -L$(ROOT_DIR)/$(PLATFORM)/$(BUILD) -DTB_LOG
CC_FLAGS.release    := -Os -Wall -fdata-sections -Wl,--gc-sections -Wl,-s \
                        -std=c99 $(INC_DIRS) -L$(ROOT_DIR)/$(PLATFORM)/$(BUILD)

LDLIBS		+= $(OC_LOG_LIB) -loctbstack -lpthread 
CPPFLAGS	+= $(CC_FLAGS.$(BUILD)) $(LDLIBS)

SOURCES     := test_logging.c

OBJECTS:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(SOURCES))

PROGRAMS	+= test_logging

all:	prep_dirs $(OBJECTS) $(PROGRAMS)

prep_dirs:
	-mkdir -p $(OUT_DIR)
	-mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(CPPFLAGS) $< -o $@

test_logging: $(OBJ_DIR)/test_logging.o 
	$(CC) $^ $(CPPFLAGS) -o $(OUT_DIR)/$(BUILD)/$@

.PHONY: clean

clean: legacy_clean
	-rm -rf release
	-rm -rf debug

legacy_clean:
	rm -f *.o $(PROGRAMS)
