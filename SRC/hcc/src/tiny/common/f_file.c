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
#include "../../psp/include/psp_string.h"
#include "../../api/api_tiny.h"
#include "tiny.h"
#include "f_volume.h"
#include "f_dir.h"
#include "f_util.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif
#include "../../version/ver_psp_string.h"
#if VER_PSP_STRING_MAJOR != 1
 #error Incompatible PSP_STRING version number!
#endif


/**************************************************************************/
/************************* INTERNAL FUNCTIONS *****************************/
/**************************************************************************/


/*
** _f_checkopen
**
** Check if file is already opened or not
**
** INPUT : depos - directory position
** RETURN: 1-opened / 0-not opened
*/
unsigned char _f_checkopen ( F_FILE_ID_TYPE fileid )
{
  unsigned char  i;

  for ( i = 0 ; i < F_MAX_OPEN_FILE ; i++ )
  {
    if ( ( files[i].mode != F_MODE_CLOSE ) && ( files[i].fileid == fileid ) )
    {
      return 1;
    }
  }

  return 0;
}


/*
** _f_checkopendir
**
** Check if file is already opened with define directory ID
**
** INPUT : depos - directory position
** RETURN: 1-opened / 0-not opened
*/
#if F_DIRECTORIES
unsigned char _f_checkopendir ( F_DIR_ID_TYPE dirid )
{
  unsigned char  i;

  for ( i = 0 ; i < F_MAX_OPEN_FILE ; i++ )
  {
    if ( ( files[i].mode != F_MODE_CLOSE ) && ( files[i].dirid == dirid ) )
    {
      return 1;
    }
  }

  return 0;
}
#endif


/*
** _f_getfreefile
**
** Gets a free file descriptor if available
**
** RETURN: file descriptor of F_INVALID if not available
*/
F_FILE * _f_getfreefile ( void )
{
  unsigned char  i;
  F_FILE       * file = NULL;

  /* get free file entry */
  for ( i = 0 ; i < F_MAX_OPEN_FILE ; i++ )
  {
    if ( files[i].mode == F_MODE_CLOSE )
    {
      file = &files[i];
      psp_memset( file, 0, sizeof( F_FILE ) );
#if (F_SEEK_WRITE)
      file->orig_cluster = (F_CLUSTER_COUNT_TYPE)F_INVALID;
#endif
      file->cluster_pos = F_CLUSTER_SIZE;
      break;
    }
  }

  return file;
} /* _f_getfreefile */

/**************************************************************************/
/*************************** USER FUNCTIONS *******************************/
/**************************************************************************/


/*
** fn_open
**
** open a file
**
** INPUT : filename - file to be opened
**         mode - open method (r,w,a,r+,w+,a+)
** RETURN: pointer to a file descriptor or 0 on error
*/
F_FILE * fn_open ( const _PTRQ char * path, const _PTRQ char * mode )
{
  F_FILE        * file = NULL;
  F_FILE_PAR      par;
  F_FIND_ID_TYPE  id;
  unsigned char   reqmode = F_MODE_CLOSE;
  unsigned char   rc = 0;
  _PTRQ char    * filename;


  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return NULL;
  }

  if ( ( mode[1] == 0 ) || ( ( mode[1] == '+' ) && ( mode[2] == 0 ) ) )
  {
    if ( mode[0] == 'r' )
    {
      reqmode |= F_MODE_READ;
    }

    if ( mode[0] == 'w' )
    {
      reqmode |= F_MODE_WRITE;
    }

    if ( mode[0] == 'a' )
    {
      reqmode |= F_MODE_APPEND;
    }

    if ( mode[1] == '+' )
    {
      reqmode |= F_MODE_PLUS;               /* set open mode */
    }
  }
  else
  {
    return NULL;
  }

  if ( ( reqmode & ( F_MODE_READ | F_MODE_WRITE | F_MODE_APPEND ) ) == 0 )
  {
    return NULL;
  }

  file = _f_getfreefile();
  if ( !file )
  {
    return NULL;            /* no more files available */
  }

  /* check if filename is available */
#if F_DIRECTORIES
  file->dirid = _f_check_path( path, &filename );
  if ( file->dirid == (F_DIR_ID_TYPE)F_INVALID )
  {
    return NULL;
  }

  if ( ( filename == NULL ) || ( *filename == 0 ) )
  {
    return NULL;
  }

  id = _f_find( filename, &par, file->dirid );
#else
  filename = (_PTRQ char *)path;
  id = _f_find( filename, &par );
