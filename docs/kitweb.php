<?PHP
/* Documentation for Kits
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 */

include("./functions.inc");

StandardHeader('KitWeb');

?>

<p>KitWeb is not yet written.  It will be a program that builds Kits through
a web-based interface.  It will be written in <a
href="http://php.net">PHP</a> and will use <a
href="http://php-pdb.sourceforge.net/">PHP-PDB</a> to write the resulting
Kit.  The main thing holding up this project is that PHP-PDB does not yet
allow reading of .prc files (it works <i>only</i> for .pdb files).  Once
that changes, the application should be very easy to write.</p>

<?PHP

StandardFooter();

?>
