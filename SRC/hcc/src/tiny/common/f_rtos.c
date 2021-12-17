/***************************************************************************
 *
 *            Copyright (c) 2003-2012 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 76
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/
#include "../../api/api_tiny.h"
#include "../../oal/oal_mutex.h"
#include "tiny.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif
#include "../../version/ver_oal.h"
#if VER_OAL_MAJOR != 2
 #error Incompatible OAL version number!
#endif


static oal_mutex_t  g_tiny_mutex;


/*
** fr_findfirst
**
** find first time a file using wildcards
**
** INPUT : filename - name of the file
**         *find - pointer to a pre-define F_FIND structure
** RETURN: F_NOERR - on success
**         F_ERR_NOTFOUND - if not found
*/
#if F_FINDING
unsigned char fr_findfirst ( const _PTRQ char * filename, F_FIND * find )
{
  unsigned char  rc;
  
  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;
  
  rc = fn_findfirst( filename, find );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_findnext
**
** find next time a file using wildcards
**
** INPUT : *find - pointer to a pre-define F_FIND structure
** RETURN: F_NOERR - on success
**         F_ERR_NOTFOUND - if not found
*/
#if F_FINDING
unsigned char fr_findnext ( F_FIND * find )
{
  unsigned char  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_findnext( find );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_filelength
**
** Get the length of a file
**
** INPUT : filename - name of the file
** RETURN: size of the file or F_ERR_INVALID if not exists or volume not working
*/
#if F_FILELENGTH
long fr_filelength ( const _PTRQ char * filename )
{
  unsigned long  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_filelength( filename );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_gettimedate
**
** Gets time and date for a file.
**
** INPUT : filename - name of the file
** OUTPUT: *pctime - where to store time
**         *pcdata - where to store date
** RETURN: F_NOERR on success, other if error
*/
#if F_GETTIMEDATE
int fr_gettimedate ( const _PTRQ char * filename, unsigned short * pctime, unsigned short * pcdate )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_gettimedate( filename, pctime, pcdate );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_settimedate
**
** Sets time and date for a file.
**
** INPUT : filename - name of the file
**         pctime - new time
**         pcdata - new date
** RETURN: F_NOERR on success, other if error
*/
#if F_SETTIMEDATE
int fr_settimedate ( const _PTRQ char * filename, unsigned short pctime, unsigned short pcdate )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;
  rc = fn_settimedate( filename, pctime, pcdate );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_getpermission
**
** Get permission of a file.
**
** INPUT : filename - name of the file
**         attr - where to store attribute
** RETURN: F_NOERR on success, other if error
*/
#if F_GETPERMISSION
int fr_getpermission ( const _PTRQ char * filename, F_ATTR_TYPE * attr )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_getpermission( filename, attr );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_setpermission
**
** Set permission of a file.
**
** INPUT : filename - name of the file
**         attr - attribute
** RETURN: F_NOERR on success, other if error
*/
#if F_SETPERMISSION
int fr_setpermission ( const _PTRQ char * filename, F_ATTR_TYPE attr )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_setpermission( filename, attr );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_open
**
** open a file
**
** INPUT : filename - file to be opened
**         mode - open method (r,w,a,r+,w+,a+)
** RETURN: pointer to a file descriptor or 0 on error
*/
F_FILE * fr_open ( const _PTRQ char * filename, const _PTRQ char * mode )
{
  F_FILE * rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_open( filename, mode );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_close
**
** Close a previously opened file.
**
** INPUT : *filehandle - pointer to the file descriptor
** RETURN: F_NOERR on success, other if error
*/
int fr_close ( F_FILE * filehandle )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_close( filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_read
**
** Read from a file.
**
** INPUT : buf - buffer to read data
**         size - number of unique
**         size_st - size of unique
**         *filehandle - pointer to file descriptor
** OUTPUT: number of read bytes
*/
long fr_read ( void * bbuf, long size, long size_st, F_FILE * filehandle )
{
  long  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_read( bbuf, size, size_st, filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_write
**
** INPUT : bbuf - buffer to write from
**         size - number of unique
**         size_st - size of unique
**         *filehandle - pointer to the file descriptor
** RETURN: number of written bytes
*/
long fr_write ( const _PTRQ void * bbuf, long size, long size_st, F_FILE * filehandle )
{
  long  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_write( bbuf, size, size_st, filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_seek
**
** Seek in a file.
**
** INPUT : *filehandle - pointer to a file descriptor
**         offset - offset
**         whence - F_SEEK_SET: position = offset
**                  F_SEEK_CUR: position = position + offset
**                  F_SEEK_END: position = end of file (offset=0)
** RETURN: F_NOERR on succes, other if error.
*/
int fr_seek ( F_FILE * filehandle, long offset, long whence )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_seek( filehandle, offset, whence );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_tell
**
** get current position in the file
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_ERR_INVALID on error or current position.
*/
long fr_tell ( F_FILE * filehandle )
{
  long  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_tell( filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_getc
**
** read one byte from a file
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: -1 if error, otherwise the read character.
*/
int fr_getc ( F_FILE * filehandle )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_getc( filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_putc
**
** write one byte to a file
**
** INPUT : ch - character to write
**         *filehandle - pointer to a file handler
** RETURN: ch on success, -1 on error
*/
int fr_putc ( int ch, F_FILE * filehandle )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_putc( ch, filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_rewind
**
** set current position in the file to the beginning
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_NOERR on succes, other if error.
*/
int fr_rewind ( F_FILE * filehandle )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_rewind( filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_eof
**
** check if current position is at the end of the file.
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_ERR_EOF - at the end of the file
**         F_NOERR - no error, end of the file not reached
**         other - on error
*/
int fr_eof ( F_FILE * filehandle )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_eof( filehandle );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** Format the device
*/
unsigned char fr_format ( void )
{
  unsigned char  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_format();

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_get_serial
**
** Get serial number
**
** OUTPUT: serial - where to write the serial number
** RETURN: error code
*/
unsigned char fr_get_serial ( unsigned long * serial )
{
  unsigned char  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_get_serial( serial );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_set_serial
**
** Set serial number
**
** INPUT: serial - new serial number
** RETURN: error code
*/
unsigned char fr_set_serial ( unsigned long serial )
{
  unsigned char  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_set_serial( serial );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


/*
** fr_delete
**
** Delete a file. Removes the chain that belongs to the file and inserts a new descriptor
** to the directory with first_cluster set to 0.
**
** INPUT : filename - name of the file to delete
** RETURN: F_NOERR on success, other if error.
*/
#if F_DELETE
int fr_delete ( const _PTRQ char * filename )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (filename[1]==':') filename +=2;

  rc = fn_delete( filename );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_rename
**
** Rename a file.
**
** INPUT : oldname - old name of the file
**         newname - new name of the file
** RETURN: F_NOERR on success, other if error
*/
#if F_RENAME
int fr_rename ( const _PTRQ char * oldname, const _PTRQ char * newname )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (oldname[1]==':') oldname +=2;
  // Skip any driveletters
  if (newname[1]==':') newname +=2;

  rc = fn_rename( oldname, newname );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_getfreespace
**
** Get free space on the volume
**
** OUTPUT: *sp - pre-defined F_SPACE structure, where information will be stored
** RETURN: F_NOERR - on success
**         F_ERR_NOTFORMATTED - if volume is not formatted
*/
#if F_GETFREESPACE
unsigned char fr_getfreespace ( F_SPACE * sp )
{
  unsigned char  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_getfreespace( sp );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_chdir
**
** Change to a directory
**
** INPUT:  path - path to the dircetory
** RETURN: 0 - on success, other if error
*/
#if F_DIRECTORIES && F_CHDIR
int fr_chdir ( const _PTRQ char * path )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (path[1]==':') path +=2;

  rc = fn_chdir( path );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_mkdir
**
** Create a directory
**
** INPUT:  path - new directory path
** RETURN: 0 - on success, other if error
*/
#if F_DIRECTORIES && F_MKDIR
int fr_mkdir ( const _PTRQ char * path )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (path[1]==':') path +=2;

  rc = fn_mkdir( path );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_rmdir
**
** Removes a directory
**
** INPUT:  path - path to remove
** RETURN: 0 - on success, other if error
*/
#if F_DIRECTORIES && F_RMDIR
int fr_rmdir ( const _PTRQ char * path )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );
  // Skip any driveletters
  if (path[1]==':') path +=2;

  rc = fn_rmdir( path );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fr_getcwd
**
** Get current working directory
**
** INPUT:  maxlen - maximum length allowed
** OUTPUT: path - current working directory
** RETURN: 0 - on success, other if error
*/
#if F_DIRECTORIES && F_GETCWD
int fr_getcwd ( char * path, int maxlen )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_getcwd( path, maxlen );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}
#endif


/*
** fn_ftruncate
**
** Truncate a file.
**
** INPUT: *filehandle - pointer to the file descriptor
**        length      - desired length
** RETURN: F_NOERR on succes, other if error.
*/
#if F_TRUNCATE
int fr_ftruncate ( F_FILE * filehandle, unsigned long length )
{
  int  rc;

  oal_mutex_get( &g_tiny_mutex );

  rc = fn_ftruncate ( filehandle, length );

  oal_mutex_put( &g_tiny_mutex );

  return rc;
}


#endif


/*
** fr_init
**
** Initialize TINY OS module
**
** RETURN: F_NOERR or F_ERR_OS
*/
unsigned char fr_init ( void )
{
  unsigned char  rc;

  rc = F_NOERR;
  if ( oal_mutex_create( &g_tiny_mutex ) != OAL_SUCCESS )
  {
    rc = F_ERR_OS;
  }

  return rc;
}

