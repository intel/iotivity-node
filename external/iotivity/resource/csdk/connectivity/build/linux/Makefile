#/******************************************************************
# *
# * Copyright 2014 Samsung Electronics All Rights Reserved.
# *
# *
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#******************************************************************/

# override with `make BUILD=debug`
# default to release build
# default to build for linux
BUILD = release
PLATFORM = linux
TRANSPORT = ALL
OBJ_DIR = ./$(BUILD)
VERSION_MAJOR = 0
VERSION_MINOR = 1
VERSION_PATCH = 0

COMMON_MAKE = common.mk

include $(COMMON_MAKE)

#Modify below values to enable/disable the Adapter
#Suffix "NO_" to disable given adapter  
EDR = NO_EDR_ADAPTER
WIFI = WIFI_ADAPTER
LE = NO_LE_ADAPTER
ETHERNET = NO_ETHERNET_ADAPTER

#Add Pre processor definitions
DEFINE_FLAG = -D$(EDR) -D$(LE) -D$(WIFI) -D$(ETHERNET)

#Add Debug flags here
DEBUG_FLAG = -DTB_LOG

##
##	definitions
##
LIB_PREFIX				= CA
ETHERNET_ADAPTER		= ethernet_adapter
WIFI_ADAPTER			= wifi_adapter
BT_ADAPTER				= bt_edr_adapter
BLE_ADAPTER				= bt_le_adapter
ETHERNET_ADAPTER_PATH	= $(PROJECT_SRC_PATH)/$(ETHERNET_ADAPTER)/$(PLATFORM)
WIFI_ADAPTER_PATH		= $(PROJECT_SRC_PATH)/$(WIFI_ADAPTER)/$(PLATFORM)
BT_ADAPTER_PATH			= $(PROJECT_SRC_PATH)/$(BT_ADAPTER)/$(PLATFORM)
BLE_ADAPTER_PATH		= $(PROJECT_SRC_PATH)/$(BLE_ADAPTER)/$(PLATFORM)
TARGET					= lib$(PROJECT_NAME).so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
TARGET_ALIAS			= lib$(PROJECT_NAME).so

##
##	libcoap
##
libcoap_build_dir = $(PROJECT_LIB_PATH)/libcoap-4.1.1

LDFLAGS:=-L$(libcoap_build_dir)
LDLIBS:=-lcoap

BUILD_FLAG.debug	= $(DEFINE_FLAG) $(DEBUG_FLAG) 
BUILD_FLAG.release	=	$(DEFINE_FLAG)
BUILD_FLAG = $(BUILD_FLAG.$(BUILD))

DEBUG_DIR = ./debug
RELEASE_DIR = ./release

##
##	file declaration
##
COMPONENT_SRCS			=  \
logger.c oic_malloc.c oic_logger.c oic_console_logger.c \
caconnectivitymanager.c caremotehandler.c cainterfacecontroller.c camessagehandler.c	\
canetworkconfigurator.c caedradapter.c caleadapter.c	\
caethernetadapter.c cawifiadapter.c \
caprotocolmessage.c cawificore.c	\
uqueue.c uarraylist.c umutex.c uthreadpool.c \

COMPONENT_OBJS			= $(COMPONENT_SRCS:%.c=$(OBJ_DIR)/%.o)

##
##	compiler flags
##
CFLAGS				= 	-g -c -Wall -fPIC `pkg-config --cflags glib-2.0`

LFLAGS				= 	-ldl -lpthread `pkg-config --libs glib-2.0`

IFLAGS				= 	-I$(PROJECT_COMMON_INC_PATH)	\
						-I$(PROJECT_API_PATH)	\
						-I$(PROJECT_INC_PATH)	\
						-I$(ETHERNET_ADAPTER_PATH)	\
						-I$(WIFI_ADAPTER_PATH)	\
						-I$(BT_ADAPTER_PATH)	\
						-I$(BLE_ADAPTER_PATH)   \
                        -I$(libcoap_build_dir)

DFLAGS				= -DLINUX		\
                      -DWITH_POSIX		\
                      $(BUILD_FLAG)\
##
##	compile and link rules
##
vpath %.c $(PROJECT_COMMON_SRC_PATH)
vpath %.c $(ETHERNET_ADAPTER_PATH)
vpath %.c $(WIFI_ADAPTER_PATH)
vpath %.c $(BT_ADAPTER_PATH)
vpath %.c $(BLE_ADAPTER_PATH)
vpath %.c $(PROJECT_SRC_PATH)

$(OBJ_DIR)/%.o: %.c
	@$(MAKE_OBJ_PATH)
	@$(CC) -o $@ $(CFLAGS) $(DFLAGS) $(IFLAGS) $<

all : $(TARGET)
	@echo	" "
	@echo	"======================"
	@echo	"	Success!!"
	@echo	"======================"

$(TARGET) : $(COMPONENT_OBJS)
	@$(MAKE_PROJECT_OUT_PATH)
	@$(CC) -shared -Wl,-soname,$(TARGET) -o $@ $(COMPONENT_OBJS) $(LFLAGS) $(LDFLAGS) $(LDLIBS) 
	@$(MV) $(TARGET) $(PROJECT_OUT_PATH)/.
	@$(CD) $(PROJECT_OUT_PATH); $(MAKE_TARGET_ALIAS)

clean :	
	@$(RM) $(DEBUG_DIR) $(RELEASE_DIR)\
			$(PROJECT_OUT_PATH)

install :
	@$(MAKE_PROJECT_OUT_PATH)

##
##	macros
##
define MAKE_OBJ_PATH
	@if [ ! -d $(OBJ_DIR) ]; then \
	mkdir $(OBJ_DIR); \
	fi
endef

define MAKE_TARGET_ALIAS
	if [ ! -f $(TARGET_ALIAS) ]; then \
	$(LN) -s $(TARGET) $(TARGET_ALIAS); \
	fi
endef

.PHONY:	all clean install


