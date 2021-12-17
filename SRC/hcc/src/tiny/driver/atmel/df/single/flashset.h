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
#ifndef __FLASHSET_H
#define __FLASHSET_H

#include "f_atmel.h"

#include "../../../../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3
 #error Incompatible TINY version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define F_BEGIN_ADDR       ( (unsigned long)ADF_PAGE_SIZE * ADF_MGM_END )
#define F_PAGE_SIZE        ADF_PAGE_SIZE
#define F_PAGE_COUNT       ADF_PAGE_COUNT
#define F_PAGE_PER_CLUSTER 8


#define f_flash_init       adf_init
#define f_flash_read       adf_read
#define f_flash_write      adf_write
#define f_flash_write_safe adf_write_safe
#define f_flash_erase      adf_erase
#define f_flash_erase_safe adf_erase_safe
#define f_flash_copy       adf_copy
#define f_flash_format     adf_low_level_format


#ifdef __cplusplus
}
#endif

#endif /* ifndef __FLASHSET_H */

