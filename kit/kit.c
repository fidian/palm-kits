// Notes:
//   Do not use globals or static variables.

#define NOZLIBDEFS
#define TABLE_ROWS 10
#define FILENAME_STATUS_LINE 1
#define FILE_PROGRESS_LINE 2
#define OVERALL_STATUS_LINE 4
#define OVERALL_STRING "Overall"
#define OVERALL_PROGRESS_LINE 5
#define WORKING_STATUS_LINE 7
#define WORKING_DECOMPRESSING "Decompressing"
#define WORKING_WORKING "Working"
#define PLEASE_WAIT_MESSAGE "Please wait ...  Loading files."
#define PLEASE_WAIT_LINE 5

#define GLOBAL_DATA 0
#define GLOBAL_FIRSTROW 1
#define GLOBAL_TIMEOUT 2

#include <PalmOS.h>
#include "../common/kitdef.h"
#include "../gunzip/inflate.h"
#include "kit.h"
#include "version.h"


typedef struct decomp_s
{
   UInt8 start_chunk;    // Chunk the file starts on
   UInt8 end_chunk;      // Chunk the file ends on
   UInt8 next_chunk;     // Which chunk to decompress next
   UInt8 last_chunk;     // Last chunk to ever process
   UInt32 bytes_left;    // Bytes left in decompression buffer
   UInt32 bytes_total;   // Number of bytes decompressed last call
   UInt8 *buffer;        // Decompression buffer (decompressed data only)
} decomp_data;


typedef struct globals_pref
{
   UInt32 pref_size;  // The size of the preferences
   UInt16 FirstRowNumber;
   UInt8 Iteration;
   UInt8 Filler;  // 4-byte boundary
   Int32 Timeout;
   kit_first_rsrc GlobalData;  // Must be last
} global_data;


void Get_Prefs(global_data **g)
{
   UInt16 size;
   UInt32 pref_size;
   
   // Read size of preferences
   size = sizeof(UInt32);
   PrefGetAppPreferences(CREATOR_ID, 0, &pref_size, &size, false);
   
   // Read real preferences
   size = pref_size;
   *g = MemPtrNew(size);
   // This should never go wrong, hence no error checking.
   PrefGetAppPreferences(CREATOR_ID, 0, *g, &size, false);
}


void Set_Prefs(global_data *g)
{
   PrefSetAppPreferences(CREATOR_ID, 0, 0, g, g->pref_size, false);
}


void PressLauncherButton(void)
{
   EventType newEvent;
   
   newEvent.eType = keyDownEvent;
   newEvent.data.keyDown.chr = launchChr;
   newEvent.data.keyDown.keyCode = 0;
   newEvent.data.keyDown.modifiers = commandKeyMask;
   
   EvtAddEventToQueue(&newEvent);
}


void WriteChars(char *str, UInt16 X, UInt16 Y, UInt16 Width)
{
   short TextLen;
   Boolean fits;
   char ellipsis;
   
   TextLen = StrLen(str);
   FntCharsInWidth(str, &Width, &TextLen, &fits);
   if (fits)
     {
	WinDrawChars(str, TextLen, X, Y);
	return;
     }
   ChrHorizEllipsis(&ellipsis);
   Width -= FntCharWidth(ellipsis);
   FntCharsInWidth(str, &Width, &TextLen, &fits);
   WinDrawChars(str, TextLen, X, Y);
   WinDrawChars(&ellipsis, 1, X + Width, Y);
}


void Status(char *str, int line)
{
   RectangleType r;
   
   line *= 12;
   line += 15;
   
   r.topLeft.x = 0;
   r.topLeft.y = line;
   r.extent.x = 160;
   r.extent.y = 12;
   WinEraseRectangle(&r, 0);
   
   if (StrLen(str))
     WriteChars(str, 0, line, 160);
}


// percent is a number [0.0 - 1.0]
void Bar(float percent, int line)
{
   RectangleType r;
   
   percent *= (float) 100;
   
   r.topLeft.x = 30;
   r.topLeft.y = 17 + (line * 12);
   r.extent.x = 100;
   r.extent.y = 8;
   
   WinDrawRectangleFrame(simpleFrame, &r);
   
   r.extent.x = percent;
   
   WinDrawRectangle(&r, 0);
}


