#include <stdio.h>
#include <zlib.h>

int main(int argc, char **argv)
{
   gzFile *gzfp;
   FILE *fp;
   char *filename;
   char *data;
   long size;
   
   if (argc < 2)
     {
	fprintf(stderr, "Syntax:  %s <filename>\n", argv[0]);
	fprintf(stderr, "This creates %s_s.gz, %s_f.gz and %s_d.gz\n",
		argv[0], argv[0], argv[0]);
	return 1;
     }
   
   fp = fopen(argv[1], "rb");
   if (! fp)
     {
	fprintf(stderr, "Problem opening %s\n", argv[1]);
	return 2;
     }
   
   filename = (char *) malloc(strlen(argv[1]) + 12);
   if (! filename)
     {
	fprintf(stderr, "Can't allocate memory\n");
	return 3;
     }
   
   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   data = (char *) malloc(size);
   if (! data)
     {
	fprintf(stderr, "Can't allocate memory for file\n");
	return 4;
     }
   fseek(fp, 0, SEEK_SET);
   fread(data, 1, size, fp);
   
   // Stored
   strcpy(filename, argv[1]);
   strcat(filename, "_s.gz");
   gzfp = gzopen(filename, "wb0");
   gzwrite(gzfp, data, size);
   gzclose(gzfp);

   // Fixed (or at least I hope that it is fixed -- often it produces
   // dynamically encoded data.)
   strcpy(filename, argv[1]);
   strcat(filename, "_f.gz");
   gzfp = gzopen(filename, "wb1f");
   gzwrite(gzfp, data, size);
   gzclose(gzfp);

   // Dynamic
   strcpy(filename, argv[1]);
   strcat(filename, "_d.gz");
   gzfp = gzopen(filename, "wb9h");
   gzwrite(gzfp, data, size);
   gzclose(gzfp);

   // These lines crash the program??
   // free(data);
   // free(filename);
   return 0;
}
