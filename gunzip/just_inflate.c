#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "crc32.h"
#include "inflate.h"


int main(int argc, char **argv)
{
   FILE *infp;
   int i, gpflags, tmp[4];
   unsigned long size, crc;
   unsigned char *fileData, *outData;
   unsigned long bytes, InflatedSize, InflatedCRC, speed_count;
   z_stream zs, zs_speed;
   time_t start_time;
   
   if (argc < 2)
     {
	fprintf(stderr, "specify filename!\n");
	return -1;
     }

   infp = fopen(argv[1], "rb");
   if (!infp) 
     {
	fprintf(stderr, "Could not open file %s for reading\n",
		argv[1]);
	return 20;
     }
   fseek(infp, 0, SEEK_END);
   bytes = ftell(infp);
   fseek(infp, 0, SEEK_SET);
   fileData = malloc(bytes);
   fread(fileData, 1, bytes, infp);
   outData = malloc(65535);
   
   zs.next_in = fileData;
   zs.avail_in = (unsigned int) bytes;
   zs.next_out = outData;
   zs.avail_out = 65535;

   if (argc < 3)
     {
	// normal test
	if (InflateData(&zs))
	  {
	     fprintf(stderr, "Problem during decompression!\n");
	     return 0;
	  }
     }
   else
     {
	// speed test
	fprintf(stderr, "SPEED TEST!\n");
	time(&start_time);
	speed_count = 0;
	while (time(NULL) - start_time < 20)
	  {
	     zs_speed.next_in = zs.next_in;
	     zs_speed.next_out = zs.next_out;
	     zs_speed.avail_in = zs.avail_in;
	     zs_speed.avail_out = zs.avail_out;
	     if (InflateData(&zs_speed))
	       {
		  fprintf(stderr, "Problem during decompression!\n");
		  return 0;
	       }
	     speed_count ++;
	  }
	fprintf(stderr, "speed count %ld\n", speed_count);
	zs.next_in = zs_speed.next_in;
	zs.next_out = zs_speed.next_out;
	zs.avail_in = zs_speed.avail_in;
	zs.avail_out = zs_speed.avail_out;
     }
   
   return 0;
}