UInt8 FillBuffer(decomp_data *dd)
{
   MemHandle hand;
   z_stream zs;
   UInt8 error;
   
   Status(WORKING_DECOMPRESSING, WORKING_STATUS_LINE);
   
   // Check if there is another chunk I can decompress
   // The Exchange Manager slurps in all remaining data into one
   // final record.
   if (dd->next_chunk > dd->end_chunk)
     {
	return 0;
     }
   
   hand = DmGetResource('KITd', dd->next_chunk);
   dd->next_chunk ++;
   if (! hand)
     {
	// StrIToA(dd->buffer, dd->end_chunk);
	return 1;
     }
   
   zs.avail_in = MemHandleSize(hand);
   zs.avail_out = 32768;
   zs.next_in = MemHandleLock(hand);
   zs.next_out = dd->buffer;
   error = InflateData(&zs);
   MemHandleUnlock(hand);
   DmReleaseResource(hand);
	
   if (error)
     {
	return 2;
     }
	
   dd->bytes_total = 32768 - zs.avail_out;
   dd->bytes_left = dd->bytes_total;
	
   Status(WORKING_WORKING, WORKING_STATUS_LINE);

   return 0;
}


Err ReadProc(void *data, UInt32 *size, void *ddp)
{
   decomp_data *dd;
   UInt8 error;
   float percent;
   UInt32 size_left, bytes;
   char debug[10];

   Status("Enter ReadProc", 3);
   dd = ddp;

   StrIToA(debug, *size);
   FrmCustomAlert(A_Debug, "Read OK", "size", debug);

   // Loop until we fill the buffer that the Exchange Manager wants,
   // or until we hit an error.
   size_left = *size;
   while (size_left)
     {
	// If there are 0 bytes available, see if I can fill the buffer again
	if (dd->bytes_left == 0)
	  {
	     Status("FillBuffer", 3);
	     error = FillBuffer(dd);
	     if (error)
	       {
		  *size = 0;
		  return error;
	       }
	  }

	Status("Size Check", 3);
	if (size_left > dd->bytes_left)
	  bytes = dd->bytes_left;
	else
	  bytes = size_left;

	if (bytes == 0)
	  {
	     Status("Early Bail", 3);
	     StrIToA(dd->buffer, *size);
	     Status(dd->buffer, 4);
	     *size = *size - size_left;
	     return 0;
	  }
	
	Status("MemMove", 3);
	MemMove(data + (*size - size_left),
		&(dd->buffer[dd->bytes_total - dd->bytes_left]), bytes);
	dd->bytes_left -= bytes;
	size_left -= bytes;
	Status("While Loop", 3);
     }
   
   // We retrieved bytes successfully
   // Say so
   StrIToA(debug, dd->bytes_left);
   FrmCustomAlert(A_Debug, "Read OK", "Bytes left", debug);

   // The calculated percentage is not 100% accurate, but it is good enough
   // for what I do with it.  To be 100% accurate, I would need to know
   // how many bytes were in the last chunk (the others are all 32768 bytes)
   // so I could use this formula:
   //     percentage = bytes_completed / bytes_total
   // Instead, I assume that each chunk is 32768 bytes long.  This makes
   // the progress bar go much faster on the last chunk than it does for the
   // rest of the chunks in the file.
   // 
   // I just picked this method because it was easy and it didn't require
   // a lot of work.
   
   // Percentage of this chunk [0.0 - 1.0]
   percent = dd->bytes_total - dd->bytes_left;
   percent /= (float) dd->bytes_total;
   
   // Add in the number of other chunks completed fully
   percent += (float) ((dd->next_chunk - dd->start_chunk) - 1);
   
   // Divide by the total number of chunks
   percent /= (float) ((dd->end_chunk - dd->start_chunk) + 1);

   Status("Bar1", 3);
   Bar(percent, FILE_PROGRESS_LINE);
 
   percent = dd->next_chunk - 2;
   percent /= (float) dd->last_chunk;
   Status("Bar2", 3);
   Bar(percent, OVERALL_PROGRESS_LINE);
   Status("Leaving ReadProc", 3);

   return 0;
}


Boolean DeleteProc(const char *name, UInt16 version, UInt16 cardNo,
		   LocalID dbID, void *dd)
{
   // Don't know how this happened!  If the database existed, it should
   // have either been deleted or not decompressed from the archive!
   // Just return false to display an error message in the decompression
   // loop.
   return false;
}


