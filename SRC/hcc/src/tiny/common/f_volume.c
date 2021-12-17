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
#include "../../config/config_tiny.h"
#include "tiny.h"
#include "f_dir.h"
#include "f_util.h"
#if RTOS_SUPPORT
 #include "f_rtos.h"
#endif

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif
#include "../../version/ver_psp_string.h"
#if VER_PSP_STRING_MAJOR != 1
 #error Incompatible PSP_STRING version number!
#endif



F_VOLUME  f_volume;               /* volume structure */
F_FILE    files[F_MAX_OPEN_FILE]; /* file structure */

#if F_FILE_CHANGED_EVENT
F_FILE_CHANGED_EVENT_FN  f_filechangedevent_fn;
#endif

/**************************************************************************/
/************************* INTERNAL FUNCTIONS *****************************/
/**************************************************************************/


/*
** Get a free directory entry
*/
#if F_DIRECTORIES
F_DIR_ID_TYPE _f_get_dir_entry ( void )
{
  F_DIR_ID_TYPE  i, j;
  unsigned char  tmp;

  for ( i = 0 ; i < F_MAX_DIR ; i++ )
  {
    j = f_volume.dir_empty_pos++;
    if ( j + 1 == F_MAX_DIR )
    {
      f_volume.dir_empty_pos = 0;
    }

    if ( f_flash_mgm_read( &tmp, _DIR_DSC_ADDR( j ) + _SADDR( F_DIR_DSC, name[0] ), 1 ) )
    {
      break;
    }

    if ( tmp == _F_INV_8 )
    {
      return j;
    }
  }

  return (F_DIR_ID_TYPE)F_INVALID;
} /* _f_get_dir_entry */
#endif /* if F_DIRECTORIES */


/*
** Get empty data cluster
*/
F_CLUSTER_COUNT_TYPE _f_get_empty_cluster ( void )
{
  F_CLUSTER_COUNT_TYPE  i, j;

  for ( i = 0 ; i < F_CLUSTER_COUNT ; i++ )
  {
    j = f_volume.cluster_empty_pos++;
    if ( j + 1 == F_CLUSTER_COUNT )
    {
      f_volume.cluster_empty_pos = 0;
    }

    if ( _GET_BIT( f_volume.cluster_table, j ) == 0 )
    {
      _SET_BIT( f_volume.cluster_table, j );
      return j;
    }
  }

  return (F_CLUSTER_COUNT_TYPE)F_INVALID;
} /* _f_get_empty_cluster */



/*
** Get empty file management page
** INPUT: fileid - file ID
**        state - state stored in header (if zero skip storing of the header)
**        seq - sequence number
*/
F_FILE_MGM_PAGE_COUNT_TYPE _f_get_file_mgm_empty_page ( F_FILE_ID_TYPE fileid, unsigned char state, unsigned char seq )
{
  F_FILE_MGM_HEADER           hdr;
  F_FILE_MGM_PAGE_COUNT_TYPE  i, j;
  unsigned char               rc;

  for ( i = 0 ; i < F_FILE_MGM_PAGE_COUNT ; i++ )
  {
    j = f_volume.file_mgm_empty_pos++;
    if ( j + 1 == F_FILE_MGM_PAGE_COUNT )
    {
      f_volume.file_mgm_empty_pos = 0;
    }

    if ( _GET_BIT( f_volume.file_mgm_table, j ) == 0 )
    {
      if ( f_flash_mgm_erase( _FILE_MGM_ADDR( j ), F_MGM_PAGE_SIZE ) )
      {
        break;
      }

      if ( state )
      {
        F_SET_FILE_MGM_PAGE_COUNT( hdr.next, _F_INV_FILE_MGM_PAGE_COUNT );
        F_SET_FILE_ID( hdr.fileid, fileid );
        hdr.seq = seq;
        hdr.inv_seq = (unsigned char)~seq;
        hdr.state = state;
        hdr.inv_state = (unsigned char)~state;
        rc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( j ), &hdr, sizeof( F_FILE_MGM_HEADER ) );
      }

      if ( ( state == 0 ) || ( rc == F_NOERR ) )
      {
        _SET_BIT( f_volume.file_mgm_table, j );
        return j;
      }
    }
  }

  return (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID;
} /* _f_get_file_mgm_empty_page */


