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
#include "../../api/api_tiny.h"
#include "../../psp/include/psp_rtc.h"
#include "tiny_types.h"
#include "f_volume.h"

#include "../../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif
#include "../../version/ver_psp_rtc.h"
#if VER_PSP_RTC_MAJOR != 1
 #error Incompatible PSP_RTC version number!
#endif



/*
** _f_setvalue_2
**
** Write a 16 bit value to a char array
**
** INPUT: value - 16 bit value
** OUTPUT: dst - where to write the value
*/
void _f_setvalue_2 ( unsigned char * dst, _U16_T value )
{
  *dst++ = (unsigned char)( value & 0xff );
  value >>= 8;
  *dst = (unsigned char)( value & 0xff );
}


/*
** _f_setvalue_4
**
** Write a 32 bit value to a char array
**
** INPUT: value - 32 bit value
** OUTPUT: dst - where to write the value
*/
void _f_setvalue_4 ( unsigned char * dst, _U32_T value )
{
  *dst++ = (unsigned char)( value & 0xff );
  value >>= 8;
  *dst++ = (unsigned char)( value & 0xff );
  value >>= 8;
  *dst++ = (unsigned char)( value & 0xff );
  value >>= 8;
  *dst = (unsigned char)( value & 0xff );
}


/*
** _f_getvalue_2
**
** Read a 16 bit value from a char array
**
** INPUT: src - pointer to char array
** OUTPUT: 16 bit value
*/
_U16_T _f_getvalue_2 ( unsigned char * src )
{
  _U16_T  rc;

  rc = ( *src & 0xff );
  rc = (_U16_T)( *src & 0xff );
  ++src;
  rc |= ( (_U16_T)( *src & 0xff ) << 8 );

  return rc;
}


/*
** _f_getvalue_4
**
** Read a 32 bit value from a char array
**
** INPUT: src - pointer to char array
** OUTPUT: 32 bit value
*/
_U32_T _f_getvalue_4 ( unsigned char * src )
{
  _U32_T  rc;

  rc = (_U32_T)( *src & 0xff );
  ++src;
  rc |= ( (_U32_T)( *src & 0xff ) << 8 );
  ++src;
  rc |= ( (_U32_T)( *src & 0xff ) << 16 );
  ++src;
  rc |= ( (_U32_T)( *src & 0xff ) << 24 );

  return rc;
}


/*
** _f_get_timedate
**
** Get current time and date
**
** OUTPUT:  time - where to write the time
**          date - where to write the date
*/
void _f_get_timedate ( unsigned short * time, unsigned short * date )
{
  t_psp_timedate  timedate;

  psp_getcurrenttimedate( &timedate );
  if ( time )
  {
    *time = (unsigned short)( ( ( (unsigned short)timedate.hour << F_CTIME_HOUR_SHIFT ) & F_CTIME_HOUR_MASK )
                             | ( ( (unsigned short)timedate.min << F_CTIME_MIN_SHIFT )  & F_CTIME_MIN_MASK )
                             | ( ( ( (unsigned short)timedate.sec >> 1 ) << F_CTIME_SEC_SHIFT )  & F_CTIME_SEC_MASK ) );
  }

  if ( date )
  {
    *date = (unsigned short)( ( ( ( timedate.year - 1980 ) << F_CDATE_YEAR_SHIFT ) & F_CDATE_YEAR_MASK )
                             | ( ( (unsigned short)timedate.month << F_CDATE_MONTH_SHIFT ) & F_CDATE_MONTH_MASK )
                             | ( ( (unsigned short)timedate.day << F_CDATE_DAY_SHIFT ) & F_CDATE_DAY_MASK ) );
  }
} /* _f_get_timedate */


/*
** tiny_used_addr
**
** Checks if the address passed as a parameter is used
** by the file system or not.
**
** INPUT: addr - address
** OUTPUT: 1-used, 0-not used
*/
unsigned char tiny_used_addr ( unsigned long addr )
{
  if ( addr >= F_FIRST_CLUSTER_ADDR )
  {
    F_CLUSTER_COUNT_TYPE  cluster = (F_CLUSTER_COUNT_TYPE)( ( addr - F_FIRST_CLUSTER_ADDR ) / F_CLUSTER_SIZE );
    if ( cluster >= F_CLUSTER_COUNT )
    {
      return 0;
    }

    if ( _GET_BIT( f_volume.cluster_table, cluster ) == 0 )
    {
      return 0;
    }
  }

  return 1;
} /* tiny_used_addr */

