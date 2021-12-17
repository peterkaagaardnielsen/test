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
#ifndef _TINY_H_
#define _TINY_H_

#include "tiny_types.h"
#include "../driver/f_driver.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif



#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************************************
**************************************** define default values *****************************************
*******************************************************************************************************/
#if F_DIRECTORIES == 0
 #define F_MAX_DIR          0
#endif

#define F_MAX_SIZE          ( F_PAGE_SIZE * F_PAGE_COUNT )
#define F_CLUSTER_SIZE      ( F_PAGE_PER_CLUSTER * F_PAGE_SIZE )
#define F_MAX_CLUSTER_COUNT ( F_PAGE_COUNT / F_PAGE_PER_CLUSTER )   /* number of maximum clusters */


/*******************************************************************************************************
********************************** define file system variable types ***********************************
*******************************************************************************************************/

/* set FILE_ID type depending on the maximum number of file allowed in the system */
#if ( F_MAX_FILE > ( _FM_16 - 1 ) )
 #define F_FILE_ID_TYPE      _U32_T
 #define F_FILE_ID_TYPE_SIZE 4
 #define _F_INV_FILE_ID      _F_INV_32
 #define F_GET_FILE_ID( p )    _f_getvalue_4( p )
 #define F_SET_FILE_ID( p, v ) _f_setvalue_4( p, v )
#elif ( F_MAX_FILE > ( _FM_8 - 1 ) )
 #define F_FILE_ID_TYPE      _U16_T
 #define F_FILE_ID_TYPE_SIZE 2
 #define _F_INV_FILE_ID      _F_INV_16
 #define F_GET_FILE_ID( p )    _f_getvalue_2( p )
 #define F_SET_FILE_ID( p, v ) _f_setvalue_2( p, v )
#else
 #define F_FILE_ID_TYPE      _U8_T
 #define F_FILE_ID_TYPE_SIZE 1
 #define _F_INV_FILE_ID      _F_INV_8
 #define F_GET_FILE_ID( p )    _f_getvalue_1( p )
 #define F_SET_FILE_ID( p, v ) _f_setvalue_1( p, v )
#endif /* if ( F_MAX_FILE > ( _FM_16 - 1 ) ) */

/* set LENGTH type depending on the size of the flash */
#if ( F_MAX_SIZE > ( _FM_16 - 1 ) )
 #define F_LENGTH_TYPE      _U32_T
 #define F_LENGTH_TYPE_SIZE 4
 #define _F_INV_LENGTH      _F_INV_32
 #define F_GET_LENGTH( p )    _f_getvalue_4( p )
 #define F_SET_LENGTH( p, v ) _f_setvalue_4( p, v )
#elif ( F_MAX_SIZE > ( _FM_8 - 1 ) )
 #define F_LENGTH_TYPE      _U16_T
 #define F_LENGTH_TYPE_SIZE 2
 #define _F_INV_LENGTH      _F_INV_16
 #define F_GET_LENGTH( p )    _f_getvalue_2( p )
 #define F_SET_LENGTH( p, v ) _f_setvalue_2( p, v )
#else
 #define F_LENGTH_TYPE      _U8_T
 #define F_LENGTH_TYPE_SIZE 1
 #define _F_INV_LENGTH      _F_INV_8
 #define F_GET_LENGTH( p )    _f_getvalue_1( p )
 #define F_SET_LENGTH( p, v ) _f_setvalue_1( p, v )
#endif /* if ( F_MAX_SIZE > ( _FM_16 - 1 ) ) */

/* set DIR_ID type depending on the maximum number of directories allowed in the system */
#if F_DIRECTORIES
 #if ( F_MAX_DIR > ( _FM_16 - 2 ) )
  #define F_DIR_ID_TYPE      _U32_T
  #define F_DIR_ID_TYPE_SIZE 4
  #define _F_INV_DIR_ID      _F_INV_32
  #define F_GET_DIR_ID( p )    _f_getvalue_4( p )
  #define F_SET_DIR_ID( p, v ) _f_setvalue_4( p, v )
 #elif ( F_MAX_DIR > ( _FM_8 - 2 ) )
  #define F_DIR_ID_TYPE      _U16_T
  #define F_DIR_ID_TYPE_SIZE 2
  #define _F_INV_DIR_ID      _F_INV_16
  #define F_GET_DIR_ID( p )    _f_getvalue_2( p )
  #define F_SET_DIR_ID( p, v ) _f_setvalue_2( p, v )
 #else
  #define F_DIR_ID_TYPE      _U8_T
  #define F_DIR_ID_TYPE_SIZE 1
  #define _F_INV_DIR_ID      _F_INV_8
  #define F_GET_DIR_ID( p )    _f_getvalue_1( p )
  #define F_SET_DIR_ID( p, v ) _f_setvalue_1( p, v )
 #endif /* if ( F_MAX_DIR > ( _FM_16 - 2 ) ) */

 #define F_DIR_ROOT ( _F_INV_DIR_ID - 1 )           /* value to indicate root directory */
