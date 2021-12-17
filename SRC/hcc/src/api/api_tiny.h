/***************************************************************************
 *
 *            Copyright (c) 2003-2013 by HCC Embedded
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
#ifndef _API_TINY_H
#define _API_TINY_H

#include "../config/config_tiny.h"
#include "../tiny/common/tiny.h"

#include "../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 *  return error codes
 ***************************************************************************/
#define F_NO_ERROR 0  
#define F_NOERR             0     /* no error, successfully returned */
#define F_ERR_INVALIDVOLUME 1     /* no volume available */
#define F_ERR_INVALIDHANDLE 2     /* invalid file handle */
#define F_ERR_INVALIDOFFSET 3     /* invalid offset */
#define F_ERR_INVALIDMODE   4     /* invalid mode */
#define F_ERR_EOF           5     /* end of file */
#define F_ERR_NOTFOUND      6     /* file not found */
#define F_ERR_DIRFULL       7     /* directory full */
#define F_ERR_INVALIDNAME   8     /* invalid name */
#define F_ERR_INVALIDDIR    9     /* invalid dir */
#define F_ERR_OPEN          10    /* file/directory in use */
#define F_ERR_NOTOPEN       11    /* file not opened or opened in different mode */
#define F_ERR_NOTFORMATTED  12    /* not formatted */
#define F_ERR_DIFFMEDIA     13    /* different media */
#define F_ERR_NOMOREENTRY   14    /* no more entry available */
#define F_ERR_DUPLICATED    15    /* duplicated filename */
#define F_ERR_NOTEMPTY      16    /* trying to remove a directory that is not empty */
#define F_ERR_INVALIDSIZE   17    /* when buffer size is too small (getcwd) */
#define F_ERR_ACCESSDENIED  18    /* access is denied */
#define F_ERR_BUSY          19    /* system busy, mutex get failure */
#define F_ERR_CORRUPTED     20    /* corrupted file */
#define F_ERR_OS            21    /* OS error */
#define F_ERROR             22    /* general error */

#define F_INVALID           ( -1 )    /* invalid value DON'T CHANGE!!! */
#define F_ERR_INVALID       F_INVALID /* invalid data (e.g. f_filelength if file not found) */


/****************************************************************************
 *  File states
 ***************************************************************************/
#define F_FILE_STATE_EOF    0x01


/****************************************************************************
 *  f_seek whence params
 ***************************************************************************/

/* from Beginning of file */
#ifdef SEEK_SET
 #define F_SEEK_SET SEEK_SET
#else
 #define F_SEEK_SET 0
#endif

/* from Current position of file pointer */
#ifdef SEEK_CUR
 #define F_SEEK_CUR SEEK_CUR
#else
 #define F_SEEK_CUR 1
#endif

/* from End of file */
#ifdef SEEK_END
 #define F_SEEK_END         SEEK_END
#else
 #define F_SEEK_END         2
#endif

/*  definitions for ctime  */
#define F_CTIME_SEC_SHIFT   0
#define F_CTIME_SEC_MASK    0x001f  /* 0-30 in 2seconds */
#define F_CTIME_MIN_SHIFT   5
#define F_CTIME_MIN_MASK    0x07e0  /* 0-59  */
#define F_CTIME_HOUR_SHIFT  11
#define F_CTIME_HOUR_MASK   0xf800  /* 0-23 */

/*  definitions for cdate  */
#define F_CDATE_DAY_SHIFT   0
#define F_CDATE_DAY_MASK    0x001f  /* 0-31 */
#define F_CDATE_MONTH_SHIFT 5
#define F_CDATE_MONTH_MASK  0x01e0  /* 1-12 */
#define F_CDATE_YEAR_SHIFT  9
#define F_CDATE_YEAR_MASK   0xfe00  /* 0-119 (1980+value) */


/****************************************************************************
 *  findfirst, findnext structure
 ***************************************************************************/
typedef struct
{
  F_ATTR_TYPE     attr;                     /* file attr */
  unsigned short  ctime;                    /* creation time */
  unsigned short  cdate;                    /* creation date */
  F_LENGTH_TYPE   filesize;                 /* length of the file */

  char            filename[F_MAX_FILE_NAME_LENGTH + 1]; /* file name */

#if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid;                     /* current directory position */
#endif

  F_FIND_ID_TYPE  findpos;                  /* internally used position */

  _PTRQ char    * findname;                 /* original filename for searching */
#if QUICK_FILE_SEARCH
  unsigned char   wildcard;                 /* wildcard filename */
  F_QSKEY_TYPE    find_qskey;               /* quick search key */
#endif
#if QUICK_WILDCARD_SEARCH
  F_QWS_PAGE_COUNT_TYPE      qws_pg;
  F_QWS_ENTRY_PER_PAGE_TYPE  qws_pgp;
#endif
} F_FIND;


