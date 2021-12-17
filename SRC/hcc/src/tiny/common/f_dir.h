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
#ifndef _F_DIR_H
#define _F_DIR_H

#include "../../api/api_tiny.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if QUICK_FILE_SEARCH
unsigned char _f_qs_genkey ( F_QSKEY_TYPE * key, const _PTRQ char * name, unsigned long addr );
unsigned char _f_qs_check ( F_QSKEY_TYPE * key, F_FILE_ID_TYPE id );
#endif

F_FILE_ID_TYPE _f_add_entry ( const _PTRQ char * name, F_FILE_ID_TYPE fileid, F_FILE * file );
#if F_DIRECTORIES
F_FIND_ID_TYPE _f_find ( const _PTRQ char * filename, F_FILE_PAR * par, F_DIR_ID_TYPE odirid );
#else
F_FIND_ID_TYPE _f_find ( const _PTRQ char * filename, F_FILE_PAR * par );
#endif
#if F_DIRECTORIES
F_DIR_ID_TYPE _f_check_path ( const _PTRQ char * name, _PTRQ char * * filename );
#endif

int _f_createfullname ( char * dst, const _PTRQ char * src );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _F_DIR_H */

