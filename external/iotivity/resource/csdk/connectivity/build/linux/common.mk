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
PROJECT_COMMON_INC_PATH		= $(PROJECT_ROOT_PATH)/common/inc
PROJECT_COMMON_SRC_PATH		= $(PROJECT_ROOT_PATH)/common/src
PROJECT_COMMON_PATH			= $(PROJECT_ROOT_PATH)/common
PROJECT_OUT_PATH			= $(PROJECT_ROOT_PATH)/build/out
PROJECT_LIB_PATH			= $(PROJECT_ROOT_PATH)/lib

##
##	macro
##
define	MAKE_PROJECT_OUT_PATH
	@if	[ ! -d $(PROJECT_OUT_PATH) ]; then	\
	mkdir $(PROJECT_OUT_PATH);	\
	fi
endef


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