/****************************************************************************
 *  F_FILE structure
 ***************************************************************************/
typedef struct
{
  F_LENGTH_TYPE         length;                         /* length of te file */
  F_LENGTH_TYPE         abs_pos;                        /* absolute position in the file */

  F_CLUSTER_TYPE        cluster_pos;                    /* actual cluster position */
  F_CLUSTER_COUNT_TYPE  cluster;                        /* actual cluster */
#if (F_SEEK_WRITE)
  F_CLUSTER_COUNT_TYPE  orig_cluster;                   /* to hold original cluster value of the actual position */
#endif

  F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  page_cl_pos;  /* actual cluster position in the page */
  F_FILE_MGM_PAGE_COUNT_TYPE              page_cnt;     /* actual page count */

  F_FILE_ID_TYPE                          fileid;         /* fileid */
  F_FILE_MGM_PAGE_COUNT_TYPE              first_mgm_page; /* first file management page */
  F_FILE_MGM_PAGE_COUNT_TYPE              act_mgm_page;   /* actual management page */

#if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid;                                 /* dirid */
#endif

  unsigned char  mode;
  unsigned char  state;                                 /* state */
#if F_FILE_CHANGED_EVENT
  char           filename[_F_FILE_CHANGED_MAXPATH];     /* filename with full path */
#endif
} F_FILE;



/****************************************************************************
 *  F_SPACE structure
 ***************************************************************************/
typedef struct
{
  F_LENGTH_TYPE  total; /* total size on volume */
  F_LENGTH_TYPE  free;  /* free space on volume */
} F_SPACE;


/****************************************************************************
 * for file changed events
 ***************************************************************************/
#if F_FILE_CHANGED_EVENT

typedef struct
{
  unsigned char  action;
  unsigned char  flags;
  char           filename[_F_FILE_CHANGED_MAXPATH];
} ST_FILE_CHANGED;

typedef void ( * F_FILE_CHANGED_EVENT_FN )( ST_FILE_CHANGED * fc );
extern F_FILE_CHANGED_EVENT_FN  f_filechangedevent_fn;
 #define f_setfilechangedevent( fn ) f_filechangedevent_fn = fn

/* flags */
 #define FFLAGS_FILE_NAME 0x00000001
 #define FFLAGS_DIR_NAME  0x00000002

/* actions */
 #define FACTION_ADDED    0x00000001
 #define FACTION_REMOVED  0x00000002
 #define FACTION_MODIFIED 0x00000003

/*#define FACTION_RENAMED_OLD_NAME        0x00000004*/
/*#define FACTION_RENAMED_NEW_NAME        0x00000005*/

#endif /* if F_FILE_CHANGED_EVENT */


/****************************************************************************
 *  functions
 ***************************************************************************/
unsigned char fn_init ( void );

unsigned char fn_initvolume ( void );
#if F_FINDING
unsigned char fn_findfirst ( const _PTRQ char * filename, F_FIND * find );
unsigned char fn_findnext ( F_FIND * find );
#endif
#if F_FILELENGTH
long fn_filelength ( const _PTRQ char * filename );
#endif
#if F_GETTIMEDATE
int fn_gettimedate ( const _PTRQ char * filename, unsigned short * pctime, unsigned short * pcdate );
#endif
#if F_SETTIMEDATE
int fn_settimedate ( const _PTRQ char * filename, unsigned short pctime, unsigned short pcdate );
#endif
#if F_GETPERMISSION
int fn_getpermission ( const _PTRQ char * filename, F_ATTR_TYPE * attr );
#endif
#if F_SETPERMISSION
int fn_setpermission ( const _PTRQ char * filename, F_ATTR_TYPE attr );
#endif
F_FILE * fn_open ( const _PTRQ char * filename, const _PTRQ char * mode );
int fn_close ( F_FILE * filehandle );
long fn_read ( void * bbuf, long size, long size_st, F_FILE * filehandle );
long fn_write ( const _PTRQ void * bbuf, long size, long size_st, F_FILE * filehandle );
int fn_seek ( F_FILE *, long, long );
long fn_tell ( F_FILE * );
int fn_getc ( F_FILE * );
int fn_putc ( int, F_FILE * );
int fn_rewind ( F_FILE * );
int fn_eof ( F_FILE * );
#if F_TRUNCATE
int  fn_ftruncate ( F_FILE * filehandle, unsigned long length );
#endif
unsigned char fn_format ( void );
unsigned char fn_get_serial ( unsigned long * serial );
unsigned char fn_set_serial ( unsigned long serial );
unsigned char fn_get_size ( unsigned long * size );
#if F_DELETE
int fn_delete ( const _PTRQ char * filename );
#endif
#if F_RENAME
int fn_rename ( const _PTRQ char * oldname, const _PTRQ char * newname );
#endif
#if F_GETFREESPACE
unsigned char fn_getfreespace ( F_SPACE * );
#endif

