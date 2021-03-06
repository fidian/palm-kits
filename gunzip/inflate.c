/* inflate routines for Palm OS (to inflate a deflated stream)
 * 
 * Based heavily upon gunzip.c by Pasi Ojala <albert@cs.tut.fi>
 *   http://www.cs.tut.fi/~albert/Dev/gunzip/
 * Many, many thanks for that code!
 * 
 * Changes:
 *  2002-12-30 - Added #defines to use zlib's struct instead of mine, just
 *               in case you want to compile it that way.
 *  2002-12-29 - Found out that this is VERY slow.  ZLib is 3x faster.
 *               Worked on speeding it up.  Partially successful.  Profiled
 *               code and marked critical areas.  1932 bytes added to a
 *               Palm program by linking in the .o file.  Schweet!
 *  2002-12-21 - Finished surgery.  Only one function to inflate dynamic and
 *               fixed data.  Rewrote table generation to be iterative.
 *               Hacked and slashed my way through unnecessary code.
 *               The size is now down to 3512 bytes.
 *  2002-12-20 - Started major surgery
 *  2002-12-19 - Looked at the code a bit more
 *  2002-12-17 - Continued work.  Down to about 6k for the .o file
 *  2002-12-09 - Continued work, added z_stream_fid instead of globals
 *  2002-12-08 - Started work so that it decompresses up to 64k (one memory
 *               chunk on the Palm.
 *  2002-12-07 - Removed bit reverse table.  Removed unzip code.
 */

#include "inflate.h"
//#include <stdio.h>    // Just in case you put a printf() function back in

void Status(char *, int);

typedef struct HufNode_struct {
   // b0 and b1 are either values in the tree array to jump to (branches)
   // or are literal values.
   // if (bX & 0x8000)
   //     value = bX ^ 0x8000;
   // else
   //     link_to_array_element = bX;
   unsigned int b0;  // Bigger than 1 byte (2 is ideal)
   unsigned int b1;  // Bigger than 1 byte (2 is ideal)
} HufNode;


// Not that rhobust anymore -- If out of data, this will
// return a whole lot of 1 bits.
// 
// This function consumes a large percentage of time (#1)
char READBIT(z_stream *zs) 
{
   char carry;

   if (zs->reserved == 1)
     {
	if (zs->avail_in == 0)
	  return 1;
	zs->reserved = *(zs->next_in ++) | 0x0100;
	zs->avail_in --;
     }
   
   carry = zs->reserved & 1;
   zs->reserved >>= 1;
   
   return carry;
}


// Make sure that [a] is <= 16
// If there are endian problems, force [a] to be <= 8
// Might be faster if all (up to [a] bits) of zs->reserved was read into res 
// right away.
// 
// This function consumes a large percentage of time (#4)
int READBITS(z_stream *zs, int a) 
{
   int res = 0, pos = 0;
   
   while (a --) 
     {
	if (zs->reserved == 1)
	  {
	     if (zs->avail_in == 0)
	       return 1;
	     zs->reserved = *(zs->next_in ++) | 0x0100;
	     zs->avail_in --;
	  }
	
	res += (zs->reserved & 1) << pos;
	zs->reserved >>= 1;
	pos ++;
     }
   
   return res;
}