/*
** Get cluster value for a file.
*/
unsigned char _f_get_cluster_value ( F_FILE * f )
{
  F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  cl_per_page;
  unsigned char                           _cluster[F_CLUSTER_COUNT_TYPE_SIZE];
  unsigned char                           rc;

  if ( f->first_mgm_page == f->act_mgm_page )
  {
    cl_per_page = F_FILE_MGM_CLUSTER_PER_PAGE0;
  }
  else
  {
    cl_per_page = F_FILE_MGM_CLUSTER_PER_PAGE;
  }

  if ( f->page_cl_pos == cl_per_page )
  {
    F_FILE_MGM_HEADER           mgm_hdr;
    F_FILE_MGM_PAGE_COUNT_TYPE  next;

    rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( f->act_mgm_page ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc )
    {
      return rc;
    }

    next = F_GET_FILE_MGM_PAGE_COUNT( mgm_hdr.next );
    if ( next != _F_INV_FILE_MGM_PAGE_COUNT )
    {
      f->act_mgm_page = next;
      f->page_cl_pos = 0;
      ++f->page_cnt;
    }
    else
    {
      return (unsigned char)F_INVALID;
    }
  }

  rc = f_flash_mgm_read( _cluster, _FILE_MGM_PAGE_CLUSTER_ADDR( f->act_mgm_page, f->page_cl_pos ), F_CLUSTER_COUNT_TYPE_SIZE );
  if ( rc )
  {
    return rc;
  }

  f->cluster = F_GET_CLUSTER_COUNT( _cluster );
  if ( f->cluster == _F_INV_CLUSTER_COUNT )
  {
    return (unsigned char)F_INVALID;
  }
  else
  {
    ++f->page_cl_pos;
  }

  return F_NOERR;
} /* _f_get_cluster_value */


/*
** Set a cluster value
*/
unsigned char _f_set_cluster_value ( F_FILE * f )
{
  F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  cl_per_page;
  unsigned char                           _cluster[F_CLUSTER_COUNT_TYPE_SIZE];
  unsigned char                           rc;


  if ( f->first_mgm_page == f->act_mgm_page )
  {
    cl_per_page = F_FILE_MGM_CLUSTER_PER_PAGE0;
  }
  else
  {
    cl_per_page = F_FILE_MGM_CLUSTER_PER_PAGE;
  }

  if ( f->page_cl_pos == cl_per_page )
  {
    F_FILE_MGM_HEADER           mgm_hdr;
    F_FILE_MGM_PAGE_COUNT_TYPE  next;

    rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( f->act_mgm_page ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc )
    {
      return rc;
    }

    next = F_GET_FILE_MGM_PAGE_COUNT( mgm_hdr.next );
    if ( next == _F_INV_FILE_MGM_PAGE_COUNT )
    {
      next = _f_get_file_mgm_empty_page( 0, 0, 0 );
      if ( next == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
      {
        return (unsigned char)F_INVALID;
      }

      F_SET_FILE_MGM_PAGE_COUNT( mgm_hdr.next, next );
      rc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( f->act_mgm_page ), &mgm_hdr, sizeof( F_FILE_MGM_HEADER ) );
      if ( rc )
      {
        return rc;
      }

      F_SET_FILE_MGM_PAGE_COUNT( mgm_hdr.next, _F_INV_FILE_MGM_PAGE_COUNT );
      mgm_hdr.state = _FM_STATE_EXT;
      mgm_hdr.inv_state = (unsigned char)~_FM_STATE_EXT;
      rc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( next ), &mgm_hdr, sizeof( F_FILE_MGM_HEADER ) );
      if ( rc )
      {
        return rc;
      }

      f->act_mgm_page = next;
    }
    else
    {
      f->act_mgm_page = next;
    }

    ++f->page_cnt;
    f->page_cl_pos = 0;
  }

  F_SET_CLUSTER_COUNT( _cluster, f->cluster );
  rc = f_flash_mgm_write( _FILE_MGM_PAGE_CLUSTER_ADDR( f->act_mgm_page, f->page_cl_pos++ ), _cluster, F_CLUSTER_COUNT_TYPE_SIZE );
  if ( rc )
  {
    return rc;
  }

  return F_NOERR;
} /* _f_set_cluster_value */