#if F_DIRECTORIES
 #if F_CHDIR
int fn_chdir ( const _PTRQ char * path );
 #endif
 #if F_MKDIR
int fn_mkdir ( const _PTRQ char * path );
 #endif
 #if F_RMDIR
int fn_rmdir ( const _PTRQ char * path );
 #endif
 #if F_GETCWD
int fn_getcwd ( char * path, int maxlen );
 #endif
#endif


#define f_init()           fn_init()
#define f_initvolume()     fn_initvolume()
#define f_get_size( size ) fn_get_size( size )

#if RTOS_SUPPORT

 #if F_FINDING
unsigned char fr_findfirst ( const _PTRQ char * filename, F_FIND * find );
  #define f_findfirst( filename, find )             fr_findfirst( filename, find )
unsigned char fr_findnext ( F_FIND * find );
  #define f_findnext( find )                        fr_findnext( find )
 #endif
 #if F_FILELENGTH
long fr_filelength ( const _PTRQ char * filename );
  #define f_filelength( filename )                  fr_filelength( filename )
 #endif
 #if F_GETTIMEDATE
int fr_gettimedate ( const _PTRQ char * filename, unsigned short * pctime, unsigned short * pcdate );
  #define f_gettimedate( filename, pctime, pcdate ) fr_gettimedate( filename, pctime, pcdate )
 #endif
 #if F_SETTIMEDATE
int fr_settimedate ( const _PTRQ char * filename, unsigned short pctime, unsigned short pcdate );
  #define f_settimedate( filename, pctime, pcdate ) fr_settimedate( filename, pctime, pcdate )
 #endif
 #if F_GETPERMISSION
int fr_getpermission ( const _PTRQ char * filename, F_ATTR_TYPE * attr );
  #define f_getpermission( filename, attr )         fr_getpermission( filename, attr )
 #endif
 #if F_SETPERMISSION
int fr_setpermission ( const _PTRQ char * filename, F_ATTR_TYPE attr );
  #define f_setpermission( filename, attr )         fr_setpermission( filename, attr )
 #endif
F_FILE * fr_open ( const _PTRQ char * filename, const _PTRQ char * mode );
 #define f_open( filename, mode )                   fr_open( filename, mode )
int fr_close ( F_FILE * filehandle );
 #define f_close( filehandle )                      fr_close( filehandle )
long fr_read ( void * bbuf, long size, long size_st, F_FILE * filehandle );
 #define f_read( bbuf, size, size_st, filehandle )  fr_read( bbuf, size, size_st, filehandle )
long fr_write ( const _PTRQ void * bbuf, long size, long size_st, F_FILE * filehandle );
 #define f_write( bbuf, size, size_st, filehandle ) fr_write( bbuf, size, size_st, filehandle )
int fr_seek ( F_FILE *, long, long );
 #define f_seek( filehandle, offset, whence )       fr_seek( filehandle, offset, whence )
long fr_tell ( F_FILE * );
 #define f_tell( filehandle )                       fr_tell( filehandle )
int fr_getc ( F_FILE * );
 #define f_getc( filehandle )                       fr_getc( filehandle )
int fr_putc ( int, F_FILE * );
 #define f_putc( ch, filehandle )                   fr_putc( ch, filehandle )
int fr_rewind ( F_FILE * );
 #define f_rewind( filehandle )                     fr_rewind( filehandle )
int fr_eof ( F_FILE * );
 #define f_eof( filehandle )                        fr_eof( filehandle )

 #if F_TRUNCATE
int  fr_ftruncate ( F_FILE * filehandle, unsigned long length );
  #define f_ftruncate( filehandle, length )         fr_ftruncate( filehandle, length )
 #endif

