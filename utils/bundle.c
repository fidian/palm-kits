/* Bundle .prc, .pdb, and .pqa files together for compiling into a Kit
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 * 
 * $Id$
 */

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>

int verbose;

// Compress and write one chunk of data to the specified file
int WriteChunk(char *data, unsigned int len, unsigned int chunk)
{
   char filename[13];
   z_stream zs;
   char OutData[32768 + 5];
   FILE *fp;
   
   sprintf(filename, "KITd%04x.bin", chunk);
   if (verbose)
     printf("Writing chunk to %s\n", filename);
   
   memset(&zs, 0, sizeof(z_stream));
//   if (deflateInit(&zs, 9) != Z_OK)
   if (deflateInit2(&zs, 9, Z_DEFLATED, -15, 8, 0) != Z_OK)
     {
	fprintf(stderr, "Problem initializing deflation buffers.\n");
	return -1;
     }
   
   zs.next_in = data;
   zs.avail_in = len;
   zs.next_out = OutData;
   zs.avail_out = 32768 + 5;
   if (deflate(&zs, Z_FINISH) != Z_STREAM_END)
     {
	fprintf(stderr, "Problem compressing data\n");
	return -2;
     }
   
   fp = fopen(filename, "wb");
   if (! fp)
     {
	fprintf(stderr, "Problem opening %s\n", filename);
	return -3;
     }
   
   fwrite(OutData, 1, zs.total_out, fp);
   fclose(fp);
   
   return 0;
}


// Very simple program.
// Create the header resource (KITd0000.bin)
// Create other resources with zlib (KITd____.bin)
int main(int argc, char **argv)
{
   int start_arg_at;
   FILE *hdr_rsrc, *fp;
   unsigned int i, chunk;
   char filename[32];
   size_t size;
   unsigned char Data[32768];
   unsigned int file_flags;
   
   // Worst case scenario: the data is stored, adding 5 bytes.
   start_arg_at = 1;
   while (start_arg_at < argc && strcmp(argv[start_arg_at], "-v") == 0)
     {
	verbose ++;
	start_arg_at ++;
	printf("Verbose mode!\n");
     }
   
   
   // If no arguments, write nothing.
   // If only the first argument is specified, write nothing.
   if (argc < start_arg_at + 2)
     {
	fprintf(stderr, "Not writing any files.\n");
	return 0;
     }
   
   hdr_rsrc = fopen("KITd0000.bin", "wb");
   if (! hdr_rsrc)
     {
	fprintf(stderr, "Unable to open KITd0000.bin!\n");
	return -1;
     }
   
   // Write the global data portion of the header
   // First, the flags
   i = atoi(argv[start_arg_at ++]);
   if (verbose)
     printf("Flags:  0x%02x\n", i);
   fputc(i, hdr_rsrc);
   // Now the number of files
   fputc(argc - start_arg_at, hdr_rsrc);
   // And a couple bytes to spare
   fputc(0, hdr_rsrc);
   fputc(0, hdr_rsrc);

   chunk = 0;
   // Process each of the files
   for (i = start_arg_at; i < argc; i ++)
     {
	file_flags = 0x01;
	fp = fopen(argv[i], "rb");
	
	// If the file name starts with a '-', remove the '-' and set the
	// flags to not select this file automatically for decompression
	if (! fp && argv[i][0] == '-')
	  {
	     fp = fopen(&(argv[i][1]), "rb");
	     file_flags = 0x00;
	  }
	
	if (! fp)
	  {
	     fprintf(stderr, "Problem opening file %s\n", argv[i]);
	     return -2;
	  }
	
	// Copy the program/database name
	fread(filename, 1, 32, fp);
	filename[31] = '\0';  // In case the prc/pdb is created incorrectly
	fwrite(filename, 1, 32, hdr_rsrc);
	if (verbose)
	  printf("Compressing %s\n", filename);
	
	// Get ready to deflate
	fseek(fp, 0, SEEK_SET);
	size = fread(Data, 1, 32768, fp);
	     
	// Deflate the file in chunks.  32k max per uncompressed chunk.
	while (size > 0)
	  {
	     chunk ++;
	     if (WriteChunk(Data, size, chunk))
	       {
		  return -5;
	       }
	     
	     size = fread(Data, 1, 32768, fp);
	  }
	
	fputc(chunk, hdr_rsrc);
	fputc(file_flags, hdr_rsrc);
	fputc(0, hdr_rsrc);
	fputc(0, hdr_rsrc);
	
	fclose(fp);
     }
   
   if (verbose)
     printf("Finishing header.\n");
   fclose(hdr_rsrc);
   
   return 0;
}