#endif /* if F_DIRECTORIES */

/* set FIND_ID type for finding routines, depends on maximum files+maximum directories */
#if F_DIRECTORIES
 #if ( ( F_MAX_DIR + F_MAX_FILE ) > ( _FM_16 - 2 ) )
  #define F_FIND_ID_TYPE _U32_T
 #elif ( ( F_MAX_DIR + F_MAX_FILE ) > ( _FM_8 - 2 ) )
  #define F_FIND_ID_TYPE _U16_T
 #else
  #define F_FIND_ID_TYPE _U8_T
 #endif
#else
 #define F_FIND_ID_TYPE  F_FILE_ID_TYPE
#endif

/* set type to address an element inside a cluster */
#if ( F_CLUSTER_SIZE > ( _FM_16 - 1 ) )
 #define F_CLUSTER_TYPE _U32_T
#elif ( F_CLUSTER_SIZE > ( _FM_8 - 1 ) )
 #define F_CLUSTER_TYPE _U16_T
#else
 #define F_CLUSTER_TYPE _U8_T
#endif

/* set type to count the total amount of clusters */
#if ( F_MAX_CLUSTER_COUNT > ( _FM_16 - 1 ) )
 #define F_CLUSTER_COUNT_TYPE      _U32_T
 #define _F_INV_CLUSTER_COUNT      _F_INV_32
 #define F_CLUSTER_COUNT_TYPE_SIZE 4
 #define F_GET_CLUSTER_COUNT( p )    _f_getvalue_4( p )
 #define F_SET_CLUSTER_COUNT( p, v ) _f_setvalue_4( p, v )
#elif ( F_MAX_CLUSTER_COUNT > ( _FM_8 - 1 ) )
 #define F_CLUSTER_COUNT_TYPE      _U16_T
 #define _F_INV_CLUSTER_COUNT      _F_INV_16
 #define F_CLUSTER_COUNT_TYPE_SIZE 2
 #define F_GET_CLUSTER_COUNT( p )    _f_getvalue_2( p )
 #define F_SET_CLUSTER_COUNT( p, v ) _f_setvalue_2( p, v )
#else
 #define F_CLUSTER_COUNT_TYPE      _U8_T
 #define _F_INV_CLUSTER_COUNT      _F_INV_8
 #define F_CLUSTER_COUNT_TYPE_SIZE 1
 #define F_GET_CLUSTER_COUNT( p )    _f_getvalue_1( p )
 #define F_SET_CLUSTER_COUNT( p, v ) _f_setvalue_1( p, v )
#endif /* if ( F_MAX_CLUSTER_COUNT > ( _FM_16 - 1 ) ) */

/* set F_ATTR type depending on the attribute size */
#if ( F_ATTR_SIZE != 1 && F_ATTR_SIZE != 2 && F_ATTR_SIZE != 4 )
 #error F_ATTR_SIZE is set to an invalid value!
#endif
#define F_MAX_ATTR   _MAXVAL( F_ATTR_SIZE << 3 )
#if ( F_MAX_ATTR > _FM_16 )
 #define F_ATTR_TYPE _U32_T
 #define F_GET_ATTR( p )    _f_getvalue_4( p )
 #define F_SET_ATTR( p, v ) _f_setvalue_4( p, v )
#elif ( F_MAX_ATTR > _FM_8 )
 #define F_ATTR_TYPE _U16_T
 #define F_GET_ATTR( p )    _f_getvalue_2( p )
 #define F_SET_ATTR( p, v ) _f_setvalue_2( p, v )
#else
 #define F_ATTR_TYPE _U8_T
 #define F_GET_ATTR( p )    _f_getvalue_1( p )
 #define F_SET_ATTR( p, v ) _f_setvalue_1( p, v )
#endif

