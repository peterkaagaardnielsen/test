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
#ifndef _F_ATMEL_H_
#define _F_ATMEL_H_

#include "../../../../../config/config_tiny.h"

#include "../../../../../version/ver_tiny_drv_atmel_df_single.h"
#if VER_TINY_DRV_ATMEL_DF_SINGLE_MAJOR != 1 || VER_TINY_DRV_ATMEL_DF_SINGLE_MINOR != 3
 #error Incompatible TINY_DRV_ATMEL_DF_SINGLE version number!
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*#define AT45DB11B*/     /* 1 Mbit chip */
/*#define AT45DB21B*/     /* 2 Mbit chip */
/*#define AT45DB41B*/     /* 4 Mbit chip */
/*#define AT45DB81B*/     /* 8 Mbit chip */
/*#define AT45DB81D*/     /* 8 Mbit chip */
#define AT45DB161B /* 16 Mbit chip */
/*#define AT45DB321B*/    /* 32 Mbit chip */
/*#define AT45DB321D*/    /* 32 Mbit chip */
/*#define AT45DB642B*/    /* 64 Mbit chip */
/*#define AT45DB642D_1024*/   /* 64 Mbit chip with pagesize set to 1024 bytes */


/* Reserve sectors at the end of the FLASH area. These will not be managed by the driver, and not used by the
   file system. Note: all sectors before F_ATMEL_MANAGEMENT_SECTOR will be reserved too.
   0 menans no reservation. */
#define F_ATMEL_RESERVE_FROM_SECTOR 0u


/* Sector number that is used for managenent purposes by the driver. The filesystem area will start with the
   block right after this sector. No page before this sector will be used by the driver or the file system. */
#define F_ATMEL_MANAGEMENT_SECTOR   0u


/****************************************************************************
 *
 *  The version number.
 *
 ***************************************************************************/
#define ADF_VERSION                 0x0110u


/****************************************************************************
*
*  Device and compiler specific types
*
****************************************************************************/
typedef unsigned long   adf_address_t;
typedef unsigned short  adf_page_address_t;
typedef unsigned int    adf_byte_address_t;


/****************************************************************************
 * Flash chip specific macor definitions. Thera are some more at the
 * beginning of the f_atmel.c file too.
 ***************************************************************************/
#ifdef AT45DB11B     /* 1 Mbit chip */
 #define ADF_PAGE_SIZE          ( 256ul + 8ul )
 #define ADF_REAL_PAGE_COUNT    512ul
 #define ADF_NUM_OF_SECTORS     3ul
 #define ADF_PAGES_PER_SECTOR   256ul
 #define ADF_BYTE_ADDRESS_WIDTH 9ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( ( sc ) < 2u ) ? 8u : ( ( ( sc ) - 1u ) << 8 ) ) )

#elif defined AT45DB21B     /* 2 Mbit chip */
 #define ADF_PAGE_SIZE          ( 256ul + 8ul )
 #define ADF_REAL_PAGE_COUNT    1024ul
 #define ADF_NUM_OF_SECTORS     4ul
 #define ADF_PAGES_PER_SECTOR   512ul
 #define ADF_BYTE_ADDRESS_WIDTH 9ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) < 3u ) ? 256 : ( ( ( ( sc ) - 2u ) << 9 ) ) ) ) )

#elif defined AT45DB41B     /* 4 Mbit chip */
 #define ADF_PAGE_SIZE          ( 256ul + 8ul )
 #define ADF_REAL_PAGE_COUNT    2048ul
 #define ADF_NUM_OF_SECTORS     6ul
 #define ADF_PAGES_PER_SECTOR   512ul
 #define ADF_BYTE_ADDRESS_WIDTH 9ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) < 3u ) ? 256 : ( ( ( ( sc ) - 2u ) << 9 ) ) ) ) )

#elif defined AT45DB81B     /* 8 Mbit chip */
 #define ADF_PAGE_SIZE          ( 256ul + 8ul )
 #define ADF_REAL_PAGE_COUNT    4096ul
 #define ADF_NUM_OF_SECTORS     10ul
 #define ADF_PAGES_PER_SECTOR   512ul
 #define ADF_BYTE_ADDRESS_WIDTH 9ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) < 3u ) ? 256u : ( ( ( ( sc ) - 2u ) << 9 ) ) ) ) )

