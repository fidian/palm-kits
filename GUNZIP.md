Gunzip
======

The gunzip routine was originaly written by Pasi Ojala.  He distributed it as [gunzip.c](http://www.cs.tut.fi/~albert/Dev/gunzip/gunzip.c) on his [Gzip Decompressor for C64](http://www.cs.tut.fi/~albert/Dev/gunzip/) page.  This code was heavily modified primarily for size, and partially for performance.  It is now in Kit as [gunzip/inflate.c](gunzip/inflate.c).  A list of the types of changes made to the original code is below.

* Removed bit reverse table in favor for a bit-shifting technique
* Removed unzip code (we will deal exclusively with gzip streams)
* Will decompress up to 64k in one pass, which is the limit for Palm OS memory chunks (as far as I know)
* Merged multiple inflate functions into one
* Rewrote table generation to be iterative instead of recursive
* Optimized the code to be small
* Profiled code and further optimized various routines
* Removed global data
* Uses either ZLib's struct or a custom one -- good if you want to compile your program to use ZLib if available or otherwise this slower code


Comparing vs ZLib
-----------------

The decompression functions take under 2k of code and provide a lot for that size.  If you have a program that decompresses and manipulates compressed data, but does not have the need to compress data, then this little bugger might be perfect for you.  It can decompress up to 64k of data at a time (64k decompressed, not compressed).  It is only 1/3 the speed of ZLib and it is about 1/15 the size of ZLib.

For some applications, this code is better than the hassle of using an external library.  For others, it might be a good fall-back option in case ZLib wasn't installed.  If you are really creative, you could (theoretically) have ZLib compressed and bundled in your program.  If you don't find ZLib, you could decompress and install it on the fly, so that you could have the far superior compression library.

|               Kit's Code              |                     ZLib                    |
|:-------------------------------------:|:-------------------------------------------:|
|              15x smaller              |                  3x faster                  |
|            works in a pinch           |            can also compress data           |
|   64k chunk limit (rarely an issue)   |                no chunk limit               |
|       compiles into your program      |  shared library so others can also benefit  |
| consumes as little memory as possible | based off the standard ZLib used everywhere |


Using in Your Program
---------------------

If you want to use ZLib if it is available, or fall back on this code if it is not available, you would alter your program along these lines:

1. Get Kit's source code.
page.)
2. Copy out `gunzip/inflate.c` and `gunzip/inflate.h` to your directory
3. Any file that does inflating should include `inflate.h` instead of `SysZLib.h`
4. Define `USE_ZLIB_STRUCT` when compiling `inflate.c` and all `*.c` files that now include `inflate.h`
5. Alter your code to not die if ZLib is not found -- instead, you should set a flag
6. Where data is decompressed, follow this procedure:
    * If ZLib is loaded, use `inflateInit()`, `inflate()`, and `inflateEnd()`
    * Otherwise, use `InflateData()` -- it does not have an init or end function
7. Compile `inflate.c` or `inflate.o` into your program when you build it
8. For a simple example on how to use `InflateData()` and how it compares to ZLib, take a look at `kittest/kittest.c`

If you merely want to use the decompression code from Kit in your program, you would do something like this:

1. Get Kit's source code.
2. Copy out `gunzip/inflate.c` and `gunzip/inflate.h` to your directory
3. Any file that does inflating should include `inflate.h`
4. When you want to decompress data, create the structure and call `InflateData()` -- see `kittest/kittest.c` for more information
5. When you compile your code, compile and link in `inflate.c` or `inflate.o`


How to Call
-----------

So, you followed the instructions above, but you want a little more information on how to call `InflateData()`.  Understandable.

    z_stream zs;  // This is the structure that will be passed to InflateData
    char *data_packed, *data_unpacked;  // Buffers that will hold data
    Err error;  // error code

    // Copy compressed data to data_packed
    // Not necessary if you are going to use a pointer from
    // MemHandleLock()
    data_packed = MemPtrNew(xxx);
    MemMove(data_packed, source_data, xxx);

    // Create a decompressed buffer
    data_unpacked = MemPtrNew(yyy);  // Make sure it is big enough

    zs.avail_in = xxx;  // The number of compressed bytes available
    zs.next_in = data_packed;  // The compressed data
    zs.next_out = data_unpacked;  // Where you want decompressed data to go
    zs.avail_out = yyy;  // The size of the output buffer
    error = InflateData(zs);

    if (error)
    {
       // Alert the user, complain and die.  Data was corrupt.
    }

    // Done.
    // zs.avail_in should be 0
    // zs.next_in should be at data_packed + xxx
    // zs.next_out should be at data_unpacked + (yyy - zs.avail_out)
    // zs.avail_out should be (hopefully) less than yyy
    // Number of bytes written is yyy - zs.avail_out

It is a big example, but it also gives some context as to what needs to be set up and how to set those things up.  ZLib is a bit harder, but an example is in `kittest/kittest.c`.