#define _F_COPY_BUF_CLUSTER_COUNT ( F_COPY_BUF_SIZE / F_CLUSTER_COUNT_TYPE_SIZE )
#define _F_COPY_BUF_SIZE          ( _F_COPY_BUF_CLUSTER_COUNT * F_CLUSTER_COUNT_TYPE_SIZE )


/*******************************************************************************************************
******* define file and directory structures and calculate the amount of management pages we need ******
*******************************************************************************************************/


/*
** File parameter block
*/
typedef struct
{
  unsigned char  attr[F_ATTR_SIZE];          /* attribute */
  unsigned char  ctime[2];                   /* creation time */
  unsigned char  cdate[2];                   /* creation date */
  unsigned char  length[F_LENGTH_TYPE_SIZE]; /* file length (F_LENGTH_TYPE) */
} F_FILE_PAR;
#define _F_FILE_PAR_SIZE ( F_ATTR_SIZE + 2 + 2 + F_LENGTH_TYPE_SIZE )


/*
** file descriptor holding the name, the parameter block and optionaly the directory ID
*/
typedef struct
{
  char           name[F_MAX_FILE_NAME_LENGTH + 1]; /* file name */
#if F_DIRECTORIES
  unsigned char  dirid[F_DIR_ID_TYPE_SIZE]; /* current directory position (F_DIR_ID_TYPE) */
#endif
  F_FILE_PAR     par;                       /* parameters */
} F_FILE_DSC;

/* calculate size the system will need to store F_FILE_DSC */
#if F_DIRECTORIES
 #define _F_FILE_DSC_SIZE ( ( F_MAX_FILE_NAME_LENGTH + 1 ) + F_DIR_ID_TYPE_SIZE + _F_FILE_PAR_SIZE )
#else
 #define _F_FILE_DSC_SIZE ( ( F_MAX_FILE_NAME_LENGTH + 1 ) + _F_FILE_PAR_SIZE )
#endif


/*
** Directory parameter block
*/
#if F_DIRECTORIES
typedef struct
{
  unsigned char  attr[F_ATTR_SIZE]; /* attribute */
  unsigned char  ctime[2];          /* creation time */
  unsigned char  cdate[2];          /* creation date */
} F_DIR_PAR;
 #define _F_DIR_PAR_SIZE ( F_ATTR_SIZE + 2 + 2 )


/*
** Directory descriptor holding the name, the parameter block and dirid
*/
typedef struct
{
  char           name[F_MAX_FILE_NAME_LENGTH + 1]; /* dir name */
  unsigned char  dirid[F_DIR_ID_TYPE_SIZE];        /* parent directory */
  F_DIR_PAR      par;                              /* parameters */
} F_DIR_DSC;

/* calculate size the system will need to store F_DIR_DSC */
 #define _F_DIR_DSC_SIZE ( ( F_MAX_FILE_NAME_LENGTH + 1 ) + F_DIR_ID_TYPE_SIZE + _F_DIR_PAR_SIZE )
#endif /* if F_DIRECTORIES */


/* calculate the maximum size the system will need to store F_FILE_MGM_HEADER */
#define F_FILE_MGM_HEADER_MAX_SIZE 12

#define F_MGM_PAGE_SIZE            F_PAGE_SIZE /* management page size = flash page size */

/* give an error if management page is too small to hold file descriptor + header */
#if ( _F_FILE_DSC_SIZE + F_FILE_MGM_HEADER_MAX_SIZE ) > F_MGM_PAGE_SIZE
 #error Unsupported configuration, try decreasing F_MAX_FILE_NAME_LENGTH.
#endif

/* calculate number of pages the directory management will need */


/* no bad configuration checking is required because if F_MGM_PAGE_SIZE can hold FILE_DSC then
   F_MAX_DIR_ENTRY_PER_PAGE will be always >=1 */