/*
** use orig_first_mgm_page otherwise us first_mgm_page
*/
F_CLUSTER_COUNT_TYPE _f_get_orig_cluster_value ( F_FILE * f )
{
  F_FILE_MGM_PAGE_COUNT_TYPE  act = _FILE_ID_MGM_PAGE( f->fileid );
  F_FILE_MGM_PAGE_COUNT_TYPE  page_cnt = f->page_cnt;
  F_FILE_MGM_HEADER           mgm_hdr;
  unsigned char               _cluster[F_CLUSTER_COUNT_TYPE_SIZE];
  unsigned char               rc2;

  while ( act != _F_INV_FILE_MGM_PAGE_COUNT && page_cnt )
  {
    rc2 = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( act ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc2 )
    {
      return (F_CLUSTER_COUNT_TYPE)F_INVALID;
    }

    act = F_GET_FILE_MGM_PAGE_COUNT( mgm_hdr.next );
    --page_cnt;
  }

  if ( act != _F_INV_FILE_MGM_PAGE_COUNT )
  {
    rc2 = f_flash_mgm_read( _cluster, _FILE_MGM_PAGE_CLUSTER_ADDR( act, f->page_cl_pos - 1 ), F_CLUSTER_COUNT_TYPE_SIZE );
    if ( !rc2 )
    {
      F_CLUSTER_COUNT_TYPE  cluster;
      cluster = F_GET_CLUSTER_COUNT( _cluster );
      if ( cluster == _F_INV_CLUSTER_COUNT )
      {
        return (F_CLUSTER_COUNT_TYPE)F_INVALID;
      }

      return cluster;
    }
  }

  return (F_CLUSTER_COUNT_TYPE)F_INVALID;
} /* _f_get_orig_cluster_value */



