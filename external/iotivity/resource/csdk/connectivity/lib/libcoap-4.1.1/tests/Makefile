# Makefile for libcoap
#
# Copyright (C) 2012 Olaf Bergmann <bergmann@tzi.org>
#
# This file is part of the CoAP library libcoap. Please see
# README for terms of use. 

# the library's version
VERSION:=4.1.1

# tools

SHELL = /bin/sh
MKDIR = mkdir

abs_builddir = /connectivity/connectivity/src/bt_edr_adapter/libcoap-4.1.1/tests
top_builddir = ..
top_srcdir = ..
# files and flags
ifneq (,)
  PROGRAMS:=testdriver
endif
SOURCES:= test_uri.c test_options.c test_pdu.c test_error_response.c \
  test_sendqueue.c test_wellknown.c
HEADERS:=$(SOURCES:.c=.h)
OBJECTS:= $(patsubst %.c, %.o, $(SOURCES))
CFLAGS:=-g -Wall -g -O2
CPPFLAGS:=-I$(top_srcdir)  -DWITH_POSIX
DISTDIR?=$(top_builddir)/libcoap-4.1.1
FILES:=Makefile.in testdriver.c $(HEADERS) $(SOURCES)
LDFLAGS:=-L$(top_builddir) 
LDLIBS:=-lcunit 
libcoap =$(top_builddir)/libcoap.a

.PHONY: clean distclean

.SUFFIXES:
.SUFFIXES:      .c .o

all:	$(PROGRAMS)

check:	
	echo DISTDIR: $(DISTDIR)
	echo top_builddir: $(top_builddir)

testdriver:	$(OBJECTS) $(libcoap)

clean:
	@rm -f $(PROGRAMS) $(OBJECTS)

distclean:	clean
	@rm -rf $(DISTDIR)
	@rm -f *~ 

dist:	$(FILES)
	test -d $(DISTDIR)/tests || mkdir $(DISTDIR)/tests
	cp $(FILES) $(DISTDIR)/tests
