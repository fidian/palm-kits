/* Common defines, structs, etc.
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 * 
 * $Id$
 */
#ifndef KITDEF_H
#define KITDEF_H

typedef struct kitglobal_s
  {
     UInt8 flags;
     UInt8 files;
     UInt16 reserved;  // padding for 4-byte boundary
  } kitglobals;

// Should I skip the "Files To Decompress" form?
// NOTE:  The warning about lengthy extraction time will not show up.
#define GF_DECOMPRESS_AUTOMATICALLY   0x01
// If decompressing automatically, should it decompress on these other
// launch codes (besides sysAppLaunchCmdNormalLaunch?
// NOTE:  These don't work yet...  Fatal Exception.
// NOTE:  The warning about lengthy extraction time will not show up.
#define GF_DECOMPRESS_ON_SYNCNOTIFY   0x02
#define GF_DECOMPRESS_ON_OPENDB       0x04
// Should I (by default) overwrite or skip duplicate files?
#define GF_DUPLICATE_OVERWRITE        0x08
// Should I prompt when there is a duplicated file?  Or skip the prompt and
// overwrite/ignore based on GF_DUPLICATE_OVERWRITE?
#define GF_DUPLICATE_PROMPT_SKIP      0x10
// Should I display a message when done decompressing?
#define GF_SKIP_DONE_ALERT            0x20
// Should the kit self-destruct when done (only on success)?
// NOTE:  This doesn't work yet...
#define GF_DELETE_WHEN_DONE           0x40
// Keep original modification times?
#define GF_KEEP_TIMES                 0x80

typedef struct kitdefinition_s
{
   char name[32];
   UInt8 last_chunk;    // last chunk # this occupies
   UInt8 flags;
   UInt16 reserved2;    // padding to 4-byte boundary
} kitdef;

// Is this file selected?
#define DF_IS_SELECTED                0x01

typedef struct kit_first_rsrc_s
{
   kitglobals g;  // globals
   kitdef files[1];
} kit_first_rsrc;

#endif
