/***************************************************************************
 *
 *            Copyright (c) 2013 by HCC Embedded
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
#include <string.h>
#include "../../../api/api_tiny.h"
#include "../../../api/api_tiny_test.h"

#include "../../../../../3rd_party/src/Debug.h"


char temp_buf[1024];

/*
 *  Used by file system test to print infos.
 */
void _f_dump ( const _PTRQ char * s )
{
  int act_len;

  act_len = strlen( s );
  if ( act_len < ( 1024 - 2 ) )
  {
    strcpy( temp_buf, s );
	strcpy( &(temp_buf[act_len]), "\r\n" );
	messageDebug(DBG_INFO, __MODULE__, __LINE__, ( char * )s );
  }

}

/*
 *  Used by file system test to print error results.
 */
unsigned char _f_result ( unsigned char testnum, int result )
{
  messageDebug(DBG_INFO, __MODULE__, __LINE__, "Error in test num %d. Error code: %d", testnum, result );

  return result;
}

/*
 *  Runs file system test.
 */
void hcc_test( void )
{
  f_dotest( 0 );
}

/*
 *  Initializes Tiny file system.
 */
int hcc_init( void )
{
  int rc;

  rc = f_init();

  return rc;
}
