#!/usr/bin/perl
#
# Generates ID numbers for a .rcp file
#
# Copyright (C) 2002-2003 - Kit development team
# Licensed under the GNU GPL software license.
# See the docs/LEGAL file for more information
# See http://kits.sourceforge.net/ for more information about the library
#
# $Id$
#
# Use STDIN for input, STDOUT for output.

$StartNumber = 1002;
$AutoNum = 0;

$UsedSpecifiedIDs = {};
$Number = $StartNumber;
$IsNext = 0;
$LineNum = 0;
foreach $Line (<STDIN>)
{
    $LineNum ++;
    $Line =~ tr/\r\n//d;
    $Line = "" if ($Line =~ /^\/\//);
    foreach $Word (split(/ /, $Line))
    {
        if ($IsNext && length($Word) && $Word =~ /[A-Za-z]/)
	{
	    if ($Defined{$Word})
	    {
	        $Error = "$Word has been defined twice in .rcp file!";
		$ErrorFirst = "First declaration of $Word at " . $Defined{$Word};
		$ErrorSecond = "Second declaration of $Word at $LineNum";
		
		print "#error $Error\n";
		print "// $ErrorFirst\n";
		print "// $ErrorSecond\n";
		
		print STDERR "ERROR:  $Error\n$ErrorFirst\n$ErrorSecond\n";
	    }
	    print "#define $Word $Number\n";
	    $Number ++;
	    $IsNext = 0;
	    $Defined{$Word} = $LineNum;
	}
	elsif ($IsNext)
	{
	   $IsNext = 0;
	   if ($Word =~ /^[0-9]+/)
	   {
	      if ($UsedSpecifiedIDs{$Word}) {
	         print STDERR "ERROR:  ID specified on line $LineNum.\n";
		 print STDERR "This is a duplicate ID, which is first "
		    . "defined on line " . $UsedSpecifiedIDs{$Word} . "\n";
	      } else {
	         $UsedSpecifiedIDs{$Word} = $LineNum;
	      }
	   }
	   else
	   {
	      print STDERR "ERROR:  Unable to find ID for line $LineNum.\n";
	      print STDERR "Line:  $Line\n";
	      print STDERR "Word:  $Word\n";
	   }
	}
        elsif (uc($Word) eq "ID")
	{
	    $IsNext = 1;
	}
	elsif (uc($Word) eq "AUTOID")
	{
	    $AutoNum ++;
	}
    }
}

print STDERR "Lines checked :  " . $LineNum . "\n";
print STDERR "Numbered by me:  " . ($Number - $StartNumber) . "\n";
print STDERR "Autonumbered  :  " . ($AutoNum) . "\n";
print STDERR "TOTAL USED    :  " . ($AutoNum + ($Number - $StartNumber)) . "\n";