#if F_DIRECTORIES
 #if USE_ECC
  #define F_MAX_DIR_ENTRY_PER_PAGE ( ( F_MGM_PAGE_SIZE - T_ECC_SIZE ) / _F_DIR_DSC_SIZE )
 #else
  #define F_MAX_DIR_ENTRY_PER_PAGE ( F_MGM_PAGE_SIZE / _F_DIR_DSC_SIZE )
 #endif

 #if F_DIR_OPTIMIZE
  #if F_MAX_DIR_ENTRY_PER_PAGE > 255
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 8
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 127
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 7
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 63
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 6
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 31
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 5
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 15
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 4
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 7
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 3
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 3
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 2
  #elif F_MAX_DIR_ENTRY_PER_PAGE > 1
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 1
  #else
   #define F_DIR_ENTRY_PER_PAGE_SHIFT 0
  #endif /* if F_MAX_DIR_ENTRY_PER_PAGE > 255 */
  #define F_DIR_ENTRY_PER_PAGE        ( 1ul << F_DIR_ENTRY_PER_PAGE_SHIFT )
 #else /* if F_DIR_OPTIMIZE */
  #define F_DIR_ENTRY_PER_PAGE        F_MAX_DIR_ENTRY_PER_PAGE
 #endif /* if F_DIR_OPTIMIZE */

 #define F_DIR_PAGE_COUNT             ( ( F_MAX_DIR + ( F_DIR_ENTRY_PER_PAGE - 1 ) ) / F_DIR_ENTRY_PER_PAGE )
#endif /* if F_DIRECTORIES */


/* file management area calculations */
#if USE_ECC
 #define F_FILE_MGM_PAGE_SIZE        ( F_MGM_PAGE_SIZE - F_FILE_MGM_HEADER_MAX_SIZE - T_ECC_SIZE )
#else
 #define F_FILE_MGM_PAGE_SIZE        ( F_MGM_PAGE_SIZE - F_FILE_MGM_HEADER_MAX_SIZE )
#endif
#define F_FILE_MGM_PAGE0_SIZE        ( F_FILE_MGM_PAGE_SIZE - _F_FILE_DSC_SIZE )
#define F_FILE_MGM_CLUSTER_PER_PAGE  ( F_FILE_MGM_PAGE_SIZE / F_CLUSTER_COUNT_TYPE_SIZE )
#define F_FILE_MGM_CLUSTER_PER_PAGE0 ( F_FILE_MGM_PAGE0_SIZE / F_CLUSTER_COUNT_TYPE_SIZE )

/* calculate number of extra pages needed to represent a file on cluster basis (1+n) */
#if ( F_MAX_CLUSTER_COUNT > F_FILE_MGM_CLUSTER_PER_PAGE0 )

 #define F_FILE_MGM_EXTRA_PAGE_COUNT_CL \
  ( ( ( ( F_MAX_CLUSTER_COUNT \
         - F_FILE_MGM_CLUSTER_PER_PAGE0 ) + ( F_FILE_MGM_CLUSTER_PER_PAGE - 1 ) ) / F_FILE_MGM_CLUSTER_PER_PAGE ) )

/* calculate number of extra pages needed if all files has base page number of clusters+1 in the chain */
 #define F_FILE_MGM_EXTRA_PAGE_COUNT_1 ( F_MAX_CLUSTER_COUNT / ( F_FILE_MGM_CLUSTER_PER_PAGE0 + 1 ) )

/* define the number of maximum extra pages needed */
 #if ( F_FILE_MGM_EXTRA_PAGE_COUNT_1 > F_FILE_MGM_EXTRA_PAGE_COUNT_CL )
  #define F_FILE_MGM_EXTRA_PAGE_COUNT  F_FILE_MGM_EXTRA_PAGE_COUNT_1
 #else
  #define F_FILE_MGM_EXTRA_PAGE_COUNT  F_FILE_MGM_EXTRA_PAGE_COUNT_CL
 #endif

#else

 #define F_FILE_MGM_EXTRA_PAGE_COUNT 0

#endif /* if ( F_MAX_CLUSTER_COUNT > F_FILE_MGM_CLUSTER_PER_PAGE0 ) */

/* calculate number of pages for the file management area */
#define F_FILE_MGM_PAGE_COUNT            ( F_MAX_FILE + F_MAX_OPEN_FILE + 2 * ( F_FILE_MGM_EXTRA_PAGE_COUNT ) )

/* set type to count through the file management pages */
#if ( F_FILE_MGM_PAGE_COUNT > ( _FM_16 - 1 ) )
 #define F_FILE_MGM_PAGE_COUNT_TYPE      _U32_T
 #define F_FILE_MGM_PAGE_COUNT_TYPE_SIZE 4
 #define _F_INV_FILE_MGM_PAGE_COUNT      _F_INV_32
 #define F_GET_FILE_MGM_PAGE_COUNT( p )    _f_getvalue_4( p )
 #define F_SET_FILE_MGM_PAGE_COUNT( p, v ) _f_setvalue_4( p, v )
