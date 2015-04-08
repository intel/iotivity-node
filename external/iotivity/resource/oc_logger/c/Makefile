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

CC_FLAGS.debug=-Wall -std=c99 -Werror -O0 -g
CC_FLAGS.release=-Wall -std=c99 -Werror -O2

INC=-I../include/
CC:=gcc

BIN_DIR=../bin
LIB_DIR=../lib
TEST_DIR=../test

LIB=$(LIB_DIR)/oc_logger.a

CCX=$(CC) $(CC_FLAGS.$(BUILD))

all: setup c_lib c_targets

setup:
	mkdir -p $(BIN_DIR) $(LIB_DIR)
	mkdir -p $(BIN_DIR)/targets
	mkdir -p $(TEST_DIR)

c_lib:
	$(CCX) $(INC) -c -o $(BIN_DIR)/oc_logger.o oc_logger.c

c_targets:
	$(CCX) $(INC) -c -o $(BIN_DIR)/targets/oc_console_logger.o oc_console_logger.c

clean:
	rm -rf -v \*\.o test_logging $(BIN_DIR)/* $(LIB_DIR)/*
