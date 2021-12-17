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
#ifndef _F_UTIL_H
#define _F_UTIL_H

#include "tiny_types.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define _f_setvalue_1( dst, value ) *dst = value
#define _f_getvalue_1( src )        *src

_U16_T _f_getvalue_2 ( unsigned char * src );
void _f_setvalue_2 ( unsigned char * dst, _U16_T value );

_U32_T _f_getvalue_4 ( unsigned char * src );
void _f_setvalue_4 ( unsigned char * dst, _U32_T value );

void _f_get_timedate ( unsigned short * time, unsigned short * date );

unsigned char tiny_used_addr ( unsigned long addr );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _F_UTIL_H */

