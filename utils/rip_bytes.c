/* File converter to change gzip compressed files into .bin resources for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 * 
 * $Id$
 */

#include <stdio.h>


int main(int argc, char **argv)
{
   FILE *infp;
   int i, gpflags, tmp[4];
   unsigned long bytes;
   
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

   tmp[0] = fgetc(infp);
   if (tmp[0] == -1)
     {
	// error reading from file
	fprintf(stderr, "error reading data from file\n");
	return 0;
     }
   tmp[1] = fgetc(infp);
   if (tmp[0] != 0x1f || tmp[1] != 0x8b) {
      fprintf(stderr, "Magic number mismatch 0x%02x%02x\n",
	      tmp[0], tmp[1]);
      return 20;
   }

   tmp[0] = fgetc(infp);
   if(tmp[0] != 8) {
      fprintf(stderr, "Unknown compression method: 0x%02x\n", tmp[0]);
      return 20;
   }

   gpflags = fgetc(infp);
   if ((gpflags & ~0x1f)) {
      fprintf(stderr, "Unknown flags set!\n");
   }

   /* Skip file modification time (4 bytes) */
   fgetc(infp);
   fgetc(infp);
   fgetc(infp);
   fgetc(infp);
   /* Skip extra flags and operating system fields (2 bytes) */
   fgetc(infp);
   fgetc(infp);

   if ((gpflags & 4)) {
      /* Skip extra field */
      tmp[0] = fgetc(infp);
      tmp[1] = fgetc(infp);
      i = tmp[0] + 256*tmp[1];
      while (i--) 
	{
	   fgetc(infp);
	}
   }
   if((gpflags & 8)) {
      while((fgetc(infp)) > 0) {
      }
   }
   if((gpflags & 16)) {
      while((fgetc(infp)) > 0) {
      }
   }
   if((gpflags & 2)) {
      /* Skip CRC16 */
      fgetc(infp);
      fgetc(infp);
   }

   // Determine number of bytes to read
   // Original size - amount we already read - CRC and size at end = bytes
   // that we need to read
   bytes = bytes - ftell(infp) - 8;
   if (bytes > 32767)
     {
	fprintf(stderr, "Warning:  Resulting file will be too large\n");
     }
   
   while (bytes --)
     fputc(fgetc(infp), stdout);
   
   return 0;
}
