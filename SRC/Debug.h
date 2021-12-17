//-----------------------------------------------------------------------------
// Debug.h                                                         CHG 20060904
//
// 
//-----------------------------------------------------------------------------
#ifndef _DEBUG_H
#define _DEBUG_H
#include <stdarg.h>

#ifdef _DEBUG_ 
#define MESSAGEDEBUG(a,b,c,d,...) messageDebug(a,b,c,d,__VA_ARGS__)
#define HEXDUMPDEBUG(a,b,c,d,e,f) hexDumpDebug(a,b,c,d,e,f)
#else
#define MESSAGEDEBUG(a,b,c,d,...)
#define HEXDUMPDEBUG(a,b,c,d,e,...)
#endif
//-----------------------------------------------------------------------------
// Type of debug message
//-----------------------------------------------------------------------------
#define  DBG_INFO 0x01  // Informational
#define  DBG_WAR  0x02  // Warning
#define  DBG_ERR  0x04  // Error

//-----------------------------------------------------------------------------
// Constants for where the debug messages should be sent to
//-----------------------------------------------------------------------------
#define DBG_NONE   0x00 // Do not send Debug messages at all (default)
#define DBG_SERIAL 0x01 // Send debug outputs to Serial port 
#define DBG_MON   0x02 // Send debug outputs to PMON/SMON


//-----------------------------------------------------------------------------
// Flush debug buffer
//-----------------------------------------------------------------------------
void flushDebug(void);

//-----------------------------------------------------------------------------
// Return true if debug buffer (circula) is empty
//-----------------------------------------------------------------------------
int isEmptyDebug(void);

//-----------------------------------------------------------------------------
// Called by SMon/PhoenixUSB when no more debug data to send (all has been delivered)
//-----------------------------------------------------------------------------
void setAllSentDebug(void);

//-----------------------------------------------------------------------------
// Save current contents of debug buffer to a file
//-----------------------------------------------------------------------------
void storeBufferDebug(char *filename);

//-----------------------------------------------------------------------------
// setOutputDebug()
//   Select where to send debug messages
// Input: 
//   dbg: DBG_NONE(default), DBG_SERIAL or DBG_USB (both DBG_SERIAL and DBG_USB 
//   can set at the same time !)
//-----------------------------------------------------------------------------
void setOutputDebug(int dbg);

//-----------------------------------------------------------------------------
// reportSeverityDebug()
//   set mask for reporting (default is all)
//-----------------------------------------------------------------------------
void reportSeverityDebug(int sev);

//-----------------------------------------------------------------------------
// InfoMsg()
//   Send a Debug message to the debug console using specified severity
//   (function works like printf regarding formatting etc)
//-----------------------------------------------------------------------------
void messageDebug(int severity, char* filename, int linenum, char *fmt,...);
void IntmessageDebug(int severity, char* filename, int linenum, char *fmt,va_list varg);

//----------------------------------------------------------------------------
// Dump a buffer as a hex listing
//---------------------------------------------------------------------------- 
void hexDumpDebug(int severity, char* filename, int linenum, char* header, void *Dataa, int Len);

//------------------------------------------------------------------------------
// removeCharDebug()
// Helper function for low-level transmit routines
//   Try to remove one character from the debug buffer, returns 1 if no char 
//   available, 0 if char available (will be put in ch)
//------------------------------------------------------------------------------
int removeCharDebug(char *ch);

//------------------------------------------------------------------------------
// numCharAvailableDebug()
// Helper function for low-level transmit routines
//   Returns number of characters currently waiting in the debug buffer
//------------------------------------------------------------------------------
int numCharAvailableDebug(void);

//------------------------------------------------------------------------------
// setFilenameDebug()
// Set what filename to use for storing debug info
//------------------------------------------------------------------------------
void setFilenameDebug(char *filename);

//-----------------------------------------------------------------------------
// initDebug()
//   Initialize debug system
//-----------------------------------------------------------------------------
void initDebug(void);

#endif
