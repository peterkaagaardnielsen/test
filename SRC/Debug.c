//-----------------------------------------------------------------------------
// Debug.c                                                         CHG 20060904
//
// 
//-----------------------------------------------------------------------------
#include "System.h"
#include "Debug.h"
#include <stdarg.h>
#include <stdio.h>
#include "UART0.h"
#include "RTC.h"
//#include "FileSystem.h"
#include "debug.h"
#include "smon.h"
#include "hcc/src/api/api_tiny.h"

//#include "RTXMonitor.h"

#pragma import(__use_c99_library)

#define MAX_DEBUG_FILESIZE 100000

static MUTEX mut;
static MUTEX mutMessageDebug;

static char buffer[256];   // for the vsprinf function !

static int reportSeverity=DBG_INFO | DBG_WAR | DBG_ERR; // Enable all debug message types
static int DebugOutput=DBG_NONE; // Messages goes nowhere..
static char Line[80], *CurLine;
static va_list vararg;

static int AllSent=TRUE;

static char saveFilename[128];
static char debugLine[256];
static unsigned char newDebugLine=FALSE;
static U64 thDebugFileWriterStack[200];  


/*----------------------------------------------------------------------------*/
// 
/*----------------------------------------------------------------------------*/
#define CIRC_SIZE (1024)//

char circular[CIRC_SIZE];
int idxMarkerInput=0, idxMarkerOutput=0, MarkerCount=0; 


//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static void disableINT(void) {
  MUTEX_ACQUIRE(mut);

}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static void enableINT(void) {
  MUTEX_RELEASE(mut);

}

//-----------------------------------------------------------------------------
// Flush debug buffer
//-----------------------------------------------------------------------------
void flushDebug(void) {
	disableINT();
  idxMarkerInput=0;
	idxMarkerOutput=0;
	MarkerCount=0; 
  enableINT();
}

//-----------------------------------------------------------------------------
// Return true if debug buffer (circula) is empty
//-----------------------------------------------------------------------------
int isEmptyDebug(void) {
	disableINT();
  if (AllSent && (MarkerCount==0)) {
    enableINT();
    return 1;
  } 
  enableINT();
  return 0;
}

//-----------------------------------------------------------------------------
// Called by SMon/PhoenixUSB when no more debug data to send (all has been delivered)
//-----------------------------------------------------------------------------
void setAllSentDebug(void) {
	disableINT();
  AllSent=TRUE;
  enableINT();
}

//-----------------------------------------------------------------------------
// Insert a single character into the circular buffer
// This has to be protected from the USB interrupts, as the USB will call the 
// removeCharDebug (and numCharAvailableDebug)
//-----------------------------------------------------------------------------
static void Insert(char ch) {
  char dummy=1;

  while (MarkerCount>CIRC_SIZE-64) OS_WAIT(10);


	disableINT();
  AllSent=FALSE;


  
	// If buffer is full, remove oldest line to free some space
  if (MarkerCount>=CIRC_SIZE) {
		// Remove one line of data
		while (dummy) removeCharDebug(&dummy);
    enableINT();
    return; // No room in circular buffer, abort..
  }

  circular[idxMarkerInput]=ch;
  idxMarkerInput=(idxMarkerInput+1) % CIRC_SIZE;
  MarkerCount++;
  enableINT();
}

//------------------------------------------------------------------------------
// removeCharDebug()
// Helper function for low-level transmit routines
//   Try to remove one character from the debug buffer, returns 1 if no char 
//   available, 0 if char available (will be put in ch)
//------------------------------------------------------------------------------
int removeCharDebug(char *ch) {
  *ch=0;
  if (MarkerCount==0) return 1;
	disableINT();
 
  *ch=circular[idxMarkerOutput];
  idxMarkerOutput=(idxMarkerOutput+1) % CIRC_SIZE;
  MarkerCount--;
  enableINT();
  return 0;
}


//-----------------------------------------------------------------------------
// setOutputDebug()
//   Select where to send debug messages
// Input: 
//   dbg: DBG_NONE(default), DBG_SERIAL or DBG_USB (both DBG_SERIAL and DBG_USB 
//   can set at the same time !)
//-----------------------------------------------------------------------------
void setOutputDebug(int dbg) {
  DebugOutput=dbg;
}


