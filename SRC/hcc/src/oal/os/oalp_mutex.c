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
#include "../oal_mutex.h"

#include "../../version/ver_oal_os.h"
#if VER_OAL_RTX_MAJOR != 1 || VER_OAL_RTX_MINOR != 1
 #error Incompatible OAL_RTX version number!
#endif
#include "../../version/ver_oal.h"
#if VER_OAL_MAJOR != 2 || VER_OAL_MINOR != 1
 #error Incompatible OAL version number!
#endif


/*
** oal_mutex_create
**
** Create mutex.
**
** Output: p_mutex - pointer to mutex
** Return: OAL_*
*/
#if OAL_MUTEX_SUPPORTED
int oal_mutex_create ( oal_mutex_t * p_mutex )
{
  os_mut_init( p_mutex );

  return OAL_SUCCESS;
} /* oal_mutex_create */
#endif  /* if OAL_MUTEX_SUPPORTED */


/*
** oal_mutex_delete
**
** Delete mutex.
**
** Input: p_mutex - pointer to mutex
** Return: OAL_*
*/
#if OAL_MUTEX_SUPPORTED
int oal_mutex_delete ( oal_mutex_t * p_mutex )
{
  return OAL_SUCCESS;
}
#endif


/*
** oal_mutex_get
**
** Get mutex.
**
** Input: p_mutex - pointer to mutex
** Return: OAL_*
*/
#if OAL_MUTEX_SUPPORTED
int oal_mutex_get ( oal_mutex_t * p_mutex )
{
  int  rc = OAL_SUCCESS;

  if ( OS_R_TMO == os_mut_wait( p_mutex, 0xffff ) )
  {
    rc = OAL_ERR_RESOURCE;
  }

  return rc;
}
#endif  /* if OAL_MUTEX_SUPPORTED */


/*
** oal_mutex_put
**
** Put mutex.
**
** Input: p_mutex - pointer to mutex
** Return: OAL_*
*/
#if OAL_MUTEX_SUPPORTED
int oal_mutex_put ( oal_mutex_t * p_mutex )
{
  int  rc = OAL_ERR_RESOURCE;

  if ( OS_R_OK == os_mut_release( p_mutex ) )
  {
    rc = OAL_SUCCESS;
  }

  return rc;
}
#endif

