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
# override with `make BUILD=release`
# default to debug build
BUILD := debug
PLATFORM := linux

# You must create the file "local.propertiessudo" on your local machine which contains any local paths, etc
# local_settings.mk should NOT be committed to repo
ROOT_DIR = ../../..
include $(ROOT_DIR)/local.properties
# GTEST_DIR contains the path to Google Test libs and must be defined in local.properties
#  Example:
#  GTEST_DIR := /home/johndoe/utils/gtest-1.7.0

# NOTE:  to run app, make sure that LD_LIBRARY_PATH env variable
#        contains $(GTEST_DIR)/lib/.libs
                                
CC := gcc
CXX	:= g++

PROG := randomtest

SRC_DIR := ../../src
TEST_DIR:= ../../test/linux
INC_DIR := ../../include

CC_FLAGS.debug := -O0 -g3 -Wall -c -fmessage-length=0 -pedantic -std=c99 -fpic -D TB_LOG
CC_FLAGS.release := -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -fpic 
CC_FLAGS := $(CC_FLAGS.$(BUILD))

CXX_FLAGS.debug := -O0 -g3 -Wall -c -fmessage-length=0 -pedantic -std=c++0x -fpic -D TB_LOG
CXX_FLAGS.release := -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -fpic 
CXX_FLAGS := $(CXX_FLAGS.$(BUILD))

INC	:= -I$(SRC_DIR)  
INC += -I$(TEST_DIR)
INC += -I$(INC_DIR)
INC += -I$(GTEST_DIR)/include


# using make's computed variables to select object and bin folders
# depending on the build type
OBJ_DIR.debug := ./obj/debug
OBJ_DIR.release := ./obj/release
OBJ_DIR := $(OBJ_DIR.$(BUILD))

BIN_DIR.debug := ./bin/debug
BIN_DIR.release := ./bin/release
BIN_DIR := $(BIN_DIR.$(BUILD))

C_FILES := $(wildcard $(SRC_DIR)/*.c)
#CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
CPP_FILES := $(wildcard $(TEST_DIR)/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o) $(C_FILES:.c=.o)))
H_FILES := $(wildcard $(TEST_DIR)/*.h)
H_FILES += $(wildcard $(INC_DIR)/*.h)
LD_FLAGS := -L/usr/lib
LD_FLAGS += -L$(GTEST_DIR)/lib/.libs


LIBS := -lpthread
LIBS += -lgtest
LIBS += -lgtest_main


all: $(PROG)
	
$(PROG): $(OBJ_FILES)  
	mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LD_FLAGS) $(LIBS)	
#	$(CXX) -o $@ $^ $(GTEST_DIR)/make/gtest_main.a -lpthread
	mv ./$(PROG) ./$(BIN_DIR)

#$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h $(H_FILES)
#	$(CC) $(CC_FLAGS) $(INC) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h $(H_FILES)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CC_FLAGS) $(INC) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(H_FILES)
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(INC) -c -o $@ $<

	
.PHONY: clean
clean :
	rm -rf ./obj/debug/*
	rm -rf ./obj/release/*
	rm -rf ./lib/debug/*
	rm -rf ./lib/release/*
	rm -rf ./bin/debug/*
	rm -rf ./bin/release/*
	rm -rf ./test/tst_*.txt


.PHONY: print_vars

print_vars:
	@echo ""
	@echo 'BUILD     = '$(value BUILD)
	@echo 'INC       = '$(value INC)
	@echo 'CPP_FILES = '$(value CPP_FILES)
	@echo 'LIBS      = '$(value LIBS)
  	

