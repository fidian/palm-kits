/* KitTest
 *
 * Copyright (C) 2002-2003 - Kit development team
 * Licensed under the GNU GPL software license.
 * See the docs/LEGAL file for more information
 * See http://kits.sourceforge.net/ for more information about the library
 * 
 * $Id$
 */

#define SECONDS_PER_TEST 15
#define NOZLIBDEFS

//#define SKIP_ZLIB_TEST
//#define SKIP_TINY_TEST

#include <PalmOS.h>
#include "../gunzip/inflate.h"  
#include "kittest.h"
#include "version.h"


int Line = 0;


void WriteMessage(char *s)
{
   WinDrawChars(s, StrLen(s), 0, 15 + Line * 12);
   Line ++;
}


void WaitForSecond(void)
{
   UInt32 counter;
   
   counter = TimGetSeconds();
   while (counter == TimGetSeconds())
     {
     }
}


void WriteChecksum(char *s, int bytes)
{
   unsigned int chk = 0;
   unsigned int i;
   unsigned int val;
   char Message[100];
   char Hex[16] = "0123456789ABCDEF";
   char Append[2];
   
   for (i = 0; i < bytes; i ++)
     {
	chk = (chk << 1) | (chk >> 15);
	chk ^= s[i];
     }
   
   Append[1] = '\0';
   StrCopy(Message, "Check:  ");
   for (i = 4; i > 0; i --)
     {
	val = chk >> ((i - 1) * 4);
	Append[0] = Hex[val & 0x04];
	StrCat(Message, Append);
     }
   WriteMessage(Message);
}

   
void DoTests(void)
{
   char Message[100];  // Spare message buffer
   char *DataUnpacked;
   char *DataPacked;
   MemHandle hand;
   char *ptr;
   UInt32 size, i;
   Err error;
   UInt16 usecount;
   z_stream zs;
   UInt32 startTime;
   

   StrCopy(Message, "Preparing (");
   StrIToA(&(Message[StrLen(Message)]), SECONDS_PER_TEST);
   StrCat(Message, " seconds per test)");
   WriteMessage(Message);

   MemSet(&zs, sizeof(z_stream), 0);
   
   DataUnpacked = MemPtrNew(32768);
   if (DataUnpacked == NULL)
     {
	WriteMessage("Can't allocate unpacked memory");
	return;
     }
   
   hand = DmGet1Resource('KITd', 1);
   if (! hand)
     {
	WriteMessage("No data resource found!");
	MemPtrFree(DataUnpacked);
	return;
     }
   
   size = MemHandleSize(hand);
   DataPacked = MemPtrNew(size);
   if (DataPacked == NULL)
     {
	WriteMessage("Can't allocate packed memory");
	MemPtrFree(DataUnpacked);
        DmReleaseResource(hand);
	return;
     }
   
   ptr = (char *) MemHandleLock(hand);
   MemMove(DataPacked, ptr, size);
   MemHandleUnlock(hand);
   DmReleaseResource(hand);

#ifndef SKIP_ZLIB_TEST   
   error = SysLibFind("Z.lib", &ZLibRef);
   if (error)
     error = SysLibLoad('libr', 'ZLib', &ZLibRef);
   
   if (error)
     {
	WriteMessage("ZLib not found!");
	MemPtrFree(DataUnpacked);
	MemPtrFree(DataPacked);
	return;
     }
   
   error = ZLibOpen(ZLibRef);
   if (error)
     {
	WriteMessage("Unable to open ZLib");
	MemPtrFree(DataUnpacked);
	MemPtrFree(DataPacked);
	return;
     }	

   WaitForSecond();
   WriteMessage("Inflating with ZLib");
	
   startTime = TimGetSeconds();
   i = 0;
   while (TimGetSeconds() - startTime < SECONDS_PER_TEST)
     {
	if (inflateInit2(&zs, -15))
	  {
	     WriteMessage("No unzip");
	     startTime = 0;
	  }
	else
	  {
	     zs.avail_in = size;
	     zs.next_in = DataPacked;
	     zs.next_out = DataUnpacked;
	     zs.avail_out = 32768;
	     error = inflate(&zs, Z_FINISH);
	     if (error != Z_STREAM_END)
	       {
		  startTime = 0;
		  if (error == Z_STREAM_ERROR)
		    WriteMessage("Z_STREAM_ERROR");
		  else if (error == Z_OK)
		    WriteMessage("Z_OK (it expects more data)");
		  else if (error == Z_ERRNO)
		    WriteMessage("Z_ERRNO");
		  else if (error == Z_DATA_ERROR)
		    WriteMessage("Z_DATA_ERROR");
		  else if (error == Z_MEM_ERROR)
		    WriteMessage("Z_MEM_ERROR");
		  else if (error == Z_BUF_ERROR)
		    WriteMessage("Z_BUF_ERROR");
		  else
		    {
		       StrCopy(Message, "Wrong return code:  ");
		       StrIToA(&(Message[StrLen(Message)]), error);
		       WriteMessage(Message);
		    }
		  StrCopy(Message, "i:  ");
		  StrIToA(&(Message[StrLen(Message)]), i);
		  WriteMessage(Message);
	       }
	     inflateEnd(&zs);
	  }
	i ++;
     }

   StrCopy(Message, "ZLib Count:  ");
   StrIToA(&(Message[StrLen(Message)]), i - 1);
   WriteMessage(Message);
   WriteChecksum(DataUnpacked, 32768);
#endif
   
#ifndef SKIP_TINY_TEST
   WaitForSecond();
   WriteMessage("Inflating with Fidian's version");
   
   startTime = TimGetSeconds();
   i = 0;
   while (TimGetSeconds() - startTime < SECONDS_PER_TEST)
     {
	zs.avail_in = size;
	zs.next_in = DataPacked;
	zs.next_out = DataUnpacked;
	zs.avail_out = 32768;
	error = InflateData(&zs);
	if (error)
	  {
	     startTime = 0;
	     StrCopy(Message, "Wrong return code:  ");
	     StrIToA(&(Message[StrLen(Message)]), error);
	     WriteMessage(Message);
	     StrCopy(Message, "i:  ");
	     StrIToA(&(Message[StrLen(Message)]), i);
	     WriteMessage(Message);
	  }
	i ++;
     }
   
   StrCopy(Message, "Fid Count:  ");
   StrIToA(&(Message[StrLen(Message)]), i - 1);
   WriteMessage(Message);
   WriteChecksum(DataUnpacked, 32768);
#endif
   
#ifdef SKIP_TINY_TEST
#ifdef SKIP_ZLIB_TEST
   WriteMessage("Looping inflate with Fidian's version");
   
   for (i = 0; i < 3; i ++)
     {
	StrCopy(Message, "i:  ");
	StrIToA(&(Message[StrLen(Message)]), i);
	WriteMessage(Message);
	zs.avail_in = size;
	zs.next_in = DataPacked;
	zs.next_out = DataUnpacked;
	zs.avail_out = 32768;
	error = DeflateLoop(&zs);
	if (error)
	  {
	     startTime = 0;
	     StrCopy(Message, "Wrong return code:  ");
	     StrIToA(&(Message[StrLen(Message)]), error);
	     WriteMessage(Message);
	  }
     }
   WriteChecksum(DataUnpacked, 32768);
#endif
#endif
   
   WriteMessage("Cleaning up");
   
   MemPtrFree(DataUnpacked);
   MemPtrFree(DataPacked);

#ifndef SKIP_ZLIB_TEST
   error = ZLibClose(ZLibRef, &usecount);
   if (error)
     {
	WriteMessage("Unable to close ZLib");
	return;
     }
   
   if (usecount == 0)
     SysLibRemove(ZLibRef);
#endif
   
   WriteMessage("Done");
}


