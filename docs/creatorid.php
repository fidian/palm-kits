<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('Creator IDs');

?>

<p>Every program on the Palm has a unique Creator ID.  It is used by the
operating system to keep things straight.  This applies to Kits as well,
because they are also programs.</p>

<p>When you write a program for Palm, you need to register your Creator ID
on <a href="http://dev.palmos.com/creatorid/">Palm's Developer Site</a>.
Likewise, if you make a Kit and you want to distribute it, you must change
the Creator ID.  <u>Do not use the default Creator ID!</u>  Change 'Fid8' in
the Makefile for Kit to be your new Creator ID.  It is at the top of
kit/Makefile, like this:</p>

<?PHP
ShowExample("CREATOR_ID = Fid8");
?>

<p>Before you distribute the application, you must change that to the
Creator ID that you registered.  Additionally, you will likely not want the
program called "Kit", so change the line in the Makefile:</p>

<?PHP
ShowExample("APP_NAME = Kit")
?>

<p>Once you do those two things, it is safe to distribute your
application.</p>

<?PHP

StandardFooter();

?>
