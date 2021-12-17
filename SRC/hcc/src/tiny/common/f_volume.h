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
#ifndef _F_VOLUME_H
#define _F_VOLUME_H

#include "../../api/api_tiny.h"
#include "../../config/config_tiny.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern F_VOLUME  f_volume;               /* volume structure */
extern F_FILE    files[F_MAX_OPEN_FILE]; /* file structure */


#if F_DIRECTORIES
F_DIR_ID_TYPE _f_get_dir_entry ( void );
#endif
F_CLUSTER_COUNT_TYPE _f_get_empty_cluster ( void );
F_FILE_MGM_PAGE_COUNT_TYPE _f_get_file_mgm_empty_page ( F_FILE_ID_TYPE fileid, unsigned char state, unsigned char seq );
unsigned char _f_get_cluster_value ( F_FILE * f );
unsigned char _f_set_cluster_value ( F_FILE * f );
F_CLUSTER_COUNT_TYPE _f_get_orig_cluster_value ( F_FILE * f );
unsigned char _f_remove_file_mgm ( F_FILE_MGM_PAGE_COUNT_TYPE first, char mode );
F_FILE_MGM_PAGE_COUNT_TYPE _f_copy_mgm ( F_FILE_MGM_PAGE_COUNT_TYPE oact );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _F_VOLUME_H */