#endif
  if ( id != (F_FIND_ID_TYPE)F_INVALID )
  {
#if F_DIRECTORIES
    if ( F_GET_ATTR( par.attr ) & F_ATTR_DIR )
    {
      return NULL;
    }

#endif
    file->fileid = (F_FILE_ID_TYPE)id;
    file->first_mgm_page = _FILE_ID_MGM_PAGE( (F_FILE_ID_TYPE)id );
    file->length = F_GET_LENGTH( par.length );
  }

  switch ( reqmode & ( ~F_MODE_PLUS ) )
  {
    case F_MODE_READ:
      if ( id == (F_FIND_ID_TYPE)F_INVALID )
      {
        return NULL;                                    /* file not available */
      }

      if ( ( reqmode & F_MODE_PLUS ) == 0 )
      {
        break;
      }

    case F_MODE_APPEND:
      if ( id != (F_FIND_ID_TYPE)F_INVALID )
      {
        if ( _f_checkopen( (F_FILE_ID_TYPE)id ) )
        {
          return NULL;
        }

        file->first_mgm_page = _f_copy_mgm( file->first_mgm_page );     /* copy management area of the original file */
        if ( file->first_mgm_page == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
        {
          return NULL;
        }

        file->act_mgm_page = file->first_mgm_page;
#if F_FILE_CHANGED_EVENT
        _f_createfullname( file->filename, path );
#endif
        if ( reqmode & F_MODE_APPEND )
        {
          (void)fn_seek( file, 0, F_SEEK_END );                         /* seek to the end of the file if append (not r+) */
          file->mode = reqmode;
          return file;
        }

        break;
      }

    case F_MODE_WRITE:
      if ( id == (F_FIND_ID_TYPE)F_INVALID )
      {
        unsigned short  ctime, cdate;
        file->fileid = _f_add_entry( filename, (F_FILE_ID_TYPE)F_INVALID, file );
        if ( file->fileid == (F_FILE_ID_TYPE)F_INVALID )
        {
          return NULL;
        }

        psp_memset( &par, 0, sizeof( F_FILE_PAR ) );
        _f_get_timedate( &ctime, &cdate );
        _f_setvalue_2( par.ctime, ctime );
        _f_setvalue_2( par.cdate, cdate );
        rc = f_flash_mgm_write( _FILE_MGM_DSC_ADDR( file->first_mgm_page ) + _SADDR( F_FILE_DSC, par ), &par, sizeof( F_FILE_PAR ) );
      }
      else
      {
        if ( _f_checkopen( id ) )
        {
          return NULL;
        }

        if ( _f_add_entry( filename, file->fileid, file ) == (F_FILE_ID_TYPE)F_INVALID )
        {
          return NULL;
        }

        file->length = 0;
      }

#if F_DIRECTORIES
      if ( rc == F_NOERR )
      {
        unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
        F_SET_DIR_ID( _dirid, file->dirid );
        rc = f_flash_mgm_write( _FILE_MGM_DSC_ADDR( file->first_mgm_page ) + _SADDR( F_FILE_DSC, dirid ), _dirid, F_DIR_ID_TYPE_SIZE );
      }

#endif
      if ( rc )
      {
        _CLEAR_BIT( f_volume.file_mgm_table, file->first_mgm_page );
        return NULL;
      }

#if F_FILE_CHANGED_EVENT
      _f_createfullname( file->filename, path );
#endif
      break;
  } /* switch */

  file->act_mgm_page = file->first_mgm_page;
  file->mode = reqmode;
  return file;
} /* fn_open */



