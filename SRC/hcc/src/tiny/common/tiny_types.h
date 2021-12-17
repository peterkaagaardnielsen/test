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
#ifndef _TINY_TYPES_H_
#define _TINY_TYPES_H_

#include "../../config/config_tiny.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif


#define _MAXVAL( s ) ( ( ( ( 1u << ( ( s ) - 1 ) ) - 1 ) << 1 ) + 1 ) /* maximum size of a variable (_SUL/_SUS/_SUC) */


#define _FM_8     0xff                                  /* max. value of an 8 bit variable */
#define _FM_16    0xffff                                /* max. value of a 16 bit variable */
#define _FM_32    0xffffffff                            /* max. value of a 32 bit variable */

#define _F_INV_8  _FM_8
#define _F_INV_16 _FM_16
#define _F_INV_32 _FM_32                                /* invalid values are equal to maximum values */

#define _U8_T     unsigned char                         /* unsigned char type in the system */

#if _MAXVAL( _SUC ) >= _FM_16
 #define _U16_T   unsigned char
#else
 #define _U16_T   unsigned short
#endif                                                  /* unsigned short type in the system */

#if _MAXVAL( _SUS ) >= _FM_32
 #define _U32_T   unsigned short
#else
 #define _U32_T   unsigned long
#endif                                                  /* unsigned long type in the system */

#endif /* ifndef _TINY_TYPES_H_ */

