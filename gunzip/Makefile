CC=gcc
CCFLAGS=-O2 -Wall

all: gunzip just_inflate


crc32.o: crc32.c crc32.h
	$(CC) $(CCFLAGS) -c -o crc32.o crc32.c
	
gunzip.o: gunzip.c crc32.h inflate.h
	$(CC) $(CCFLAGS) -c -o gunzip.o gunzip.c
	
inflate.o: inflate.c inflate.h
	$(CC) $(CCFLAGS) -c -o inflate.o inflate.c

just_inflate.o: just_inflate.c inflate.h


gunzip: gunzip.o crc32.o inflate.o
	$(CC) -o gunzip gunzip.o crc32.o inflate.o $(CCFLAGS)
	
just_inflate: just_inflate.o inflate.o
	$(CC) -o just_inflate just_inflate.o inflate.o $(CCFLAGS)


clean:
	rm -f *.o *~ gunzip just_inflate