#ifdef USE_ZLIB_STRUCT
#include "SysZLib.h"  // For PalmOS.  Change to your liking.
#else
typedef struct z_stream_s
{
   unsigned char *next_in;  // Next input byte (CHANGES)
   unsigned int avail_in;  // number of bytes left at next_in
   unsigned char *next_out;  // Next output byte goes here (CHANGES)
   unsigned int avail_out;  // number of bytes left at next_out
   unsigned int reserved;  // For the readbit() and readbits() functions
                           // Bigger than 1 byte
} z_stream;
#endif

int InflateData(z_stream *zs);
