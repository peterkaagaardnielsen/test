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
#ifndef _CONFIG_TINY_TEST_H
#define _CONFIG_TINY_TEST_H

#include "../version/ver_tiny_test.h"
#if VER_TINY_TEST_MAJOR != 1 || VER_TINY_TEST_MINOR != 1
 #error Incompatible TINY_TEST version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
** Maximum size for seek test.
** Options: 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
*/
#define F_MAX_SEEK_TEST 512


#ifdef __cplusplus
}
#endif

#endif /* ifndef _CONFIG_TINY_TEST_H */