void *GetObjectPointer(LocalID id)
{
   FormPtr formPtr;
   
   formPtr = FrmGetActiveForm();
   return FrmGetObjectPtr(formPtr, FrmGetObjectIndex(formPtr, id));
}


void SetObject(LocalID id, Boolean Viewable)
{
   FormPtr formPtr;
   Int16 index;
   
   formPtr = FrmGetActiveForm();
   index = FrmGetObjectIndex(formPtr, id);
   if (Viewable)
     FrmShowObject(formPtr, index);
   else
     FrmHideObject(formPtr, index);
}


void ToggleFile(void)
{
   global_data *g;
   unsigned short row, col;
   UInt16 filenum;
   TableType *table;
   
   table = GetObjectPointer(Tbl_FileList);
   
   // Determine the row that's selected
   if (! TblGetSelection(table, &row, &col))
     {
	// Weird.
	TblUnhighlightSelection(table);
	return;
     }
   
   Get_Prefs(&g);
   filenum = row + g->FirstRowNumber;
   g->GlobalData.files[filenum].flags ^= DF_IS_SELECTED;
   Set_Prefs(g);
   MemPtrFree(g);
   
   // Update table
   TblUnhighlightSelection(table);
}


void TableDrawFunc(void *tableP, Int16 row, Int16 col, RectanglePtr bounds)
{
   FontID oldFont;
   UInt16 filenum;
   global_data *g;
   UInt16 width;
   char checkstr;
   
   Get_Prefs(&g);
   
   filenum = row + g->FirstRowNumber;
   if (filenum >= g->GlobalData.g.files)
     {
	TblSetRowSelectable(tableP, row, false);
	MemPtrFree(g);
	return;
     }
 
   TblSetRowSelectable(tableP, row, true);
   g->GlobalData.files[filenum].name[31] = '\0';
   oldFont = FntSetFont(symbol11Font);
   // Since DF_IS_SELECTED is 0x01, this results in 0x00 or 0x01.  Perfect.
   checkstr = (g->GlobalData.files[filenum].flags & DF_IS_SELECTED);
   width = FntCharWidth(checkstr);
   width += FntCharWidth(' ') * 2;
   WinDrawChars(&checkstr, 1, bounds->topLeft.x, bounds->topLeft.y);
   FntSetFont(oldFont);
   WriteChars(g->GlobalData.files[filenum].name, 
	      bounds->topLeft.x + width, bounds->topLeft.y, 
	      bounds->extent.x - 2);
   MemPtrFree(g);
}


void UpdateTable(Boolean UpdateScrollbar)
{
   TablePtr table;
   ScrollBarPtr bar;
   global_data *g;
   
   table = GetObjectPointer(Tbl_FileList);
   TblUnhighlightSelection(table);
   TblMarkTableInvalid(table);
   TblRedrawTable(table);
   
   if (! UpdateScrollbar)
     return;

   Get_Prefs(&g);
   
   if (g->GlobalData.g.files <= TABLE_ROWS)
     {
	SetObject(Sb_FileList, 0);
	MemPtrFree(g);
	return;
     }
   
   SetObject(Sb_FileList, 1);
   bar = GetObjectPointer(Sb_FileList);
   SclSetScrollBar(bar, g->FirstRowNumber, 0, 
		   g->GlobalData.g.files - TABLE_ROWS, TABLE_ROWS - 1);
   SclDrawScrollBar(bar);
   MemPtrFree(g);
}


void Scroll(Int16 Number, Boolean UpdateScrollbar)
{
   Int16 numRows;
   int OldFirstRow;
   global_data *g;
   
   Get_Prefs(&g);
   
   OldFirstRow = g->FirstRowNumber;
   
   if (Number < 0)
     {
	if (g->FirstRowNumber < - Number)
	  g->FirstRowNumber = 0;
	else
	  g->FirstRowNumber += Number;
     }
   else
     {
	if (g->GlobalData.g.files < TABLE_ROWS)
	  g->FirstRowNumber = 0;
	else
	  {
	     numRows = g->GlobalData.g.files - TABLE_ROWS;
	     if (g->FirstRowNumber + Number > numRows)
	       g->FirstRowNumber = numRows;
	     else
	       g->FirstRowNumber += Number;
	  }
     }

   Set_Prefs(g);
   
   if (OldFirstRow != g->FirstRowNumber)
     UpdateTable(UpdateScrollbar);
   
   MemPtrFree(g);
}


