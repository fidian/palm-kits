#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "crc32.h"
#include "inflate.h"


int READBYTE(z_stream *zs) {
   if (zs->avail_in <= 0)
     return -1;

   zs->avail_in --;
   return *(zs->next_in ++);
}


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
   
   tmp[0] = READBYTE(&zs);
   if (tmp[0] == -1)
     {
	// error reading from file
	fprintf(stderr, "error reading data from file\n");
	return 0;
     }
   tmp[1] = READBYTE(&zs);
   
   if (tmp[0] != 0x1f || tmp[1] != 0x8b) {
      fprintf(stderr, "Magic number mismatch 0x%02x%02x\n",
	      tmp[0], tmp[1]);
      return 20;
   }

   tmp[0] = READBYTE(&zs);
   if(tmp[0] != 8) {
      fprintf(stderr, "Unknown compression method: 0x%02x\n", tmp[0]);
      return 20;
   }

   gpflags = READBYTE(&zs);
   if ((gpflags & ~0x1f)) {
      fprintf(stderr, "Unknown flags set!\n");
   }

   /* Skip file modification time (4 bytes) */
   READBYTE(&zs);
   READBYTE(&zs);
   READBYTE(&zs);
   READBYTE(&zs);
   /* Skip extra flags and operating system fields (2 bytes) */
   READBYTE(&zs);
   READBYTE(&zs);

   if ((gpflags & 4)) {
      /* Skip extra field */
      tmp[0] = READBYTE(&zs);
      tmp[1] = READBYTE(&zs);
      i = tmp[0] + 256*tmp[1];
      while (i--) 
	{
	   READBYTE(&zs);
	}
   }
   if((gpflags & 8)) {
      while((READBYTE(&zs))) {
      }
   }
   if((gpflags & 16)) {
      while((READBYTE(&zs))) {
      }
   }
   if((gpflags & 2)) {
      /* Skip CRC16 */
      READBYTE(&zs);
      READBYTE(&zs);
   }

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
   
   crc = READBYTE(&zs);
   crc |= (READBYTE(&zs)<<8);
   crc |= (READBYTE(&zs)<<16);
   crc |= (READBYTE(&zs)<<24);
   
   size = READBYTE(&zs);
   size |= (READBYTE(&zs)<<8);
   size |= (READBYTE(&zs)<<16);
   size |= (READBYTE(&zs)<<24);
   
   InflatedSize = 65535 - zs.avail_out;
   InflatedCRC = MakeCRC32(outData, InflatedSize, 0);
   fprintf(stderr, "CRC:  %08lx %08lx %s\n", crc, InflatedCRC,
	   (crc != InflatedCRC)?"**error**":"");
   fprintf(stderr, "Size: %08lx %08lx %s\n", size, InflatedSize, 
	   (size != InflatedSize)?"**error**":"");
   
   return 0;
}