unsigned char fr_format ( void );
 #define f_format()                                 fr_format()
unsigned char fr_get_serial ( unsigned long * serial );
 #define f_get_serial( serial )                     fr_get_serial( serial )
unsigned char fr_set_serial ( unsigned long serial );
 #define f_set_serial( serial )                     fr_set_serial( serial )
 #if F_DELETE
int fr_delete ( const _PTRQ char * filename );
  #define f_delete( filename )                      fr_delete( filename )
 #endif
 #if F_RENAME
int fr_rename ( const _PTRQ char * oldname, const _PTRQ char * newname );
  #define f_rename( oldname, newname )              fr_rename( oldname, newname )
 #endif
 #if F_GETFREESPACE
unsigned char fr_getfreespace ( F_SPACE * );
  #define f_getfreespace( sp )                      fr_getfreespace( sp )
 #endif

 #if F_DIRECTORIES
  #if F_CHDIR
int fr_chdir ( const _PTRQ char * path );
   #define f_chdir( path )          fr_chdir( path )
  #endif
  #if F_MKDIR
int fr_mkdir ( const _PTRQ char * path );
   #define f_mkdir( path )          fr_mkdir( path )
  #endif
  #if F_RMDIR
int fr_rmdir ( const _PTRQ char * path );
   #define f_rmdir( path )          fr_rmdir( path )
  #endif
  #if F_GETCWD
int fr_getcwd ( char * path, int maxlen );
   #define f_getcwd( path, maxlen ) fr_getcwd( path, maxlen )
  #endif
 #endif /* if F_DIRECTORIES */

#else   /* no RTOS */

 #if F_FINDING
  #define f_findfirst( filename, find )             fn_findfirst( filename, find )
  #define f_findnext( find )                        fn_findnext( find )
 #endif
 #if F_FILELENGTH
  #define f_filelength( filename )                  fn_filelength( filename )
 #endif
 #if F_GETTIMEDATE
  #define f_gettimedate( filename, pctime, pcdate ) fn_gettimedate( filename, pctime, pcdate )
 #endif
 #if F_SETTIMEDATE
  #define f_settimedate( filename, pctime, pcdate ) fn_settimedate( filename, pctime, pcdate )
 #endif
 #if F_GETPERMISSION
  #define f_getpermission( filename, attr )         fn_getpermission( filename, attr )
 #endif
 #if F_SETPERMISSION
  #define f_setpermission( filename, attr )         fn_setpermission( filename, attr )
 #endif
 #define f_open( filename, mode )                   fn_open( filename, mode )
 #define f_close( filehandle )                      fn_close( filehandle )
 #define f_read( bbuf, size, size_st, filehandle )  fn_read( bbuf, size, size_st, filehandle )
 #define f_write( bbuf, size, size_st, filehandle ) fn_write( bbuf, size, size_st, filehandle )
 #define f_seek( filehandle, offset, whence )       fn_seek( filehandle, offset, whence )
 #define f_tell( filehandle )                       fn_tell( filehandle )
 #define f_getc( filehandle )                       fn_getc( filehandle )
 #define f_putc( ch, filehandle )                   fn_putc( ch, filehandle )
 #define f_rewind( filehandle )                     fn_rewind( filehandle )
 #define f_eof( filehandle )                        fn_eof( filehandle )

 #if F_TRUNCATE
  #define f_ftruncate( filehandle, length )         fn_ftruncate( filehandle, length )
 #endif

 #define f_format()                                 fn_format()
 #define f_get_serial( serial )                     fn_get_serial( serial )
 #define f_set_serial( serial )                     fn_set_serial( serial )
 #if F_DELETE
  #define f_delete( filename )                      fn_delete( filename )
 #endif
 #if F_RENAME
  #define f_rename( oldname, newname )              fn_rename( oldname, newname )
 #endif
 #if F_GETFREESPACE
  #define f_getfreespace( sp )                      fn_getfreespace( sp )
 #endif

 #if F_DIRECTORIES
  #if F_CHDIR
   #define f_chdir( path )          fn_chdir( path )
  #endif
  #if F_MKDIR
   #define f_mkdir( path )          fn_mkdir( path )
  #endif
  #if F_RMDIR
   #define f_rmdir( path )          fn_rmdir( path )
  #endif
  #if F_GETCWD
   #define f_getcwd( path, maxlen ) fn_getcwd( path, maxlen )
  #endif
 #endif

#endif /* if RTOS_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* ifndef _API_TINY_H */

