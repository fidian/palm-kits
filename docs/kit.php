<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('Kit');

?>

<p>Kit is the main decompressing program.  It will take compressed records
from within the program and write them to the handheld.  It does not rely
upon ZLib for decompression -- the inflate code is built-in.</p>

<p>Kit's user interface is designed to be simple.  There is a list of files
that are contained in the Kit, so you can select what you want to extract.
When ready, one tap will start the decompression.  Many aspects can be
tweaked with the program -- particular files can be pre-selected to be
extracted, the selection screen can be entirely bypassed, files can be
skipped/overwritten automatically, and other prompts can be removed.</p>

<p>Right now, you need to follow these steps in order to build a kit from
the command line:</p>

<ol>
<li>Copy the .prc, .pdb, and .pqa files that you want to bundle into a Kit
into the kit/bundles directory</li>
<li>Register a <a href="creatorid.php">Creator ID</a> with Palm for the
Kit</li>
<li>Edit the Makefile and change the Creator ID and the Application's
Name</li>
<li>Edit the kit.rcp file and change the Creator ID and the Application's
name.  Also, edit any wording that doesn't suit you.
<li>'make'</li>
</ol>

<p>With <a href="kitpro.php">KitPro</a> and <a href="kitweb.php">KitWeb</a>,
you will need to just build the Kit program and not include any data with
it.  It has a somewhat simpler procedure:</p>

<ol>
<li>Register a <a href="creatorid.php">Creator ID</a> with Palm for the
Kit</li>
<li>Run KitPro or use KitWeb and enter the Creator ID, Application Name, and
pick which files you want to bundle</li>
</ol>

<?PHP

StandardFooter();

?>
