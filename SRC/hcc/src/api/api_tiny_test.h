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
#ifndef _API_TINY_TEST_H
#define _API_TINY_TEST_H

#include "../config/config_tiny_test.h"

#include "../version/ver_tiny_test.h"
#if VER_TINY_TEST_MAJOR != 1 || VER_TINY_TEST_MINOR != 1
 #error Incompatible TINY_TEST version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
** Start filesystem test.
** Parameter:
**  0  - run all the tests
**  1  - format
**  2  - directory
**  3  - find
**  4  - powerfail
**  5* - seek 128
**  6* - seek 256
**  7* - seek 512
**  8* - seek 1024
**  9* - seek 2048
**  10*- seek 4096
**  11*- seek 8192
**  12*- seek 16384
**  13*- seek 32768
**  14 - open
**  15 - append
**  16 - write
**  17 - dots
**  18 - rit
**  19 - truncate
**  *Note that only seek tests allowed by F_MAX_SEEK_TEST are executed.
**
**  The following options needs to be enabled for the specific test:
**                                  1 1 1 1 1 1 1 1 1 1
**                1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
** F_CHDIR        - x x - - - - - - - - - - - x - x - -
** F_MKDIR        - x x x - - - - - - - - - - - - x - -
** F_RMDIR        - x x - - - - - - - - - - - x - x - -
** F_DELETE       - x x - x x x x x x x x x x x x x x x
** F_FILELENGTH   - - - - x x x x x x x x x x x x - - x
** F_FINDING      x x x x - - - - - - - - - - x - - - -
** F_DIRECTORIES  - x x x - - - - - - - - - - x - x - -
** F_CHECKNAME    - x - - - - - - - - - - - - - - x - -
** F_TRUNCATE     - - - - - - - - - - - - - - - - - - x
*/
void f_dotest ( unsigned char );


#ifdef __cplusplus
}
#endif

#endif /* ifndef _API_TINY_TEST_H */


