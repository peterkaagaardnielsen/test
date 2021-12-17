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
#include "f_file.h"
#include "f_util.h"
#include "f_dir.h"


#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif
#include "../../version/ver_psp_string.h"
#if VER_PSP_STRING_MAJOR != 1
 #error Incompatible PSP_STRING version number!
#endif


#define FIND_SHADOW_DIR_FIRST    0
#define FIND_SHADOW_DIR_LAST     ( FIND_SHADOW_DIR_FIRST + 2 - 1 )
#define FIND_SHADOW_IDX( p )     ( ( p ) - FIND_SHADOW_DIR_FIRST )
#define FIND_POS_SHADOW_DIR( p ) ( ( p ) <= FIND_SHADOW_DIR_LAST )

#define FIND_DIR_FIRST           ( FIND_SHADOW_DIR_LAST + 1 )
#define FIND_DIR_LAST            ( FIND_DIR_FIRST + F_MAX_DIR - 1 )
#define FIND_DIR_IDX( p )        ( ( p ) - FIND_DIR_FIRST )
#define FIND_POS_DIR( p )        ( ( ( p ) >= FIND_DIR_FIRST ) && ( ( p ) <= FIND_DIR_LAST ) )

#define FIND_FILE_FIRST          ( FIND_DIR_LAST + 1 )
#define FIND_FILE_LAST           ( FIND_FILE_FIRST + F_MAX_FILE - 1 )
#define FIND_FILE_IDX( p )       ( ( p ) - FIND_FILE_FIRST )
#define FIND_POS_FILE( p )       ( ( ( p ) >= FIND_FILE_FIRST ) && ( ( p ) <= FIND_FILE_LAST ) )

#define FIND_POS_CNT             ( FIND_FILE_LAST + 1 )

/**************************************************************************/
/************************* INTERNAL FUNCTIONS *****************************/
/**************************************************************************/


/*
** Check key against a key associated to a file ID
** INPUT: id - file ID
**        key - key
** RETURN: 0 - if matches
**         1 - if doesn't match
*/
#if QUICK_FILE_SEARCH
unsigned char _f_qs_check ( F_QSKEY_TYPE * key, F_FILE_ID_TYPE id )
{
  return (unsigned char)( f_volume.qskey[id] == *key ? 0 : 1 );
}
#endif


/*
** Converts a lower case character to upper case
** INPUT: ch - character
** RETURN: upper case character
*/
#if F_CHECKNAME
char _f_toupper ( char ch )
{
  return (char)( ( ch >= 'a' && ch <= 'z' ) ? ( ch - 'a' + 'A' ) : ch );
}
#endif


/*
** Generate key from a flash address or file name.
**
** INPUT:  name - if not null then use it to generate key
**         addr - address in flash (if name==NULL)
** OUTPUT: key - generated key
*/
#if QUICK_FILE_SEARCH
 #define QS_RH 4
unsigned char _f_qs_genkey ( F_QSKEY_TYPE * key, const _PTRQ char * name, unsigned long addr )
{
  unsigned char  rc, p;
  unsigned char  ch = (unsigned char)-1;
  unsigned char  b[QS_RH];
  unsigned char  bp = QS_RH;

  *key = 0;
  for ( p = 0 ; p < F_MAX_FILE_NAME_LENGTH && ch ; p++ )
  {
    if ( name )
    {
      ch = (unsigned char)*name++;
    }
    else
    {
      if ( bp == QS_RH )
      {
        rc = f_flash_mgm_read( b, addr, QS_RH );
        if ( rc )
        {
          return rc;
        }

        bp = 0;
        addr += QS_RH;
      }

      ch = b[bp++];
    }

 #if F_CHECKNAME
    ch = _f_toupper( ch );
 #endif
    *key ^= ( ch << ( p & 3 ) );
  }

  return 0;
} /* _f_qs_genkey */
#endif /* if QUICK_FILE_SEARCH */


/*
** Checks if a name conatins directory delimiter '/' or '\\'
** INPUT: name - name to check for.
** RETURN: position if delimiter found, 0 if not found
*/
#if F_CHECKNAME
unsigned char _check_dir_delimiter ( const _PTRQ char * name )
{
  unsigned char  i;

  for ( i = 0 ; name[i] && name[i] != '/' && name[i] != '\\' ; i++ )
  {
  }

  if ( name[i] )
  {
    return i;
  }

  return (unsigned char)F_INVALID;
}
#endif


/*
** Checks a portion of flash if it is equal to the required string or wildcard name
** INPUT: wname - name to look for
**        cname - name to compare with (can be NULL)
**        addr - address in flash
**        fname - where to store real file name
**        wca_on - 1-wildcard allowed
** RETURN: 0-matches / 1-doesn't match
*/
#if F_WILDCARD
static unsigned char _f_cmp_wname ( const _PTRQ char * wname
                                   , const _PTRQ char * cname
                                   , unsigned long addr
                                   , char * fname
                                   , char wca_on )
#else
static unsigned char _f_cmp_wname ( const _PTRQ char * wname
                                   , const _PTRQ char * cname
                                   , unsigned long addr
                                   , char * fname )
