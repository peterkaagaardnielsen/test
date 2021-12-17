/***************************************************************************
 *
 *            Copyright (c) 2010-2011 by HCC Embedded
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
 * Vaci Ut 76
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/
#ifndef _PSP_ENDIANNESS_H
#define _PSP_ENDIANNESS_H

#include <stdint.h>

#include "../../version/ver_psp_endianness.h"
#if VER_PSP_ENDIANNESS_MAJOR != 1 || VER_PSP_ENDIANNESS_MINOR != 2
 #error Incompatible PSP_ENDIANNESS version number!
#endif

/* little endian read macros */
#define PSP_RD_LE16( p ) \
  (uint16_t)( ( (uint16_t)( ( p )[1] ) << 8 ) \
             + ( (uint16_t)( ( p )[0] ) ) )


#define PSP_RD_LE24( p ) \
  (uint32_t)( ( (uint32_t)( ( p )[2] ) << 16 ) \
             + ( (uint32_t)( ( p )[1] ) << 8 ) \
             + ( (uint32_t)( ( p )[0] ) ) )

#define PSP_RD_LE32( p ) \
  (uint32_t)( ( (uint32_t)( ( p )[3] ) << 24 ) \
             + ( (uint32_t)( ( p )[2] ) << 16 ) \
             + ( (uint32_t)( ( p )[1] ) << 8 ) \
             + ( (uint32_t)( ( p )[0] ) ) )


/* little endian write macros */
#define PSP_WR_LE16( p, v ) \
  { ( p )[0] = (uint8_t)( ( (uint16_t)( v ) ) & 0xff ); \
    ( p )[1] = (uint8_t)( ( ( (uint16_t)( v ) ) >> 8 ) & 0xff ); }

#define PSP_WR_LE24( p, v ) \
  { ( p )[0] = (uint8_t)( ( (uint32_t)( v ) ) & 0xff ); \
    ( p )[1] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 8 ) & 0xff ); \
    ( p )[2] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 16 ) & 0xff ); }

#define PSP_WR_LE32( p, v ) \
  { ( p )[0] = (uint8_t)( ( (uint32_t)( v ) ) & 0xff ); \
    ( p )[1] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 8 ) & 0xff ); \
    ( p )[2] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 16 ) & 0xff ); \
    ( p )[3] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 24 ) & 0xff ); }

/* big endian read macros */
#define PSP_RD_BE16( p ) \
  (uint16_t)( ( (uint16_t)( ( p )[0] ) << 8 ) \
             + ( (uint16_t)( ( p )[1] ) ) )


#define PSP_RD_BE24( p ) \
  (uint32_t)( ( (uint32_t)( ( p )[0] ) << 16 ) \
             + ( (uint32_t)( ( p )[1] ) << 8 ) \
             + ( (uint32_t)( ( p )[2] ) ) )

#define PSP_RD_BE32( p ) \
  (uint32_t)( ( (uint32_t)( ( p )[0] ) << 24 ) \
             + ( (uint32_t)( ( p )[1] ) << 16 ) \
             + ( (uint32_t)( ( p )[2] ) << 8 ) \
             + ( (uint32_t)( ( p )[3] ) ) )

/* big endian write macros */
#define PSP_WR_BE16( p, v ) \
  { ( p )[1] = (uint8_t)( ( (uint16_t)( v ) ) & 0xff ); \
    ( p )[0] = (uint8_t)( ( ( (uint16_t)( v ) ) >> 8 ) & 0xff ); }

#define PSP_WR_BE24( p, v ) \
  { ( p )[2] = (uint8_t)( ( (uint32_t)( v ) ) ) & 0xff ); \
    ( p )[1] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 8 ) & 0xff ); \
    ( p )[0] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 16 ) & 0xff ); }

#define PSP_WR_BE32( p, v ) \
  { ( p )[3] = (uint8_t)( ( (uint32_t)( v ) ) & 0xff ); \
    ( p )[2] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 8 ) & 0xff ); \
    ( p )[1] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 16 ) & 0xff ); \
    ( p )[0] = (uint8_t)( ( ( (uint32_t)( v ) ) >> 24 ) & 0xff ); }


#endif /* ifndef _PSP_ENDIANNESS_H */