/*
** fn_close
**
** Close a previously opened file.
**
** INPUT : *filehandle - pointer to the file descriptor
** RETURN: F_NOERR on success, other if error
*/
int fn_close ( F_FILE * filehandle )
{
  unsigned char  rc = F_NOERR;
  unsigned char  tmp;
  F_FILE_PAR     par;

#if F_FILE_CHANGED_EVENT
  ST_FILE_CHANGED  fc;
#endif

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_INVALIDHANDLE;
  }

  if ( !filehandle )
  {
    return F_ERR_INVALIDHANDLE;
  }

  switch ( filehandle->mode & ( F_MODE_READ | F_MODE_WRITE | F_MODE_APPEND ) )
  {
    case F_MODE_READ:   /* when opened for read, there is nothing to do */
      if ( ( filehandle->mode & F_MODE_PLUS ) == 0 )
      {
        break;
      }

    case F_MODE_WRITE:
    case F_MODE_APPEND:
      if ( ( filehandle->mode & ( F_MODE_READ | F_MODE_APPEND ) ) && ( ( filehandle->mode & F_MODE_CHANGED ) == 0 )
          && ( _FILE_ID_MGM_PAGE( filehandle->fileid ) != filehandle->first_mgm_page ) )
      {
        rc = _f_remove_file_mgm( filehandle->first_mgm_page, 0 ); /* remove new file */
      }
      else
      {
        rc = f_flash_mgm_read( &par, _FILE_ID_MGM_DSC_ADDR( filehandle->fileid ) + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
        if ( rc == F_NOERR )
        {
          unsigned short  ctime, cdate;
#if SMALL_FILE_OPT
          F_LENGTH_TYPE   olen = F_GET_LENGTH( par.length );
#endif

          /* store parameter block */
          _f_get_timedate( &ctime, &cdate );
          _f_setvalue_2( par.ctime, ctime );
          _f_setvalue_2( par.cdate, cdate );
          F_SET_ATTR( par.attr, 0 );
          F_SET_LENGTH( par.length, filehandle->length );
          rc = f_flash_mgm_write( _FILE_MGM_DSC_ADDR( filehandle->first_mgm_page ) + _SADDR( F_FILE_DSC, par ), &par, sizeof( F_FILE_PAR ) );
          if ( rc == F_NOERR )
          {
            /* set closed state on the first file management block */
            tmp = _FM_STATE_CLOSE;
            rc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( filehandle->first_mgm_page ) + _SADDR( F_FILE_MGM_HEADER, state ), &tmp, sizeof( unsigned char ) );
            tmp = (unsigned char)~_FM_STATE_CLOSE;
            rc = f_flash_mgm_write_safe( _FILE_MGM_HEADER_ADDR( filehandle->first_mgm_page )
                                        + _SADDR( F_FILE_MGM_HEADER, inv_state ), &tmp, sizeof( unsigned char ) );
            if ( rc == F_NOERR )
            {
              /* if current file is a modification of a previous file */
              if ( _FILE_ID_MGM_PAGE( filehandle->fileid ) != filehandle->first_mgm_page )
              {
                /* remove old file (if small files can be in management page then only remove clusters if needed) */
#if SMALL_FILE_OPT
                rc = _f_remove_file_mgm( _FILE_ID_MGM_PAGE( filehandle->fileid ), (char)( ( olen > F_FILE_MGM_PAGE0_SIZE ) ? 1 : 0 ) );
#else
                rc = _f_remove_file_mgm( _FILE_ID_MGM_PAGE( filehandle->fileid ), 1 );
#endif
                if ( rc )
                {
                  break;
                }

#if SMALL_FILE_OPT

                /* mark new clusters if file is using clusters (instead of MGM page) */
                if ( ( filehandle->length ) > F_FILE_MGM_PAGE0_SIZE )
#endif
                {
                  filehandle->act_mgm_page = filehandle->first_mgm_page;
                  filehandle->page_cnt = 0;
                  for ( filehandle->page_cl_pos = 0 ; ; )
                  {
                    rc = _f_get_cluster_value( filehandle );
                    if ( rc )
                    {
                      rc = F_NOERR;
                      break;
                    }

                    if ( filehandle->cluster >= F_CLUSTER_COUNT )
                    {
                      rc = F_ERR_CORRUPTED;
                      break;
                    }

                    _SET_BIT( f_volume.cluster_table, filehandle->cluster );    /* and mark new file clusters */
                  }
                }

                /* change fileid first mgm page */
                if ( rc == F_NOERR )
                {
                  F_FILE_ID_TYPE  i;
                  for ( i = 0 ; i < F_MAX_FILE && _FILE_ID_MGM_PAGE( i ) != _FILE_ID_MGM_PAGE( filehandle->fileid ) ; i++ )
                  {
                  }

                  if ( i < F_MAX_FILE )
                  {
                    _FILE_ID_MGM_PAGE( i ) = filehandle->first_mgm_page;
                  }
                }

#if F_FILE_CHANGED_EVENT
                fc.action = FACTION_MODIFIED;
#endif
              }

#if F_FILE_CHANGED_EVENT
              else
              {
                fc.action = FACTION_ADDED;
              }
#endif
            }
          }
        }

        if ( rc )
        {
          rc = _f_remove_file_mgm( filehandle->first_mgm_page, 1 );
        }

#if F_FILE_CHANGED_EVENT
        else if ( f_filechangedevent_fn != NULL )
        {
          fc.flags = FFLAGS_FILE_NAME;
          psp_strncpy( fc.filename, filehandle->filename, _F_FILE_CHANGED_MAXPATH );
          f_filechangedevent_fn( &fc );
        }
#endif
      }

      break;

    default:
      return F_ERR_NOTOPEN;
  } /* switch */

  filehandle->mode = F_MODE_CLOSE;
  return rc;
} /* fn_close */