//------------------------------------------------------------------------------
// numCharAvailableDebug()
// Helper function for low-level transmit routines
//   Returns number of characters currently waiting in the debug buffer
//------------------------------------------------------------------------------
int numCharAvailableDebug(void) {
  return MarkerCount;
}

//------------------------------------------------------------------------------
// setFilenameDebug()
// Set what filename to use for storing debug info
//------------------------------------------------------------------------------
void setFilenameDebug(char *filename) {
  strcpy(saveFilename, filename);
}


//-----------------------------------------------------------------------------
// InfoMsg()
//   Send a Debug message to the debug console using specified severity
//   (function works like printf regarding formatting etc)
//-----------------------------------------------------------------------------
void IntmessageDebug(int severity, char* filename, int linenum, char *fmt, va_list varg) {
  int i,x,maxsize;
  char strSev[3];
//  char test[32];
	
  // No debugging enabled...  
  if (DebugOutput == DBG_NONE)
    return;

  // Check if the specified severity is enabled, if not, do nothing
  if ((reportSeverity & severity)==FALSE)
    return;

  switch (severity) {
    case DBG_INFO: strcpy(strSev,"I"); break;
    case DBG_WAR: strcpy(strSev,"W"); break;
    case DBG_ERR: strcpy(strSev,"E"); break;
    default: strcpy(strSev,"?:"); break;
  }

  MUTEX_ACQUIRE(mutMessageDebug);

  if (strlen(filename)) {
    i=sprintf(buffer,"%s:[%s:%i] ", strSev,filename, linenum);
  } else
    i=sprintf(buffer,"%s: ", strSev);

	maxsize=sizeof(buffer)-i-1-5;
	
  x=vsnprintf(&buffer[i],maxsize,fmt,varg);
	if (x>maxsize)
		strcat(buffer,"...");

  
  
  // Put the string into the circular buffer or the UART0 (depending on DebugOutput) (including the 0 terminator)
  for (i=0; i<strlen(buffer)+1; i++) {
    if (DebugOutput & DBG_MON && (buffer[i]!=0x0D && buffer[i]!=0x0A))
      Insert(buffer[i]);
  }
  MUTEX_RELEASE(mutMessageDebug);
} 





//-----------------------------------------------------------------------------
// InfoMsg()
//   Send a Debug message to the debug console using specified severity
//   (function works like printf regarding formatting etc)
//-----------------------------------------------------------------------------
void messageDebug(int severity, char* filename, int linenum, char *fmt,...) {
//  va_list      varg;  
  int i,x,maxsize;
  char strSev[3];

  // No debugging enabled...  
  if (DebugOutput == DBG_NONE)
    return;

  // Check if the specified severity is enabled, if not, do nothing
  if ((reportSeverity & severity)==FALSE)
    return;

  switch (severity) {
    case DBG_INFO: strcpy(strSev,"I"); break;
    case DBG_WAR: strcpy(strSev,"W"); break;
    case DBG_ERR: strcpy(strSev,"E"); break;
    default: strcpy(strSev,"?:"); break;
  }

  MUTEX_ACQUIRE(mutMessageDebug);

  if (strlen(filename)) {
    i=sprintf(buffer,"%s:[%s:%i] ", strSev,filename, linenum);
  } else
    i=sprintf(buffer,"%s: ", strSev);

  va_start(vararg,fmt);

  maxsize=sizeof(buffer)-i-1-5;
	
  x=vsnprintf(&buffer[i],maxsize,fmt,vararg);
	if (x>maxsize)
		strcat(buffer,"...");
	
  va_end(vararg);

  
  // Put the string into the circular buffer or the UART0 (depending on DebugOutput) (including the 0 terminator)
  for (i=0; i<strlen(buffer)+1; i++) {
    if (DebugOutput & DBG_MON && (buffer[i]!=0x0D && buffer[i]!=0x0A))
      Insert(buffer[i]);
  }

  MUTEX_RELEASE(mutMessageDebug);
} 

//----------------------------------------------------------------------------
// Convert one byte as 2 ASCII hex bytes.
//----------------------------------------------------------------------------
static void byte2hex(unsigned char Data, char *str) {
  unsigned char Tmp;
  Tmp=((Data >> 4) & 0x0F);
  str[0]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);

  Tmp= Data & 0x0F;
  str[1]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);
  str[2]=0;
}