// Huffman tree structures, variables and related routines
// 
// These routines are one-bit-at-a-time decode routines. They
// are not as fast as multi-bit routines, but maybe a bit easier
// to understand and use a lot less memory.
//
// The tree is created in an array
//
// currentTree = where to put the tree (in an array)
// numval = Number of elements in the lengths array
// lengths = array of lengths
//
// 
// This function consumes a large percentage of time (#5)
int CreateTree(HufNode *currentTree, int numval, unsigned char *lengths) {
   int i, j, len;  // basically scratch values
   int BlankNode;  // Where is the next blank array index
   int this_code, mask, *bitData;  // used in tree generation
   int bl_count[16] = { 0, };  // Counter of code lengths
   int next_code[16] = { 0, };  // Code for a specific length
   // 16 = (15 is max length of a code when inflating) + (1 for zero)

   // Step 1:  Count the code lengths
   for (i = 0; i < numval; i ++)
     {
	j = lengths[i];
	if (j > 15)
	  return 1;
	bl_count[j] ++;
     }
   
   // Step 2:  Find numerical value of the smallest code of each length
   // Also note that I've inserted some weak validation code here.  I'm
   // not 100% sure that it is up to RFC specs, but it seems to work fine
   // in my tests
   // 
   // The validation theory is that at the root node, you have 2 branches or
   // values possible.  If you branch, you get 2 more potentials.  If you
   // get a value, you lose one potential.  So, if the root node has one of
   // each, the number of potentials at the next level is still two.  If that
   // node just has branches, the number of potentials is four.  If both of
   // the nodes on the following level just have values, the number of
   // potentials is 0, leaving us with a complete tree.
   // 
   // If I don't validate and if an invalid tree gets generated, an
   // infinite loop is possible
   bl_count[0] = 0;
   j = 0;
   len = 2;
   for (i = 1; i < 16; i ++)
     {
	len -= bl_count[i];
	len *= 2;
	j = (j + bl_count[i - 1]) << 1;
	next_code[i] = j;
     }
   if (len)
     return 1;
   
   
   // Step 3:  Assign numerical values to all codes
   BlankNode = 1;
   currentTree[0].b0 = 0x0000;
   currentTree[0].b1 = 0x0000;
   for (i = 0; i < numval; i ++)
     {
	len = lengths[i];
	if (len != 0)
	  {
	     this_code = next_code[len];
	     next_code[len] ++;
	     mask = 1 << (len - 1);
	     j = 0;
	     while (mask > 1)
	       {
		  if (this_code & mask)
		    bitData = &(currentTree[j].b1);
		  else
		    bitData = &(currentTree[j].b0);
		  
		  if (*bitData == 0x0000)
		    {
		       *bitData = BlankNode;
		       j = BlankNode;
		       BlankNode ++;
		       currentTree[j].b0 = 0x0000;
		       currentTree[j].b1 = 0x0000;
		    }
		  else
		    j = *bitData;
		  
		  mask >>= 1;
	       }
	     if (this_code & 0x01)
	       currentTree[j].b1 = 0x8000 | i;
	     else
	       currentTree[j].b0 = 0x8000 | i;
	  }
     }
   
#if 0
   fprintf(stderr, "%d table entries used\n",
	   BlankNode);
   if (numval < 20) {
      for (i = 0; i < BlankNode; i ++)
	{
	   fprintf(stdout, "0x%03x - ", i);
	   if (currentTree[i].b0 & 0x8000)
	     fprintf(stdout, "value: 0x%03x ", currentTree[i].b0 ^ 0x8000);
	   else
	     fprintf(stdout, " link: 0x%03x ", currentTree[i].b0);

	 if (currentTree[i].b1 & 0x8000)
	   fprintf(stdout, "value: 0x%03x\n", currentTree[i].b1 ^ 0x8000);
	 else
	   fprintf(stdout, " link: 0x%03x\n", currentTree[i].b1);
      }
   }
#endif
   
   return 0;
}


// Using the tree passed in, read bits from the data stream until we arrive
// at the proper value
// 
// This function consumes a large percentage of time (#2)
int DecodeValue(z_stream *zs, HufNode *currentTree) 
{
   unsigned int i = 0;
   
   // decode one symbol of the data per iteration
   // Infinite loop detection code could go here.  Maximum
   // bits to read is 15.
   while (i < 0x8000)
     {
	if (READBIT(zs)) 
	  i = currentTree[i].b1;
	else
	  i = currentTree[i].b0;
     }
   return i & 0x7FFF;
}


int Decompress_Stored(z_stream *zs)
{
   int blockLen, cSum;
   
#if 0
   fprintf(stderr, "Stored\n");
#endif
   
   zs->reserved = 1;;
   if (zs->avail_in < 4)
     return -1;
   
   zs->avail_in -= 4;
   blockLen = *(zs->next_in ++);
   blockLen |= *(zs->next_in ++) << 8;
   
   cSum = *(zs->next_in ++);
   cSum |= (*(zs->next_in ++) << 8);
   
   if ((blockLen + cSum) ^ 0xFFFF)
     return 1;
   if (zs->avail_in < blockLen || zs->avail_out < blockLen)
     return -1;
   zs->avail_in -= blockLen;
   zs->avail_out -= blockLen;
   
   while (blockLen --)
     {
	*(zs->next_out) = *(zs->next_in);
	zs->next_out ++;
	zs->next_in ++;
     }
   
   return 0;
}


