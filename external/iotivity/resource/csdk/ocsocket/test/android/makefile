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
include ../local.properties

ROOT_DIR = ../../..
OCSOCK_DIR = $(ROOT_DIR)/ocsocket
LOGGER_DIR = $(ROOT_DIR)/logger
BIN_DIR = $(OCSOCK_DIR)/bin/android/
OBJ_DIR = $(OCSOCK_DIR)/obj/android/
INC_DIRS = -I$(OCSOCK_DIR)/include/ -I$(LOGGER_DIR)/include

CXX=arm-linux-androideabi-g++
CC=arm-linux-androideabi-gcc
CFLAGS_ANDROID =  -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -DGTEST_HAS_PTHREAD=0
LDFLAGS_ANDROID = -march=armv7-a -Wl,--fix-cortex-a8 -llog

DEP_LIBS = $(GTEST_ANDROID_DIR)/lib/.libs/libgtest_main.a $(GTEST_ANDROID_DIR)/lib/.libs/libgtest.a

CFLAGS += -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -DTB_LOG

TEST_APP = ocsocket_gtest

COBJ = ocsocket.o logger.o

VPATH = $(OCSOCK_DIR)/src:$(LOGGER_DIR)/src

all: $(TEST_APP)
	mkdir -p $(BIN_DIR)
	cp $(TEST_APP) $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	cp *.o $(OBJ_DIR)

%.o: %.c
	$(CXX) -c $(CFLAGS) $(CFLAGS_ANDROID) $(INC_DIRS) $< -o $@

$(TEST_APP).o: $(TEST_APP).cpp
	$(CXX) -c $(CFLAGS) $(CFLAGS_ANDROID) $(INC_DIRS) -I$(GTEST_ANDROID_DIR)/include -o $@ $<

$(TEST_APP): $(TEST_APP).o $(DEP_LIBS) $(COBJ)
	$(CXX) -Os -Wl,--gc-sections $(LDFLAGS_ANDROID) $^ -o $@

install: all
#	adb remount
	adb push $(BIN_DIR)$(TEST_APP) /data/local/tmp/

.PHONY: clean

clean:
	rm -f *.o $(TEST_APP) recv
	rm -fr $(BIN_DIR)
	rm -fr $(OBJ_DIR)

