<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('Gunzip');

?>

<p>The gunzip routine was originaly written by Pasi Ojala 
&lt;<?PHP HideEmail('albert', 'cs.tut.fi') ?>&gt;.  He distributed it as
<a href="http://www.cs.tut.fi/~albert/Dev/gunzip/gunzip.c">gunzip.c</a>
on his <a href="http://www.cs.tut.fi/~albert/Dev/gunzip/">Gzip Decompressor
for C64</a> page.  This code was heavily modified primarily for size, and
partially for performance.  It is now in Kit as gunzip/inflate.c.  A list 
of the types of changes made to the original code is below.</p>

<ul>
<li>Removed bit reverse table in favor for a bit-shifting technique</li>
<li>Removed unzip code (we will deal exclusively with gzip streams)</li>
<li>Will decompress up to 64k in one pass, which is the limit for Palm OS
memory chunks (as far as I know)</li>
<li>Merged multiple inflate functions into one</li>
<li>Rewrote table generation to be iterative instead of recursive</li>
<li>Optimized the code to be small</li>
<li>Profiled code and further optimized various routines</li>
<li>Removed global data</li>
<li>Uses either ZLib's struct or a custom one -- good if you want to compile
your program to use ZLib if available or otherwise this slower code</li>
</ul>


<h3>Comparing vs. ZLib</h3>

<p>The decompression functions take under 2k of code and provide a lot for
that size.  If you have a program that decompresses and manipulates
compressed data, but does not have the need to compress data, then this
little bugger might be perfect for you.  It can decompress up to 64k of data
at a time (64k decompressed, not compressed).  It is only 1/3 the speed of
ZLib and it is about 1/15 the size of ZLib.</p>

<p>For some applications, this code is better than the hassle of using an
external library.  For others, it might be a good fall-back option in case 
ZLib wasn't installed.  If you are really creative, you could
(theoretically) have ZLib compressed and bundled in your program.  If you
don't find ZLib, you could decompress and install it on the fly, so that you
could have the [far superior] compression library.</p>

<table align=center border=1 cellpadding=5 cellspacing=0>
<tr><th>Kit's Code</th>
<th><a href="http://palmzlib.sourceforge.net">ZLib</a></th></tr>
<tr><td>15x smaller</td><td>3x faster</td></tr>
<tr><td>works in a pinch</td><td>can compress data</td></tr>
<tr><td>64k chunk limit (rarely an issue)</td><td>no chunk limit</td></tr>
<tr><td>compiles into your program</td><td>shared library so others can
benefit also</td></tr>
<tr><td>consumes as little memory as possible</td><td>based off the standard
ZLib</td></tr>
</table>


<h3>Using in Your Program</h3>

<p>If you want to use ZLib if it is available, or fall back on this code if
it is not available, you would alter your program along these lines:</p>

<ol>
<li>Get Kit's source code.  (See the <a href="download.php">download</a>
page.)</li>
<li>Copy out gunzip/inflate.c and gunzip/inflate.h to your directory</li>
<li>Any file that does inflating should include inflate.h instead of
SysZLib.h</li>
<li>Define USE_ZLIB_STRUCT when compiling inflate.c and all *.c files that
now include inflate.h</li>
<li>Alter your code to not die if ZLib is not found -- instead, you should
set a flag
<li>Where data is decompressed, follow this procedure:
  <ol>
  <li>If ZLib is loaded, use inflateInit(), inflate(), and inflateEnd()
  <li>Otherwise, use InflateData() -- it does not have an init or end function
  </ol></li>
<li>Compile inflate.c or inflate.o into your program when you build it</li>
<li>For a simple example on how to use InflateData() and how it compares to
ZLib, take a look at kittest/kittest.c</li>
</ol>

<p>If you merely want to use the decompression code from Kit in your
program, you would do something like this:</p>

<ol>
<li>Get Kit's source code.  (See the <a href="download.php">download</a>
page.)</li>
<li>Copy out gunzip/inflate.c and gunzip/inflate.h to your directory</li>
<li>Any file that does inflating should include inflate.h</li>
<li>When you want to decompress data, create the structure and call
InflateData() -- see kittest/kittest.c for more information</li>
<li>When you compile your code, compile and link in inflate.c or
inflate.o</li>
</ol>


<h3>How to Call</h3>

<p>So, you followed the instructions above, but you want a little more
information on how to call InflateData().  Understandable.</p>

<?PHP
ShowExample('
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
');
?>

<p>It is a big example, but it also gives some context as to what needs to
be set up and how to set those things up.  ZLib is a bit harder, but an
example is in kittest/kittest.c.</p>

<?PHP

StandardFooter();

?>
