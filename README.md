Palm Kits
=========

A Kit is a Palm OS program that contains one or more compressed programs, databases, or PQAs.  The Kit will decompress itself, without requiring external libraries, into the bigger programs.  This has several benefits:

* A Kit will HotSync faster because it has fewer, larger records.  The HotSync protocol sends a record at a time, and that can be very slow for large databases with small records.
* You can beam multiple things all at once.  Send an application, sample data, documentation, and more in just one file.
* The decompression library is optimized for size, requiring as little memory as possible.  At the most, the amount of memory used is 32k for a temporary buffer + the size of the Kit + the size of all extracted applications.


Features
--------

* The decompression library is just under 2k of compiled code, which can easily be used in your application if you need some tight decompression code when zlib is not around.
* Free to use Kit to bundle your applications.  Kit is under the GPL.
* Works on 32k chunks of decompressed data at a time for a high compression ratio.

There's a lot more information about the [gunzip](GUNZIP.md) routines that Kit uses.  They could be useful for many embedded and constrained devices.  It's the smallest decompression code I was able to find/make.


Creator IDs
-----------

Every program on the Palm has a unique Creator ID.  It is used by the operating system to keep things straight.  This applies to Kits as well, because they are also programs.

When you write a program for Palm, you need to register your Creator ID on [Palm's Developer Site](http://dev.palmos.com/creatorid/).  Likewise, if you make a Kit and you want to distribute it, you must change the Creator ID.  _Do not use the default Creator ID!_  Change 'Fid8' in the Makefile for Kit to be your new Creator ID.  It is at the top of kit/Makefile, like this:

    CREATOR_ID = Fid8

It is also in the .rcp file:

    APPLICATION ID ApplicationId "Fid8"

Before you distribute the application, you must change that to the Creator ID that you registered.  Additionally, you will likely not want the program called "Kit", so change the line in the Makefile:

    APP_NAME = Kit

This is also in the .rcp file:

    APPLICATIONICONNAME ID AppIconNameID "Kit"

Once you change those things, it is safe to distribute your application.


To Do
-----

* Add a "Cancel" button to stop decompression.
* A couple flags don't work.
* Most of the Kit flags don't work yet, like decompress on HotSync, delete when done, etc.
* Use ZLib if it found (for speed), otherwise use standard decompression routines.
* Write Kit Pro (used to create Kits on the Palm)


License
-------

Kit is under a GPL license.  See [COPYING](COPYING) for the license text.


Credits and Thanks
------------------

I did actually write all code from scratch, except the inflate routines were initially from Pasi Ojala's code.  Credit should also be given to people who wrote other open-source programs that I learned from.  Also, this list is to give thanks to people who have helped me out with Kit.  Lastly, I also show appreciation to companies and anything else that helped me out with writing this program.

* Ojala, Pasi -- Wrote [gunzip.c](http://www.cs.tut.fi/~albert/Dev/gunzip/gunzip.c), from which I altered into the inflate routines used in Kit.
* Zerucha, Tom -- Wrote [palmzlib](http://palmzlib.sourceforge.net/) and [boxer / ZBoxZ](http://palmboxer.sourceforge.net/).  He initially wrote crater, which was excactly what I needed to base Kit off of, but it did not compress databases.  So I took the idea and started from scratch.


Kit (The Program)
-----------------

Kit is the main decompressing program.  It will take compressed records from within the program and write them to the handheld.  It does not rely upon ZLib for decompression -- the inflate code is built-in.

Kit's user interface is designed to be simple.  There is a list of files that are contained in the Kit, so you can select what you want to extract.  When ready, one tap will start the decompression.  Many aspects can be tweaked with the program -- particular files can be pre-selected to be extracted, the selection screen can be entirely bypassed, files can be skipped/overwritten automatically, and other prompts can be removed.

Right now, you need to follow these steps in order to build a kit from the command line:

1. Copy the .prc, .pdb, and .pqa files that you want to bundle into a Kit into the kit/bundles directory
2. Register a Creator ID with Palm for the Kit
3. Edit the Makefile and change the Creator ID and the Application's Name
4. Edit the kit.rcp file and change the Creator ID and the Application's name.  Also, edit any wording that doesn't suit you.
5. `make`


KitTest
-------

KitTest is a sample program to show how fast decompression is on the Palm with regard to ZLib.  If other Palm libraries are written, or if other gunzip code is submitted, this program will benchmark those tests as well.  KitTest is also a great program if you want to learn how to use ZLib or the Kit gunzip routine in your program.  For ZLib, it shows library loading, initialization, decompression, cleanup, and library releasing.  For Kit's gunzip, it shows the setup and how to call the single function to decompress the data.

KitTest runs in one of 4 modes:

* Run Kit's gunzip routine for X seconds and count the iterations
* Run Zlib's gunzip routine for X seconds and count the iterations
* Run both gunzip routines for X seconds and count the iterations (very useful for comparing one against the other)
* Run Kit's gunzip routine three times (very useful for profiling to see where the time is being consumed)

Since this is not really an end-user program, there is more information
in the [kittest/](kittest/) directory and even a few comments in the source.