/*
** fn_read
**
** Read from a file.
**
** INPUT : buf - buffer to read data
**         size - number of unique
**         size_st - size of unique
**         *filehandle - pointer to file descriptor
** OUTPUT: number of read bytes
*/
long fn_read ( void * bbuf, long size, long size_st, F_FILE * filehandle )
{
  F_CLUSTER_TYPE  pos, rsize;
  _PTRQ char    * buf = (_PTRQ char *)bbuf;
  long            retsize;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return 0;
  }

  if ( !filehandle )
  {
    return 0;                           /* no file pointer */
  }

  if ( ( filehandle->mode & ( F_MODE_READ | F_MODE_PLUS ) ) == 0 )
  {
    return 0;                                                       /* read not allowed */
  }

  retsize = size;
  size *= size_st;                /* size to read */
  size_st = retsize;              /* size_st stores the item size from now */
  retsize = 0;

  if ( size <= 0 )
  {
    return 0;
  }

  /* change read size, if less bytes available than requested */
  if ( size > (long)( filehandle->length - filehandle->abs_pos ) )
  {
    size = (long)( filehandle->length - filehandle->abs_pos );
    filehandle->state |= F_FILE_STATE_EOF;
  }

  pos = filehandle->cluster_pos;
  while ( size )
  {
#if SMALL_FILE_OPT

    /* read data from management page if the file resides there */
    if ( ( filehandle->length ) <= F_FILE_MGM_PAGE0_SIZE )
    {
      if ( pos == F_CLUSTER_SIZE )
      {
        pos = 0;
      }

      rsize = (F_CLUSTER_TYPE)size;
      if ( f_flash_mgm_read( buf, _FILE_MGM_PAGE_CLUSTER_ADDR( filehandle->first_mgm_page, 0 ) + pos, rsize ) )
      {
        break;
      }
    }
    else
#endif /* if SMALL_FILE_OPT */
    {
      if ( pos == F_CLUSTER_SIZE )
      {
        if ( _f_get_cluster_value( filehandle ) )
        {
          break;
        }

        pos = 0;      /* change actual block to next block if data is left */
      }

      /* rsize contains size read left or the amount of bytes left in the current cluster */
      if ( size >= (long)( F_CLUSTER_SIZE - pos ) )
      {
        rsize = (F_CLUSTER_TYPE)( F_CLUSTER_SIZE - pos );
      }
      else
      {
        rsize = (F_CLUSTER_TYPE)size;
      }

      /* put data from file system to user data */
      if ( f_flash_data_read( buf, _CLUSTER_ADDR( filehandle->cluster ) + pos, rsize ) )
      {
        break;
      }
    }

    pos += rsize;
    filehandle->abs_pos += rsize;
    buf += rsize;
    size -= rsize;
    retsize += rsize;
  }

  filehandle->cluster_pos = pos;

  return retsize / size_st;
} /* fn_read */



/*
** fn_write
**
** INPUT : bbuf - buffer to write from
**         size - number of unique
**         size_st - size of unique
**         *filehandle - pointer to the file descriptor
** RETURN: number of written bytes
*/
long fn_write ( const _PTRQ void * bbuf, long size, long size_st, F_FILE * filehandle )
{
  F_CLUSTER_TYPE  pos, wsize;
  _PTRQ char    * buf = (_PTRQ char *)bbuf;
  unsigned char   rc = F_NOERR;
  long            retsize;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return 0;
  }

  if ( !filehandle )
  {
    return 0;                           /* no file pointer */
  }

  if ( ( filehandle->mode & ( F_MODE_WRITE | F_MODE_APPEND | F_MODE_PLUS ) ) == 0 )
  {
    return 0;                                                                   /* write not allowed */
  }

  retsize = size;
  size *= size_st;                /* size to write */
  size_st = retsize;              /* size_st stores the size of an item */
  retsize = 0;

  if ( size <= 0 )
  {
    return 0;
  }

  if ( filehandle->mode & F_MODE_APPEND
#if ( F_SEEK_WRITE == 0 )
      || ( filehandle->abs_pos != filehandle->length )
#endif
     )
  {
    if ( fn_seek( filehandle, 0, F_SEEK_END ) )
    {
      return 0;
    }
  }

  filehandle->state &= ~F_FILE_STATE_EOF;
#if SMALL_FILE_OPT


  /* if size of the file will exceed space available for data in management page then go for the standard method
     and copy data to a cluster */
  if ( filehandle->length && ( filehandle->length <= F_FILE_MGM_PAGE0_SIZE ) && ( ( filehandle->abs_pos + size ) > F_FILE_MGM_PAGE0_SIZE ) )
  {
    F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  i;
    F_CLUSTER_COUNT_TYPE                    inv = (F_CLUSTER_COUNT_TYPE)F_INVALID;

    /* allocate cluster and copy data out from management page to cluster */
    filehandle->cluster = _f_get_empty_cluster();
    if ( filehandle->cluster == (F_CLUSTER_COUNT_TYPE)F_INVALID )
    {
      return 0;
    }

    if ( f_flash_data_copy( _CLUSTER_ADDR( filehandle->cluster ), _FILE_MGM_ADDR( filehandle->first_mgm_page ), F_MGM_PAGE_SIZE ) )
    {
      return 0;
    }

    /* set all cluster address entries in management page to invalid except 1st */
    for ( i = 1 ; i < F_FILE_MGM_CLUSTER_PER_PAGE0 ; i++ )
    {
      if ( f_flash_mgm_write( _FILE_MGM_PAGE_CLUSTER_ADDR( filehandle->first_mgm_page, i ), &inv, sizeof( F_CLUSTER_COUNT_TYPE ) ) )
      {
        return 0;
      }
    }

    /* set first cluster */
    if ( _f_set_cluster_value( filehandle ) )
    {
      return 0;
    }
  }