#elif ( F_FILE_MGM_PAGE_COUNT > ( _FM_8 - 1 ) )
 #define F_FILE_MGM_PAGE_COUNT_TYPE      _U16_T
 #define F_FILE_MGM_PAGE_COUNT_TYPE_SIZE 2
 #define _F_INV_FILE_MGM_PAGE_COUNT      _F_INV_16
 #define F_GET_FILE_MGM_PAGE_COUNT( p )    _f_getvalue_2( p )
 #define F_SET_FILE_MGM_PAGE_COUNT( p, v ) _f_setvalue_2( p, v )
#else
 #define F_FILE_MGM_PAGE_COUNT_TYPE      _U8_T
 #define F_FILE_MGM_PAGE_COUNT_TYPE_SIZE 1
 #define _F_INV_FILE_MGM_PAGE_COUNT      _F_INV_8
 #define F_GET_FILE_MGM_PAGE_COUNT( p )    _f_getvalue_1( p )
 #define F_SET_FILE_MGM_PAGE_COUNT( p, v ) _f_setvalue_1( p, v )
#endif /* if ( F_FILE_MGM_PAGE_COUNT > ( _FM_16 - 1 ) ) */

/* set type to count clusters in one file management page */
#if ( F_FILE_MGM_CLUSTER_PER_PAGE > ( _FM_16 - 1 ) )
 #define F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE _U32_T
#elif ( F_FILE_MGM_CLUSTER_PER_PAGE > ( _FM_8 - 1 ) )
 #define F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE _U16_T
#else
 #define F_FILE_MGM_CLUSTER_PER_PAGE_COUNT_TYPE _U8_T
#endif

/* calculate number of pages quick wildcard search requires */
/* stores all filenames in pages (1.pg0p0 / 2.pg1p0 / ... ) to avoid read delays */
/* An entry consists of the optional directory ID and the file name */
#if F_WILDCARD && QUICK_WILDCARD_SEARCH
typedef struct
{
  char           name[F_MAX_FILE_NAME_LENGTH + 1]; /* file name */
 #if F_DIRECTORIES
  unsigned char  dirid[F_DIR_ID_TYPE_SIZE];     /* directory ID (F_DIR_ID_TYPE) */
 #endif
} F_QWS_ENTRY;


 #if F_DIRECTORIES
  #define _F_QWS_ENTRY_SIZE         ( F_MAX_FILE_NAME_LENGTH + 1 + F_DIR_ID_TYPE_SIZE )
 #else
  #define _F_QWS_ENTRY_SIZE         ( F_MAX_FILE_NAME_LENGTH + 1 )
 #endif
 #define F_QWS_ENTRY_PER_PAGE       ( F_MGM_PAGE_SIZE / _F_QWS_ENTRY_SIZE )
 #define F_QWS_PAGE_COUNT           ( ( F_MAX_FILE + F_QWS_ENTRY_PER_PAGE - 1 ) / F_QWS_ENTRY_PER_PAGE )

/* set type to count through the qws filenames in a page */
 #if ( F_QWS_ENTRY_PER_PAGE > ( _FM_8 - 1 ) )
  #define F_QWS_ENTRY_PER_PAGE_TYPE _U16_T
 #else
  #define F_QWS_ENTRY_PER_PAGE_TYPE _U8_T
 #endif

/* set type to count through the qws management pages */
 #if ( F_QWS_PAGE_COUNT > ( _FM_16 - 1 ) )
  #define F_QWS_PAGE_COUNT_TYPE _U32_T
 #elif ( F_QWS_PAGE_COUNT > ( _FM_8 - 1 ) )
  #define F_QWS_PAGE_COUNT_TYPE _U16_T
 #else
  #define F_QWS_PAGE_COUNT_TYPE _U8_T
 #endif
#else /* if F_WILDCARD && QUICK_WILDCARD_SEARCH */
 #define F_QWS_PAGE_COUNT       0
#endif /* if F_WILDCARD && QUICK_WILDCARD_SEARCH */

/* calculate the number of data clusters in the system (-1 is the ID page)*/
#if F_DIRECTORIES

/* give an error if not enough pages are present for the required configuration min. 1 cluster is required */
 #if F_PAGE_COUNT <= ( ( 1 + F_DIR_PAGE_COUNT + F_FILE_MGM_PAGE_COUNT + F_QWS_PAGE_COUNT ) + ( F_PAGE_PER_CLUSTER - 1 ) )
  #error Unsupported configuration, try decreasing F_MAX_FILE and/or F_MAX_DIR.
 #endif
 #define F_CLUSTER_COUNT ( ( F_PAGE_COUNT - 1 - F_DIR_PAGE_COUNT - F_FILE_MGM_PAGE_COUNT - F_QWS_PAGE_COUNT ) / F_PAGE_PER_CLUSTER )
