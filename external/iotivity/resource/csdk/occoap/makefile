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
# default to release build
BUILD := release

# You must create the file "local.properties" on your local machine which contains any local paths, etc
# local_settings.mk should NOT be committed to repo
include ./local.properties
# GTEST_DIR contains the path to Google Test libs and must be defined in local.properties
#  Example:
#  GTEST_DIR := /home/johndoe/utils/gtest-1.7.0

# NOTE:  to run app, make sure that LD_LIBRARY_PATH env variable
#        contains $(GTEST_DIR)/lib/.libs
                                
CC := gcc
CXX	:= g++

PROG := occoaptest

#SRC_DIR := ./src
TEST_DIR:= ./test
#INC_DIR := ./include
#LOG_DIR := ../logger
#STACK_DIR := ../stack

CC_FLAGS.debug := -O0 -g3 -Wall -c -fmessage-length=0 -pedantic -std=c99 -fpic -DTB_LOG
CC_FLAGS.release := -Os -Wall -c -fmessage-length=0 -std=c99 -fpic 
CC_FLAGS := $(CC_FLAGS.$(BUILD))

CXX_FLAGS.debug := -O0 -g3 -Wall -c -fmessage-length=0 -pedantic -std=c++0x -fpic -DTB_LOG
CXX_FLAGS.release := -Os -Wall -c -fmessage-length=0 -std=c++0x -fpic 
CXX_FLAGS := $(CXX_FLAGS.$(BUILD))

#INC	:= -I$(SRC_DIR)  
#INC += -I$(TEST_DIR)
#INC += -I$(INC_DIR)
#INC += -I$(LOG_DIR)/include
#INC += -I$(STACK_DIR)/include
INC += -I$(GTEST_DIR)/include


# using make's computed variables to select object and bin folders
# depending on the build type
OBJ_DIR.debug := ./obj/debug
OBJ_DIR.release := ./obj/release
OBJ_DIR := $(OBJ_DIR.$(BUILD))

BIN_DIR.debug := ./bin/debug
BIN_DIR.release := ./bin/release
BIN_DIR := $(BIN_DIR.$(BUILD))

#C_FILES := $(wildcard $(LOG_DIR)/src/*.c)
#C_FILES += $(wildcard $(SRC_DIR)/*.c)
#CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
CPP_FILES := $(wildcard $(TEST_DIR)/*.cpp)
#CPP_FILES += $(wildcard $(LOG_DIR)/src/*.c)
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o) $(C_FILES:.c=.o)))
H_FILES := $(wildcard $(TEST_DIR)/*.h)
#H_FILES += $(wildcard $(INC_DIR)/*.h)
#H_FILES += $(wildcard $(LOG_DIR)/include/*.h)
LD_FLAGS := -L/usr/lib
LD_FLAGS += -L$(GTEST_DIR)/lib/.libs


LIBS := -lgtest
LIBS += -lgtest_main
LIBS += -lpthread


all: $(PROG)
	mkdir -p ./$(OBJ_DIR)
	mkdir -p ./$(BIN_DIR)
	
$(PROG): $(OBJ_FILES)
	$(CXX) -o $@ $^ $(LD_FLAGS) $(LIBS) 	
	mkdir -p ./$(BIN_DIR)
	mv ./$(PROG) ./$(BIN_DIR)

#$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.h $(H_FILES)
#	$(CXX) $(CXX_FLAGS) $(INC) -c -o $@ $<

#$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.h $(H_FILES)
#	$(CXX) $(CXX_FLAGS) $(INC) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h $(H_FILES)
	mkdir -p ./$(OBJ_DIR)
	$(CC) $(CC_FLAGS) $(INC) -c -o $@ $<	
		
$(OBJ_DIR)/%.o: $(LOG_DIR)/src/%.c $(LOG_DIR)/include/%.h $(H_FILES)
	mkdir -p ./$(OBJ_DIR)
	$(CC) $(CC_FLAGS) $(INC) -c -o $@ $<	

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(H_FILES)
	mkdir -p ./$(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(INC) -c -o $@ $<	
	
.PHONY: clean
clean :
	rm -rf ./obj/debug/*
	rm -rf ./obj/release/*
	rm -rf ./lib/*
	rm -rf ./bin/debug/*
	rm -rf ./bin/release/*


.PHONY: print_vars

print_vars:
	@echo ""
	@echo 'BUILD     = '$(value BUILD)
	@echo 'INC       = '$(value INC)
	@echo 'CPP_FILES = '$(value CPP_FILES)
	@echo 'C_FILES   = '$(value C_FILES)
	@echo 'LIBS      = '$(value LIBS)
	@echo 'OBJ_FILES = '$(value OBJ_FILES)
	@echo 'SRC_DIR   = '$(value SRC_DIR)
	
  	