#endif /* if SMALL_FILE_OPT */

  pos = filehandle->cluster_pos;
  while ( size )
  {
#if SMALL_FILE_OPT

    /* if file fits in the management page then write data to it instead of a cluster */
    if ( ( filehandle->length <= F_FILE_MGM_PAGE0_SIZE ) && ( ( filehandle->abs_pos + size ) <= F_FILE_MGM_PAGE0_SIZE ) )
    {
      wsize = (F_CLUSTER_TYPE)size;
      if ( pos == F_CLUSTER_SIZE )
      {
        pos = 0;
      }

      if ( f_flash_mgm_write( _FILE_MGM_PAGE_CLUSTER_ADDR( filehandle->first_mgm_page, 0 ) + pos, buf, wsize ) )
      {
        break;
      }
    }
    else
#endif /* if SMALL_FILE_OPT */
    {
      /* if no more space left in current cluster or first time after open*/
      if ( pos == F_CLUSTER_SIZE )
      {
        if ( filehandle->abs_pos == filehandle->length )
        {
          filehandle->cluster = _f_get_empty_cluster();
          if ( filehandle->cluster == (F_CLUSTER_COUNT_TYPE)F_INVALID )
          {
            break;
          }

          if ( _f_set_cluster_value( filehandle ) )
          {
            break;
          }

#if (F_SEEK_WRITE)
          filehandle->orig_cluster = filehandle->cluster;
#endif
        }
        else
        {
          if ( _f_get_cluster_value( filehandle ) )
          {
            break;
          }

#if (F_SEEK_WRITE)
          filehandle->orig_cluster = (F_CLUSTER_COUNT_TYPE)F_INVALID;
#endif
        }

        pos = 0;
      }

#if ( F_SEEK_WRITE )

      /* allocate new cluster and copy old file cluster content to it if modification will be done to */
      /* a cluster present in a previous file */
      if ( filehandle->orig_cluster == (F_CLUSTER_COUNT_TYPE)F_INVALID )
      {
        if ( _FILE_ID_MGM_PAGE( filehandle->fileid ) != filehandle->first_mgm_page )
        {
          F_CLUSTER_COUNT_TYPE  ocl;
          ocl = _f_get_orig_cluster_value( filehandle );
          if ( ( ocl != (F_CLUSTER_COUNT_TYPE)F_INVALID ) && ( ocl == filehandle->cluster ) )
          {
            filehandle->cluster = _f_get_empty_cluster();
            if ( filehandle->cluster != (F_CLUSTER_COUNT_TYPE)F_INVALID )
            {
              rc = f_flash_data_copy( _CLUSTER_ADDR( filehandle->cluster ), _CLUSTER_ADDR( ocl ), F_CLUSTER_SIZE );
              if ( rc == F_NOERR )
              {
                --filehandle->page_cl_pos;
                rc = _f_set_cluster_value( filehandle );
              }
            }
            else
            {
              break;
            }
          }
        }

        if ( rc )
        {
          break;
        }

        filehandle->orig_cluster = filehandle->cluster;
      }

#endif /* if ( F_SEEK_WRITE ) */

      if ( size >= (long)( F_CLUSTER_SIZE - pos ) )
      {
        wsize = (F_CLUSTER_TYPE)( F_CLUSTER_SIZE - pos );
      }
      else
      {
        wsize = (F_CLUSTER_TYPE)size;
      }

      /* wsize contains number of bytes left to write or number of bytes left in the current cluster*/
      if ( f_flash_data_write( _CLUSTER_ADDR( filehandle->cluster ) + pos, buf, wsize ) )
      {
        break;
      }
    }

    pos += wsize;
    filehandle->abs_pos += wsize;
    buf += wsize;
    size -= wsize;
    retsize += wsize;

    if ( filehandle->abs_pos > filehandle->length )
    {
      filehandle->length = filehandle->abs_pos;
    }
  }

  filehandle->cluster_pos = pos;
  filehandle->mode |= F_MODE_CHANGED;

  return retsize / size_st;
} /* fn_write */



/*
** fn_seek
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
int fn_seek ( F_FILE * filehandle, long offset, long whence )
{
  F_LENGTH_TYPE         abs_pos;
  F_CLUSTER_COUNT_TYPE  cco, ccn;

  if ( !filehandle )
  {
    return F_ERR_INVALIDHANDLE;
  }

  abs_pos = filehandle->abs_pos;
  switch ( whence )   /* calculate new position */
  {
    case F_SEEK_SET:
      break;

    case F_SEEK_CUR:
      offset = (long)( abs_pos + offset );
      break;

    case F_SEEK_END:
      offset = (long)( filehandle->length + offset );
      break;

    default:
      return F_ERR_INVALIDMODE;
  }

  if ( offset < 0 )
  {
    return F_ERR_INVALIDOFFSET;
  }

  if ( offset > (long)( filehandle->length ) )
  {
    offset = (long)( filehandle->length );
  }

  filehandle->state &= ~F_FILE_STATE_EOF;
  if ( (long)abs_pos == offset )
  {
    return F_NOERR;
  }

#if SMALL_FILE_OPT

  /* step through clusters if file data is in the cluster area */
  if ( ( filehandle->length ) > F_FILE_MGM_PAGE0_SIZE )
