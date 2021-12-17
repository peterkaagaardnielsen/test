/***************************************************************************
 *
 *            Copyright (c) 2003-2012 by HCC Embedded
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
#ifndef _CONFIG_TINY_H
#define _CONFIG_TINY_H

#include "../version/ver_tiny.h"
#if VER_TINY_MAJOR != 3 || VER_TINY_MINOR != 2
 #error Incompatible TINY version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************** flash driver ***********************/
#define F_FLASH_DRIVER         "atmel/df/single/flashset.h"


/*********************** filesystem options *************************/
#define F_WILDCARD             1        /* enable usage of wildcards */
#define QUICK_WILDCARD_SEARCH  0        /* enable quick wildcard search, useful if F_MAX_FILE is big */

#define F_CHECKNAME            1        /* check for valid file name characters, accept multiple / or \ */
                                        /* accepts multiple * in wildcard and handles / at the end of a dirname */
                                        /* if mkdir is called (e.g.: a/b/) or filename at f_open, handles upper/lower case */

#define F_CHECKMEDIA           1        /* check for different media at startup (tiny with different drive geometry) */

#define F_DIRECTORIES          1        /* enable usage of directories */
#define F_CHDIR                1        /* enable chdir */
#define F_MKDIR                1        /* enable mkdir */
#define F_RMDIR                1        /* enable rmdir */
#define F_GETCWD               1        /* enable getcwd */
#define F_DIR_OPTIMIZE         1        /* enable directory storage optimization */

#define F_FINDING              1        /* enable findfirst/findnext */

#define F_FILELENGTH           1        /* enable filelength */
#define F_GETTIMEDATE          1        /* enable gettimedate */
#define F_SETTIMEDATE          1        /* enable settimedate */
#define F_GETFREESPACE         1        /* enable getfreespace */
#define F_DELETE               1        /* enable delete */
#define F_RENAME               1        /* enable rename */

#define F_GETPERMISSION        1        /* enable get permission */
#define F_SETPERMISSION        1        /* enable set permission */

#define F_SEEK_WRITE           1        /* allow seeking for write */

#define F_TRUNCATE             1        /* allow truncate */

#define SMALL_FILE_OPT         1        /* enable small file optimization */

#define QUICK_FILE_SEARCH      0        /* enable quick search, useful if F_MAX_FILE is big */

#define USE_ECC                0        /* use ECC on file management pages */

#define RTOS_SUPPORT           1        /* enable RTOS support */

#define F_FILE_CHANGED_EVENT   0        /* Set to 1 if want event about file state changed */
#define F_FILE_CHANGED_MAXPATH 64       /* Only used if  F_FILE_CHANGED_EVENT is enabled */


/************************* filesystem settings *************************/
#define F_MAX_OPEN_FILE        8ul          /* maximum opened files simultaneously */
#define F_MAX_FILE_NAME_LENGTH 32ul         /* maximum name length of a file or directory */
#define F_MAX_FILE             32ul         /* maximum number of files allowed in the system */
#if F_DIRECTORIES
 #define F_MAX_DIR             16ul             /* maximum number of directories allowed in the system */
#endif

#define F_ATTR_SIZE            1            /* size of attribute in bytes (1/2/4) */

#define F_COPY_BUF_SIZE        32       /* size of a copy buffer */

/*************************** directory attribute ***************************/
#define F_ATTR_DIR             0x20         /* directory attribute (must be in F_ATTR_SIZE range) */


/*************************** system features ***************************/
#define _SUC                   8  /* size of unsigned char in bits */
#define _SUS                   16 /* size of unsigned short in bits */
#define _SUL                   32 /* size of unsigned long in bits */

#define _PTRQ                  /* pointer qualifiers for consts (e.g. for PIC: far) */


#ifdef __cplusplus
}
#endif

#endif /* ifndef _CONFIG_TINY_H */