#endif
{
  char           act;
  char           wact;
  unsigned char  rc;
  unsigned char  fpos = 0;

#if F_WILDCARD
  char           wild;
  char           wild_end = 0;
  unsigned char  wild_fpos = 0;
  const char   * wild_wname = NULL;
  char         * wild_fname = NULL;
  unsigned long  wild_addr;


  wild = 0;
  wild_addr = 0;
#endif

  if ( cname != NULL )
  {
    addr = 0;
  }

  for ( ; ; )
  {
    if ( cname != NULL )
    {
      act = cname[addr];
      ++addr;
    }
    else
    {
      rc = f_flash_mgm_read( &act, addr++, 1 );
      if ( rc )
      {
        return 1;                 /* get next character of the original file name */
      }
    }

    if ( fpos++ == F_MAX_FILE_NAME_LENGTH + 1 )
    {
      return 1;                                     /* something is wrong, zero terminator missing */
    }

    if ( fname )
    {
      *fname++ = act;               /* set return string if needed */
    }

#if F_CHECKNAME
    act = _f_toupper( act );
#endif


    if ( (unsigned char)act == _F_INV_8 )
    {
      return 1;                                 /* return if invalid file */
    }

    if ( wname == NULL )
    {
      return 0;                     /* return if we are just checking for anything */
    }

#if F_CHECKNAME
    wact = _f_toupper( *wname );
#else
    wact = *wname;
#endif

    if ( act == 0 )             /* if end of file name reached */
    {
      while ( wact == '?' || wact == '*' )
      {
        wact = *( ++wname );                            /* skip '?'/'*' in wildcard name */
      }

      if ( wact )
      {
        return 1;               /* if anything left in wildcard name, the compare failed */
      }

      break;                /* otherwise it was successfull */
    }

#if F_WILDCARD
    if ( wca_on )     /* if compare is a wildcard compare */
    {
      if ( wild )     /* if found * earlier */
      {
__f_cmp_wname_check_end:
        if ( act == wild_end )  /* check if current character is the last character of the wildcard name */
        {
          wild = 0;
          wild_fpos = fpos;
          wild_wname = wname++;
          wild_fname = fname;
          wild_addr = addr;   /* if yes, store current positions */
        }

        continue;       /* get next character */
      }

      if ( wact == '*' )        /* if wildcard name is * */
      {
        wild = 1;         /* set * found */
 #if F_CHECKNAME
        while ( *wname == '*' )
        {
          ++wname;                      /* skip *-s */
        }

        wild_end = _f_toupper( *wname );
 #else
        wild_end = *( ++wname );        /* get end character for * */
 #endif
        goto __f_cmp_wname_check_end;   /* check whether the current character is an end character */
      }
      else if ( ( *wname != '?' ) && ( wact != act ) )    /* if wildcard name compare failes and it is not ? */
      {
        if ( wild_addr )      /* check if we had earlier a * in the wildcard name */
        {
          wild = 1;
          fpos = wild_fpos;
          wname = wild_wname;
          fname = wild_fname;
          addr = wild_addr;   /* if yes, the go back to that state and try to rematch again */
        }
        else
        {
          return 1;         /* otherwise compare failed */
        }
      }
      else
      {
        ++wname;
      }

      continue;
    }

#endif /* if F_WILDCARD */

    if ( wact != act )
    {
      return 1;
    }

    ++wname;
  }

  return 0;
} /* _f_cmp_wname */