#endif
  {
    if ( ( filehandle->cluster_pos == F_CLUSTER_SIZE ) && abs_pos )
    {
      --abs_pos;
    }

    ccn = (F_CLUSTER_COUNT_TYPE)( ( (unsigned long)offset ) / F_CLUSTER_SIZE );
    if ( offset && ( ( ( (unsigned long)offset ) % F_CLUSTER_SIZE ) == 0 ) )
    {
      --ccn;
    }

    cco = (F_CLUSTER_COUNT_TYPE)( abs_pos / F_CLUSTER_SIZE );
    if ( ( filehandle->page_cnt == 0 ) && ( filehandle->page_cl_pos == 0 ) )
    {
      (void)_f_get_cluster_value( filehandle );
    }

    if ( cco != ccn )
    {
      /* calculate new block and block position */
      if ( cco > ccn )
      {
        filehandle->act_mgm_page = filehandle->first_mgm_page;
        filehandle->page_cnt = 0;
        filehandle->page_cl_pos = 0;
        ++ccn;
      }
      else
      {
        ccn -= cco;
      }

      while ( ccn-- )
      {
        (void)_f_get_cluster_value( filehandle );
      }

#if (F_SEEK_WRITE)
      filehandle->orig_cluster = (F_CLUSTER_COUNT_TYPE)F_INVALID;
#endif
    }

    if ( filehandle->cluster != (F_CLUSTER_COUNT_TYPE)F_INVALID )
    {
      filehandle->cluster_pos = (F_CLUSTER_TYPE)( ( (unsigned long)offset ) % F_CLUSTER_SIZE );
      if ( offset && ( filehandle->cluster_pos == 0 ) )
      {
        filehandle->cluster_pos = F_CLUSTER_SIZE;
      }
    }
  }

#if SMALL_FILE_OPT
  else
  {
    filehandle->cluster_pos = (F_CLUSTER_TYPE)( ( (unsigned long)offset ) % F_CLUSTER_SIZE );
  }
#endif

  filehandle->abs_pos = (F_LENGTH_TYPE)offset;

  return F_NOERR;
} /* fn_seek */



/*
** fn_getc
**
** read one byte from a file
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: -1 if error, otherwise the read character.
*/
int fn_getc ( F_FILE * filehandle )
{
  unsigned char  ch;

  if ( fn_read( &ch, 1, 1, filehandle ) == 1 )
  {
    return ch;
  }
  else
  {
    return -1;
  }
}



/*
** fn_putc
**
** write one byte to a file
**
** INPUT : ch - character to write
**         *filehandle - pointer to a file handler
** RETURN: ch on success, -1 on error
*/
int fn_putc ( int ch, F_FILE * filehandle )
{
  unsigned char  tmpch = (unsigned char)( ch );

  if ( fn_write( &tmpch, 1, 1, filehandle ) == 1 )
  {
    return tmpch;
  }

  return -1;
}



/*
** fn_tell
**
** get current position in the file
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_ERR_INVALID on error or current position.
*/
long fn_tell ( F_FILE * filehandle )
{
  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return (long)F_ERR_INVALID;
  }

  if ( filehandle->mode == F_MODE_CLOSE )
  {
    return (long)F_ERR_INVALID;
  }

  return (long)filehandle->abs_pos;
}


/*
** fn_eof
**
** check if current position is at the end of the file.
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_ERR_EOF - at the end of the file
**         F_NOERR - no error, end of the file not reached
**         other - on error
*/
int fn_eof ( F_FILE * filehandle )
{
  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_INVALIDHANDLE;
  }

  if ( !filehandle )
  {
    return F_ERR_INVALIDHANDLE;
  }

  if ( filehandle->mode == F_MODE_CLOSE )
  {
    return F_ERR_INVALIDMODE;
  }

  if ( filehandle->state & F_FILE_STATE_EOF )
  {
    return F_ERR_EOF;
  }

  return F_NOERR;
} /* fn_eof */