void SetupTable()
{
   TablePtr table;
   Int16 i;
   global_data *g;
   
   Get_Prefs(&g);
   g->FirstRowNumber = 0;
   Set_Prefs(g);
   MemPtrFree(g);
   
   table = GetObjectPointer(Tbl_FileList);
   TblHasScrollBar(table, true);
   for (i = 0; i < TABLE_ROWS; i ++)
     TblSetItemStyle(table, i, 0, customTableItem);
   TblSetColumnUsable(table, 0, true);
   TblSetCustomDrawProcedure(table, 0, TableDrawFunc);
}


void SelectAll(Boolean Toggle)
{
   global_data *g;
   UInt16 filenum;
   
   Get_Prefs(&g);
   for (filenum = 0; filenum < g->GlobalData.g.files; filenum ++)
     {
	if (Toggle)
	  g->GlobalData.files[filenum].flags |= DF_IS_SELECTED;
	else
	  // Hardcoded 0xFF to match the size of .flags
	  // Subtracted one to get 0xFE
	  g->GlobalData.files[filenum].flags &= 0xFE;
     }
   
   Set_Prefs(g);
   MemPtrFree(g);
   
   // Update table
   UpdateTable(false);
}


Boolean DecompressEventHandler(EventPtr event)
{
   decomp_data dd;
   LocalID dbID;
   Boolean needReset = false, keep_times, process;
   Err error;
   global_data *g;
   
   if (event->eType == frmOpenEvent)
     {
	FrmDrawForm(FrmGetActiveForm());
	Get_Prefs(&g);
	g->Timeout = 0;
	g->Iteration = 0;
	Set_Prefs(g);
	MemPtrFree(g);

	Status(OVERALL_STRING, OVERALL_STATUS_LINE);
	Bar(0, OVERALL_PROGRESS_LINE);
	
	return true;
     }
   if (event->eType == nilEvent)
     {
	// Main decompression loop
	// This will process a chunk, then wait for the next nilEvent
	// so that I can abort the sequence if necessary.
	
	Get_Prefs(&g);
	if (g->GlobalData.g.flags & GF_KEEP_TIMES)
	  keep_times = true;
        else
	  keep_times = false;

	if (g->Iteration >= g->GlobalData.g.files)
	  {
	     // Done
	     if (g->GlobalData.g.flags & GF_ALERT_WHEN_DONE || 1)
	       FrmAlert(A_Done);
	     
	     g->Timeout = -1;
	     Set_Prefs(g);
	     MemPtrFree(g);
	     PressLauncherButton();
	     return true;
	  }
	
	// Skip files that are not marked to be extracted
	if (! (g->GlobalData.files[g->Iteration].flags & DF_IS_SELECTED))
	  {
	     g->Iteration ++;
	     Set_Prefs(g);
	     MemPtrFree(g);
	     return true;
	  }
	
	// Draw the name
	Status(g->GlobalData.files[g->Iteration].name, FILENAME_STATUS_LINE);

	Set_Prefs(g);
	MemPtrFree(g);
	// Clear progress bar
	Status("", FILE_PROGRESS_LINE);
	Bar(0, FILE_PROGRESS_LINE);
	
	// Set up struct to pass to decompression routine
	if (g->Iteration == 0)
	  dd.start_chunk = 1;
	else
	  dd.start_chunk = g->GlobalData.files[g->Iteration - 
					       1].last_chunk + 1;
	dd.end_chunk = g->GlobalData.files[g->Iteration].last_chunk;
	dd.last_chunk = g->GlobalData.files[g->GlobalData.g.files - 
					    1].last_chunk;
	dd.next_chunk = dd.start_chunk;
	dd.bytes_left = 0;
	dd.buffer = MemPtrNew(32768);

	if (dd.buffer == NULL)
	  {
	     g->Timeout = -1;
	     Set_Prefs(g);
	     MemPtrFree(g);
	     FrmAlert(A_NoMemory);
	     PressLauncherButton();
	     return true;
	  }
	
	// Handle duplicates
	process = true;
	dbID = DmFindDatabase(0, g->GlobalData.files[g->Iteration].name);
	if (dbID != 0)
	  {
	     if (! (g->GlobalData.g.flags & GF_DUPLICATE_PROMPT_SKIP))
	       {
		  if (FrmCustomAlert(A_Overwrite, 
				     g->GlobalData.files[g->Iteration].name,
				     NULL, NULL))
		    process = false;
		  else
		    DmDeleteDatabase(0, dbID);
	       }
	     else if (g->GlobalData.g.flags & GF_DUPLICATE_OVERWRITE)
	       DmDeleteDatabase(0, dbID);
	     else
	       process = false;
	  }
	
	// Data structures are set, we know the bounds, and now we just
	// need to decompress the data.
	// Decompress
	if (process)
	  {
	     // If this works, we should be at the chunk beyond the ending
	     // chunk.
	     error = ExgDBRead(ReadProc, DeleteProc, &dd, &dbID, 
			       0, &needReset, keep_times);
	     if (error || dd.bytes_left || dd.next_chunk <= dd.end_chunk)
	       {
		  // ERROR
		  process = 1;
		  if (error)
		    {
		       Status("error", process ++);
		       StrIToA(dd.buffer, error);
		       Status(dd.buffer, process ++);
		    }
		  if (dd.bytes_left)
		    {
		       Status("bytes left", process ++);
		    }
		  if (dd.next_chunk <= dd.end_chunk)
		    {
		       Status("chunk", process ++);
		    }
		  FrmAlert(A_DecompressProblem);
		  PressLauncherButton();
		  g->Timeout = -1;
		  Set_Prefs(g);
		  MemPtrFree(g);
		  MemPtrFree(dd.buffer);
		  return true;
	       }
	  }
	Bar((float) g->GlobalData.files[g->Iteration].last_chunk /
	    (float) g->GlobalData.files[g->GlobalData.g.files - 1].last_chunk,
	    OVERALL_PROGRESS_LINE);

	g->Iteration ++;
	Set_Prefs(g);
	MemPtrFree(g);
	MemPtrFree(dd.buffer);
	
	return true;
     }
   
   return false;
}
  
  
Boolean MainEventHandler(EventPtr event)
{
   LocalID id;

   if (event->eType == frmOpenEvent)
     {
	FrmDrawForm(FrmGetActiveForm());
	SetupTable();
	// TODO:  Draw border around table
	UpdateTable(true);
	return true;
     }
   if (event->eType == menuEvent)
     {
	id = event->data.menu.itemID;
	
	if (id == M_M_WhatIs)
	  {
	     FrmHelp(S_WhatIsAKit);
	     return true;
	  }
	if (id == M_M_About)
	  {
	     FrmCustomAlert(A_Version, APP_VERSION, COMPILE_DATE, NULL);
	     return true;
	  }
     }
   if (event->eType == ctlSelectEvent)
     {
	id = event->data.ctlEnter.controlID;
	
	if (id == B_M_All)
	  {
	     SelectAll(1);
	     return true;
	  }
	if (id == B_M_None)
	  {
	     SelectAll(0);
	     return true;
	  }
	if (id == B_M_Go)
	  {
	     if (! FrmAlert(A_TimeWarning))
	       {
		  // TODO:  Check for free space
	     
		  // Switch forms and start decompression
		  FrmGotoForm(F_Decompress);
	       }
	     return true;
	  }
     }
   if (event->eType == keyDownEvent)
     {
	if (event->data.keyDown.chr == pageUpChr)
	  {
	     Scroll(1 - TABLE_ROWS, true);
	     return true;
	  }
	if (event->data.keyDown.chr == pageDownChr)
	  {
	     Scroll(TABLE_ROWS - 1, true);
	     return true;
	  }
     }
   if (event->eType == tblSelectEvent)
     {
	ToggleFile();
	return true;
     }
   if (event->eType == sclRepeatEvent)
     {
	Scroll(event->data.sclRepeat.newValue - event->data.sclRepeat.value,
	       false);
	// Do not return true here so the OS updates stuff properly.
     }
   
   return false;
}
  
  
// Select a new form
void FormLoadEvent(EventPtr event)
{
   int formID;

   //short err;
   FormPtr form;
   //EventType event;
   
   formID = event->data.frmLoad.formID;
   form = FrmInitForm(formID);
   FrmSetActiveForm(form);

   if (formID == F_Decompress)
     FrmSetEventHandler(form, DecompressEventHandler);
   else
     FrmSetEventHandler(form, MainEventHandler);
}


