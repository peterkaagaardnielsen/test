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
** oal_task_create
**
** Create a task.
**
** Input: p_task - pointer to the task
**        oal_task_dsc - task descriptor
** Output: task_id - created task ID
** Return: OAL_*
*/
#if OAL_TASK_SUPPORTED
int oal_task_create ( oal_task_t * p_task, const oal_task_dsc_t * task_dsc, oal_task_id_t * task_id )
{
  int  rc = OAL_SUCCESS;

  *task_id = os_tsk_create_user( task_dsc->entry, task_dsc->priority, task_dsc->stack_ptr, task_dsc->stack_size );
  if ( 0 != *task_id )
  {
    *p_task = *task_id;
  }
  else
  {
    rc = OAL_ERR_RESOURCE;
  }

  return rc;
} /* oal_task_create */
#endif /* if OAL_TASK_SUPPORTED */


/*
** oal_task_delete
**
** Delete the caller task.
**
** Input: p_task - pointer to the task
** Return: OAL_*
*/
#if OAL_TASK_SUPPORTED
int oal_task_delete ( oal_task_t * p_task )
{
  int  rc = OAL_ERR_RESOURCE;

  if ( OS_R_OK == os_tsk_delete( *p_task ) )
  {
    rc = OAL_SUCCESS;
  }

  return rc;
}
#endif


/*
** oal_task_yield
**
** Yield current task
*/
#if OAL_TASK_SUPPORTED
void oal_task_yield ( void )
{
}
#endif


/*
** oal_task_get_id
**
** Get task ID.
**
** Return: Task ID
*/
#if OAL_TASK_GET_ID_SUPPORTED
oal_task_id_t oal_task_get_id ( void )
{
  return os_tsk_self();
}
#endif


/*
** oal_task_sleep
**
** Suspends the caller task for 'ms' milliseconds
**
** Input: ms - milliseconds to sleep
*/
#if OAL_TASK_SLEEP_SUPPORTED
void oal_task_sleep ( uint32_t ms )
{
  ms /= OAL_TICK_RATE;
  if ( ms == 0 )
  {
    ms = 1;
  }

  os_dly_wait( ms );
}
#endif

