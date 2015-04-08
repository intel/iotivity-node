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

ifeq ($(ROOT_DIR), )
ROOT_DIR	= $(PWD)/../../..
endif

# You must create the file "local.properties" on your local machine which contains any local paths, etc
# local.properties should NOT be committed to repo
include $(ROOT_DIR)/local.properties
# GTEST_DIR contains the path to Google Test libs and must be defined in local.properties
#  Example:
#  GTEST_DIR := /home/johndoe/utils/gtest-1.7.0

# NOTE:  to run app, make sure that LD_LIBRARY_PATH env variable
#        contains $(GTEST_DIR)/lib/.libs

OUT_DIR	  := $(PWD)/$(BUILD)
OBJ_DIR	  := $(OUT_DIR)/obj


OUT_DIR	  := $(PWD)

OCLOGGER_DIR	= $(ROOT_DIR)/logger
OCMALLOC_DIR	= $(ROOT_DIR)/ocmalloc
OCRANDOM_DIR	= $(ROOT_DIR)/ocrandom
OCSOCKET_DIR	= $(ROOT_DIR)/ocsocket
LCOAP_DIR		= $(ROOT_DIR)/libcoap-4.1.1
OCCOAP_DIR		= $(ROOT_DIR)/occoap
OCTBSTACK_DIR	= $(ROOT_DIR)/stack

OCLOGGER_INC	= $(OCLOGGER_DIR)/include
OCMALLOC_INC	= $(OCMALLOC_DIR)/include
OCRANDOM_INC	= $(OCRANDOM_DIR)/include
OCSOCKET_INC	= $(OCSOCKET_DIR)/include
LCOAP_INC		= $(LCOAP_DIR)
OCCOAP_INC		= $(OCCOAP_DIR)/include
OCTBSTACK_INC	= $(OCTBSTACK_DIR)/include

INC_DIRS	:= -I$(OCLOGGER_INC)
INC_DIRS	+= -I$(OCMALLOC_INC)
INC_DIRS	+= -I$(OCRANDOM_INC)
INC_DIRS	+= -I$(OCSOCKET_INC)
INC_DIRS	+= -I$(LCOAP_INC)
INC_DIRS	+= -I$(OCCOAP_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)/internal
INC_DIRS  += -I$(GTEST_DIR)/include

CC_FLAGS.debug      := -g -O0 -g3 -Wall -ffunction-sections -fdata-sections -fno-exceptions \
                        -std=c++0x -pedantic $(INC_DIRS) -L$(ROOT_DIR)/linux/$(BUILD) -DTB_LOG
CC_FLAGS.release    := -Os -Wall -fdata-sections -Wl,--gc-sections -Wl,-s -fno-exceptions \
                        -std=c++0x $(INC_DIRS) -L$(ROOT_DIR)/linux/$(BUILD)

LDLIBS		+= -loctbstack -lpthread
CPPFLAGS	+= $(CC_FLAGS.$(BUILD)) $(LDLIBS) $(GTEST_DIR)/lib/.libs/libgtest.a \
			   $(GTEST_DIR)/lib/.libs/libgtest_main.a

SOURCES := unittest.cpp

OBJECTS:= $(patsubst %.c, $(OBJ_DIR)/%.o, $(SOURCES))

PROGRAMS := unittest

all:	prep_dirs $(OBJECTS) $(PROGRAMS)

prep_dirs:
	-mkdir -p $(OUT_DIR)
	-mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

unittest: $(OBJ_DIR)/unittest.o
	$(CC) $^ $(CPPFLAGS) -o $(OUT_DIR)/$(BUILD)/$@

.PHONY: clean

clean: legacy_clean
	-rm -rf release
	-rm -rf debug

legacy_clean:
	rm -f *.o $(PROGRAMS)
