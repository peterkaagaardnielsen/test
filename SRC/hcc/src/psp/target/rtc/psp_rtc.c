/****************************************************************************
 *
 *            Copyright (c) 2011 by HCC Embedded
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
#include <stdint.h>
#include <stddef.h>
#include "../../include/psp_rtc.h"

#include "../../../version/ver_psp_rtc.h"
#if VER_PSP_RTC_MAJOR != 1
 #error "VER_PSP_RTC_MAJOR invalid"
#endif
#if VER_PSP_RTC_MINOR != 0
 #error "VER_PSP_RTC_MINOR invalid"
#endif


/****************************************************************************
 *
 * psp_getcurrenttimedate
 *
 * need to be ported depending on system, it retreives the
 * current time and date.
 * Please take care of correct roll-over handling.
 * Roll-over problem is to read a date at 23.59.59 and then reading time at
 * 00:00.00.
 *
 * INPUT
 *
 * p_timedate - pointer where to store time and date
 *
 ***************************************************************************/
void psp_getcurrenttimedate ( t_psp_timedate * p_timedate )
{
  if ( p_timedate != NULL )
  {
    p_timedate->sec = 0;
    p_timedate->min = 0;
    p_timedate->hour = 12u;

    p_timedate->day = 1u;
    p_timedate->month = 1u;
    p_timedate->year = 1980u;
  }
} /* psp_getcurrenttimedate */