#elif defined AT45DB81D     /* 8 Mbit chip */
 #define ADF_PAGE_SIZE          ( 256ul + 8ul )
 #define ADF_REAL_PAGE_COUNT    4096ul
 #define ADF_NUM_OF_SECTORS     17ul
 #define ADF_PAGES_PER_SECTOR   256ul
 #define ADF_BYTE_ADDRESS_WIDTH 9ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) < 3u ) ? 256u : ( ( ( ( sc ) - 1u ) << 8 ) ) ) ) )

#elif defined AT45DB161B    /* 16 Mbit chip */
 #define ADF_PAGE_SIZE          ( 512ul + 16ul )
 #define ADF_REAL_PAGE_COUNT    4096ul
 #define ADF_NUM_OF_SECTORS     17ul
 #define ADF_PAGES_PER_SECTOR   256ul
 #define ADF_BYTE_ADDRESS_WIDTH 10ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) - 1u ) << 8 ) ) )

#elif defined AT45DB321B    /* 32 Mbit chip */
 #define ADF_PAGE_SIZE          ( 512ul + 16ul )
 #define ADF_REAL_PAGE_COUNT    8192ul
 #define ADF_NUM_OF_SECTORS     17ul
 #define ADF_PAGES_PER_SECTOR   512ul
 #define ADF_BYTE_ADDRESS_WIDTH 10ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) - 1u ) << 9 ) ) )

#elif defined AT45DB321D    /* 32 Mbit chip */
 #define ADF_PAGE_SIZE          ( 512ul + 16ul )
 #define ADF_REAL_PAGE_COUNT    8192ul
 #define ADF_NUM_OF_SECTORS     65ul
 #define ADF_PAGES_PER_SECTOR   128ul
 #define ADF_BYTE_ADDRESS_WIDTH 10ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) - 1u ) << 7 ) ) )

#elif defined AT45DB642B    /* 64 Mbit chip */
 #define ADF_PAGE_SIZE          ( 1024ul + 32ul )
 #define ADF_REAL_PAGE_COUNT    8192ul
 #define ADF_NUM_OF_SECTORS     33ul
 #define ADF_PAGES_PER_SECTOR   256ul
 #define ADF_BYTE_ADDRESS_WIDTH 11ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) - 1u ) << 8 ) ) )

#elif defined AT45DB642D_1024    /* 64 Mbit chip with pagesize set to 1024 bytes */
 #define ADF_PAGE_SIZE          1024ul
 #define ADF_REAL_PAGE_COUNT    8192ul
 #define ADF_NUM_OF_SECTORS     33ul
 #define ADF_PAGES_PER_SECTOR   256ul
 #define ADF_BYTE_ADDRESS_WIDTH 10ul
 #define SECTOR2PAGE( sc ) ( ( ( sc ) < 1u ) ? 0x0u : ( ( sc ) < 2u ? 8u : ( ( ( sc ) - 1u ) << 8 ) ) )

#endif /* ifdef AT45DB11B */

#define ADF_MGM_END     SECTOR2PAGE( F_ATMEL_MANAGEMENT_SECTOR + 1 )
#if F_ATMEL_RESERVE_FROM_SECTOR == 0
 #define ADF_PAGE_COUNT ( ADF_REAL_PAGE_COUNT - ADF_MGM_END )
#else
 #define ADF_PAGE_COUNT ( SECTOR2PAGE( F_ATMEL_RESERVE_FROM_SECTOR ) - ADF_MGM_END )
#endif


#if USE_ECC
typedef unsigned long  t_ecc;
 #define T_ECC_SIZE _SUL
#endif


/****************************************************************************
 *
 *  dll functions device specific
 *
 ***************************************************************************/
unsigned char adf_init  ( void );

unsigned char adf_erase ( unsigned long flashaddr, long size );
unsigned char adf_erase_safe ( adf_address_t flashaddr, unsigned int size );

#if USE_ECC
unsigned char adf_read  ( void * ramaddr, unsigned long flashaddr, unsigned int size, unsigned char use_ecc );
unsigned char adf_write ( unsigned long flashaddr, _PTRQ void * ramaddr, unsigned int size, unsigned char use_ecc );
#else
unsigned char adf_read  ( void * ramaddr, unsigned long flashaddr, unsigned int size );
unsigned char adf_write ( unsigned long flashaddr, _PTRQ void * ramaddr, unsigned int size );
#endif

unsigned char adf_write_safe ( unsigned long flashaddr, _PTRQ void * ramaddr, unsigned int size );
unsigned char adf_copy ( unsigned long dstflashaddr, unsigned long srcflashaddr, unsigned int size );

unsigned char adf_low_level_format ( void );



#ifdef __cplusplus
}
#endif

#endif /* ifndef _F_ATMEL_H_ */