void HideGoButton(void)
{
   FormPtr formPtr;
   Int16 index;
   
   formPtr = FrmGetActiveForm();
   index = FrmGetObjectIndex(formPtr, B_M_Go);
   FrmHideObject(formPtr, index);
}

		
Boolean MainEventHandler(EventPtr event)
{
   LocalID id;
   
   if (event->eType == frmOpenEvent)
     {
	FrmDrawForm(FrmGetActiveForm());
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
	
	if (id == B_M_Go)
	  {
	     // Remove Go button
	     HideGoButton();
	     
	     // Run test
	     DoTests();
	     return true;
	  }
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

   FrmSetEventHandler(form, MainEventHandler);
}


static void EventLoop(void)
{
   short err;
   EventType event;
   
   do
    {
       // Wait indefinitely for an event
       EvtGetEvent(&event, -1);
       
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


UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
   UInt32 romVersion;
   EventType newEvent;

   // Make sure we are running on PalmOS 2.0 or later
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
   if (romVersion < sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0))
     {
	if ((launchFlags & (sysAppLaunchFlagNewGlobals |
			   sysAppLaunchFlagUIApp)) ==
	    (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
	  {
	     FrmAlert(A_RomIncompatible);
	     
	     newEvent.eType = keyDownEvent;
	     newEvent.data.keyDown.chr = launchChr;
	     newEvent.data.keyDown.keyCode = 0;
	     newEvent.data.keyDown.modifiers = commandKeyMask;
	     
	     EvtAddEventToQueue(&newEvent);
	  }
	
	return sysErrRomIncompatible;
     }
	
   if (cmd == sysAppLaunchCmdNormalLaunch)
     {	
	FrmGotoForm(F_Main);
	
	EventLoop();
	
	// Close any open forms -- a frmCloseForm event doesn't arrive
	// if there are open forms.
	FrmCloseAllForms();
     }
   return 0;
}