/*
** _f_addentry
**
** Adds an entry to the directory structure. If no more space left in the directory and
** there is a deleted item, calls mergedir and adds the new entry.
**
** fileid -> F_INVALID - allocate new fileid
**           other - put thta to the new page
*/
F_FILE_ID_TYPE _f_add_entry ( const _PTRQ char * filename, F_FILE_ID_TYPE fileid, F_FILE * file )
{
  F_FILE_MGM_PAGE_COUNT_TYPE  mgm_page;
  F_FILE_ID_TYPE              i;
  unsigned char               rc, len;

#if F_WILDCARD && QUICK_WILDCARD_SEARCH
  unsigned long  addr;
#endif

#if F_DIRECTORIES == 0
  if ( ( *filename == '/' ) || ( *filename == '\\' ) )
  {
    ++filename;
  }

#endif
  len = (unsigned char)psp_strnlen( filename, F_MAX_FILE_NAME_LENGTH + 1 );
#if F_CHECKNAME
  if ( len > F_MAX_FILE_NAME_LENGTH )
  {
    return (F_FILE_ID_TYPE)F_INVALID;                               /* invalid name */
  }

#endif
  ++len;                        /* calculate length of the filename */

#if F_CHECKNAME
  if ( _check_dir_delimiter( filename ) != (unsigned char)F_INVALID )
  {
    return (F_FILE_ID_TYPE)F_INVALID;
  }

#endif

  if ( fileid == (F_FILE_ID_TYPE)F_INVALID )
  {
    for ( i = 0 ; i < F_MAX_FILE && _FILE_ID_MGM_PAGE( i ) != (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID ; i++ )
    {
    }

    if ( i == F_MAX_FILE )
    {
      return (F_FILE_ID_TYPE)F_INVALID;                         /* no more files allowed */
    }

    mgm_page = _f_get_file_mgm_empty_page( i, _FM_STATE_OPEN, 0 );
#if QUICK_FILE_SEARCH
    _f_qs_genkey( f_volume.qskey + i, filename, 0 );
#endif
  }
  else
  {
    unsigned char  seq;
    rc = f_flash_mgm_read( &seq, _FILE_ID_MGM_HEADER_ADDR( fileid ) + _SADDR( F_FILE_MGM_HEADER, seq ), 1 );
    mgm_page = _f_get_file_mgm_empty_page( fileid, _FM_STATE_OPEN, (unsigned char)( seq + 1 ) );
    i = fileid;
  }

  if ( mgm_page == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
  {
    return (F_FILE_ID_TYPE)F_INVALID;
  }

  rc = f_flash_mgm_write( _FILE_MGM_DSC_ADDR( mgm_page ) + _SADDR( F_FILE_DSC, name[0] ), (_PTRQ char *)filename, len );
  if ( rc )
  {
    return (F_FILE_ID_TYPE)F_INVALID;
  }

#if F_WILDCARD && QUICK_WILDCARD_SEARCH
  addr = _QWS_ENTRY_ADDR( _QWS_EPAGE( i ), _QWS_EPOS( i ) );
 #if F_DIRECTORIES
  {
    unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
    F_SET_DIR_ID( _dirid, file->dirid );
    rc = f_flash_mgm_write( addr + _SADDR( F_QWS_ENTRY, dirid ), _dirid, F_DIR_ID_TYPE_SIZE );
    if ( rc )
    {
      return (F_FILE_ID_TYPE)F_INVALID;
    }
  }
 #endif
  rc = f_flash_mgm_write( addr + _SADDR( F_QWS_ENTRY, name[0] ), (_PTRQ char *)filename, len );
  if ( rc )
  {
    return (F_FILE_ID_TYPE)F_INVALID;
  }

#endif /* if F_WILDCARD && QUICK_WILDCARD_SEARCH */

  if ( i != fileid )
  {
    _FILE_ID_MGM_PAGE( i ) = mgm_page;
  }

  file->first_mgm_page = mgm_page;
  return i;
} /* _f_add_entry */



/*
** Find a directory with a defined parent directory
** INPUT: dirname - name of the directory
**        id - parent directory ID
** RETURN:ID of the directory, F_INVALID if not available
*/
#if F_DIRECTORIES
F_DIR_ID_TYPE _f_find_dir ( const _PTRQ char * dirname, F_DIR_ID_TYPE id )
{
  F_DIR_ID_TYPE  i;
  unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
  unsigned char  rc;

  if ( dirname )
  {
    /* dirname = . | .. */
    if ( psp_strncmp( dirname, _DIR_ACT, F_MAX_FILE_NAME_LENGTH ) == 0 )
    {
      return id;
    }

    if ( psp_strncmp( dirname, _DIR_TOP, F_MAX_FILE_NAME_LENGTH ) == 0 )
    {
      F_DIR_ID_TYPE  dirid;
      if ( id == F_DIR_ROOT )
      {
        return F_DIR_ROOT;
      }

      rc = f_flash_mgm_read( _dirid, _DIR_DSC_ADDR( id ) + _SADDR( F_DIR_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
      if ( rc )
      {
        return (F_DIR_ID_TYPE)F_INVALID;
      }

      dirid = F_GET_DIR_ID( _dirid );
      if ( dirid == _F_INV_DIR_ID )
      {
        return (F_DIR_ID_TYPE)F_INVALID;
      }

      return dirid;
    }
  }

  for ( i = 0 ; i < F_MAX_DIR ; i++ )
  {
    if ( i == id )
    {
      continue;
    }

 #if F_WILDCARD
    rc = _f_cmp_wname( dirname, NULL, _DIR_DSC_ADDR( i ) + _SADDR( F_DIR_DSC, name[0] ), NULL, 0 );
 #else
    rc = _f_cmp_wname( dirname, NULL, _DIR_DSC_ADDR( i ) + _SADDR( F_DIR_DSC, name[0] ), NULL );
 #endif
    if ( rc == 0 )
    {
      rc = f_flash_mgm_read( _dirid, _DIR_DSC_ADDR( i ) + _SADDR( F_DIR_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
      if ( rc )
      {
        return (F_DIR_ID_TYPE)F_INVALID;
      }

      if ( F_GET_DIR_ID( _dirid ) == id )
      {
        return i;
      }
    }
  }

  return (F_DIR_ID_TYPE)F_INVALID;
} /* _f_find_dir */
#endif /* if F_DIRECTORIES */



/*
** Find a file with a defined parent directory (if directories are allowed)
** INPUT: filename - name of the file
**        id - parent directory ID (optional)
** RETURN:ID of the file, F_INVALID if not available
*/
#if F_DIRECTORIES
F_FILE_ID_TYPE _f_find_file ( const _PTRQ char * filename, F_DIR_ID_TYPE dirid )
#else
F_FILE_ID_TYPE _f_find_file ( const _PTRQ char * filename )
#endif
{
  F_FILE_ID_TYPE  i;
  unsigned char   rc;

#if QUICK_FILE_SEARCH
  F_QSKEY_TYPE  key;
#endif

#if F_DIRECTORIES == 0
  if ( ( *filename == '/' ) || ( *filename == '\\' ) )
  {
    ++filename;
  }

#endif
#if QUICK_FILE_SEARCH
  _f_qs_genkey( &key, filename, 0 );
#endif

  for ( i = 0 ; i < F_MAX_FILE ; i++ )
  {
    if ( _FILE_ID_MGM_PAGE( i ) == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
    {
      continue;
    }

#if QUICK_FILE_SEARCH
    if ( _f_qs_check( &key, i ) )
    {
      continue;
    }

#endif
#if F_DIRECTORIES

    /* check if current directory is the required directory */
    {
      unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
      rc = f_flash_mgm_read( _dirid, _FILE_ID_MGM_DSC_ADDR( i ) + _SADDR( F_FILE_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
      if ( rc )
      {
        return (F_FILE_ID_TYPE)F_INVALID;
      }

      if ( F_GET_DIR_ID( _dirid ) != dirid )
      {
        continue;
      }
    }
#endif

#if F_WILDCARD
    rc = _f_cmp_wname( filename, NULL, _FILE_ID_MGM_DSC_ADDR( i ) + _SADDR( F_FILE_DSC, name[0] ), NULL, 0 );
#else
    rc = _f_cmp_wname( filename, NULL, _FILE_ID_MGM_DSC_ADDR( i ) + _SADDR( F_FILE_DSC, name[0] ), NULL );
#endif
    if ( rc == 0 )
    {
      return i;
    }
  }

  return (F_FILE_ID_TYPE)F_INVALID;
} /* _f_find_file */



#if F_DIRECTORIES


/*
** find a directory or a file with filename, with parent directory..
** INPUT:  filename - name of the file or directory
**         odirid - parent directory ID
** OUTPUT: par - if not NULL puts the parameter block there
** RETURN: the ID of the file or directory matched, F_INVALID if not available
*/
F_FIND_ID_TYPE _f_find ( const _PTRQ char * filename, F_FILE_PAR * par, F_DIR_ID_TYPE odirid )
{
  F_FIND_ID_TYPE  find_id;
  F_FILE_ID_TYPE  fileid;
  F_DIR_ID_TYPE   dirid;
  unsigned char   rc;

  fileid = _f_find_file( filename, odirid );
  if ( fileid == (F_FILE_ID_TYPE)F_INVALID )
  {
    dirid = _f_find_dir( filename, odirid );
    if ( dirid != (F_DIR_ID_TYPE)F_INVALID )
    {
      find_id = (F_FIND_ID_TYPE)dirid;
      if ( dirid == F_DIR_ROOT )
      {
        if ( par )
        {
          F_SET_ATTR( par->attr, F_ATTR_DIR );
          _f_setvalue_2( par->ctime, 0u );
          _f_setvalue_2( par->cdate, 0u );
          F_SET_LENGTH( par->length, 0u );
        }
      }
      else
      {
        if ( par )
        {
          rc = f_flash_mgm_read( par, _DIR_DSC_ADDR( dirid ) + _SADDR( F_DIR_DSC, par ), sizeof( F_DIR_PAR ) );
          if ( rc == F_NOERR )
          {
            F_SET_LENGTH( par->length, 0u );
          }
          else
          {
            find_id = (F_FIND_ID_TYPE)F_INVALID;
          }
        }
      }
    }
    else
    {
      find_id = (F_FIND_ID_TYPE)F_INVALID;
    }
  }
  else
  {
    find_id = (F_FIND_ID_TYPE)fileid;
    if ( par )
    {
      rc = f_flash_mgm_read( par, _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
      if ( rc != F_NOERR )
      {
        find_id = (F_FIND_ID_TYPE)F_INVALID;
      }
    }
  }

  return find_id;
}     /* _f_find */
#else /* if F_DIRECTORIES */


/*
** find a file with filename, with parent directory..
** INPUT:  filename - name of the file or directory
** OUTPUT: par - if not NULL puts the parameter block there
** RETURN: the ID of the file matched, F_INVALID if not available
*/
F_FIND_ID_TYPE _f_find ( const _PTRQ char * filename, F_FILE_PAR * par )
{
  F_FIND_ID_TYPE  find_id;
  F_FILE_ID_TYPE  fileid;
  unsigned char   rc;

  fileid = _f_find_file( filename );
  if ( fileid != (F_FILE_ID_TYPE)F_INVALID )
  {
    find_id = (F_FIND_ID_TYPE)fileid;
    if ( par )
    {
      rc = f_flash_mgm_read( par, _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
      if ( rc != F_NOERR )
      {
        find_id = (F_FIND_ID_TYPE)F_INVALID;
      }
    }
  }
  else
  {
    find_id = (F_FIND_ID_TYPE)F_INVALID;
  }

  return find_id;
} /* _f_find */
#endif /* if F_DIRECTORIES */




/*
** Return current dir obtained from name.
** INPUT:  path - path to check
** OUTPUT: if filename present name is handled as dir/filename and returns file name start position
**         in filename
** RETURN: directory ID to path, F_INVALID if doesn't exist
*/
#if F_DIRECTORIES
F_DIR_ID_TYPE _f_check_path ( const _PTRQ char * path, _PTRQ char * * filename )
{
  F_DIR_ID_TYPE      rc;
  const _PTRQ char * start = NULL;
  char               name[F_MAX_FILE_NAME_LENGTH + 1];

  if ( filename )
  {
    *filename = NULL;
  }

  if ( ( *path == '/' ) || ( *path == '\\' ) )
  {
    rc = F_DIR_ROOT;
 #if F_CHECKNAME
    while ( *path && ( *path == '/' || *path == '\\' ) )
    {
      ++path;
    }

 #else
    ++path;
 #endif
  }
  else
  {
    rc = f_volume.current_dir;
  }

  if ( *path )
  {
    for ( start = path ; ; path++ )
    {
      /* if actual char is / or \ or form was a/b and b is supposed to be a directory */
      if ( ( *path == '/' ) || ( *path == '\\' ) || ( ( *path == 0 ) && ( filename == NULL ) ) )
      {
        psp_memcpy( name, start, path - start );
        name[path - start] = 0;
 #if F_CHECKNAME
        while ( *path && ( *( path + 1 ) == '/' || *( path + 1 ) == '\\' ) )
        {
          ++path;
        }

 #endif
        if ( filename && ( *( path + 1 ) == 0 ) )
        {
          break;                                /* if form was a/b/ and filename not zero -> filename="b/" */
        }

        rc = _f_find_dir( name, rc );        /* find directory */
        if ( rc == (F_DIR_ID_TYPE)F_INVALID )
        {
          return rc;
        }

        if ( *( path + 1 ) == 0 )
        {
          break;                        /* if form was a/b/ and b/ directory found stop */
        }

        start = path + 1;               /* set next name start pointer */
      }

      if ( *path == 0 )
      {
        break;                      /* stop if end of path reached (form: a/b) */
      }
    }
  }

  if ( filename )
  {
    *filename = (_PTRQ char *)start;
  }

  return rc;
} /* _f_check_path */
#endif /* if F_DIRECTORIES */




/*
** _f_find_file_wcard
**
** Find a file using wild card (*,?). If a file is found using find->findname then the
** parameters of it are setted in the find structure, otherwise find->curpos is set to
** invalid.
**
** INPUT : *find - pointer to F_FIND structure
*/
#if F_FINDING
static void _f_find_file_wcard ( F_FIND * find )
{
  unsigned long   addr;
  unsigned long   addrn;
  unsigned long   caddr;
  unsigned char   match;
  unsigned char   rc;
  char          * cname;
  F_FIND_ID_TYPE  pos;

 #if F_DIRECTORIES
  unsigned char  check;
  unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
 #endif

  if ( ( ( find->findpos ) == FIND_SHADOW_DIR_FIRST )
      && ( ( find->dirid ) == F_DIR_ROOT ) )
  {
    find->findpos = FIND_DIR_FIRST;
  }

  for ( ; find->findpos < FIND_POS_CNT ; find->findpos++ )
  {
    if ( FIND_POS_SHADOW_DIR( find->findpos ) )
    {
      if ( ( find->findpos ) == FIND_SHADOW_DIR_FIRST )
      {
        cname = _DIR_ACT;
      }
      else
      {
        cname = _DIR_TOP;
      }
    }

 #if F_DIRECTORIES
    else if ( FIND_POS_DIR( find->findpos ) )
    {
      pos = (F_FIND_ID_TYPE)FIND_DIR_IDX( find->findpos );
      addr = _DIR_DSC_ADDR( pos );
      addrn = addr + _SADDR( F_DIR_DSC, name[0] );
      cname = NULL;
    }
 #endif
    else
    {
 #if F_WILDCARD && QUICK_WILDCARD_SEARCH
      pos = (F_FIND_ID_TYPE)_QWS_ENTRY_POS( find->qws_pg, find->qws_pgp );
      addr = _FILE_ID_MGM_DSC_ADDR( pos );
      addrn = _QWS_ENTRY_ADDR( find->qws_pg, find->qws_pgp );

      ++find->qws_pgp;
      if ( ( find->qws_pgp == F_QWS_ENTRY_PER_PAGE ) || ( _QWS_ENTRY_POS( find->qws_pg, find->qws_pgp ) >= F_MAX_FILE ) )
      {
        ++find->qws_pg;
        find->qws_pgp = 0;
      }

 #else
      pos = (F_FIND_ID_TYPE)FIND_FILE_IDX( find->findpos );
      addr = _FILE_ID_MGM_DSC_ADDR( pos );
      addrn = addr;
 #endif
      if ( _FILE_ID_MGM_PAGE( pos ) == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
      {
        continue;
      }

 #if QUICK_FILE_SEARCH
      if ( find->wildcard == 0 )
      {
        if ( _f_qs_check( &find->find_qskey, pos ) )
        {
          continue;
        }
      }

 #endif
      cname = NULL;
    }

    match = 0;
 #if F_DIRECTORIES
    check = 0;
    if ( FIND_POS_SHADOW_DIR( find->findpos ) )
    {
      check = 1;
      caddr = 0;
    }
    else
    {
      rc = f_flash_mgm_read( _dirid, addrn + _SADDR( F_DIR_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
      if ( rc )
      {
        break;
      }

      if ( F_GET_DIR_ID( _dirid ) == find->dirid )
      {
        check = 1;
        caddr = addrn + _SADDR( F_FILE_DSC, name[0] );
      }
    }

    if ( check != 0 )
 #endif /* if F_DIRECTORIES */
    {
 #if F_WILDCARD
      if ( _f_cmp_wname( find->findname, cname, caddr, find->filename, 1 ) == 0 )
 #else
      if ( _f_cmp_wname( find->findname, cname, caddr, find->filename ) == 0 )
 #endif
      {
        match = 1;
      }
    }

    if ( match != 0 )
    {
      if ( FIND_POS_SHADOW_DIR( find->findpos ) )
      {
        find->attr = F_ATTR_DIR;
        find->ctime = 0;
        find->cdate = 0;
        find->filesize = 0;
      }

 #if F_DIRECTORIES
      else if ( FIND_POS_DIR( find->findpos ) )
      {
        F_DIR_PAR  dp;
        rc = f_flash_mgm_read( &dp, addr + _SADDR( F_DIR_DSC, par ), sizeof( F_DIR_PAR ) );
        if ( rc )
        {
          break;
        }

        find->attr = F_GET_ATTR( dp.attr );
        find->ctime = _f_getvalue_2( dp.ctime );
        find->cdate = _f_getvalue_2( dp.cdate );
        find->filesize = 0;
      }
 #endif
      else
      {
        F_FILE_PAR  fp;
        rc = f_flash_mgm_read( &fp, addr + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
        if ( rc )
        {
          break;
        }

        find->attr = F_GET_ATTR( fp.attr );
        find->ctime = _f_getvalue_2( fp.ctime );
        find->cdate = _f_getvalue_2( fp.cdate );
        find->filesize = F_GET_LENGTH( fp.length );
      }

      ++find->findpos;

      return;
    }
  }

  find->findpos = (F_FIND_ID_TYPE)F_INVALID;
} /* _f_find_file_wcard */
#endif /* if F_FINDING */


/**************************************************************************/
/*************************** USER FUNCTIONS *******************************/
/**************************************************************************/


/*
** fn_findfirst
**
** find first time a file using wildcards
**
** INPUT : filename - name of the file
**         *find - pointer to a pre-define F_FIND structure
** RETURN: F_NOERR - on success
**         F_ERR_NOTFOUND - if not found
*/
#if F_FINDING
unsigned char fn_findfirst ( const _PTRQ char * filename, F_FIND * find )
{
  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFOUND;
  }

 #if F_DIRECTORIES
  find->dirid = _f_check_path( filename, &( find->findname ) );
  if ( find->dirid == (F_DIR_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

 #else
  if ( ( *filename == '/' ) || ( *filename == '\\' ) )
  {
    ++filename;
  }

  find->findname = (_PTRQ char *)filename;
 #endif
  find->findpos = 0;
 #if QUICK_FILE_SEARCH
  {
    unsigned char  i;
    find->wildcard = 0;
    for ( i = 0 ; *( filename + i ) ; i++ )
    {
      unsigned char  ch = (unsigned char)*( filename + i );
      if ( ( ch == '*' ) || ( ch == '?' ) )
      {
        find->wildcard = 1;
        break;
      }
    }

    if ( find->wildcard == 0 )
    {
      _f_qs_genkey( &find->find_qskey, find->findname, 0 );
    }
  }
 #endif /* if QUICK_FILE_SEARCH */
 #if F_WILDCARD && QUICK_WILDCARD_SEARCH
  find->qws_pg = 0;
  find->qws_pgp = 0;
 #endif
  return fn_findnext( find );
} /* fn_findfirst */
#endif /* if F_FINDING */



/*
** fn_findnext
**
** find next time a file using wildcards
**
** INPUT : *find - pointer to a pre-define F_FIND structure
** RETURN: F_NOERR - on success
**         F_ERR_NOTFOUND - if not found
*/
#if F_FINDING
unsigned char fn_findnext ( F_FIND * find )
{
  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFOUND;
  }

  _f_find_file_wcard( find );
  if ( find->findpos == (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

  return F_NOERR;
}
#endif


/*
** fn_filelength
**
** Get the length of a file
**
** INPUT : filename - name of the file
** RETURN: size of the file or F_ERR_INVALID if not exists or volume not working
*/
#if F_FILELENGTH
long fn_filelength ( const _PTRQ char * filename )
{
  F_FILE_PAR  par;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_INVALID;
  }

 #if F_DIRECTORIES
  {
    F_DIR_ID_TYPE  dirid;
    _PTRQ char   * tmp;
    dirid = _f_check_path( filename, &tmp );
    if ( _f_find( tmp, &par, dirid ) == (F_FIND_ID_TYPE)F_INVALID )
    {
      return F_ERR_INVALID;
    }
  }
 #else
  if ( _f_find( filename, &par ) == (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_INVALID;
  }

 #endif
  return (long)F_GET_LENGTH( par.length );
} /* fn_filelength */
#endif /* if F_FILELENGTH */



/*
** fn_gettimedate
**
** Gets time and date for a file.
**
** INPUT : filename - name of the file
** OUTPUT: *pctime - where to store time
**         *pcdata - where to store date
** RETURN: F_NOERR on success, other if error
*/
#if F_GETTIMEDATE
int fn_gettimedate ( const _PTRQ char * filename, unsigned short * pctime, unsigned short * pcdate )
{
  F_FILE_PAR  par;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  {
    F_DIR_ID_TYPE  dirid;
    _PTRQ char   * tmp;
    dirid = _f_check_path( filename, &tmp );
    if ( _f_find( tmp, &par, dirid ) == (F_FIND_ID_TYPE)F_INVALID )
    {
      return F_ERR_NOTFOUND;
    }
  }
 #else
  if ( _f_find( filename, &par ) == (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

 #endif
  if ( pctime )
  {
    *pctime = _f_getvalue_2( par.ctime );
  }

  if ( pcdate )
  {
    *pcdate = _f_getvalue_2( par.cdate );
  }

  return F_NOERR;
} /* fn_gettimedate */
#endif /* if F_GETTIMEDATE */


/*
** fn_settimedate
**
** Sets time and date for a file.
**
** INPUT : filename - name of the file
**         pctime - time
**         pcdata - date
** RETURN: F_NOERR on success, other if error
*/
#if F_SETTIMEDATE
int fn_settimedate ( const _PTRQ char * filename, unsigned short pctime, unsigned short pcdate )
{
  unsigned char  rc;

 #if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid, fdirid;
 #endif
  _PTRQ char   * _filename;
  F_FILE_PAR     par;
  unsigned long  addr = (unsigned long)F_INVALID;


  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  dirid = _f_check_path( filename, &_filename );
  if ( dirid == (F_DIR_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

 #else
  _filename = (_PTRQ char *)filename;
 #endif

 #if F_DIRECTORIES
  fdirid = _f_find_dir( _filename, dirid );
  if ( fdirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    addr = _DIR_DSC_ADDR( fdirid ) + _SADDR( F_DIR_DSC, par ) + _SADDR( F_DIR_PAR, ctime );
  }
  else
 #endif
  {
    F_FILE_ID_TYPE  fileid;
 #if F_DIRECTORIES
    fileid = _f_find_file( _filename, dirid );
 #else
    fileid = _f_find_file( _filename );
 #endif
    if ( fileid != (F_FILE_ID_TYPE)F_INVALID )
    {
      if ( _f_checkopen( fileid ) )
      {
        return F_ERR_OPEN;
      }

      addr = _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, par ) + _SADDR( F_FILE_PAR, ctime );
    }
  }

  if ( addr != (unsigned long)F_INVALID )
  {
    _f_setvalue_2( par.ctime, pctime );
    _f_setvalue_2( par.cdate, pcdate );
    rc = f_flash_mgm_write_safe( addr, &par.ctime, sizeof( par.ctime ) + sizeof( par.cdate ) );
    if ( rc )
    {
      return rc;
    }
  }
  else
  {
    return F_ERR_NOTFOUND;
  }

  return F_NOERR;
} /* fn_settimedate */
#endif /* if F_SETTIMEDATE */


/*
** fn_getpermission
**
** Get permission of a file.
**
** INPUT : filename - name of the file
**         attr - where to store attribute
** RETURN: F_NOERR on success, other if error
*/
#if F_GETPERMISSION
int fn_getpermission ( const _PTRQ char * filename, F_ATTR_TYPE * attr )
{
  F_FILE_PAR  par;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  {
    F_DIR_ID_TYPE  dirid;
    _PTRQ char   * tmp;
    dirid = _f_check_path( filename, &tmp );
    if ( _f_find( tmp, &par, dirid ) == (F_FIND_ID_TYPE)F_INVALID )
    {
      return F_ERR_NOTFOUND;
    }
  }
 #else
  if ( _f_find( filename, &par ) == (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

 #endif
  if ( attr )
  {
    *attr = F_GET_ATTR( par.attr );
  }

  return F_NOERR;
} /* fn_getpermission */
#endif /* if F_GETPERMISSION */


/*
** fn_setpermission
**
** Set permission of a file.
**
** INPUT : filename - name of the file
**         attr - attribute
** RETURN: F_NOERR on success, other if error
*/
#if F_SETPERMISSION
int fn_setpermission ( const _PTRQ char * filename, F_ATTR_TYPE attr )
{
  unsigned char  rc;

 #if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid, fdirid;
 #endif
  _PTRQ char   * _filename;
  F_FILE_PAR     par;
  unsigned long  addr = (unsigned long)F_INVALID;


  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  dirid = _f_check_path( filename, &_filename );
  if ( dirid == (F_DIR_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

 #else
  _filename = (_PTRQ char *)filename;
 #endif

 #if F_DIRECTORIES
  fdirid = _f_find_dir( _filename, dirid );
  if ( fdirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    addr = _DIR_DSC_ADDR( fdirid ) + _SADDR( F_DIR_DSC, par ) + _SADDR( F_DIR_PAR, attr );
  }
  else
 #endif
  {
    F_FILE_ID_TYPE  fileid;
 #if F_DIRECTORIES
    fileid = _f_find_file( _filename, dirid );
 #else
    fileid = _f_find_file( _filename );
 #endif
    if ( fileid != (F_FILE_ID_TYPE)F_INVALID )
    {
      if ( _f_checkopen( fileid ) )
      {
        return F_ERR_OPEN;
      }

      addr = _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, par ) + _SADDR( F_FILE_PAR, attr );
    }
  }

  if ( addr != (unsigned long)F_INVALID )
  {
    F_SET_ATTR( par.attr, attr );
    rc = f_flash_mgm_write_safe( addr, &par.attr, F_ATTR_SIZE );
    if ( rc )
    {
      return rc;
    }
  }
  else
  {
    return F_ERR_NOTFOUND;
  }

  return F_NOERR;
} /* fn_setpermission */
#endif /* if F_SETPERMISSION */


/*
** fn_getfreespace
**
** Get free space on the volume
**
** OUTPUT: *sp - pre-defined F_SPACE structure, where information will be stored
** RETURN: F_NOERR - on success
**         F_ERR_NOTFORMATTED - if volume is not formatted
*/
#if F_GETFREESPACE
unsigned char fn_getfreespace ( F_SPACE * sp )
{
  F_CLUSTER_COUNT_TYPE  i;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

  sp->total = F_SIZE;
  sp->free = 0;
  for ( i = 0 ; i < F_CLUSTER_COUNT ; i++ )
  {
    if ( _GET_BIT( f_volume.cluster_table, i ) == 0 )
    {
      ++sp->free;
    }
  }

  sp->free *= F_CLUSTER_SIZE;
  return F_NOERR;
} /* fn_getfreespace */
#endif /* if F_GETFREESPACE */



/*
** fn_delete
**
** Delete a file. Removes the chain that belongs to the file and inserts a new descriptor
** to the directory with first_cluster set to 0.
**
** INPUT : filename - name of the file to delete
** RETURN: F_NOERR on success, other if error.
*/
#if F_DELETE
int fn_delete ( const _PTRQ char * filename )
{
  F_FILE_ID_TYPE  fileid;
  unsigned char   rc;

 #if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid;
 #endif
 #if SMALL_FILE_OPT
  F_FILE_PAR  par;
 #endif

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  {
    _PTRQ char * tmp;
    dirid = _f_check_path( filename, &tmp );
    fileid = _f_find_file( tmp, dirid );
  }
 #else
  fileid = _f_find_file( filename );
 #endif
  if ( fileid == (F_FILE_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

  if ( _f_checkopen( fileid ) )
  {
    return F_ERR_OPEN;
  }

 #if SMALL_FILE_OPT
  rc = f_flash_mgm_read( &par, _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
  if ( rc )
  {
    return rc;
  }

  rc = _f_remove_file_mgm( _FILE_ID_MGM_PAGE( fileid ), (char)( ( F_GET_LENGTH( par.length ) > F_FILE_MGM_PAGE0_SIZE ) ? 1 : 0 ) );
 #else
  rc = _f_remove_file_mgm( _FILE_ID_MGM_PAGE( fileid ), 1 );
 #endif
  if ( rc )
  {
    return rc;
  }

  _FILE_ID_MGM_PAGE( fileid ) = (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID;

 #if F_FILE_CHANGED_EVENT
  if ( f_filechangedevent_fn != NULL )
  {
    ST_FILE_CHANGED  fc;
    fc.action = FACTION_REMOVED;
    fc.flags = FFLAGS_FILE_NAME;
    if ( _f_createfullname( fc.filename, filename ) == 0 )
    {
      f_filechangedevent_fn( &fc );
    }
  }

 #endif

  return F_NOERR;
} /* fn_delete */
#endif /* if F_DELETE */



/*
** fn_rename
**
** Rename a file.
**
** INPUT : oldname - old name of the file
**         newname - new name of the file
** RETURN: F_NOERR on success, other if error
*/
#if F_RENAME
int fn_rename ( const _PTRQ char * oldname, const _PTRQ char * newname )
{
  unsigned char  rc;

 #if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid, fdirid;
 #endif
  _PTRQ char   * filename;
  unsigned long  addr = (unsigned long)F_INVALID;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

 #if F_DIRECTORIES
  #if F_CHECKNAME
  if ( _check_dir_delimiter( newname ) != (unsigned char)F_INVALID )
  {
    return F_ERR_INVALIDNAME;
  }

  if ( psp_strnlen( newname, F_MAX_FILE_NAME_LENGTH + 1 ) > F_MAX_FILE_NAME_LENGTH )
  {
    return F_ERR_INVALIDNAME;
  }

  #endif
  dirid = _f_check_path( oldname, &filename );
  if ( dirid == (F_DIR_ID_TYPE)F_INVALID )
  {
    return F_ERR_NOTFOUND;
  }

  if ( _f_find( newname, NULL, dirid ) != (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_DUPLICATED;
  }

 #else /* if F_DIRECTORIES */
  if ( ( *newname == '/' ) || ( *newname == '\\' ) )
  {
    ++newname;
  }

  if ( _f_find( newname, NULL ) != (F_FIND_ID_TYPE)F_INVALID )
  {
    return F_ERR_DUPLICATED;
  }

  filename = (_PTRQ char *)oldname;
 #endif /* if F_DIRECTORIES */

 #if F_DIRECTORIES
  fdirid = _f_find_dir( filename, dirid );
  if ( fdirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    dirid = f_volume.current_dir;
    while ( dirid != F_DIR_ROOT )
    {
      unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
      if ( dirid == fdirid )
      {
        return F_ERR_OPEN;
      }

      rc = f_flash_mgm_read( _dirid, _DIR_DSC_ADDR( dirid ) + _SADDR( F_DIR_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
      if ( rc )
      {
        return rc;
      }

      dirid = F_GET_DIR_ID( _dirid );
    }

    if ( _f_checkopendir( fdirid ) )
    {
      return F_ERR_OPEN;
    }

    addr = _DIR_DSC_ADDR( fdirid ) + _SADDR( F_DIR_DSC, name[0] );
  }
  else
 #endif /* if F_DIRECTORIES */
  {
    F_FILE_ID_TYPE  fileid;
 #if F_DIRECTORIES
    fileid = _f_find_file( filename, dirid );
 #else
    fileid = _f_find_file( filename );
 #endif
    if ( fileid != (F_FILE_ID_TYPE)F_INVALID )
    {
      if ( _f_checkopen( fileid ) )
      {
        return F_ERR_OPEN;
      }

 #if F_WILDCARD && QUICK_WILDCARD_SEARCH
      addr = _QWS_ENTRY_ADDR( _QWS_EPAGE( fileid ), _QWS_EPOS( fileid ) );
      rc = f_flash_mgm_write( addr + _SADDR( F_QWS_ENTRY, name[0] ), (_PTRQ char *)newname, F_MAX_FILE_NAME_LENGTH + 1 );
      if ( rc )
      {
        return rc;
      }

 #endif
      addr = _FILE_ID_MGM_DSC_ADDR( fileid ) + _SADDR( F_FILE_DSC, name[0] );
 #if QUICK_FILE_SEARCH
      _f_qs_genkey( f_volume.qskey + fileid, newname, 0 );
 #endif
    }
  }

  if ( addr != (unsigned long)F_INVALID )
  {
    rc = f_flash_mgm_write_safe( addr, (_PTRQ char *)newname, F_MAX_FILE_NAME_LENGTH + 1 );
    if ( rc )
    {
      return rc;
    }
  }
  else
  {
    return F_ERR_NOTFOUND;
  }

  return F_NOERR;
} /* fn_rename */
#endif /* if F_RENAME */



/*
** fn_mkdir
**
** Create a directory
**
** INPUT:  path - new directory path
** RETURN: 0 - on success, other if error
*/
#if ( F_DIRECTORIES && F_MKDIR )
int fn_mkdir ( const _PTRQ char * path )
{
  F_DIR_DSC      dsc;
  F_DIR_ID_TYPE  dirid, ndirid;
  _PTRQ char   * name;
  unsigned char  rc;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

  dirid = _f_check_path( path, &name );
  if ( name && *name )
  {
    psp_memcpy( dsc.name, name, F_MAX_FILE_NAME_LENGTH + 1 );
 #if F_CHECKNAME
    {
      unsigned char  pos;
      if ( psp_strnlen( name, F_MAX_FILE_NAME_LENGTH + 1 ) > F_MAX_FILE_NAME_LENGTH )
      {
        return F_ERR_INVALIDNAME;
      }

      pos = _check_dir_delimiter( dsc.name );
      if ( pos != (unsigned char)F_INVALID )
      {
        dsc.name[pos] = 0;
      }
    }
 #endif

    if ( _f_find( dsc.name, NULL, dirid ) == (F_FIND_ID_TYPE)F_INVALID )
    {
      ndirid = _f_get_dir_entry();
      if ( ndirid != (F_DIR_ID_TYPE)F_INVALID )
      {
        unsigned short  ctime, cdate;
        _f_get_timedate( &ctime, &cdate );
        F_SET_ATTR( dsc.par.attr, F_ATTR_DIR );
        _f_setvalue_2( dsc.par.ctime, ctime );
        _f_setvalue_2( dsc.par.cdate, cdate );
        F_SET_DIR_ID( dsc.dirid, dirid );
        rc = f_flash_mgm_write_safe( _DIR_DSC_ADDR( ndirid ), &dsc, sizeof( F_DIR_DSC ) );
 #if F_FILE_CHANGED_EVENT
        if ( f_filechangedevent_fn != NULL )
        {
          ST_FILE_CHANGED  fc;
          fc.action = FACTION_ADDED;
          fc.flags = FFLAGS_DIR_NAME;
          if ( _f_createfullname( fc.filename, path ) == 0 )
          {
            f_filechangedevent_fn( &fc );
          }
        }

 #endif
        return rc;
      }

      return F_ERR_NOMOREENTRY;
    }

    return F_ERR_DUPLICATED;
  }

  return F_ERR_INVALIDDIR;
} /* fn_mkdir */
#endif /* if ( F_DIRECTORIES && F_MKDIR ) */



/*
** fn_rmdir
**
** Removes a directory
**
** INPUT:  path - path to remove
** RETURN: 0 - on success, other if error
*/
#if ( F_DIRECTORIES && F_RMDIR )
int fn_rmdir ( const _PTRQ char * path )
{
  F_DIR_ID_TYPE  dirid;
  char           del_flag = (char)_F_INV_8;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

  dirid = _f_check_path( path, NULL );
  if ( dirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    if ( f_volume.current_dir != dirid )
    {
      if ( _f_find( NULL, NULL, dirid ) == (F_FIND_ID_TYPE)F_INVALID )
      {
        unsigned char  rc;
        rc = f_flash_mgm_write_safe( _DIR_DSC_ADDR( dirid ) + _SADDR( F_DIR_DSC, name[0] ), &del_flag, 1 );
 #if F_FILE_CHANGED_EVENT
        if ( f_filechangedevent_fn != NULL )
        {
          ST_FILE_CHANGED  fc;
          fc.action = FACTION_REMOVED;
          fc.flags = FFLAGS_DIR_NAME;
          if ( _f_createfullname( fc.filename, path ) == 0 )
          {
            f_filechangedevent_fn( &fc );
          }
        }

 #endif
        return rc;
      }
      else
      {
        return F_ERR_NOTEMPTY;
      }
    }
    else
    {
      return F_ERR_ACCESSDENIED;
    }
  }

  return F_ERR_NOTFOUND;
} /* fn_rmdir */
#endif /* if ( F_DIRECTORIES && F_RMDIR ) */



/*
** fn_chdir
**
** Change to a directory
**
** INPUT:  path - path to the dircetory
** RETURN: 0 - on success, other if error
*/
#if ( F_DIRECTORIES && F_CHDIR )
int fn_chdir ( const _PTRQ char * path )
{
  F_DIR_ID_TYPE  dirid;

  if ( f_volume.state == F_STATE_NOTFORMATTED )
  {
    return F_ERR_NOTFORMATTED;
  }

  dirid = _f_check_path( path, NULL );
  if ( dirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    f_volume.current_dir = dirid;
    return F_NOERR;
  }

  return F_ERR_NOTFOUND;
} /* fn_chdir */
#endif /* if ( F_DIRECTORIES && F_CHDIR ) */



/*
** _f_getdir
**
** Get directory name for dirid
**
** INPUT:  maxlen - maximum length allowed
** OUTPUT: path - current working directory
** INPUT:  dirid - directory id
** RETURN: 0 - on success, other if error
*/
#if F_DIRECTORIES
static int _f_getdir ( char * path, int maxlen, F_DIR_ID_TYPE dirid )
{
  char           tmp[F_MAX_FILE_NAME_LENGTH + 1];
  unsigned char  _dirid[F_DIR_ID_TYPE_SIZE];
  unsigned char  rc;
  unsigned char  len;

  if ( maxlen < 2 )
  {
    return F_ERR_INVALIDSIZE;
  }

  *path++ = '/';
  *path = 0;
  maxlen -= 2;

  while ( dirid != F_DIR_ROOT )
  {
    rc = f_flash_mgm_read( tmp, _DIR_DSC_ADDR( dirid ) + _SADDR( F_DIR_DSC, name[0] ), F_MAX_FILE_NAME_LENGTH + 1 );
    if ( rc )
    {
      return rc;
    }

    len = (unsigned char)psp_strnlen( tmp, F_MAX_FILE_NAME_LENGTH );
    if ( maxlen < len + 1 )
    {
      return F_ERR_INVALIDSIZE;
    }

    psp_memmove( path + len + 1, path, psp_strnlen( path, maxlen ) + 1 );
    psp_memcpy( path, tmp, len );
    *( path + len ) = '/';
    maxlen -= ( len + 1 );
    rc = f_flash_mgm_read( _dirid, _DIR_DSC_ADDR( dirid ) + _SADDR( F_DIR_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
    if ( rc )
    {
      return rc;
    }

    dirid = F_GET_DIR_ID( _dirid );
  }

  return F_NOERR;
} /* _f_getdir */
#endif /* if F_DIRECTORIES */


/*
** _f_createfullname
**
** Creates full path
**
** OUTPUT: dst - where to write absolute path
** INPUT: src - source path
** RETURN: 0 - on success, other if error
*/
#if F_FILE_CHANGED_EVENT
int _f_createfullname ( char * dst, const _PTRQ char * src )
{
  int           rc = F_NOERR;
  unsigned int  len;

 #if F_DIRECTORIES
  F_DIR_ID_TYPE  dirid;
  _PTRQ char   * tmp;

  dirid = _f_check_path( src, &tmp );
  if ( dirid != (F_DIR_ID_TYPE)F_INVALID )
  {
    rc = _f_getdir( dst, _F_FILE_CHANGED_MAXPATH, dirid );
    if ( rc == F_NOERR )
    {
      len = psp_strnlen( dst, _F_FILE_CHANGED_MAXPATH );
      if ( len + psp_strnlen( tmp, _F_FILE_CHANGED_MAXPATH ) < _F_FILE_CHANGED_MAXPATH )
      {
        psp_strncat( dst, tmp, _F_FILE_CHANGED_MAXPATH );
      }
      else
      {
        rc = F_ERR_INVALIDSIZE;
      }
    }
  }

 #else /* if F_DIRECTORIES */
  if ( psp_strnlen( src, _F_FILE_CHANGED_MAXPATH ) < _F_FILE_CHANGED_MAXPATH - 1 )
  {
    *dst = '/';
    psp_strncpy( dst + 1, src, _F_FILE_CHANGED_MAXPATH );
  }
  else
  {
    rc = F_ERR_INVALIDSIZE;
  }

 #endif /* if F_DIRECTORIES */

  return rc;
} /* _f_createfullname */
#endif /* if F_FILE_CHANGED_EVENT */


/*
** fn_getcwd
**
** Get current working directory
**
** INPUT:  maxlen - maximum length allowed
** OUTPUT: path - current working directory
** RETURN: 0 - on success, other if error
*/
#if ( F_DIRECTORIES && F_GETCWD )
int fn_getcwd ( char * path, int maxlen )
{
  return _f_getdir( path, maxlen, f_volume.current_dir );
}
#endif

