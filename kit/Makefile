# Documentation for Kits
#
# Copyright (C) 2002-2003 - Kit development team
# Licensed under the GNU GPL software license.
# See the docs/LEGAL file for more information
# See http://kits.sourceforge.net/ for more information about the library
#
# $Id$

# Set this to the proper Creator ID.  More information about this in the
# documentation
CREATOR_ID = Fid8

# Set this to be the application's name.  More information about this in the
# documentation
APP_NAME = Kit

# DEBUG = -mdebug-labels -g
DEFINES = -DUSE_ZLIB_STRUCT
ZLIB_INCLUDE = -I../palmzlib/ -DNEWGCC
O_FILES = inflate.o kit.o
BMP_FILES = icon.bmp icon2.bmp
CC = m68k-palmos-gcc
CCOPTS = -O1 -Wno-char-subscripts -Wall $(ZLIB_INCLUDE) $(DEBUG) $(DEFINES)
PILRC =	pilrc -q
BUILD-PRC = build-prc
UTILS_DIR = ../utils
BUNDLE = $(UTILS_DIR)/bundle
BUNDLE_FLAGS = 0


all:  kit.prc


install: kit.prc
	pilot-xfer -i kit.prc


inflate.o: ../gunzip/inflate.c ../gunzip/inflate.h
	$(CC) $(CCOPTS) -c ../gunzip/inflate.c -o inflate.o

kit.def: version.h
	echo "application { \"$(APP_NAME)\" $(CREATOR_ID) backup }" > kit.def
	
kit.h: kit.rcp
	$(UTILS_DIR)/gen_id.pl < kit.rcp > kit.h_new
	$(UTILS_DIR)/check_h kit
	
kit.o: kit.c kit.h ../common/kitdef.h ../gunzip/inflate.h version.h
	$(CC) $(CCOPTS) -c kit.c -DCREATOR_ID="'$(CREATOR_ID)'"


version.h: kit.c kit.h ../gunzip/inflate.h ../gunzip/inflate.c \
	   $(BMP_FILES) kit.rcp
	$(UTILS_DIR)/gen_version kit > version.h


$(BUNDLE):
	$(MAKE) -C $(UTILS_DIR)


kit.prc: $(O_FILES) $(BMP_FILES) version.h kit.rcp kit.h $(BUNDLE) kit.def
	$(PILRC) kit.rcp > /dev/null
	$(CC) $(CCOPTS) $(O_FILES) -o kit
	$(BUNDLE) $(BUNDLE_FLAGS) bundles/*
	$(BUILD-PRC) -o kit.prc kit.def kit *.bin
	rm -f *.bin
	
kit_only: $(O_FILES) $(BMP_FILES) version.h kit.rcp kit.h kit.def
	$(PILRC) kit.rcp > /dev/null
	$(CC) $(CCOPTS) $(O_FILES) -o kit
	$(BUILD-PRC) -o kit.prc kit.def kit *.bin
	rm -f *.bin

clean:
	rm -f *.bin kit *.o *~ *.gif images/*.gif version.h \
	    kit.h KITd0001.src kit.def
