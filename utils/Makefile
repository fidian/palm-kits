# Utility program Makefile
#
# Copyright (C) 2002-2003 - Kit development team
# Licensed under the GNU GPL software license.
# See the docs/LEGAL file for more information
# See http://kits.sourceforge.net/ for more information about the library
#
# $Id$

CC = gcc
CCOPTS = -O2 -Wall



all:  bundle rip_bytes


bundle.o: bundle.c
	$(CC) $(CCOPTS) -c bundle.c
	
rip_bytes.o: rip_bytes.c
	$(CC) $(CCOPTS) -c rip_bytes.c
	

bundle: bundle.o
	$(CC) $(CCOPTS) -lz bundle.o -o bundle
	
rip_bytes: rip_bytes.o
	$(CC) $(CCOPTS) -o rip_bytes rip_bytes.o


clean:
	rm -f *~ *.o rip_bytes