/*
** Remove file management chain.
*/
unsigned char _f_remove_file_mgm ( F_FILE_MGM_PAGE_COUNT_TYPE first, char mode )
{
  F_FILE_MGM_HEADER                       mgm_hdr;
  F_FILE_MGM_PAGE_COUNT_TYPE              act_page;
  F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE  cl_cnt;
  F_CLUSTER_COUNT_TYPE                    cl_act;
  F_CLUSTER_COUNT_TYPE                    cl_pos;
  unsigned char                           cl_buf[F_CLUSTER_COUNT_TYPE_SIZE];
  unsigned char                           rc;

  if ( mode )  /* remove clusters from cluster table */
  {
    act_page = first;
    rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( act_page ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc == F_NOERR )
    {
      cl_pos = 0;
      if ( mgm_hdr.state == _FM_STATE_EXT )
      {
        cl_cnt = F_FILE_MGM_CLUSTER_PER_PAGE;
      }
      else
      {
        cl_cnt = F_FILE_MGM_CLUSTER_PER_PAGE0;
      }

      for ( ; ; )
      {
        if ( cl_pos == cl_cnt )
        {
          act_page = F_GET_FILE_MGM_PAGE_COUNT( mgm_hdr.next ); /* get next management page */
          if ( act_page == _F_INV_FILE_MGM_PAGE_COUNT )         /* next page is invalid */
          {
            break;                                               /* exit loop */
          }
          else
          {
            rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( act_page ), sizeof( F_FILE_MGM_HEADER ) );
            if ( rc )
            {
              return rc;
            }
          }

          cl_pos = 0;
          cl_cnt = F_FILE_MGM_CLUSTER_PER_PAGE;
        }

        /* get cluster value */
        rc = f_flash_mgm_read( &( cl_buf[0] ), _FILE_MGM_PAGE_CLUSTER_ADDR( act_page, cl_pos ), F_CLUSTER_COUNT_TYPE_SIZE );
        if ( rc )
        {
          return rc;
        }

        cl_act = F_GET_CLUSTER_COUNT( cl_buf );
        if ( cl_act == _F_INV_CLUSTER_COUNT )
        {
          break;                                              /* stop if invalid (last) cluster */
        }

        if ( cl_act < F_CLUSTER_COUNT )
        {
          _CLEAR_BIT( f_volume.cluster_table, cl_act );       /* unmark if cluster is in range */
        }

        ++cl_pos;
      }
    }
  }

  act_page = first;
  while ( ( act_page != (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
         && ( act_page < F_FILE_MGM_PAGE_COUNT ) )
  {
    rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( act_page ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc )
    {
      return rc;
    }

    if ( mgm_hdr.state == _FM_STATE_EXT )
    {
      (void)f_flash_mgm_erase( _FILE_MGM_ADDR( act_page ), F_MGM_PAGE_SIZE );
    }

    _CLEAR_BIT( f_volume.file_mgm_table, act_page );
    act_page = F_GET_FILE_MGM_PAGE_COUNT( mgm_hdr.next );
  }

  rc = f_flash_mgm_erase_safe( _FILE_MGM_ADDR( first ), F_MGM_PAGE_SIZE );

  return rc;
} /* _f_remove_file_mgm */


/*
** copy file managment chain to a new place
*/
F_FILE_MGM_PAGE_COUNT_TYPE _f_copy_mgm ( F_FILE_MGM_PAGE_COUNT_TYPE oact )
{
  F_FILE_MGM_HEADER           hdr;
  F_FILE_MGM_PAGE_COUNT_TYPE  nfirst, nact, nnext;
  unsigned char               frc;

  frc = 0;
  nfirst = _f_get_file_mgm_empty_page( 0, 0, 0 );      /* get an empty file management page */
  nact = nfirst;
  nnext = _F_INV_FILE_MGM_PAGE_COUNT;
  while ( nact != _F_INV_FILE_MGM_PAGE_COUNT )
  {
    frc = f_flash_mgm_read( &hdr, _FILE_MGM_HEADER_ADDR( oact ), sizeof( F_FILE_MGM_HEADER ) );
    if ( frc )
    {
      break;
    }

    nnext = F_GET_FILE_MGM_PAGE_COUNT( hdr.next );          /* get next page of the current chain */
    if ( nnext != _F_INV_FILE_MGM_PAGE_COUNT )              /* there is a next element */
    {
      nnext = _f_get_file_mgm_empty_page( 0, 0, 0 );        /* get an empty file management page */
      if ( nnext == (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID ) /* couldn't get a new management page */
      {
        frc = 1;                                            /* exit loop, not possible to copy */
        break;
      }
    }

    frc = f_flash_mgm_copy( _FILE_MGM_PAGE_CLUSTER_ADDR( nact, 0 ), _FILE_MGM_PAGE_CLUSTER_ADDR( oact, 0 ), F_MGM_PAGE_SIZE );
    if ( frc )
    {
      break;
    }

    oact = F_GET_FILE_MGM_PAGE_COUNT( hdr.next );  /* get next page of the exisiting chain */

    ++hdr.seq;
    --hdr.inv_seq;                                 /* increase sequence number */
    F_SET_FILE_MGM_PAGE_COUNT( hdr.next, nnext );
    if ( nfirst == nact )
    {
      hdr.state = _FM_STATE_OPEN;                  /* set open state for the first page */
    }
    else
    {
      hdr.state = _FM_STATE_EXT;                   /* and extended for the others */
    }

    hdr.inv_state = (unsigned char)~hdr.state;     /* get the inverse of the state */

    /* write the header */
    frc = f_flash_mgm_write( _FILE_MGM_HEADER_ADDR( nact ), &hdr, sizeof( F_FILE_MGM_HEADER ) );
    if ( frc )
    {
      break;
    }

    nact = nnext;
  }

  if ( frc )      /* if there was a problem remove everything and return error */
  {
    /* need to remove nnext here (if valid) as the link between the actual page and next page is */
    /* not created yet (via the header) */
    if ( nnext != _F_INV_FILE_MGM_PAGE_COUNT )
    {
      (void)f_flash_mgm_erase( _FILE_MGM_ADDR( nnext ), F_MGM_PAGE_SIZE );
      _CLEAR_BIT( f_volume.file_mgm_table, nnext );
    }

    (void)_f_remove_file_mgm( nfirst, 0 );

    return (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID;
  }

  return nfirst;
} /* _f_copy_mgm */


/*
** _f_qws_check
** Validate quick wildcard search management area and fix erroneous pages
*/
#if QUICK_WILDCARD_SEARCH
unsigned char _f_qws_check ( void )
{
  unsigned char               rc = F_NOERR;
  F_FILE_MGM_PAGE_COUNT_TYPE  filepos;
  F_QWS_PAGE_COUNT_TYPE       pg;
  F_QWS_ENTRY_PER_PAGE_TYPE   pgpos;
  unsigned char               j;
  unsigned long               addr;
  char                        tmp[F_MAX_FILE_NAME_LENGTH + 1];
  char                        ch;

  for ( pg = 0 ; pg < F_QWS_PAGE_COUNT ; pg++ )
  {
    filepos = pg;
    for ( pgpos = 0 ; pgpos < F_QWS_ENTRY_PER_PAGE && filepos < F_MAX_FILE ; pgpos++ )
    {                                                       /* find the first valid file name entry in the table */
      if ( _FILE_ID_MGM_PAGE( filepos ) != (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
      {
        break;
      }

      filepos += F_QWS_PAGE_COUNT;
    }

    if ( ( pgpos < F_QWS_ENTRY_PER_PAGE ) && ( filepos < F_MAX_FILE ) )
    {
      rc = f_flash_mgm_read( tmp, _FILE_ID_MGM_DSC_ADDR( filepos ) + _SADDR( F_FILE_DSC, name[0] ), F_MAX_FILE_NAME_LENGTH + 1 );
      if ( rc )
      {
        return rc;                                          /* read file name in the management page */
      }

      addr = _QWS_ENTRY_ADDR( pg, pgpos ) + _SADDR( F_QWS_ENTRY, name[0] );
      for ( j = 0 ; j < F_MAX_FILE_NAME_LENGTH + 1 ; j++ )              /* check QWS filename against original */
      {
        rc = f_flash_mgm_read( &ch, addr++, 1 );
        if ( rc )
        {
          return rc;
        }

        if ( !ch || ( ch != tmp[j] ) )
        {
          break;                                            /* stop if zero termination or differs */
        }
      }

      if ( ch != tmp[j] )                                       /* if differs update whole page */
      {
        F_FILE_MGM_PAGE_COUNT_TYPE  filepos2;
        F_QWS_ENTRY_PER_PAGE_TYPE   pgpos2;
 #if F_DIRECTORIES
        unsigned char               dirid[F_DIR_ID_TYPE_SIZE];
 #endif

        filepos2 = pg;
        for ( pgpos2 = 0 ; pgpos2 < F_QWS_ENTRY_PER_PAGE && filepos2 < F_MAX_FILE ; pgpos2++ )
        {                                                   /* step through QWS page */
          if ( _FILE_ID_MGM_PAGE( filepos2 ) != (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
          {
            rc = f_flash_mgm_read( tmp, _FILE_ID_MGM_DSC_ADDR( filepos2 ) + _SADDR( F_FILE_DSC, name[0] ), F_MAX_FILE_NAME_LENGTH + 1 );
            if ( rc )
            {
              return rc;                                    /* read file name in the management page */
            }

 #if F_DIRECTORIES
            rc = f_flash_mgm_read( dirid, _FILE_ID_MGM_DSC_ADDR( filepos2 ) + _SADDR( F_FILE_DSC, dirid ), F_DIR_ID_TYPE_SIZE );
            if ( rc )
            {
              return rc;                                    /* read file name in the management page */
            }

 #endif
            addr = _QWS_ENTRY_ADDR( pg, pgpos2 );
            rc = f_flash_mgm_write( addr + _SADDR( F_QWS_ENTRY, name[0] ), tmp, F_MAX_FILE_NAME_LENGTH + 1 );
            if ( rc )
            {
              return rc;                                    /* write file name to the QWS page */
            }

 #if F_DIRECTORIES
            rc = f_flash_mgm_write( addr + _SADDR( F_QWS_ENTRY, dirid ), dirid, F_DIR_ID_TYPE_SIZE );
            if ( rc )
            {
              return rc;                                    /* write file name to the QWS page */
            }

 #endif
          }

          filepos2 += F_QWS_PAGE_COUNT;
        }
      }
    }
  }

  return rc;
} /* _f_qws_check */
#endif /* if QUICK_WILDCARD_SEARCH */


/**************************************************************************/
/*************************** USER FUNCTIONS *******************************/
/**************************************************************************/


/*
** fn_init
**
** Initialize TINY file system
**
** RETURN: F_NOERR on success, other if error.
*/
unsigned char fn_init ( void )
{
  unsigned char  rc;

#if F_FILE_CHANGED_EVENT
  f_filechangedevent_fn = NULL;
#endif
  rc = f_flash_init();
  if ( rc )
  {
    return rc;
  }

#if RTOS_SUPPORT
  rc = fr_init();
  if ( rc )
  {
    return rc;
  }

#endif
  return rc;
} /* fn_init */


/*
** fn_initvolume
**
** Initialize volume and fix inconsistencies on the volume.
**
** RETURN: F_NOERR on success, other if error.
*/
unsigned char fn_initvolume ( void )
{
  unsigned char               rc;
  F_FILE_MGM_PAGE_COUNT_TYPE  fi;

#if F_DIRECTORIES
  F_DIR_ID_TYPE  di;
#endif
#if SMALL_FILE_OPT
  F_FILE_PAR         par;
#endif
  F_FILE_MGM_HEADER  mgm_hdr;
  F_FILE_ID_TYPE     fileid;

  f_volume.state = F_STATE_NOTFORMATTED;

#if F_CHECKMEDIA
  {
    F_FS_ID        fs_id;
    unsigned char  idpar;
    idpar = 0;
    rc = f_flash_mgm_read( &fs_id, F_FS_ID_ADDR, sizeof( F_FS_ID ) );
    if ( rc )
    {
      return rc;
    }

    if ( psp_strncmp( fs_id.tinystr, _TINYSTR, sizeof( fs_id.tinystr ) ) )
    {
      return F_ERR_NOTFORMATTED;
    }

 #if SMALL_FILE_OPT
    idpar |= 1;
 #endif
 #if QUICK_WILDCARD_SEARCH
    idpar |= 4;
 #endif
    idpar |= ( ( F_ATTR_SIZE - 1 ) << 3 );

    if ( ( _f_getvalue_2( fs_id.version ) != _TINY_FORMAT_VERSION )
        || ( _f_getvalue_2( fs_id.f_cluster_size ) != F_CLUSTER_SIZE )
        || ( _f_getvalue_2( fs_id.f_mgm_page_size ) != F_MGM_PAGE_SIZE )
 #if F_DIRECTORIES
        || ( _f_getvalue_2( fs_id.f_dir_page_count ) != F_DIR_PAGE_COUNT )
 #else
        || _f_getvalue_2( fs_id.f_dir_page_count )
 #endif
        || ( _f_getvalue_4( fs_id.f_file_mgm_page_count ) != F_FILE_MGM_PAGE_COUNT )
        || ( _f_getvalue_4( fs_id.f_cluster_count ) != F_CLUSTER_COUNT )
        || ( fs_id.par != idpar )
       )
    {
      return F_ERR_DIFFMEDIA;
    }
  }
#endif /* if F_CHECKMEDIA */

  psp_memset( &files[0], 0, sizeof( files ) );
  psp_memset( &f_volume, 0, sizeof( f_volume ) );
  psp_memset( f_volume.file_id_page, -1, sizeof( f_volume.file_id_page ) );

#if F_DIRECTORIES
  f_volume.current_dir = F_DIR_ROOT;  /* change to root directory */
  for ( di = 0 ; di < F_MAX_DIR ; di++ )
  {
    unsigned char  tmp;
    rc = f_flash_mgm_read( &tmp, _DIR_DSC_ADDR( di ) + _SADDR( F_DIR_DSC, name[0] ), 1 );
    if ( rc )
    {
      return rc;
    }

    if ( tmp != (unsigned char)F_INVALID )
    {
      unsigned char  attr[F_ATTR_SIZE];
      rc = f_flash_mgm_read( attr, _DIR_DSC_ADDR( di ) + _SADDR( F_DIR_DSC, par ) + _SADDR( F_DIR_PAR, attr ), F_ATTR_SIZE );
      if ( rc )
      {
        return rc;
      }

      if ( ( F_GET_ATTR( attr ) & F_ATTR_DIR ) == 0 )
      {
        return F_ERR_NOTFORMATTED;
      }
    }
  }

#endif /* if F_DIRECTORIES */

  for ( fi = 0 ; fi < F_FILE_MGM_PAGE_COUNT ; fi++ )
  {
    rc = f_flash_mgm_read( &mgm_hdr, _FILE_MGM_HEADER_ADDR( fi ), sizeof( F_FILE_MGM_HEADER ) );
    if ( rc )
    {
      return rc;
    }

    fileid = F_GET_FILE_ID( mgm_hdr.fileid );
    if ( fileid != _F_INV_FILE_ID )
    {
      if ( ( mgm_hdr.inv_state + mgm_hdr.state == _F_INV_8 ) && ( mgm_hdr.inv_seq + mgm_hdr.seq == _F_INV_8 ) )
      {
        switch ( mgm_hdr.state )
        {
          case _FM_STATE_CLOSE:
          {
            F_FILE                    * f = &files[0];
            F_FILE_MGM_HEADER           mgm_hdr2;
            F_FILE_MGM_PAGE_COUNT_TYPE  prev_mgm_page = (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID;

            if ( _FILE_ID_MGM_PAGE( fileid ) != (F_FILE_MGM_PAGE_COUNT_TYPE)F_INVALID )
            {
              rc = f_flash_mgm_read( &mgm_hdr2, _FILE_ID_MGM_HEADER_ADDR( fileid ), sizeof( F_FILE_MGM_HEADER ) );
              if ( rc )
              {
                return rc;
              }

              if ( mgm_hdr2.seq + 1 == mgm_hdr.seq )                /* current mgm page is newer than previously read */
              {
                rc = _f_remove_file_mgm( _FILE_ID_MGM_PAGE( fileid ), 1 );
                if ( rc )
                {
                  return rc;
                }
              }
              else if ( mgm_hdr2.seq == mgm_hdr.seq )           /* current mgm page is identical to previous */
              {
                rc = f_flash_mgm_erase_safe( _FILE_MGM_ADDR( fi ), F_MGM_PAGE_SIZE );
                if ( rc )
                {
                  return rc;
                }

                break;
              }
              else                                                  /* current mgm page is older than the previous one */
              {
                rc = _f_remove_file_mgm( fi, 0 );
                if ( rc )
                {
                  return rc;
                }

                break;
              }
            }

#if SMALL_FILE_OPT

            /* check if file data is in the management page or in the cluster area */
            rc = f_flash_mgm_read( &par, _FILE_MGM_DSC_ADDR( fi ) + _SADDR( F_FILE_DSC, par ), sizeof( F_FILE_PAR ) );
            if ( rc )
            {
              return rc;
            }

            if ( F_GET_LENGTH( par.length ) <= F_FILE_MGM_PAGE0_SIZE )
            {
              _SET_BIT( f_volume.file_mgm_table, fi );                              /* mark management page as used */
            }
            else
#endif
            {
              f->page_cl_pos = 0;
              f->page_cnt = 0;
              f->first_mgm_page = fi;
              f->act_mgm_page = fi;
              for ( ; ; )
              {
                if ( prev_mgm_page != f->act_mgm_page )
                {
                  _SET_BIT( f_volume.file_mgm_table, f->act_mgm_page );
                  prev_mgm_page = f->act_mgm_page;                                  /* mark used management pages */
                }

                rc = _f_get_cluster_value( f );
                if ( rc )
                {
                  break;
                }

                if ( f->cluster >= F_CLUSTER_COUNT )
                {
                  return F_ERR_CORRUPTED;
                }

                _SET_BIT( f_volume.cluster_table, f->cluster );                     /* mark used clusters */
              }
            }

            _FILE_ID_MGM_PAGE( fileid ) = fi;                                       /* save 1st MGM page of a file ID  */
#if QUICK_FILE_SEARCH

            /* generate quick search key */
            _f_qs_genkey( f_volume.qskey + fileid, NULL, _FILE_MGM_DSC_ADDR( fi ) + _SADDR( F_FILE_DSC, name[0] ) );
#endif
          }
          break;

          case _FM_STATE_OPEN:  /* no need to do anything */
            break;

          case _FM_STATE_EXT:   /* no need to do anything */
            break;

          default:
            return F_ERR_CORRUPTED;
        } /* switch */
      }
    }
  }

#if QUICK_WILDCARD_SEARCH
  rc = _f_qws_check();
  if ( rc )
  {
    return rc;
  }

#endif

  f_volume.state = F_STATE_WORKING;

  return F_NOERR;
} /* fn_initvolume */


/*
** Format the device
*/
unsigned char fn_format ( void )
{
  unsigned char  rc;

  rc = f_flash_init();
  if ( rc )
  {
    return rc;
  }

  rc = f_flash_format();
  if ( rc )
  {
    return rc;
  }

  rc = f_flash_mgm_erase( F_BEGIN_ADDR, F_FS_ID_SIZE + F_DIR_SIZE + F_FILE_MGM_SIZE );
  if ( rc )
  {
    return rc;
  }

#if F_CHECKMEDIA
  {
    F_FS_ID  fs_id;
    _f_setvalue_2( fs_id.version, _TINY_FORMAT_VERSION );
    _f_setvalue_2( fs_id.f_cluster_size, F_CLUSTER_SIZE );
    _f_setvalue_2( fs_id.f_mgm_page_size, F_MGM_PAGE_SIZE );
 #if F_DIRECTORIES
    _f_setvalue_2( fs_id.f_dir_page_count, F_DIR_PAGE_COUNT );
 #else
    _f_setvalue_2( fs_id.f_dir_page_count, 0 );
 #endif
    _f_setvalue_4( fs_id.f_file_mgm_page_count, F_FILE_MGM_PAGE_COUNT );
    _f_setvalue_4( fs_id.f_cluster_count, F_CLUSTER_COUNT );
    fs_id.par = 0;
 #if SMALL_FILE_OPT
    fs_id.par |= 1;
 #endif
 #if QUICK_WILDCARD_SEARCH
    fs_id.par |= 4;
 #endif
    fs_id.par |= ( ( F_ATTR_SIZE - 1 ) << 3 );
    psp_memcpy( fs_id.tinystr, _TINYSTR, sizeof( _TINYSTR ) );
    rc = f_flash_mgm_write_safe( F_FS_ID_ADDR, &fs_id, sizeof( F_FS_ID ) );
    if ( rc )
    {
      return rc;
    }
  }
#endif /* if F_CHECKMEDIA */
  return f_initvolume();
} /* fn_format */


/*
** fn_get_serial
**
** Get serial number
**
** OUTPUT: serial - where to write the serial number
** RETURN: error code
*/
unsigned char fn_get_serial ( unsigned long * serial )
{
  return f_flash_mgm_read( serial, F_SER_ADDR, sizeof( unsigned long ) );
}


/*
** fn_set_serial
**
** Set serial number
**
** INPUT: serial - new serial number
** RETURN: error code
*/
unsigned char fn_set_serial ( unsigned long serial )
{
  return f_flash_mgm_write( F_SER_ADDR, &serial, sizeof( unsigned long ) );
}


/*
** fn_get_size
**
** Get total size of the flash the file system can use
**
** OUTPUT: size - where to write the size
** RETURN: error code
*/
unsigned char fn_get_size ( unsigned long * size )
{
  *size = F_SIZE;
  return 0;
}