/*
** fn_rewind
**
** set current position in the file to the beginning
**
** INPUT : *filehandle - pointer to a file descriptor
** RETURN: F_NOERR on succes, other if error.
*/
int fn_rewind ( F_FILE * filehandle )
{
  return fn_seek( filehandle, 0, F_SEEK_SET );
}


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
int fn_ftruncate ( F_FILE * filehandle, unsigned long length )
{
  int                                     rc;
  F_CLUSTER_COUNT_TYPE                    cluster;
  F_CLUSTER_COUNT_TYPE                    ocluster;
  F_CLUSTER_COUNT_TYPE                    cl_cnt;
  F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  cl_pg_cnt;
  F_CLUSTER_COUNT_TYPE                    cl_pos;
  F_FILE_MGM_PAGE_COUNT_TYPE              opg_act;
  F_FILE_MGM_PAGE_COUNT_TYPE              opg_next;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_act;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_next;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_last;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_del;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_pos;
  F_FILE_MGM_PAGE_COUNT_TYPE              pg_cnt;
  unsigned char                           pg_buf[F_FILE_MGM_PAGE_COUNT_TYPE_SIZE];
  unsigned char                           cl_buf[F_CLUSTER_COUNT_TYPE_SIZE];

  rc = F_NOERR;
  if ( ( f_volume.state ) == F_STATE_NOTFORMATTED )
  {
    return F_ERR_INVALIDHANDLE;
  }

  if ( filehandle == NULL )
  {
    return F_ERR_INVALIDHANDLE;
  }

  if ( ( ( filehandle->mode ) & ( F_MODE_WRITE | F_MODE_APPEND | F_MODE_PLUS ) ) == 0 )
  {
    return F_ERR_INVALIDMODE;
  }

  if ( ( filehandle->length ) < length )
  {
    return F_ERR_INVALIDSIZE;
  }


  if ( ( filehandle->length ) > length )
  {
    if ( ( filehandle->abs_pos ) > length )
    {
      rc = fn_seek( filehandle, length, F_SEEK_SET );
    }

    if ( rc == F_NOERR )
    {
 #if SMALL_FILE_OPT
      if ( ( filehandle->length ) > F_FILE_MGM_PAGE0_SIZE )
 #endif
      {
        cl_cnt = (F_CLUSTER_COUNT_TYPE)( ( length + F_CLUSTER_SIZE - 1 ) / F_CLUSTER_SIZE );
        pg_cnt = 1;
        if ( cl_cnt > F_FILE_MGM_CLUSTER_PER_PAGE0 )
        {
          pg_cnt += (F_FILE_MGM_PAGE_COUNT_TYPE)( ( ( cl_cnt - F_FILE_MGM_CLUSTER_PER_PAGE0 )
                                                   + F_FILE_MGM_CLUSTER_PER_PAGE - 1 )
                                                 / F_FILE_MGM_CLUSTER_PER_PAGE );
        }

        /***** Step to the last valid management page in the file management page chain. *****/
        cl_pos = 0;
        cl_pg_cnt = F_FILE_MGM_CLUSTER_PER_PAGE0;
        pg_next = filehandle->first_mgm_page;

        /* If the file already existed then step through the old management page chain as well. */
        /* This is required because we need to drop clusters that were allocated for the new file. */
        /* When an existing file is opened for write a copy of the cluster chain will be made and if a */
        /* a new cluster is allocated for the modified cluster which will be removed due to truncate */
        /* then the new cluster should be removed from the cluster table */
        if ( _FILE_ID_MGM_PAGE( filehandle->fileid ) != filehandle->first_mgm_page )
        {
          opg_next = _FILE_ID_MGM_PAGE( filehandle->fileid );
        }
        else
        {
          opg_act = _F_INV_FILE_MGM_PAGE_COUNT;
        }

        for ( pg_pos = 0
             ; ( pg_pos < pg_cnt ) && ( rc == F_NOERR )
             ; pg_pos++ )
        {
          pg_act = pg_next;
          rc = f_flash_mgm_read( &( pg_buf[0] )
                                , _FILE_MGM_HEADER_ADDR( pg_act ) + _SADDR( F_FILE_MGM_HEADER, next )
                                , F_FILE_MGM_PAGE_COUNT_TYPE_SIZE );
          pg_next = F_GET_FILE_MGM_PAGE_COUNT( pg_buf );

          /* The file already existed and there is no error, then step in the old chain. */
          if ( ( rc == F_NOERR )
              && ( _FILE_ID_MGM_PAGE( filehandle->fileid ) != filehandle->first_mgm_page ) )
          {
            if ( opg_next != _F_INV_FILE_MGM_PAGE_COUNT )     /* next management page is valid */
            {
              opg_act = opg_next;                             /* set current page to be the next one */
              rc = f_flash_mgm_read( &( pg_buf[0] )           /* read current page header */
                                    , _FILE_MGM_HEADER_ADDR( opg_act ) + _SADDR( F_FILE_MGM_HEADER, next )
                                    , F_FILE_MGM_PAGE_COUNT_TYPE_SIZE );
              opg_next = F_GET_FILE_MGM_PAGE_COUNT( pg_buf ); /* get next page index */
            }
            else                                              /* next page is invalid */
            {
              opg_act = _F_INV_FILE_MGM_PAGE_COUNT;           /* set current page to be invalid */
            }
          }

          if ( ( pg_pos + 1 ) < pg_cnt )                /* not the last page */
          {
            if ( pg_pos == 0 )                          /* first page */
            {
              cl_pos += F_FILE_MGM_CLUSTER_PER_PAGE0;   /* increase cluster pos. with no. clusters in page 0 */
            }
            else
            {
              cl_pos += F_FILE_MGM_CLUSTER_PER_PAGE;    /* increase cluster pos. with no. clusters in page n */
              cl_pg_cnt = F_FILE_MGM_CLUSTER_PER_PAGE;  /* set cluster page count to no. clusters in page n */
            }
          }
        }

        pg_last = pg_act;  /* save last required page in the chain */
        pg_del = pg_next;  /* save page index from which the chain needs to be dropped */

        if ( rc == F_NOERR )
        {
          /* invalidate all needless clusters in the current management page and remove the ones that are not present */
          /* in the old chain (if exists) */
          cl_pos = cl_cnt - cl_pos;
          ocluster = _F_INV_CLUSTER_COUNT;
          while ( ( pg_act != _F_INV_FILE_MGM_PAGE_COUNT )  /* step through all pages from the last */
                 && ( rc == F_NOERR ) )
          {
            /* step through all clusters in the current page */
            while ( ( cl_pos < cl_pg_cnt )
                   && ( rc == F_NOERR ) )
            {
              /* get cluster in the current page */
              rc = f_flash_mgm_read( &( cl_buf[0] ), _FILE_MGM_PAGE_CLUSTER_ADDR( pg_act, cl_pos ), F_CLUSTER_COUNT_TYPE_SIZE );
              if ( rc == F_NOERR )
              {
                cluster = F_GET_CLUSTER_COUNT( cl_buf );      /* read cluster value */
                if ( cluster == _F_INV_CLUSTER_COUNT )        /* it is invalid */
                {
                  break;                                      /* exit loop */
                }

                if ( opg_act != _F_INV_FILE_MGM_PAGE_COUNT )  /* if old management page is valid */
                {
                  rc = f_flash_mgm_read( &( cl_buf[0] ), _FILE_MGM_PAGE_CLUSTER_ADDR( opg_act, cl_pos ), F_CLUSTER_COUNT_TYPE_SIZE );
                  if ( rc == F_NOERR )
                  {
                    ocluster = F_GET_CLUSTER_COUNT( cl_buf ); /* read old cluster value */
                    if ( ocluster == _F_INV_CLUSTER_COUNT )   /* it is invalid */
                    {
                      opg_act = _F_INV_FILE_MGM_PAGE_COUNT;   /* invalidate old page as we reached the last */
                    }                                         /* cluster of the old file */
                  }
                }
              }

              if ( rc == F_NOERR )
              {
                if ( cluster != ocluster )
                {
                  _CLEAR_BIT( f_volume.cluster_table, cluster );  /* unmark cluster */
                }

                /* set cluster to invalid if the current page is the last valid one in the chain (other pages will */
                /* be dropped and erased) */
                if ( pg_act == pg_last )
                {
                  F_SET_CLUSTER_COUNT( cl_buf, _F_INV_CLUSTER_COUNT );
                  rc = f_flash_mgm_write( _FILE_MGM_PAGE_CLUSTER_ADDR( pg_act, cl_pos ), &( cl_buf[0] ), F_CLUSTER_COUNT_TYPE_SIZE );
                }
              }

              ++cl_pos;
            }

            rc = f_flash_mgm_read( &( pg_buf[0] )
                                  , _FILE_MGM_HEADER_ADDR( pg_act ) + _SADDR( F_FILE_MGM_HEADER, next )
                                  , F_FILE_MGM_PAGE_COUNT_TYPE_SIZE );
            pg_act = F_GET_FILE_MGM_PAGE_COUNT( pg_buf );

            /* The file already existed and there is no error, then step in the old chain. */
            if ( ( rc == F_NOERR )
                && ( opg_act != _F_INV_FILE_MGM_PAGE_COUNT ) )
            {
              rc = f_flash_mgm_read( &( pg_buf[0] )           /* read current page header */
                                    , _FILE_MGM_HEADER_ADDR( opg_act ) + _SADDR( F_FILE_MGM_HEADER, next )
                                    , F_FILE_MGM_PAGE_COUNT_TYPE_SIZE );
              opg_act = F_GET_FILE_MGM_PAGE_COUNT( pg_buf ); /* get next page index */
            }

            cl_pos = 0;
            cl_pg_cnt = F_FILE_MGM_CLUSTER_PER_PAGE;
          }
        }

        /* next management page is valid */
        if ( ( rc == F_NOERR )
            && ( pg_del != _F_INV_FILE_MGM_PAGE_COUNT ) )
        {
          /* set next management page to invalid */
          F_SET_FILE_MGM_PAGE_COUNT( pg_buf, _F_INV_FILE_MGM_PAGE_COUNT );
          rc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( pg_last ) + _SADDR( F_FILE_MGM_HEADER, next )
                                 , &( pg_buf[0] )
                                 , F_FILE_MGM_PAGE_COUNT_TYPE_SIZE );

          /* remove needless management pages */
          if ( rc == F_NOERR )
          {
            rc = _f_remove_file_mgm( pg_del, 0 );
          }
        }
      }

      if ( rc == F_NOERR )
      {
        filehandle->length = (F_LENGTH_TYPE)length;
        filehandle->mode |= F_MODE_CHANGED;
      }
    }
  }

  return rc;
} /* fn_ftruncate */
#endif /* if F_TRUNCATE */

