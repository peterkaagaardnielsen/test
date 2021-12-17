/***************************************************************************
 *
 *            Copyright (c) 2010 by HCC Embedded
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
#include <stdint.h>
#include "../include/psp_reg.h"

#include "../../version/ver_psp_reg.h"
#if VER_PSP_REG_MAJOR != 1 || VER_PSP_REG_MINOR != 0
 #error Incompatible PSP_REG version number!
#endif

uint16_t psp_rev16 ( uint16_t v )
{
  return (uint16_t)( ( ( ( v ) & 0xff ) << 8 ) | ( ( ( v ) & 0xff00 ) >> 8 ) );
}

uint32_t psp_rev32 ( uint32_t v )
{
  return (uint32_t)( ( ( ( v ) & 0xff ) << 24 ) | ( ( ( v ) & 0xff00 ) << 8 ) | ( ( ( v ) & 0xff0000 ) >> 8 ) | ( ( ( v ) & 0xff000000 ) >> 24 ) );
}

