<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('Main');

?>

<p>A Kit is a Palm OS program that contains one or more compressed programs,
databases, or PQAs.  The Kit will decompress itself, without requiring
external libraries, into the bigger programs.  This has several benefits:</p>

<ul>
<li>A Kit will HotSync faster because it has fewer, larger records.  The
HotSync protocol sends a record at a time, and that can be very slow for
large databases with small records.</li>
<li>You can beam multiple things all at once.  Send an application, sample
data, documentation, and more in just one file.</li>
<li>The decompression library is optimized for size, requiring as little
memory as possible.  At the most, the amount of memory used is 32k for a
temporary buffer + the size of the Kit + the size of all extracted
applications.</li>
</ul>

<h3>Features</h3>

<ul>
<li>The decompression library is just under 2k of compiled code, which can
easily be used in your application if you need some tight decompression 
code.<li>
<li>Free to use Kit to bundle your applications.  (GPL)</li>
<li>Works on 32k chunks of decompressed data at a time for a high
compression ratio.</li>
</ul>

<h3>To Do</h3>

<ul>
<li>Add a "Cancel" button to stop decompression.</li>
<li>Most of the Kit flags don't work yet, like decompress on
HotSync, delete when done, etc.</li>
<li>Use ZLib if it found (for speed), otherwise use standard decompression
routines.</li>
<li>Be able to select which databases to decompress.</li>
<li>Write Kit Pro (used to create Kits on the Palm)</li>
</ul>

<?PHP

StandardFooter();

?>
