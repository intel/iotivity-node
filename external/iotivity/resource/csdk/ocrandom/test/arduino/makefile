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
# Override with `make BUILD=release`
# default to debug build
BUILD := debug
PLATFORM := arduinomega

# You must create the file "local.propertiessudo" on your local machine which contains any local paths, etc
# local_settings.mk should NOT be committed to repo
ROOT_DIR = ../../..
include $(ROOT_DIR)/local.properties

# Compilers                   
CC := avr-gcc
CXX	:= avr-g++
AR := avr-ar
OBJCopy := avr-objcopy

# Name of testing application
PROG := randomtest
C_COMPONENTS_DIR := ocrandom

# Folders that contain C source code
C_SRC_DIR := $(foreach d, $(C_COMPONENTS_DIR),../../../$d/src)

#Folders that contain C source code related to arduino
C_SRC_DIR_ARDUINO := $(ARDUINO_DIR)/hardware/arduino/cores/arduino

#Folders that contain CPP source code
CPP_SRC_DIR := ../../test/arduino

#Folders that contain CPP source code related to arduino
CPP_SRC_DIR_ARDUINO := $(ARDUINO_DIR)/hardware/arduino/cores/arduino

# Folders that contain Headers
TEMP_INC_DIR := $(foreach d, $(C_COMPONENTS_DIR),../../../$d/include)
TEMP_INC_DIR2 := $(foreach d, $(CXX_COMPONENTS_DIR),../../../$d/include)
INC_DIR := $(TEMP_INC_DIR) $(TEMP_INC_DIR2)
INC_DIR += $(ARDUINO_DIR)/hardware/arduino/cores/arduino
INC_DIR += $(ARDUINO_DIR)/hardware/arduino/variants/mega
INC_CMD = $(foreach d, $(INC_DIR), -I$d)

# Compiler flags specific to arduino
ARDUINO_FLAGS = -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=156 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -MMD

# Compiler flags
CC_FLAGS.debug := -c -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -std=c99 -DATMEGA2560 -DTB_LOG
CC_FLAGS.release := -c -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -std=c99 -DATMEGA2560
CC_FLAGS := $(CC_FLAGS.$(BUILD))

CXX_FLAGS.debug := -c -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -felide-constructors -std=c++0x -DATMEGA2560 -DTB_LOG
CXX_FLAGS.release := -c -Os -Wall -Wno-write-strings -ffunction-sections -fdata-sections -fno-exceptions -felide-constructors -std=c++0x -DATMEGA2560
CXX_FLAGS := $(CXX_FLAGS.$(BUILD))

# using make's computed variables to select object and bin folders
# depending on the build type
OBJ_DIR.debug := ./obj/debug
OBJ_DIR.release := ./obj/release
OBJ_DIR := $(OBJ_DIR.$(BUILD))

BIN_DIR.debug := ./bin/debug
BIN_DIR.release := ./bin/release
BIN_DIR := $(BIN_DIR.$(BUILD))

C_FILES := $(shell find $(C_SRC_DIR) -maxdepth 1 -name '*.c')
CPP_FILES := $(shell find $(CPP_SRC_DIR) -maxdepth 1 -name '*.cpp')
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o) $(C_FILES:.c=.o)))

C_FILES_ARDUINO := $(shell find $(C_SRC_DIR_ARDUINO) -maxdepth 1 -name '*.c')
CPP_FILES_ARDUINO := $(shell find $(CPP_SRC_DIR_ARDUINO) -maxdepth 1 -name '*.cpp')
OBJ_FILES_ARDUINO := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES_ARDUINO:.cpp=.o) $(C_FILES_ARDUINO:.c=.o)))

all: $(BIN_DIR)/$(PROG).hex

$(BIN_DIR)/core.a: $(OBJ_FILES_ARDUINO)
	mkdir -p $(BIN_DIR)
	$(AR) rcs $@ $^
	#mv ./core.a ./$(BIN_DIR)
	@echo ""
	@echo 'done making core.a'