#else

/* give an error if not enough pages are present for the required configuration min. 1 cluster is required */
 #if F_PAGE_COUNT <= ( ( 1 + F_FILE_MGM_PAGE_COUNT + F_QWS_PAGE_COUNT ) + ( F_PAGE_PER_CLUSTER - 1 ) )
  #error Unsupported configuration, try decreasing F_MAX_FILE.
 #endif
 #define F_CLUSTER_COUNT     ( ( F_PAGE_COUNT - 1 - F_FILE_MGM_PAGE_COUNT - F_QWS_PAGE_COUNT ) / F_PAGE_PER_CLUSTER )
#endif
#define F_SIZE               ( ( (unsigned long)F_CLUSTER_COUNT * F_CLUSTER_SIZE ) )


/*******************************************************************************************************
********** define the begin address of FSID, directory, file management and data sections **************
*******************************************************************************************************/
#define F_FS_ID_ADDR         ( F_BEGIN_ADDR )
#define F_FS_ID_SIZE         ( F_MGM_PAGE_SIZE )

#if F_DIRECTORIES
 #define F_DIR_ADDR          ( F_FS_ID_ADDR + F_FS_ID_SIZE )
 #define F_DIR_SIZE          ( F_DIR_PAGE_COUNT * F_MGM_PAGE_SIZE )
#else
 #define F_DIR_ADDR          ( F_FS_ID_ADDR + F_FS_ID_SIZE )
 #define F_DIR_SIZE          ( 0 )
#endif

#define F_FILE_MGM_ADDR      ( F_DIR_ADDR + F_DIR_SIZE )
#define F_FILE_MGM_SIZE      ( F_FILE_MGM_PAGE_COUNT * F_MGM_PAGE_SIZE )

#define F_QWS_MGM_ADDR       ( F_FILE_MGM_ADDR + F_FILE_MGM_SIZE )
#define F_QWS_MGM_SIZE       ( F_QWS_PAGE_COUNT * F_MGM_PAGE_SIZE )

#define F_FIRST_CLUSTER_ADDR ( ( ( ( F_QWS_MGM_ADDR + F_QWS_MGM_SIZE ) + ( F_CLUSTER_SIZE - 1 ) ) / F_CLUSTER_SIZE ) * F_CLUSTER_SIZE )


/*******************************************************************************************************
*********************** defines the address to access different sections *******************************
*******************************************************************************************************/
#define _SADDR( s, e ) ( (unsigned long)( &( ( (s *)0 )->e ) ) )
#define _SSIZE( s, e ) sizeof( ( (s *)0 )->e )

/* address of a directory descriptor */
#if F_DIRECTORIES
 #if F_DIR_OPTIMIZE
  #define _DIR_DSC_ADDR( x ) \
  ( F_DIR_ADDR \
   + ( (unsigned long)( x ) \
      >> F_DIR_ENTRY_PER_PAGE_SHIFT ) * F_MGM_PAGE_SIZE \
   + ( ( (unsigned long)( x ) & ( F_DIR_ENTRY_PER_PAGE - 1 ) ) * _F_DIR_DSC_SIZE ) )
 #else
  #define _DIR_DSC_ADDR( x ) \
  ( F_DIR_ADDR \
   + ( (unsigned long)( x ) \
      / F_DIR_ENTRY_PER_PAGE ) * F_MGM_PAGE_SIZE + ( ( (unsigned long)( x ) % F_DIR_ENTRY_PER_PAGE ) * _F_DIR_DSC_SIZE ) )
 #endif
#endif

/* address of a file management page */
/* mgm page: | cluster addresses | file dsc | header | */
#define _FILE_ID_MGM_PAGE( id )                ( f_volume.file_id_page[( id )] )
#define _FILE_MGM_ADDR( page )                 ( F_FILE_MGM_ADDR + (unsigned long)( page ) * F_MGM_PAGE_SIZE )
#if USE_ECC
 #define _FILE_MGM_HEADER_ADDR( page )         ( _FILE_MGM_ADDR( ( page ) + 1 ) - F_FILE_MGM_HEADER_MAX_SIZE - T_ECC_SIZE )
