<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('KitTest');

?>

<p>KitTest is a sample program to show how fast decompression is on the Palm
with regard to ZLib.  If other Palm libraries are written, or if other
gunzip code is submitted, this program will benchmark those tests as well.
KitTest is also a great program if you want to learn how to use ZLib or the
Kit gunzip routine in your program.  For ZLib, it shows library loading,
initialization, decompression, cleanup, and library releasing.  For Kit's
gunzip, it shows the setup and how to call the single function to decompress
the data.</p>

<p>KitTest runs in one of 4 modes:</p>

<ul>
<li>Run Kit's gunzip routine for X seconds and count the iterations</li>
<li>Run Zlib's gunzip routine for X seconds and count the iterations</li>
<li>Run both gunzip routines for X seconds and count the iterations (very
useful for comparing one against the other)</li>
<li>Run Kit's gunzip routine three times (very useful for profiling to see
where the time is being consumed)</li>
</ul>

<p>Since this is not really an end-user program, there is more information
in the kittest/ directory and even a few comments in the source.</p>

<?PHP

StandardFooter();

?>
