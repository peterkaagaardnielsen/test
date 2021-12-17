/***************************************************************************
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
 * Vaci Ut 76
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/
#ifndef _CONFIG_OAL_OS_H
#define _CONFIG_OAL_OS_H

/* OAL user definition file for no OS */
#include "../version/ver_oal_os.h"
#if VER_OAL_RTX_MAJOR != 1 || VER_OAL_RTX_MINOR != 1
 #error Incompatible OAL_RTX version number!
#endif

#include "../version/ver_oal.h"
#if VER_OAL_MAJOR != 2 || VER_OAL_MINOR != 1
 #error Incompatible OAL version number!
#endif


/* tick rate in ms */
#define OAL_TICK_RATE        10

/* priorities - no meaning without OS */
#define OAL_HIGHEST_PRIORITY 200
#define OAL_HIGH_PRIORITY    150
#define OAL_NORMAL_PRIORITY  100
#define OAL_LOW_PRIORITY     50
#define OAL_LOWEST_PRIORITY  10

/* Event flag to use for user tasks invoking internal functions. */
/* NOTE: The value of this flag should be > 0x80 because lower bits */
/* might be used by internal tasks */
#define OAL_EVENT_FLAG 0x100

#endif /* ifndef _CONFIG_OAL_OS_H */