#else
 #define _FILE_MGM_HEADER_ADDR( page )         ( _FILE_MGM_ADDR( ( page ) + 1 ) - F_FILE_MGM_HEADER_MAX_SIZE )
#endif
#define _FILE_ID_MGM_HEADER_ADDR( fileid )     ( _FILE_MGM_HEADER_ADDR( _FILE_ID_MGM_PAGE( fileid ) ) )
#define _FILE_MGM_PAGE_CLUSTER_ADDR( page, x ) ( _FILE_MGM_ADDR( page ) + (unsigned long)( x ) * F_CLUSTER_COUNT_TYPE_SIZE )
#if USE_ECC
 #define _FILE_MGM_DSC_ADDR( page )            ( _FILE_MGM_ADDR( ( page ) + 1 ) - _F_FILE_DSC_SIZE - F_FILE_MGM_HEADER_MAX_SIZE - T_ECC_SIZE )
#else
 #define _FILE_MGM_DSC_ADDR( page )            ( _FILE_MGM_ADDR( ( page ) + 1 ) - _F_FILE_DSC_SIZE - F_FILE_MGM_HEADER_MAX_SIZE )
#endif
#define _FILE_ID_MGM_DSC_ADDR( fileid )        ( _FILE_MGM_DSC_ADDR( _FILE_ID_MGM_PAGE( fileid ) ) )

/* address of cluster */
#define _CLUSTER_ADDR( x )                     ( F_FIRST_CLUSTER_ADDR + (unsigned long)x * F_CLUSTER_SIZE )

/* calculations for quick wildcard search */
#define _QWS_ENTRY_ADDR( pg, pgpos )           ( ( F_QWS_MGM_ADDR + ( ( pg ) * F_MGM_PAGE_SIZE ) ) + ( ( pgpos ) * _F_QWS_ENTRY_SIZE ) )
#define _QWS_EPAGE( n )                        ( ( n ) % F_QWS_PAGE_COUNT )
#define _QWS_EPOS( n )                         ( ( n ) / F_QWS_PAGE_COUNT )
#define _QWS_ENTRY_POS( pg, pgpos )            ( ( ( pgpos ) * F_QWS_PAGE_COUNT ) + ( pg ) )

/* bit operations */
#define _SET_BIT( array, pos )                 ( array[( pos ) / _SUC] |= ( 1U << ( ( pos ) & ( _SUC - 1 ) ) ) )
#define _CLEAR_BIT( array, pos )               ( array[( pos ) / _SUC] &= ~( 1U << ( ( pos ) & ( _SUC - 1 ) ) ) )
#define _GET_BIT( array, pos )                 ( array[( pos ) / _SUC] & ( 1U << ( ( pos ) & ( _SUC - 1 ) ) ) )


/****************************************************************************
 *  File open modes
 ***************************************************************************/
#define F_MODE_CLOSE    0x00
#define F_MODE_READ     0x01
#define F_MODE_WRITE    0x02
#define F_MODE_APPEND   0x04
#define F_MODE_PLUS     0x08

#define F_MODE_CHANGED  0x80


/****************************************************************************
 * define file management page header
 ***************************************************************************/
#define _FM_STATE_CLOSE 0x01    /* management page holding a closed file */
#define _FM_STATE_EXT   0x02    /* extension page */
#define _FM_STATE_OPEN  0xfe    /* opened file */


/*
** header of a file management page
*/
typedef struct
{
  unsigned char  next[F_FILE_MGM_PAGE_COUNT_TYPE_SIZE]; /* next page (F_FILE_MGM_PAGE_COUNT_TYPE) */
  unsigned char  fileid[F_FILE_ID_TYPE_SIZE];           /* fileid (F_FILE_ID_TYPE) */
  unsigned char  seq;                                   /* Sequence number */
  unsigned char  state;                                 /* Possible states: _FM_STATE_* */
  unsigned char  inv_seq;                               /* inverted sequence number */
  unsigned char  inv_state;                             /* inverted state */
} F_FILE_MGM_HEADER;


/****************************************************************************
 * quick file search
 ***************************************************************************/
#if QUICK_FILE_SEARCH
 #define F_QSKEY_TYPE unsigned char
#endif


/****************************************************************************
 * file changed size
 ***************************************************************************/