//----------------------------------------------------------------------------
// Convert one 16bit word as 4 ASCII hex bytes.
//----------------------------------------------------------------------------
static void short2hex(unsigned short Data, char *str) {
  unsigned char Tmp;
  Tmp=((Data >> 12) & 0x0F);
  str[0]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);

  Tmp=((Data >> 8) & 0x0F);
  str[1]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);

  Tmp=((Data >> 4) & 0x0F);
  str[2]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);

  Tmp= Data & 0x0F;
  str[3]=(Tmp <= 9 ? Tmp+0x30 : Tmp+0x37);
  str[4]=0;
}


//----------------------------------------------------------------------------
// Dump a buffer as a hex listing
//---------------------------------------------------------------------------- 
void hexDumpDebug(int severity, char* filename, int linenum, char* header, void *Dataa, int Len) {  
  char  *Data=(char*)Dataa; 
  char  *CurData;
  int   linelen, j;
  unsigned char tmp;

  MUTEX_ACQUIRE(mutMessageDebug);

  
  messageDebug(severity, filename, linenum, "%s (%i bytes)", header, Len);
  OS_WAIT(20);
  while( Len ){
    linelen = Len < 16 ? Len : 16;

    *Line='\0';
    CurLine = Line;

    short2hex((int)Data-(int)Dataa, Line);
    strcat(Line, ": ");
    //sprintf(Line, "%04X: ", (int)Data-(int)Dataa);
    CurLine = Line+strlen(Line);

    for( CurData=Data, j=0; j<linelen; j++ ) {
      tmp=*CurData;
      byte2hex(tmp, CurLine);
      strcat(CurLine, " ");
      //sprintf( CurLine, "%02X ", tmp);
      CurLine+=3; CurData++;
    }           
    for( j=linelen; j<17; j++ ) {
      *CurLine  =' ';
      CurLine[1]=' ';
      CurLine[2]=' ';
      CurLine+=3;
    }
    for( j=0; j<linelen; j++ ) {
      sprintf( CurLine, "%c ", ((*Data >= ' ') && (*Data <= 'z')) ? *Data : '_' );
      Data++;CurLine+=1;
    }
    messageDebug(severity, filename, linenum, "%s", Line);
    OS_WAIT(100);

    Len-=linelen;
  }
  MUTEX_RELEASE(mutMessageDebug);

}

//----------------------------------------------------------------------------
// thDebugFileWriter()
//  
//----------------------------------------------------------------------------
void thDebugFileWriter(void) {
  F_FILE *fp;
  static int filesize;
//  char str[32];
  static int full=FALSE;

//  setNameRTXMON("thDebugFileWriter");

//  f_enterFS();

  while (1) {

    while (saveFilename[0]==0) {
      OS_WAIT(100);
      newDebugLine=FALSE;
    }

    // If file is full, stop saving to file
    if (full) {
      full=FALSE;
      newDebugLine=FALSE;
      strcpy(saveFilename,"");
    }

    if (newDebugLine) {
      filesize=fn_filelength(saveFilename);
      // Check size of file...
      if (filesize==F_ERR_INVALID) {
        filesize=0;
      }
      if (filesize > MAX_DEBUG_FILESIZE)
        full=TRUE;
      

    	fp = f_open(saveFilename,"a");
	    if(fp != NULL) {
  	    f_write(debugLine,1,strlen(debugLine),fp);
    	  f_close(fp);
	    }
      newDebugLine=FALSE;
    } else
      OS_WAIT(30);
  }
}
//-----------------------------------------------------------------------------
// reportSeverityDebug()
//   set mask for reporting (default is all)
//-----------------------------------------------------------------------------
void reportSeverityDebug(int sev) {
  reportSeverity=sev;
}


//-----------------------------------------------------------------------------
// initDebug()
//   Initialize debug system
//-----------------------------------------------------------------------------
void initDebug(void) {
  MUTEX_INIT(mut);
  MUTEX_INIT(mutMessageDebug);
  strcpy(saveFilename, "");
  START_THREAD_USR (thDebugFileWriter, 2, &thDebugFileWriterStack, sizeof(thDebugFileWriterStack));

}

