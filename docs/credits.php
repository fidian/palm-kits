<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('Credits and Thanks');

?>

<p>I did actually write all code from scratch, except the inflate routines 
were initially from Pasi Ojala's code.  Credit should also be given to
people who wrote other open-source programs that I learned from.  Also, this
list is to give thanks to people who have helped me out with Kit.  Lastly,
I also show appreciation to companies and anything else that helped me out
with writing this program.</p>

<ul>
<li>Ojala, Pasi &lt;<?PHP HideEmail('albert', 'cs.tut.fi') ?>&gt; --
Wrote <a href="http://www.cs.tut.fi/~albert/Dev/gunzip/gunzip.c">gunzip.c</a>,
from which I altered into the inflate routines used in Kit.</li>
<li>Zerucha, Tom &lt;<?PHP HideEmail('tz', 'execpc') ?>&gt; -- Wrote 
<a href="http://palmzlib.sourceforge.net/">palmzlib</a> and
<a href="http://palmboxer.sourceforge.net/">boxer / ZBoxZ</a>.  He initially
wrote crater, which was excactly what I needed to base Kit off of, but it
did not compress databases.  So I took the idea and started from scratch.</li>
</ul>

<?PHP

StandardFooter();

?>
