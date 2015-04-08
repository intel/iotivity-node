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
PLATFORM := linux

ROOT_DIR = ../../..
include $(ROOT_DIR)/local.properties

OCSOCK_DIR = $(ROOT_DIR)/ocsocket
OC_LOG_DIR    = $(ROOT_DIR)/../oc_logger
LOGGER_DIR = $(ROOT_DIR)/logger
BIN_DIR = $(OCSOCK_DIR)/bin/linux/
OBJ_DIR = $(OCSOCK_DIR)/obj/linux/
INC_DIRS = -I$(OCSOCK_DIR)/include/ -I$(LOGGER_DIR)/include -I$(OC_LOG_DIR)/include

CXX = g++
CC = gcc

CFLAGS += -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -DTB_LOG
DEP_LIBS = $(GTEST_LINUX_DIR)/lib/.libs/libgtest_main.a $(GTEST_LINUX_DIR)/lib/.libs/libgtest.a
VPATH = $(OCSOCK_DIR)/src:$(LOGGER_DIR)/src

TEST_APP = ocsocket_gtest
COBJ = logger.o ocsocket.o 

all: $(TEST_APP)
	mkdir -p $(BIN_DIR)
	cp $(TEST_APP) $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	cp *.o $(OBJ_DIR)

%.o: %.c
	$(CC) -c $(CFLAGS) $(INC_DIRS) $< -o $@

$(TEST_APP).o: $(TEST_APP).cpp
	$(CXX) -c $(CFLAGS) -pthread $(INC_DIRS)  -I$(GTEST_LINUX_DIR)/include -o $@ $<

$(TEST_APP): $(TEST_APP).o $(DEP_LIBS) $(COBJ)
	$(CXX) -Os -Wl,--gc-sections $^ -lpthread -o $@

.PHONY: clean

clean:
	rm -f *.o $(TEST_APP)
	rm -fr $(BIN_DIR)
	rm -fr $(OBJ_DIR)


