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

##
##	Definitions
##
SHELL						= /bin/bash
PROJECT_NAME				= connectivity_abstraction
PROJECT_ROOT_PATH			= ../..
PROJECT_API_PATH			= $(PROJECT_ROOT_PATH)/api
PROJECT_INC_PATH			= $(PROJECT_ROOT_PATH)/inc
PROJECT_SRC_PATH			= $(PROJECT_ROOT_PATH)/src
PROJECT_COMMON_PATH		= $(PROJECT_ROOT_PATH)/common
PROJECT_OUT_PATH			= $(PROJECT_ROOT_PATH)/build/out
COMPONENT_OUT_PATH		= ./out
COMPONENT_OBJ_PATH		= ./.objs

TARGET	= sample_main

##
##	Commands
##
CC			= gcc
CXX			= g++
RM			= rm -rf
CP			= cp
MV			= mv
AR			= ar
LD			= ld
LN			= ln
CD			= cd
RANLIB		= ranlib

##
##	file declaration
##
COMPONENT_SRCS			=  sample_main.c	\

COMPONENT_OBJS			= $(COMPONENT_SRCS:%.c=$(COMPONENT_OBJ_PATH)/%.o)

##
##	compiler flags
##
LIBS				= -L$(PROJECT_OUT_PATH) -lconnectivity_abstraction

CFLAGS				= -g -c -Wall -fPIC

LFLAGS				= -ldl -lpthread

IFLAGS				= -I$(PROJECT_API_PATH)	\

DFLAGS				= -DLINUX -DTB_LOG

##
##	compile and link rules
##
vpath %.c ./

$(COMPONENT_OBJ_PATH)/%.o: %.c
	@$(MAKE_OBJ_PATH)
	$(CC) -o $@ $(CFLAGS) $(DFLAGS) $(IFLAGS) $<

all : $(TARGET)
	@echo	" "
	@echo	"======================"
	@echo	"	Success!!"
	@echo	"======================"

$(TARGET) : $(COMPONENT_OBJS)
	@$(MAKE_COMPONENT_OUT_PATH)
	$(CC) -o $@ $(COMPONENT_OBJS) $(LIBS) $(LFLAGS)
	@$(MV) $(TARGET) $(COMPONENT_OUT_PATH)/.

clean :	
	@$(RM) $(COMPONENT_OBJ_PATH) \
			$(COMPONENT_OUT_PATH)

install :
	@$(MAKE_COMPONENT_OUT_PATH)
	@$(CP) $(PROJECT_OUT_PATH)/* $(COMPONENT_OUT_PATH)/.

##
##	macros
##
define MAKE_OBJ_PATH
	@if [ ! -d $(COMPONENT_OBJ_PATH) ]; then \
	mkdir $(COMPONENT_OBJ_PATH); \
	fi
endef


##
##	macro
##
define	MAKE_COMPONENT_OUT_PATH
	@if	[ ! -d $(COMPONENT_OUT_PATH) ]; then	\
	mkdir $(COMPONENT_OUT_PATH);	\
	fi
endef

.PHONY:	all clean install