$(OBJ_DIR)/%.o: $(CPP_SRC_DIR_ARDUINO)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(ARDUINO_FLAGS) $(INC_CMD) $< -o $@

$(OBJ_DIR)/%.o: $(CPP_SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(ARDUINO_FLAGS) $(INC_CMD) $< -o $@

$(OBJ_DIR)/%.o: $(C_SRC_DIR_ARDUINO)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CC_FLAGS) $(ARDUINO_FLAGS) $(INC_CMD) $< -o $@

$(OBJ_DIR)/%.o: $(C_SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) $(ARDUINO_FLAGS) $(INC_CMD) $< -o $@

$(BIN_DIR)/$(PROG).elf: $(OBJ_FILES) $(BIN_DIR)/core.a
	#$(CC) -Os -Wl,--gc-sections,--relax $(ARDUINO_FLAGS) $< $(BIN_DIR)/core.a -lm -o $@
	#$(CC) -Os -Wl,--gc-sections,--relax $(ARDUINO_FLAGS) $(OBJ_FILES) $(OBJ_FILES_ARDUINO) -lm -o $@
	$(CC) -Os -Wl,--gc-sections,--relax $(ARDUINO_FLAGS) $(OBJ_FILES) $(BIN_DIR)/core.a -lm -o $@
	@echo ""
	@echo 'done making elf'

$(BIN_DIR)/$(PROG).hex: $(BIN_DIR)/$(PROG).elf
	$(OBJCopy) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $(BIN_DIR)/$(PROG).eep
	$(OBJCopy) -O ihex -R .eeprom $< $@
	@echo ""
	@echo 'done making hex'

install: all
	#$(ARDUINO_DIR)/hardware/tools/avrdude -C$(ARDUINO_DIR)/hardware/tools/avrdude.conf -v -v -v -v -patmega2560 -cstk500v2 -P/dev/ttyACM0 -b115200 -D -Uflash:w:$(BIN_DIR)/$(PROG).hex:i
	avrdude -C$(ARDUINO_DIR)/hardware/tools/avrdude.conf -v -v -v -v -patmega2560 -cstk500v2 -P/dev/ttyACM0 -b115200 -D -Uflash:w:$(BIN_DIR)/$(PROG).hex:i

.PHONY: clean
clean :
	rm -rf ./obj/debug/*
	rm -rf ./obj/release/*
	rm -rf ./lib/debug/*
	rm -rf ./lib/release/*
	rm -rf ./bin/debug/*
	rm -rf ./bin/release/*
	rm -rf ./test/tst_*.txt
	rm -fr $(BIN_DIR)
	rm -fr $(OBJ_DIR)


.PHONY: print_vars
print_vars:
	@echo ""
	@echo 'BUILD                = '$(value BUILD)
	@echo 'C_SRC_DIR            = '$(value C_SRC_DIR)
	@echo 'C_SRC_DIR_ARDUINO    = '$(value C_SRC_DIR_ARDUINO)
	@echo 'CPP_SRC_DIR          = '$(CPP_SRC_DIR)
	@echo 'CPP_SRC_DIR_ARDUINO  = '$(value CPP_SRC_DIR_ARDUINO)
	@echo 'INC_DIR              = '$(INC_DIR)
	@echo 'H_FILES              = '$(H_FILES)
	@echo 'C_FILES              = '$(C_FILES)
	@echo 'CPP_FILES            = '$(CPP_FILES)
	@echo 'C_FILES_ARDUINO      = '$(C_FILES_ARDUINO)
	@echo 'CPP_FILES_ARDUINO    = '$(CPP_FILES_ARDUINO)
	@echo 'OBJ_FILES            = '$(OBJ_FILES)
	@echo 'OBJ_FILES_ARDUINO    = '$(OBJ_FILES_ARDUINO)