static void EventLoop(void)
{
   short err;
   EventType event;
   global_data *g;
   
   Get_Prefs(&g);
   g->Timeout = -1;
   Set_Prefs(g);
   MemPtrFree(g);
   
   do
    {
       // Wait indefinitely for an event
       Get_Prefs(&g);
       EvtGetEvent(&event, g->Timeout);
       MemPtrFree(g);
       
       if (! SysHandleEvent(&event))
	 {
	    if (! MenuHandleEvent(MenuGetActiveMenu(), &event, &err)) 
	      {
		 if (event.eType == frmLoadEvent)
		   {
		      FormLoadEvent(&event);
		   }
		 FrmDispatchEvent(&event);
	      }
	 }
    } while (event.eType != appStopEvent);
}


UInt32 StartApplication(void)
{
   /*
   Err error;
   
   error = SysLibFind("Z.lib", &ZLibRef);
   if (error)
     error = SysLibLoad('libr', 'ZLib', &ZLibRef);
   
   if (error)
     {
	ZLibRef = 0;
     }
   else
     {
	error = ZLibOpen(ZLibRef);
	if (error)
	  {
	     ZLibRef = 0;
	  }
     }
    */
   
   FrmGotoForm(F_Main);
   
   return 0;
}


void StopApplication()
{
   /*
   UInt16 usecount;
   Err error;
    */
   
   FrmCloseAllForms();

   /*
   if (ZLibRef)
     {
	error = ZLibClose(ZLibRef, &usecount);
	// TODO:  Maybe add error handling if I can't close ZLib?
	// But why bother?  I'm just closing the program anyway.
	if (! error && usecount == 0)
	  {
	     SysLibRemove(ZLibRef);
	  }
     }
    */
}


UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
   UInt32 romVersion;
   UInt32 error, size, pref_size;
   MemHandle hand;
   global_data *g;
   
   // Make sure we are running on PalmOS 2.0 or later
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
   if (romVersion < sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0))
     {
	if ((launchFlags & (sysAppLaunchFlagNewGlobals |
			   sysAppLaunchFlagUIApp)) ==
	    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
	  {
	     FrmAlert(A_RomIncompatible);

	     PressLauncherButton();
	  }
	
	return sysErrRomIncompatible;
     }
   
   // Load resource #0 to get global data
   hand = DmGetResource('KITd', 0);
   if (! hand)
     {
	if (cmd == sysAppLaunchCmdNormalLaunch)
	  FrmAlert(A_InvalidKit);
	return sysErrNotAllowed;
     }
   
   size = MemHandleSize(hand);
   pref_size = sizeof(global_data) - sizeof(kit_first_rsrc) + size;
   g = (global_data *) MemPtrNew(pref_size);
   if (g == NULL)
     {
	if (cmd == sysAppLaunchCmdNormalLaunch)
	  FrmAlert(A_NoMemory);
	return sysErrNotAllowed;
     }

   MemMove(&(g->GlobalData), MemHandleLock(hand), size);
   MemHandleUnlock(hand);
   DmReleaseResource(hand);
   
   g->pref_size = pref_size;
   Set_Prefs(g);
	
   if (cmd == sysAppLaunchCmdNormalLaunch || 
       (cmd == sysAppLaunchCmdSyncNotify && 
	g->GlobalData.g.flags & GF_DECOMPRESS_ON_SYNCNOTIFY) ||
       (cmd == sysAppLaunchCmdOpenDB &&
	g->GlobalData.g.flags & GF_DECOMPRESS_ON_OPENDB))
     {
	MemPtrFree(g);
	error = StartApplication();
	if (error) 
	  {
	     PrefSetAppPreferences(CREATOR_ID, 0, 0, NULL, 0, false);
	     return error;
	  }
	
	EventLoop();
	
	// Close any open forms -- a frmCloseForm event doesn't arrive
	// if there are open forms.
	StopApplication();
     }
   else
     MemPtrFree(g);

   // Delete the preferences
   PrefSetAppPreferences(CREATOR_ID, 0, 0, NULL, 0, false);
   
   return 0;
}