#if F_FILE_CHANGED_EVENT
 #if F_DIRECTORIES
  #define _F_FILE_CHANGED_MAXPATH ( F_FILE_CHANGED_MAXPATH + 1 ) /* 0 terminator */
 #else
  #define _F_FILE_CHANGED_MAXPATH ( F_MAX_FILE_NAME_LENGTH + 2 ) /* 0 terminator + '/' */
 #endif
#endif


/****************************************************************************
 * define volume
 ***************************************************************************/

/* states of the volume */
#define F_STATE_NOTFORMATTED 0      /* volume state not formatted */
#define F_STATE_WORKING      1      /* volume state working */

/* set bit table sizes */
#define _CLUSTER_TABLE_SIZE  ( ( F_CLUSTER_COUNT + _SUC - 1 ) / _SUC )
#define _FILE_MGM_TABLE_SIZE ( ( F_FILE_MGM_PAGE_COUNT + _SUC - 1 ) / _SUC )


/*
** structure holding file system information
*/
typedef struct
{
  unsigned char               file_mgm_table[_FILE_MGM_TABLE_SIZE]; /* file management page table */
  unsigned char               cluster_table[_CLUSTER_TABLE_SIZE];   /* free cluster table */
  F_FILE_MGM_PAGE_COUNT_TYPE  file_id_page[F_MAX_FILE];             /* start management page of a file */
#if QUICK_FILE_SEARCH
  F_QSKEY_TYPE                qskey[F_MAX_FILE];        /* quick search key array */
#endif

  F_CLUSTER_COUNT_TYPE        cluster_empty_pos;        /* empty cluster position */
  F_FILE_MGM_PAGE_COUNT_TYPE  file_mgm_empty_pos;       /* empty management position */

#if F_DIRECTORIES
  F_DIR_ID_TYPE  dir_empty_pos;                         /* current directory */
  F_DIR_ID_TYPE  current_dir;                           /* current directory */
#endif

  unsigned char  state;                                 /* state of the volume */
} F_VOLUME;


#if F_CHECKMEDIA


/*
** Identification structure
** par:
**   D0 - small file opt
**   D2 - quick wildcard search
**   D4:3 - max attr size (F_ATTR_SIZE-1)
*/
typedef struct
{
  unsigned char  version[2];
  unsigned char  f_cluster_size[2];
  unsigned char  f_mgm_page_size[2];
  unsigned char  f_dir_page_count[2];
  unsigned char  f_file_mgm_page_count[4];
  unsigned char  f_cluster_count[4];
  unsigned char  par;
  char           tinystr[6];
} F_FS_ID;
 #define _TINYSTR  "_TINY"
#endif

#define F_SER_ADDR ( F_FS_ID_ADDR + 64u )


/*
** define relative directory strings
*/
#define _DIR_ACT   "."      /* points to actual directory */
#define _DIR_TOP   ".."     /* points to parent directory */


/*
** File located in MGM page
*/
#define F_ATTR_MGM 0x80         /* file located in MGM page */


/*
** Define flash access function prototypes
*/
#if USE_ECC
 #define f_flash_mgm_read( dst, src, len )      f_flash_read( dst, src, len, 1 )
 #define f_flash_mgm_write( dst, src, len )     f_flash_write( dst, src, len, 1 )
 #define f_flash_data_read( dst, src, len )     f_flash_read( dst, src, len, 0 )
 #define f_flash_data_write( dst, src, len )    f_flash_write( dst, src, len, 0 )
#else
 #define f_flash_mgm_read( dst, src, len )      f_flash_read( dst, src, len )
 #define f_flash_mgm_write( dst, src, len )     f_flash_write( dst, src, len )
 #define f_flash_data_read( dst, src, len )     f_flash_read( dst, src, len )
 #define f_flash_data_write( dst, src, len )    f_flash_write( dst, src, len )
#endif
#define f_flash_mgm_write_safe( dst, src, len ) f_flash_write_safe( dst, src, len )
#define f_flash_mgm_copy( dst, src, len )       f_flash_copy( dst, src, len )
#define f_flash_mgm_erase( addr, len )          f_flash_erase( addr, len )
#define f_flash_mgm_erase_safe( addr, len )     f_flash_erase_safe( addr, len )
#define f_flash_data_copy( dst, src, len )      f_flash_copy( dst, src, len )


/*
** Tiny format version number
** This version number is increased when the format of the system
** changes and becomes incompatible with an erlier format.
*/
#define _TINY_FORMAT_VERSION 0x0202


#ifdef __cplusplus
}
#endif

#endif /* ifndef _TINY_H_ */

