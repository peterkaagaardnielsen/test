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
#include <stddef.h>
#include <stdint.h>
#include "../../config/config_oal.h"
#include "../oal_event.h"
#include "../oal_task.h"

#include "../../version/ver_oal_os.h"
#if VER_OAL_RTX_MAJOR != 1 || VER_OAL_RTX_MINOR != 1
 #error Incompatible OAL_RTX version number!
#endif
#include "../../version/ver_oal.h"
#if VER_OAL_MAJOR != 2 || VER_OAL_MINOR != 1
 #error Incompatible OAL version number!
#endif


/*
** oal_event_create
**
** Create event.
**
** Output: p_event - pointer to event
** Return: OAL_*
*/
#if OAL_EVENT_SUPPORTED
int oal_event_create ( oal_event_t * p_event )
{
  (void)p_event;
  return OAL_SUCCESS;
} /* oal_event_create */
#endif  /* if OAL_EVENT_SUPPORTED */


/*
** oal_event_delete
**
** Delete event.
**
** Input: p_event - pointer to event
** Return: OAL_*
*/
#if OAL_EVENT_SUPPORTED
int oal_event_delete ( oal_event_t * p_event )
{
  (void)p_event;
  return OAL_SUCCESS;
}
#endif


/*
** oal_event_get
**
** Get event.
**
** Input: p_event - pointer to event
**        wflags - wait for these flags
**        timeout - timeout (OAL_WAIT_FOREVER/..)
** Output: sflags - set flags
** Return: OAL_*
*/
#if OAL_EVENT_SUPPORTED
int oal_event_get ( oal_event_t * p_event, oal_event_flags_t wflags, oal_event_flags_t * sflags, oal_event_timeout_t timeout )
{
  int  rc = OAL_ERROR;

  (void)p_event;

  if ( timeout != OAL_WAIT_FOREVER )
  {
    timeout /= OAL_TICK_RATE;
    if ( timeout == 0 )
    {
      timeout = 1;
    }
  }

  if ( OS_R_EVT == os_evt_wait_or( wflags, timeout ) )
  {
    *sflags = os_evt_get();
    rc = OAL_SUCCESS;
  }

  return rc;
} /* oal_event_get */
#endif  /* if OAL_EVENT_SUPPORTED */


/*
** oal_event_set
**
** Set event.
**
** Input: p_event - pointer to event
**        flags - flags to set
**        task_id - task ID (may not be required)
** Return: OAL_*
*/
#if OAL_EVENT_SUPPORTED
int oal_event_set ( oal_event_t * p_event, oal_event_flags_t flags, oal_task_id_t task_id )
{
  (void)p_event;
  os_evt_set( flags, task_id );
  return OAL_SUCCESS;
}
#endif


/*
** oal_event_set_int
**
** Set event from interrupt.
**
** Input: p_event - pointer to event
**        flags - flags to set
**        task_id - task ID (may not be required)
** Return: OAL_*
*/
#if OAL_EVENT_SUPPORTED
int oal_event_set_int ( oal_event_t * p_event, oal_event_flags_t flags,  oal_task_id_t task_id )
{
  (void)p_event;
  isr_evt_set( flags, task_id );
  return OAL_SUCCESS;
}
#endif