int MakeTrees(z_stream *zs, char is_fixed, HufNode *literalTree,
		     HufNode *distanceTree)
{
   // Order of the bit length code lengths
   static const unsigned border[] = {
      16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
   unsigned char ll[288+32];
   int i, j, n, l, literalCodes, distCodes;
  
   if (is_fixed)
     {
	literalCodes = 288;

	// Set a large range to 8
	for (i = 0; i < 288; i ++)
	  ll[i] = 8;
	// In that range, set some to 9 and others to 7
	// (smaller code, but slightly slower table generation)
	for (i = 144; i < 256; i ++)
	  ll[i] = 9;
	for (; i < 280; i ++)
	  ll[i] = 7;

	distCodes = 32;
	
	for (i = 288; i < 320; i ++)
	  ll[i] = 5;
     }
   else
     {
	literalCodes = 257 + READBITS(zs, 5);
	distCodes = 1 + READBITS(zs, 5);
	l = 4 + READBITS(zs, 4);
	
	for (j = 0; j < 19; j ++) 
	  ll[j] = 0;
   
	// Get the decode tree code lengths
	// The decode tree is Huffman encoded
   
	for (j = 0; j < l; j++) 
	  {
	     ll[border[j]] = READBITS(zs, 3);
	  }
   
	if (CreateTree(distanceTree, 19, ll))
	   return 1;
	
	// read in literal and distance code lengths
	n = literalCodes + distCodes;
	i = 0;
	while (i < n) 
	  {
	     j = DecodeValue(zs, distanceTree);
	     if (j < 16)	// length of code in bits (0..15)
	       ll[i++] = j;
	     else if (j == 16) 
	       {	// repeat last length 3 to 6 times
		  j = 3 + READBITS(zs, 2);
		  
		  if (i + j > n) 
		    return 1;
	 
		  l = i ? ll[i-1] : 0;
		  while (j --) 
		    ll[i++] = l;
	       } 
	     else 
	       {
		  if (j == 17)		// 3 to 10 zero length codes
		    j = 3 + READBITS(zs, 3);
		  else		// j == 18: 11 to 138 zero length codes
		    j = 11 + READBITS(zs, 7);
		  
		  if (i + j > n) 
		       return 1;
	 
		  while (j --)
		    ll[i++] = 0;
	       }
	  }
     }
   
   
   // Can overwrite tree decode tree as it is not used anymore
   if (CreateTree(literalTree, literalCodes, &ll[0]))
     return 1;
   
   if(CreateTree(distanceTree, distCodes, &ll[literalCodes]))
     return 1;
   
   return 0;
}


// This function consumes a large percentage of time (#3)
// Most output produced by gzip/zlib/etc is dynamic.
int Decompress_DynamicOrFixed(z_stream *zs, char is_fixed)
{
   // Copy lengths for literal codes 257..285
   static const unsigned short cplens[] = {
      3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0 };
   // Extra bits for literal codes 257..285
   static const unsigned short cplext[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99 }; // 99==invalid
   // Copy offsets for distance codes 0..29
   static const unsigned short cpdist[] = {
      0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0009, 0x000d,
	0x0011, 0x0019, 0x0021, 0x0031, 0x0041, 0x0061, 0x0081, 0x00c1,
	0x0101, 0x0181, 0x0201, 0x0301, 0x0401, 0x0601, 0x0801, 0x0c01,
	0x1001, 0x1801, 0x2001, 0x3001, 0x4001, 0x6001 };
   // Extra bits for distance codes
   static const unsigned short cpdext[] = {
      0,  0,  0,  0,  1,  1,  2,  2,
	3,  3,  4,  4,  5,  5,  6,  6,
	7,  7,  8,  8,  9,  9, 10, 10,
	11, 11, 12, 12, 13, 13 };
   HufNode literalTree[288];
   HufNode distanceTree[32];
   int j, l, dist;

#if 0
   if (is_fixed)
     fprintf(stderr, "Fixed Huffman codes\n");
   else
     fprintf(stderr, "Dynamic Huffman codes\n");
#endif

   if (MakeTrees(zs, is_fixed, literalTree, distanceTree))
     return 1;
   
   while (1)
     {
	j = DecodeValue(zs, literalTree);
	if (j >= 256) 
	  {
	     if (j == 256)     // EOF
	       break;
	 
	     //printf("%04x ", j);
	     j -= 256 + 1;  // bytes + EOF
	 
	     l = READBITS(zs, cplext[j]) + cplens[j];
	     //printf("%04x ", l);

	     j = DecodeValue(zs, distanceTree);
	     //printf("%02x ", j);
	     
	     dist = READBITS(zs, cpdext[j]) + cpdist[j];
	     //printf("%04x ", dist);
	 
	     //printf("LZ77 len %d dist %d @%04x\n", l, dist, bIdx);
	     while(l--) 
	       {
		  //printf("%02x ", c);
		  if (! zs->avail_out --)
		    return -1;
		  
		  *(zs->next_out ++) = *(zs->next_out - dist);
	       }
	     //printf("\n");
	  } 
	else 
	  {
	     //printf("%02x\n", j);
	     if (! zs->avail_out --)
	       return -1;
	     *(zs->next_out ++) = (unsigned char) j;
	  }
     }
   return 0;
}


// Returns 0 if success
int InflateData(z_stream *zs) {
   int last, type;

   zs->reserved = 1;;
   
   do
     {
	last = READBIT(zs);
#if 0
	if (last)
	  fprintf(stderr, "Last Block: ");
	else 
	  fprintf(stderr, "Not Last Block: ");
#endif

	type = READBITS(zs, 2);
       
	if (type == 0) 
	  {
	     if (Decompress_Stored(zs))
	       return 1;
	  } 
	else if (type > 2)
	  {
#if 0	    
	     if (type == 3)
	       fprintf(stderr, "Reserved block type!!\n");
	     else  // the "else" should never happen
	       fprintf(stderr, "Unexpected value %d!\n", type);
#endif
	     zs->reserved = 1;;
	     return 1;
	  }
	else
	  {
	     if (Decompress_DynamicOrFixed(zs, type & 0x01))
	       return 1;
	  }
     } while(!last);

   zs->reserved = 1;;
   return 0;
}
