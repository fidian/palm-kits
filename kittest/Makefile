# KitTest Makefile
#
# Copyright (C) 2002-2003 - Kit development team
# Licensed under the GNU GPL software license.
# See the docs/LEGAL file for more information
# See http://kits.sourceforge.net/ for more information about the library
#
# $Id$

# Uncomment this if you want debugger information added to the program
#DEBUG = -mdebug-labels

DEFINES = -DUSE_ZLIB_STRUCT
ZLIB_INCLUDE = -I../palmzlib/ -DNEWGCC
O_FILES = kittest.o inflate.o
BMP_FILES = icon.bmp icon2.bmp
CC = m68k-palmos-gcc
PILRC =	pilrc -q
BUILD-PRC = build-prc
CCOPTS = -O1 -Wno-char-subscripts -Wall $(ZLIB_INCLUDE) $(DEBUG) $(DEFINES)
UTILS_DIR = ../utils


all:  kittest.prc

install: kittest.prc
	pilot-xfer -i kittest.prc


inflate.o: ../gunzip/inflate.c ../gunzip/inflate.h
	$(CC) $(CCOPTS) -c ../gunzip/inflate.c -o inflate.o

kittest.h: kittest.rcp
	$(UTILS_DIR)/gen_id.pl < kittest.rcp > kittest.h_new
	$(UTILS_DIR)/check_h kittest
	
kittest.o: kittest.c kittest.h ../gunzip/inflate.h version.h
	$(CC) $(CCOPTS) -c kittest.c


$(UTILS_DIR)/rip_bytes:
	(cd $(UTILS_DIR) ; make rip_bytes)

KITd0001.src: data $(UTILS_DIR)/rip_bytes
	cat data | gzip -9 > data.gz
	$(UTILS_DIR)/rip_bytes data.gz > KITd0001.src
	rm data.gz


version.h: kittest.c kittest.h ../gunzip/inflate.h ../gunzip/inflate.c \
	   KITd0001.src $(BMP_FILES) kittest.rcp
	$(UTILS_DIR)/gen_version kittest > version.h


kittest.prc: $(O_FILES) $(BMP_FILES) version.h kittest.rcp kittest.h \
	     KITd0001.src
	$(PILRC) kittest.rcp > /dev/null
	$(CC) $(CCOPTS) $(O_FILES) -o kittest
	cp KITd0001.src KITd0001.bin
	$(BUILD-PRC) -o kittest.prc kittest.def kittest *.bin
	ls -l *.prc
	rm -f *.bin
	pilot-file -l kittest.prc | grep code

clean:
	rm -f *.bin kittest *.o *~ *.gif images/*.gif version.h \
	    kittest.h KITd0001.src
