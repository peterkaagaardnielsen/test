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
#include "f_atmel.h"
#include "../../../../../psp/include/psp_spi.h"
#include "../../../../common/f_util.h"

#include "../../../../../version/ver_tiny_drv_atmel_df_single.h"
#if VER_TINY_DRV_ATMEL_DF_SINGLE_MAJOR != 1 || VER_TINY_DRV_ATMEL_DF_SINGLE_MINOR != 3
 #error Incompatible TINY_DRV_ATMEL_DF_SINGLE version number!
#endif
#include "../../../../../version/ver_psp_spi.h"
#if VER_PSP_SPI_MAJOR != 2
 #error Incompatible PSP_SPI version number!
#endif


/*****************************************************************************
 *
 * Macro definitions.
 *
 ****************************************************************************/


/* Some dataflash chips support fast programming (command opcode  93, 96, 98, 99). If your chip supports these
   features set the value of the macro below to one. */
#define F_ATMEL_USE_FAST_WRITE 0u

/* Check if the management sector selection is valid. */
#if F_ATMEL_MANAGEMENT_SECTOR > ADF_NUM_OF_SECTORS
 #error "Invalid management sector selection. Please correct the value of F_ATMEL_MANAGEMENT_SECTOR"
#endif

/* Command opcodes for the flash chip. */
#define ADF_READ_CONT       0xe8
#define ADF_READ            0xd2
#define ADF_READ_BUF1       0xd4
#define ADF_READ_BUF2       0xd6
#define ADF_STATUS          0xd7
#define ADF_WRITE_BUF1      0x84
#define ADF_WRITE_BUF2      0x87
#if F_ATMEL_USE_FAST_WRITE != 0
 #define ADF_PROGERASE_BUF1 0x93
 #define ADF_PROGERASE_BUF2 0x96
 #define ADF_PROG_BUF1      0x98
 #define ADF_PROG_BUF2      0x99
#else
 #define ADF_PROGERASE_BUF1 0x83
 #define ADF_PROGERASE_BUF2 0x86
 #define ADF_PROG_BUF1      0x88
 #define ADF_PROG_BUF2      0x89
#endif
#define ADF_ERASE_PAGE      0x81
#define ADF_ERASE_BLOCK     0x50
#define ADF_READ_MAIN2BUF1  0x53
#define ADF_READ_MAIN2BUF2  0x55

#ifdef AT45DB11B    /****************************************************************** 1 Mbit chip */
/* Chip id from the status register. */
 #define ADF_ID             ( 0x03u << 2 )

/* Returns the number of the sector that contains the specified page. */
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) >> 8 ) + 1u ) ) )

/* Returns the size of a sector. */
 #define SECTORSIZE( sc )   ( ( ( sc ) < 1u ) ? 8u : ( ( ( sc ) < 2u ) ? ( 256u - 8u ) : ADF_PAGES_PER_SECTOR ) )

/* Returns the start address of a page. */
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 8u ) + ( ( (adf_address_t)( pg ) ) << 3 ) )

#elif defined AT45DB21B     /****************************************************************** 2 Mbit chip */
 #define ADF_ID ( 0x05u << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) < 512u ) ? 2u : ( ( pg ) >> 9 ) + 2u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ( ( ( sc ) < 3u ) ? 256 : ADF_PAGES_PER_SECTOR ) ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 8u ) + ( ( (adf_address_t)( pg ) ) << 3 ) )

#elif defined AT45DB41B     /****************************************************************** 4 Mbit chip */
 #define ADF_ID ( 0x7u << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) < 512 ) ? 2u : ( ( pg ) >> 9 ) + 2u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ( ( ( sc ) < 3u ) ? 256 : ADF_PAGES_PER_SECTOR ) ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 8u ) + ( ( (adf_address_t)( pg ) ) << 3 ) )

#elif defined AT45DB81B     /****************************************************************** 8 Mbit chip */
 #define ADF_ID ( 0x9u << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) < 512u ) ? 2u : ( ( pg ) >> 9 ) + 2u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ( ( ( sc ) < 3u ) ? 256u : ADF_PAGES_PER_SECTOR ) ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 8u ) + ( ( (adf_address_t)( pg ) ) << 3 ) )

#elif defined AT45DB81D     /****************************************************************** 8 Mbit chip */
 #define ADF_ID ( 0x9u << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) >> 8 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ( ( ( sc ) < 3u ) ? 256u : ADF_PAGES_PER_SECTOR ) ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 8u ) + ( ( (adf_address_t)( pg ) ) << 3 ) )

#elif defined AT45DB161B    /***************************************************************** 16 Mbit chip */
 #define ADF_ID ( 0x0b << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) >> 8 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ADF_PAGES_PER_SECTOR ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 9u ) + ( ( (adf_address_t)( pg ) ) << 4 ) )

#elif defined AT45DB321B    /***************************************************************** 32 Mbit chip */
 #define ADF_ID ( 0xdu << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 512u ) ? 1u : ( ( ( pg ) >> 9 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 512u - 8u ) : ADF_PAGES_PER_SECTOR ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 9u ) + ( ( (adf_address_t)( pg ) ) << 4 ) )

#elif defined AT45DB321D    /***************************************************************** 32 Mbit chip */
 #define ADF_ID ( 0xdu << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 128u ) ? 1u : ( ( ( pg ) >> 7 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 128u - 8u ) : ADF_PAGES_PER_SECTOR ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 9u ) + ( ( (adf_address_t)( pg ) ) << 4 ) )

#elif defined AT45DB642B    /**************************************************************** 64 Mbit chip */
 #define ADF_ID ( 0xfu << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) >> 8 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ADF_PAGES_PER_SECTOR ) )
 #define PADE2ADDRESS( pg ) ( ( ( (adf_address_t)( pg ) ) << 10u ) + ( ( (adf_address_t)( pg ) ) << 5 ) )

#elif defined AT45DB642D_1024 /****************************** 64 Mbit chip with pagesize set to 1024 bytes */
 #define ADF_ID ( 0xfu << 2 )
 #define PAGE2SECTOR( pg )  ( ( ( pg ) < 8u ) ? 0x0u : ( ( ( pg ) < 256u ) ? 1u : ( ( ( pg ) >> 8 ) + 1u ) ) )
 #define SECTORSIZE( sc )   ( ( sc ) < 1u ? 8u : ( ( sc ) < 2u ? ( 256u - 8u ) : ADF_PAGES_PER_SECTOR ) )
 #define PADE2ADDRESS( pg ) ( ( (adf_address_t)( pg ) ) << 10u )

#endif /* ifdef AT45DB11B */

/* Determine if the driver shall use two bitmap sectors. */
#if F_ATMEL_RESERVE_FROM_SECTOR == 0
 #define ADF_BMP_SIZE ( ( ADF_REAL_PAGE_COUNT - ADF_MGM_END ) / 8 + ( ADF_NUM_OF_SECTORS - ( F_ATMEL_MANAGEMENT_SECTOR - 1 ) ) )
#else
 #define ADF_BMP_SIZE \
  ( ( ( SECTOR2PAGE( F_ATMEL_RESERVE_FROM_SECTOR ) \
       - ADF_MGM_END ) ) / 8 + ( F_ATMEL_RESERVE_FROM_SECTOR - F_ATMEL_MANAGEMENT_SECTOR - 1 ) )
#endif

#define ADF_MGM_START  SECTOR2PAGE( F_ATMEL_MANAGEMENT_SECTOR )
#define ADF_MGM_LENGTH ( ADF_MGM_END - ADF_MGM_START )

/* Turns a page address into a flash address that is understanded by the FLASH chip. */
#define PGA2FLA( p ) ( ( (adf_address_t)( p ) ) << ADF_BYTE_ADDRESS_WIDTH )

/* READY x_bit in the satus register. */
#define ADF_BREADY ( 1u << 7 )

/* Calculates wich x_bit of the map byte contains information about a particular page. */
#define PAGE2BITOFS( pg ) ( ( pg ) & 0x7u )

/* Most significan x_bit of the page address. */
#define PAGE_ADDRESS_MSB ( ( (adf_page_address_t)1u ) << ( ( sizeof( adf_page_address_t ) << 3 ) - 1u ) )

/* This will avaluate to 1 if the bitmap will not fit to one flash page. */
#define ADF_2_MAP_SECTOR ( ADF_PAGE_SIZE < ADF_BMP_SIZE )

/* Define the size of the management units. This depends on the size of the bitmap. */
#if ADF_2_MAP_SECTOR
 #if F_ATMEL_RESERVE_FROM_SECTOR == 0
  #define ADF_LAST_0BMP_SECTOR ( F_ATMEL_MANAGEMENT_SECTOR + ( ADF_NUM_OF_SECTORS - F_ATMEL_MANAGEMENT_SECTOR ) / 2 )
 #else
  #define ADF_LAST_0BMP_SECTOR ( F_ATMEL_MANAGEMENT_SECTOR + ( F_ATMEL_RESERVE_FROM_SECTOR - F_ATMEL_MANAGEMENT_SECTOR ) / 2 )
 #endif

/* Calculates wich map byte contains information about a particular page. */
 #define PAGE2BYTEOFS( pg ) \
  ( ( ( pg ) < SECTOR2PAGE( ADF_LAST_0BMP_SECTOR + 1 ) ) ? \
   ( ( ( pg ) - ADF_MGM_END ) >> 3u ) : ( ( ( pg ) - SECTOR2PAGE( ADF_LAST_0BMP_SECTOR + 1u ) ) >> 3u ) )
 #define BMPCTROFS( sc ) \
  ( ( sc <= ADF_LAST_0BMP_SECTOR ) ? \
   ( ( ADF_PAGE_SIZE - 1 ) - ( sc - ( F_ATMEL_MANAGEMENT_SECTOR + 1 ) ) ) : ( ADF_PAGE_SIZE - ( sc - ADF_LAST_0BMP_SECTOR ) ) )
 #define BMPOFS2PAGE( bo, sel ) \
  ( ( ( sel ) == 0 ) ? (adf_page_address_t)( ( ( bo ) << 3 ) + ADF_MGM_END ) : \
   ( (adf_page_address_t)( ( bo ) << 3 ) + SECTOR2PAGE( ADF_LAST_0BMP_SECTOR + 1u ) ) )
 #define OTHERMGMSEL( ndx ) ( ( ~( ndx ) ) & 0x1u )

#else /* if ADF_2_MAP_SECTOR */

/* Calculates wich map byte contains information about a particular page. */
 #define PAGE2BYTEOFS( pg )     ( ( ( pg ) - ADF_MGM_END ) >> 3u )
 #define BMPCTROFS( sc )        ( ADF_PAGE_SIZE - 1 - ( sc - ( F_ATMEL_MANAGEMENT_SECTOR + 1 ) ) )
 #define BMPOFS2PAGE( bo, sel ) ( (adf_page_address_t)( ( ( bo ) << 3 ) + ADF_MGM_END ) )
#endif /* if ADF_2_MAP_SECTOR */

#if ADF_2_MAP_SECTOR
 #define ADF_MGM1_START ADF_MGM_START
 #define ADF_MGM1_END   ( ADF_MGM_START + ( ADF_MGM_LENGTH >> 1 ) )
 #define ADF_MGM2_START ADF_MGM1_END
 #define ADF_MGM2_END   ADF_MGM_END

/* Will return the next management page pair.*/
 #define ADF_NEXT_MGM_BUFFER( b ) \
  ( ( ( b ) < ADF_MGM2_START ) ? ( ( b ) < ( ADF_MGM1_END - 2u ) ? ( b ) + 2u : ADF_MGM1_START ) : \
   ( ( b ) < ( ADF_MGM2_END - 2u ) ? ( b ) + 2u : ( ADF_MGM2_START ) ) )
#else

/* Will return the next management page pair.*/
 #define ADF_NEXT_MGM_BUFFER( b ) \
  ( ( b ) < ( ADF_MGM_END - 2u ) ? ( b ) + 2u : ADF_MGM_START )
#endif

#if USE_ECC
 #define ADF_ECC_START ( ADF_PAGE_SIZE - T_ECC_SIZE )

 #define F_ECC_USE     ( 1 << 0 )
 #define F_ECC_VALID   ( 1 << 1 )
#endif

/* redefine CS low/high functions */
#define SPI_CS_HI      { psp_spi_cs_hi( 0 ); (void)psp_spi_unlock( 0 ); }
#define SPI_CS_LO      { (void)psp_spi_lock( 0 ); psp_spi_cs_lo( 0 ); }


/*****************************************************************************
 *
 * Type definitions.
 *
 ****************************************************************************/
typedef unsigned char   adf_bool;
typedef unsigned char   adf_u8;
typedef unsigned short  adf_u16;
typedef unsigned long   adf_u32;

typedef struct
{
  adf_page_address_t  my_addr;
  adf_page_address_t  p_addr;
} adf_lock_data_t[2];


/*****************************************************************************
 *
 * Static variables.
 *
 ****************************************************************************/
static adf_u16  adf_update_ctr[ADF_NUM_OF_SECTORS + 1];

#if ADF_2_MAP_SECTOR
static adf_page_address_t  adf_curr_mgm_pg[2];
static adf_u16             adf_map_lock_seq[2];
static adf_u8              adf_mgm_sel;
#else
static adf_page_address_t  adf_curr_mgm_pg[1];
static adf_u16             adf_map_lock_seq[1];
 #define adf_mgm_sel 0u
#endif

static struct
{
  adf_page_address_t  page;
  adf_bool            changed;
#if USE_ECC
  t_ecc               ecc_value; /* ECC value stored in flash */
  adf_u8              ecc_flags; /* ECC flags */
#endif
} adf_buf1_info;


/*****************************************************************************
 *
 * Function predefinitions.
 *
 ****************************************************************************/
static adf_bool adf_recover ( void );
static adf_bool adf_transfer_buff_1 ( const adf_page_address_t );
static adf_bool adf_program_buff_1 ( void );
static adf_bool adf_lock_page ( const adf_page_address_t, const adf_page_address_t );
static adf_bool update_bmp ( adf_page_address_t );
static adf_bool adf_unlock_page ( const adf_page_address_t );
#if ADF_2_MAP_SECTOR
static adf_bool inc_mgm_wr_ctr ( adf_u8 );
#endif
#if USE_ECC
static void adf_write_ecc ( t_ecc ecc );
#endif


/*****************************************************************************
 *
 * Functions need to be defined elsewhere.
 *
 ****************************************************************************/
extern adf_bool is_used_page ( adf_address_t );



/*****************************************************************************************************
************************************** LOCAL FUNCTIONS ***********************************************
*****************************************************************************************************/


/****************************************************************************
 *
 * adf_cmd
 *
 * Sends the specified command and three address bytes trough the SPI.
 *
 * INPUT:
 *
 * none
 *
 * OUTPUT:
 *
 * none
 *
 ***************************************************************************/
void adf_cmd ( adf_u8 cmd, adf_address_t addr )
{
  SPI_CS_LO;
  psp_spi_tx1( 0, cmd );
  psp_spi_tx1( 0, (unsigned char)( addr >> 16 ) );
  psp_spi_tx1( 0, (unsigned char)( ( addr >> 8 ) & 0xff ) );
  psp_spi_tx1( 0, (unsigned char)( addr & 0xff ) );
}


/****************************************************************************
 *
 * adf_wait_ready
 *
 * Waits till the flash chip is busy.
 *
 * INPUT:
 *
 * none
 *
 * OUTPUT:
 *
 * 0: if all ok
 * 1: if timeout passed
 *
 *
 ***************************************************************************/
static adf_bool adf_wait_ready ( void )
{
  unsigned char  st;
  unsigned int   tmout = 60000;

  SPI_CS_LO;
  psp_spi_tx1( 0, ADF_STATUS );
  do
  {
    tmout--;
    psp_spi_rx( 0, &st, 1 );
  }
  while ( ( ( st & ADF_BREADY ) == 0 ) && ( tmout != 0 ) );

  SPI_CS_HI;
  return (adf_bool)( tmout == 0 );
}



/*******************************************************************************
 *
 * adf_transfer_buff_1
 *
 * This function fills ram buffer 1 with the contents of a page from the FLASH
 * main area. Note: buffer one functions as a chace.
 *
 * INPUT:
 *
 * page_addr: address of page to be trnasferred to buffer one
 *
 * OUTPUT:
 *
 * none
 *
  *******************************************************************************/
static adf_bool adf_transfer_buff_1 ( const adf_page_address_t page_addr )
{
  /* If the buffer contains a different page */
  if ( adf_buf1_info.page != page_addr )
  { /* if the buffer conatains pending data */
    if ( adf_buf1_info.changed )
    { /* Write it into the main flash area. */
      if ( adf_program_buff_1() )
      {
        return 1;
      }
    }

    /* Read page into the buffer. */
    adf_cmd( ADF_READ_MAIN2BUF1, PGA2FLA( page_addr ) );
    SPI_CS_HI;


    /* Remember which page is in the buffer. Note: adf_buf1_info.changed will be cleared
      by ...program_buf_1() above. */
    adf_buf1_info.page = page_addr;
    if ( adf_wait_ready() )
    {
      return 1;
    }

#if USE_ECC
    adf_buf1_info.ecc_flags = 0;
#endif
  }

  return 0;
} /* adf_transfer_buff_1 */


/*******************************************************************************
 *
 * adf_program_buff_1
 *
 * This function programs the contents of ram buffer 1 to the FLASH main area.
 * Note: buffer one functions as a chace.
 *
 * INPUT:
 *
 * none
 *
 * OUTPUT:
 *
 * none
 *
  *******************************************************************************/
static adf_bool adf_program_buff_1 ( void )
{ /* If the contents of the buffer where changed. */
  if ( adf_buf1_info.changed )
  {
    adf_buf1_info.changed = 0;

#if USE_ECC
    if ( adf_buf1_info.ecc_flags & F_ECC_USE )
    {
      adf_cmd( ADF_WRITE_BUF1, ADF_ECC_START );
      adf_write_ecc( adf_buf1_info.ecc_value );
      SPI_CS_HI;
    }

#endif

    adf_cmd( ADF_PROGERASE_BUF1, PGA2FLA( adf_buf1_info.page ) );
    SPI_CS_HI;
    if ( adf_wait_ready() )
    {
      return 1;
    }

    /* If this is not a reserved page, update the 10000 bitmap. */
    /*lint -e{506} */
    if (
#if ( ADF_MGM_START > 0 )
        ( adf_buf1_info.page < ADF_MGM_START ) ||
#endif
        ( adf_buf1_info.page >= ADF_MGM_END ) )
    {
      if ( update_bmp( adf_buf1_info.page ) )
      {
        return 1;
      }
    }
  }

  return 0;
} /* adf_program_buff_1 */


/*******************************************************************************
 * This function programs the contents of ram buffer 2 to the FLASH main area.
 *******************************************************************************/
#if ADF_2_MAP_SECTOR
static adf_bool adf_transfer_buff_2 ( const adf_page_address_t page_addr )
{
  adf_cmd( ADF_READ_MAIN2BUF2, PGA2FLA( page_addr ) );
  SPI_CS_HI;
  return adf_wait_ready();
}
#endif


/*******************************************************************************
 * This function programs the contents of ram buffer 2 to the FLASH main area.
 *******************************************************************************/
static adf_bool adf_program_buff_2 ( adf_page_address_t pg )
{
  adf_cmd( ADF_PROGERASE_BUF2, PGA2FLA( pg ) );
  SPI_CS_HI;
  return adf_wait_ready();
}


/*******************************************************************************
 *
 * adf_lock_page
 *
 * This function will generate and program a flash page that logically marks a
 * page. NOTE: this function will destroy the contents of buffer one.
 *
 * INPUT
 *
 * safe_page_addr: address of the management page where the lock is programed
 * page_sddr: address of the page that is marked (locked).
 *
 *******************************************************************************/
static adf_bool adf_lock_page ( const adf_page_address_t safe_page_addr, const adf_page_address_t page_addr )
{
  unsigned int     x;
  unsigned int     y;
  adf_lock_data_t  lock;

  lock[0].my_addr = safe_page_addr;
  lock[0].p_addr = page_addr;
  lock[1].my_addr = ( adf_page_address_t ) ~safe_page_addr;
  lock[1].p_addr = ( adf_page_address_t ) ~page_addr;

  adf_cmd( ADF_WRITE_BUF1, 0 );
  for ( y = 0 ; y < 2 ; y++ )
  {
    for ( x = 0 ; x < sizeof( lock[y].my_addr ) ; x++ )
    {
      psp_spi_tx1( 0, (adf_u8)( lock[y].my_addr & 0xff ) );
      lock[y].my_addr = (adf_u8)( lock[y].my_addr >> 8 );
    }

    for ( x = 0 ; x < sizeof( lock[y].p_addr ) ; x++ )
    {
      psp_spi_tx1( 0, (adf_u8)lock[y].p_addr );
      lock[y].p_addr = (adf_u8)( lock[y].p_addr >> 8 );
    }
  }

  SPI_CS_HI;
  adf_buf1_info.page = safe_page_addr;
  adf_buf1_info.changed = 1;
  if ( adf_program_buff_1() )
  {
    return 1;
  }

  return 0;
} /* adf_lock_page */


/*******************************************************************************
 *
 * adf_unlock_page
 *
 * This function will remove a lock page.
 *
 * INPUT
 *
 * safe_page_addr: address of the management page where the lock is programed
 *
 *******************************************************************************/
static adf_bool adf_unlock_page ( const adf_page_address_t safe_page_addr )
{
  adf_cmd( ADF_ERASE_PAGE, PGA2FLA( safe_page_addr ) );
  SPI_CS_HI;
  return adf_wait_ready();
}


/*******************************************************************************
 *
 * adf_check_lock
 *
 * Will read a "lock" page and check its contents to see if it is valid.
 *
 * INPUT
 *
 * safe_page_addr: address of the lock page
 *
 * OUTPUT
 *
 * lock: the lock data read from the flash
 * 0: if lock is not valid
 * 1: if page is valid.
 *
 *******************************************************************************/
static int adf_check_lock ( const adf_page_address_t safe_page_addr, adf_lock_data_t * lock )
{
  unsigned int   x, y;
  unsigned char  val;

  adf_cmd( ADF_READ, PGA2FLA( safe_page_addr ) );
  psp_spi_tx1( 0, 0xff );
  psp_spi_tx1( 0, 0xff );
  psp_spi_tx1( 0, 0xff );
  psp_spi_tx1( 0, 0xff );
  for ( y = 0 ; y < 2 ; y++ )
  {
    ( *lock )[y].my_addr = 0;
    ( *lock )[y].p_addr = 0;
    for ( x = 0 ; x < sizeof( ( *lock )[y].my_addr ) ; x++ )
    {
      psp_spi_rx( 0, &val, 1 );
      ( *lock )[y].my_addr |= ( (adf_page_address_t)val ) << ( x << 3 );
    }

    for ( x = 0 ; x < sizeof( ( *lock )[y].p_addr ) ; x++ )
    {
      psp_spi_rx( 0, &val, 1 );
      ( *lock )[y].p_addr |= ( (adf_page_address_t)val ) << ( x << 3 );
    }
  }

  SPI_CS_HI;

  x = 0;

  /* Is this a valid lock? */
  if ( ( ( *lock )[0].my_addr == ( adf_u16 ) ~( *lock )[1].my_addr )
      && ( ( *lock )[0].my_addr == safe_page_addr )
      && ( ( *lock )[0].p_addr == ( adf_u16 ) ~( *lock )[1].p_addr ) )
  {
    x = 1;
  }

  return (int)x;
} /* adf_check_lock */


/******************************************************************************
 *
 * find_bmp_page
 *
 * Finds the most recent bitmap page in the specified management sector based
 * on the stored sequence numbers.
 *
 * INPUT
 *
 * start_pg: start address of management area.
 * end_pg:   the next page after the last page of the management area
 * seq: pointer to seguence number variable taht will be used for the mgm area.
 * cpg: pointer to the variable that keeps info about which is the most recent bitmap page.
 *
 * OUTPUT
 *
 * 0: if all ok
 * 1: in case of any error
 *
 ******************************************************************************/
static adf_bool find_bmp_page ( adf_page_address_t start_pg, adf_page_address_t end_pg, adf_u16 * seq, adf_page_address_t * cpg )
{
  adf_lock_data_t     lock;
  adf_page_address_t  i;
  adf_u16             bigest_seq = 1u << 15;
  adf_u16             curr_seq = ( adf_u16 ) - 1;
  adf_page_address_t  curr_mgm_pg = 0;

  for ( i = start_pg ; i < end_pg ; i += 2 )
  {
    curr_seq++;

    /*lint -e{545} */
    if ( adf_check_lock( i, &lock ) )
    {
      if ( lock[0].p_addr & PAGE_ADDRESS_MSB )
      { /* If this is the log of a broken transaction,  */
        if ( lock[0].p_addr & ( PAGE_ADDRESS_MSB >> 1 ) )
        { /* We found a broken safe erase operation. */
          adf_cmd( ADF_ERASE_PAGE, PGA2FLA( lock[0].p_addr & ~( PAGE_ADDRESS_MSB | ( PAGE_ADDRESS_MSB >> 1 ) ) ) );
        }
        else
        { /* We found a broken safe write operation. */
          /* Resume the broken transaction. */
          if ( adf_transfer_buff_1( (adf_page_address_t)( i + 1 ) ) )
          {
            return 1;
          }

          adf_cmd( ADF_PROGERASE_BUF1, (adf_address_t)PGA2FLA( lock[0].p_addr & ~PAGE_ADDRESS_MSB ) );
        }

        SPI_CS_HI;
        if ( adf_wait_ready() )
        {
          return 1;
        }

        /* Log transaction success. */
        if ( adf_unlock_page( i ) )
        {
          return 1;
        }

        /* If we already found a valid bitmap sector, then we found the most recent mgm pagepair. */
        if ( ( bigest_seq & ( 1u << 15 ) ) == 0 )
        {
          break;
        }
      }
      else
      {                                               /* This is a lock of a bitmap sector. */
        if ( ( ( bigest_seq & ( 1u << 15 ) ) != 0 ) ) /* This is te first valid mgm page pair. */
        {
          curr_seq = (adf_u16)lock[0].p_addr;
          goto found_goodseq;
        }

        /* If it has the right seq. */
        if ( ( curr_seq & 0x7fff ) == (adf_u16)lock[0].p_addr )
        { /* then treat this page pair as the most recent one. */
found_goodseq:
          bigest_seq = (adf_u16)lock[0].p_addr;
          curr_mgm_pg = i;
        }
        else
        {
          /* A wrong sequence number means that we already hit the end of the seq. list and thus
             the most recent page pair is already found. */
          break;
        }
      }
    } /* endif: valid lock found.*/
  }

  if ( ( bigest_seq & 0x8000 ) == 0 )
  {
    *seq = bigest_seq;
    *cpg = curr_mgm_pg;
  }

  return 0;
} /* find_bmp_page */


/*******************************************************************************
 *
 * adf_recover
 *
 * This function will recover any interrupted safe writes or safe_erases.
 *
 * INPUT
 *
 * none
 *
 * OUTPUT
 *
 * none
 *
 *******************************************************************************/
static adf_bool adf_recover ( void )
{
  /******************* Find map page and load it into buffer 2. */
#if ADF_2_MAP_SECTOR

  /*lint -e{506, 648, 778 } */
  if ( find_bmp_page( ADF_MGM1_START, ADF_MGM1_END, &adf_map_lock_seq[0], &adf_curr_mgm_pg[0] ) )
  {
    return 1;
  }

  /*lint -e{506, 648, 778 } */
  if ( find_bmp_page( ADF_MGM2_START, ADF_MGM2_END, &adf_map_lock_seq[1], &adf_curr_mgm_pg[1] ) )
  {
    return 1;
  }

#else

  /*lint -e{506, 648, 778 } */
  if ( find_bmp_page( ADF_MGM_START, ADF_MGM_END, &adf_map_lock_seq[0], &adf_curr_mgm_pg[0] ) )
  {
    return 1;
  }

#endif /* if ADF_2_MAP_SECTOR */
  return 0;
} /* adf_recover */


/******************************************************************************
 *
 * inc_mgm_wr_ctr
 *
 * Increases the write counter of the management sector. Will do an update if
 * needed to avoid the 10000 problem.
 *
 * INPUT
 *
 * value: the amount of the increase
 *
 * OUTPUT
 *
 * 0: if all ok
 * 1: in case of an error
 *
 ******************************************************************************/
#if ADF_2_MAP_SECTOR
static adf_bool inc_mgm_wr_ctr ( adf_u8 value )
{
  adf_update_ctr[ADF_NUM_OF_SECTORS] += value;


  /* Worst case the counter will be updated after 6 writes. This means that the
    ctr may be less by 6 than the real value. */

  if ( adf_update_ctr[ADF_NUM_OF_SECTORS] > ( 10000 - 6 ) )
  {
    adf_page_address_t  next_pg;
    adf_u8              omgm = (adf_u8)OTHERMGMSEL( adf_mgm_sel );

    if ( adf_transfer_buff_2( (adf_page_address_t)( adf_curr_mgm_pg[omgm] + 1 ) ) )
    {
      return 1;
    }

    adf_update_ctr[ADF_NUM_OF_SECTORS] = 2;

    /*lint -e{506 } */
    next_pg = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( (adf_page_address_t)( adf_curr_mgm_pg[omgm] ) ) );

    if ( adf_program_buff_2( (adf_page_address_t)( next_pg + 1 ) ) )
    {
      return 1;
    }

    if ( adf_lock_page( next_pg, (adf_page_address_t)( ++adf_map_lock_seq[omgm] & 0x7fff ) ) )
    {
      return 1;
    }

    adf_mgm_sel = omgm;
    adf_curr_mgm_pg[adf_mgm_sel] = next_pg;
  }

  return 0;
} /* inc_mgm_wr_ctr */


/******************************************************************************
 *
 * adf_write_mgm
 *
 * Will write buffer2 to the management area, and will update neccessary global
 * variables.
 *
 * INPUT
 *
 * mgm_sel: A valu that selects the active management area.
 *
 * OUTPUT
 *
 * 0: if all ok
 * 1: in case of an error
 *
 ******************************************************************************/
static adf_u8 adf_write_mgm ( adf_u8 mgm_sel )
{
  adf_page_address_t  next_pg;

  /*lint -e{506 } */
  next_pg = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( adf_curr_mgm_pg[mgm_sel] ) );
  if ( adf_program_buff_2( (adf_page_address_t)( next_pg + 1 ) ) )
  {
    return 1;
  }

  if ( adf_lock_page( next_pg, (adf_page_address_t)( ++adf_map_lock_seq[mgm_sel] & 0x7fff ) ) )
  {
    return 1;
  }

  adf_curr_mgm_pg[mgm_sel] = next_pg;
  if ( inc_mgm_wr_ctr( 2 ) )
  {
    return 1;
  }

  return 0;
} /* adf_write_mgm */

#else /* if ADF_2_MAP_SECTOR */


/******************************************************************************
 *
 * adf_write_mgm
 *
 * Will write buffer2 to the management area, and will update neccessary global
 * variables.
 *
 * INPUT
 *
 * none.
 *
 * OUTPUT
 *
 * 0: if all ok
 * 1: in case of an error
 *
 ******************************************************************************/
static adf_u8 adf_write_mgm ( void )
{
  adf_page_address_t  next_pg;

  /*lint -e{506 } */
  next_pg = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( adf_curr_mgm_pg[0] ) );
  if ( adf_program_buff_2( (adf_page_address_t)( next_pg + 1 ) ) )
  {
    return 1;
  }

  if ( adf_lock_page( next_pg, (adf_page_address_t)( ++adf_map_lock_seq[0] & 0x7fff ) ) )
  {
    return 1;
  }

  adf_curr_mgm_pg[0] = next_pg;
  return 0;
} /* adf_write_mgm */
#endif /* if ADF_2_MAP_SECTOR */


/******************************************************************************
 *
 * update_bmp
 *
 * This function will update the "10000 bitmap" after a write operation.
 *
 * INPUT
 *
 * page: address of the updated page.
 *
 * OUTPUT
 *
 * none
 *
 ******************************************************************************/
static adf_bool update_bmp ( adf_page_address_t page )
{
  adf_byte_address_t  map_byte_offset;
  adf_u8              sector;
  unsigned char       bmp_byte;

#if ADF_2_MAP_SECTOR
  adf_u8  new_mgm_sel;
#endif

  /************** Some calculations */
  sector = (adf_u8)PAGE2SECTOR( page );

  /*lint -e{506} */
  map_byte_offset = PAGE2BYTEOFS( page );

#if ADF_2_MAP_SECTOR

  /***** Load right map page into buffer 2. */
  /* Determine which map page do we need. */
  new_mgm_sel = (unsigned char)( ( sector <= ADF_LAST_0BMP_SECTOR ) ? 0u : 1u );

  /* if the page in the buffer is not the right one, load the right one. */
  if ( adf_mgm_sel != new_mgm_sel )
  { /* First write the other page out. */
    if ( adf_write_mgm( adf_mgm_sel ) )
    {
      return 1;
    }

    if ( adf_transfer_buff_2( (adf_page_address_t)( adf_curr_mgm_pg[new_mgm_sel] + 1 ) ) )
    {
      return 1;
    }

    adf_update_ctr[ADF_NUM_OF_SECTORS] = 0;
    adf_mgm_sel = new_mgm_sel;
  }

#endif /* if ADF_2_MAP_SECTOR */

  /************** Update the flag of this page. */
  /* Load byte from buffer. */
  adf_cmd( ADF_READ_BUF2, map_byte_offset );
  psp_spi_tx1( 0, 0xff );
  psp_spi_rx( 0, &bmp_byte, 1 );
  SPI_CS_HI;

  /* Update bitmap only if the status of the bit would be changed. */
  if ( bmp_byte & ( 1u << PAGE2BITOFS( page ) ) )
  {
    bmp_byte &= ~( 1u << PAGE2BITOFS( page ) );

    /* Write back to buffer. */
    adf_cmd( ADF_WRITE_BUF2, map_byte_offset );
    psp_spi_tx1( 0, bmp_byte );
    SPI_CS_HI;
  }

  /************** Check if the update of another sector is due */
  /* If we need to update the next page in the sector.*/
  if ( adf_update_ctr[sector]++ >= ( ( ( 10000u / 2u ) / SECTORSIZE( sector ) ) - 1 ) )
  {
    unsigned char       val;
    adf_byte_address_t  bmp_ndx;
    unsigned char       x_bit;
    adf_page_address_t  pg_ndx;
    unsigned char       x;
    adf_byte_address_t  ctr_ofs;

    adf_update_ctr[sector] = 1;

    /* Read bitmap index for this sector. */
    ctr_ofs = BMPCTROFS( sector );
    adf_cmd( ADF_READ_BUF2, ctr_ofs );
    psp_spi_tx1( 0, 0xff );
    psp_spi_rx( 0, &val, 1 );
    bmp_ndx = val;
    SPI_CS_HI;

    /*lint -e{506} */
    map_byte_offset = PAGE2BYTEOFS( SECTOR2PAGE( sector ) ) + bmp_ndx;

    /* Read in the bitmap byte for this sector; */
    adf_cmd( ADF_READ_BUF2, map_byte_offset );
    psp_spi_tx1( 0, 0xff );
    psp_spi_rx( 0, &bmp_byte, 1 );
    SPI_CS_HI;

    /* Scan for the first bit set to one. Scan starts from LSB. */
    for ( x_bit = 1, x = 0 ; x_bit ; x_bit <<= 1, x++ )
    {
      if ( x_bit & bmp_byte )
      {
        break;
      }
    }

    /* Calculate to which page the bitmap bit is assigned. */
    /*lint -e{506} */
    pg_ndx = (adf_page_address_t)( BMPOFS2PAGE( map_byte_offset, adf_mgm_sel ) + x );

    /**************** First update the bitmap. */
    bmp_byte &= ~x_bit;

    /* If we need to go for the next byte in the bitmap. */
    if ( !x_bit || ( x_bit == 0x80 ) )
    {
      /* First clear the current bitmap byte. */
      bmp_byte = 0xff;

      /* Write new bitmap index to buffer. */
      adf_cmd( ADF_WRITE_BUF2, ctr_ofs );

      /* If this was the last page of the sector. */
      if ( sector != PAGE2SECTOR( pg_ndx + 1 ) )
      {
        psp_spi_tx1( 0, 0 );
      }
      else
      {
        psp_spi_tx1( 0, (adf_u8)( bmp_ndx + 1 ) );
      }

      SPI_CS_HI;
    }

    /* Write updated bitmap byte into the buffer. */

    adf_cmd( ADF_WRITE_BUF2, map_byte_offset );
    psp_spi_tx1( 0, bmp_byte );
    SPI_CS_HI;

    /* Burn buffer2 into the main flash area.*/
#if ADF_2_MAP_SECTOR
    if ( adf_write_mgm( adf_mgm_sel ) )
    {
      return 1;
    }

#else
    if ( adf_write_mgm() )
    {
      return 1;
    }

#endif

    /***************** Do the page update. */
    /* If we found a page to be updated. */
    if ( x_bit )
    {
      adf_address_t  addr;
      addr = PADE2ADDRESS( pg_ndx );

      /* If page is used by the filesystem, */
      if ( tiny_used_addr( addr ) )
      {
        adf_page_address_t  safe;

        /* Update with safe write. */
        if ( adf_transfer_buff_1( pg_ndx ) )
        {
          return 1;
        }

        /*lint -e{506} */
        safe = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( adf_curr_mgm_pg[adf_mgm_sel] ) );

        /* First write data into safe. */
        adf_cmd( ADF_PROGERASE_BUF1, PGA2FLA( safe + 1 ) );
        SPI_CS_HI;
        if ( adf_wait_ready() )
        {
          return 1;
        }

        /* Log transatcion start. */
        if ( adf_lock_page( safe, (adf_page_address_t)( PAGE_ADDRESS_MSB | pg_ndx ) ) )
        {
          return 1;
        }

        /* Do the transaction. */
        if ( adf_transfer_buff_1( (adf_page_address_t)( safe + 1 ) ) )
        {
          return 1;
        }

        adf_buf1_info.page = pg_ndx;
        adf_buf1_info.changed = 0;
        adf_cmd( ADF_PROGERASE_BUF1, PGA2FLA( pg_ndx ) );
        SPI_CS_HI;
        if ( adf_wait_ready() )
        {
          return 1;
        }

        /* Log transaction success. */
        if ( adf_unlock_page( safe ) )
        {
          return 1;
        }

#if ADF_2_MAP_SECTOR
        if ( inc_mgm_wr_ctr( 3 ) )
        {
          return 1;
        }

#endif
      }
    }
  }

  return 0;
} /* update_bmp */


/******************************************************************************
 *
 * format_mgm
 *
 * This function will format a management sector (write bitmap pages and locks to it).
 *
 * INPUT
 *
 * start_pg: Starting page of management area.
 * bmp_size: number of bitmap bytes needed.
 * *seq: pointer to seguence number variable taht will be used for the mgm area.
 * *cpg: pointer to the variable that keeps info about which is the most recent bitmap page.
 *
 * OUTPUT
 *
 * 0: if all ok
 * 1: in case of any error
 *
 ******************************************************************************/
static adf_bool format_mgm ( adf_page_address_t start_pg, adf_u16 bmp_size, adf_u16 * seq, adf_page_address_t * cpg )
{
  /* Create a cleared bitmap sector. */
  *seq = 0u;
  adf_cmd( ADF_WRITE_BUF2, 0 );
  {
    adf_byte_address_t  x;
    for ( x = 0 ; x < bmp_size ; x++ )
    {
      psp_spi_tx1( 0, 0xff );
    }

    for ( /*empty*/ ; x < ADF_PAGE_SIZE ; x++ )
    {
      psp_spi_tx1( 0, 0x0 );
    }
  }
  SPI_CS_HI;

  /* Burn the cleared bitmap sector where it is needed. */
  /*lint -e{506, 778} */
  adf_cmd( ADF_PROGERASE_BUF2, PGA2FLA( start_pg + 1 ) );
  SPI_CS_HI;
  if ( adf_wait_ready() )
  {
    return 1;
  }

  if ( adf_lock_page( (adf_page_address_t)start_pg, (adf_page_address_t)( ( ++*seq ) & 0x7fff ) ) )
  {
    return 1;
  }

  /*lint -e{506 } */
  if ( adf_unlock_page( (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( start_pg ) ) ) )
  {
    return 1;
  }

  *cpg = start_pg;
  adf_update_ctr[ADF_NUM_OF_SECTORS] = 2;
  return 0;
} /* format_mgm */






#if USE_ECC


/******************************************************************************
 * Read ECC value from RAM buffer
 *
 * RETURN: ECC value
 ******************************************************************************/
static t_ecc adf_read_ecc ( void )
{
  t_ecc          ecc;
  unsigned char  val;

  psp_spi_rx( 0, &val, 1 );
  ecc = val;
  ecc <<= 8;
  psp_spi_rx( 0, &val, 1 );
  ecc |= val;
  ecc <<= 8;
  psp_spi_rx( 0, &val, 1 );
  ecc |= val;
  ecc <<= 8;
  psp_spi_rx( 0, &val, 1 );
  ecc |= val;
  return ecc;
} /* adf_read_ecc */


/******************************************************************************
 * Write ECC value to RAM buffer
 *
 * INPUT: ecc - ECC value
 ******************************************************************************/
static void adf_write_ecc ( t_ecc ecc )
{
  psp_spi_tx1( 0, (adf_u8)( ecc >> 24 ) );
  psp_spi_tx1( 0, (adf_u8)( ( ecc >> 16 ) & 0xff ) );
  psp_spi_tx1( 0, (adf_u8)( ( ecc >> 8 ) & 0xff ) );
  psp_spi_tx1( 0, (adf_u8)( ecc & 0xff ) );
}


/******************************************************************************
 * Initialize ECC value for current page in DF RAM buffer.
 *
 * RETURN: 0 - ECC valid
 *         1 - invalid
 ******************************************************************************/
static adf_bool adf_calc_ecc ( void )
{
  adf_u32  n;
  t_ecc    ecc_calc, ecc_tmp;
  adf_u16  ecc_hi, ecc_lo;
  adf_u16  i;
  adf_u8   j, b, empty;

  if ( adf_buf1_info.ecc_flags & F_ECC_USE )    /* if current ECC belongs to the buffer in DF RAM */
  {
    if ( adf_buf1_info.ecc_flags & F_ECC_VALID )
    {
      return 0;                                         /* valid ECC */
    }
    else
    {
      return 1;                                         /* invalid ECC */
    }
  }

  adf_buf1_info.ecc_flags = F_ECC_USE;        /* set flag USE flag */
  n = 0xffff;
  ecc_calc = 0;
  empty = 1;
  adf_cmd( ADF_READ_BUF1, 0 );
  psp_spi_tx1( 0, 0xff );
  for ( i = 0 ; i < ADF_ECC_START ; i++ )
  {
    psp_spi_rx( 0, &b, 1 );
    if ( b != 0xff )
    {
      empty = 0;                    /* if read byte is other than 0xff then clear page empty flag */
    }

    for ( j = 0 ; j < 8 ; j++ )
    {
      if ( b & 1 )
      {
        ecc_calc ^= n;              /* calculate ecc */
      }

      n += 0xffff;
      b >>= 1;
    }
  }

  adf_buf1_info.ecc_value = adf_read_ecc();   /* save ECC value from DF RAM buffer */
  if ( adf_buf1_info.ecc_value != 0xffffffff )
  {
    empty = 0;                                      /* clear empty flag if other than 0xffffffff */
  }

  SPI_CS_HI;

  if ( empty )                    /* if page is empty */
  {
    adf_buf1_info.ecc_value = ecc_calc;       /* save calculated ECC for empty page for possible writes */
  }
  else if ( ecc_calc != adf_buf1_info.ecc_value )   /* if page is not empty */
  {
    ecc_tmp = ecc_calc ^ adf_buf1_info.ecc_value;
    ecc_hi = (adf_u16)( ( ecc_tmp >> 16 ) & 0xffff );
    ecc_lo = (adf_u16)( ecc_tmp & 0xffff );

    if ( ecc_hi + ecc_lo != 0xffff )          /* there is more than 1 bit error or the error is in the ECC */
    {
      if ( !( ecc_tmp & ( ecc_tmp - 1 ) ) )
      {
        adf_buf1_info.ecc_value = ecc_calc;                         /* error is in the ECC area */
      }
      else
      {
        return 1;                                       /* not possible to fix */
      }
    }
    else                    /* correctable error */
    {
      adf_cmd( ADF_READ_BUF1, ecc_hi >> 3 );
      psp_spi_tx1( 0, 0xff );
      psp_spi_rx( 0, &b, 1 );
      SPI_CS_HI;             /* get wrong value */

      b ^= ( 1 << ( ecc_hi & 7 ) );           /* fix error */

      adf_cmd( ADF_WRITE_BUF1, ecc_hi >> 3 );
      psp_spi_tx1( 0, b );
      SPI_CS_HI;             /* write back to buffer */
    }
  }

  adf_buf1_info.ecc_flags |= F_ECC_VALID;     /* ecc is valid */

  return 0;
} /* adf_calc_ecc */


/******************************************************************************
 * Clear part of ECC value in case part of the page needs to be updated
 *
 * INPUT: pos - position inside the page to clear ECC from
 *        len - number of bytes to remove
 ******************************************************************************/
static void adf_clear_ecc ( adf_byte_address_t pos, unsigned int len )
{
  adf_u32  n;
  adf_u8   j, b;

  n = ( (adf_u32)pos << 19 ) + ( 0xffff - ( pos << 3 ) );
  adf_cmd( ADF_READ_BUF1, pos );
  psp_spi_tx1( 0, 0xff );
  while ( len-- )
  {
    psp_spi_rx( 0, &b, 1 );
    for ( j = 0 ; j < 8 ; j++ )
    {
      if ( b & 1 )
      {
        adf_buf1_info.ecc_value ^= n;
      }

      n += 0xffff;
      b >>= 1;
    }
  }

  SPI_CS_HI;
} /* adf_clear_ecc */


/******************************************************************************
 * Generate new ECC value using page position and new value.
 *
 * INPUT: pos - position in the page
 *        b - new byte value
 ******************************************************************************/
static void adf_add_ecc ( adf_byte_address_t pos, const unsigned char * src, unsigned int len )
{
  adf_u32  n = ( (adf_u32)pos << 19 ) + ( 0xffff - ( pos << 3 ) );
  adf_u8   j, b;

  while ( len-- )
  {
    b = *src++;
    for ( j = 0 ; j < 8 ; j++ )
    {
      if ( b & 1 )
      {
        adf_buf1_info.ecc_value ^= n;
      }

      n += 0xffff;
      b >>= 1;
    }
  }
} /* adf_add_ecc */
#endif /* if USE_ECC */


/*****************************************************************************************************
*************************************** USER FUNCTIONS ***********************************************
*****************************************************************************************************/


/******************************************************************************
 *
 * adf_low_level_format
 *
 * This function will write default contents into the management area of the
 * flash chip. (Clean bitmap, etc...)
 *
 * INPUT
 *
 * none
 *
 * OUTPUT
 *
 * none
 *
 ******************************************************************************/
unsigned char adf_low_level_format ( void )
{
#if ADF_2_MAP_SECTOR

  /*lint -e{506, 648}  */
  if ( format_mgm( ADF_MGM1_START, PAGE2BYTEOFS( SECTOR2PAGE( ADF_LAST_0BMP_SECTOR + 1 ) - 1 ) + 1
                  , &adf_map_lock_seq[0], &adf_curr_mgm_pg[0] ) )
  {
    return 1;
  }

  /*lint -e{506 } */
  if ( format_mgm( ADF_MGM2_START, PAGE2BYTEOFS( ADF_REAL_PAGE_COUNT ), &adf_map_lock_seq[1]
                  , &adf_curr_mgm_pg[1] ) )
  {
    return 1;
  }

  adf_mgm_sel = 1;
#else /* if ADF_2_MAP_SECTOR */

  /*lint -e{506 } */
  if ( format_mgm( ADF_MGM_START, PAGE2BYTEOFS( ADF_REAL_PAGE_COUNT ), &adf_map_lock_seq[0]
                  , &adf_curr_mgm_pg[0] ) )
  {
    return 1;
  }

#endif /* if ADF_2_MAP_SECTOR */
  return 0;
} /* adf_low_level_format */



/****************************************************************************
 *
 * adf_init
 *
 * Initializes the driver.
 *
 * INPUT:
 *
 * none
 *
 * OUTPUT:
 *
 * 0 if all ok
 * 1 othervise
 *
 ***************************************************************************/
unsigned char adf_init  ( void )
{
  unsigned char  ch;
  adf_u8         sector;

  /* Initialize the SPI. */
  psp_spi_init( 0 );
  psp_spi_start( 0 );
  psp_spi_set_baudrate( 0, 20000000 );

  /* Read and check the type of the FLASH chip. */
  SPI_CS_LO;
  psp_spi_tx1( 0, ADF_STATUS );
  psp_spi_rx( 0, &ch, 1 );
  SPI_CS_HI;

  if ( ( ch & ( 0xfu << 2 ) ) != ADF_ID )
  {
    return 1;
  }


  /* If the MCU was reset and the flash chip not, it may be busy. So wait till it
     finishes. */
  if ( adf_wait_ready() )
  {
    return 1;
  }

  /* initialize global variables with default values. */
  adf_buf1_info.page = (adf_page_address_t)-1;
  adf_buf1_info.changed = 0;

  /* The first access to any sector will cause a bitmap update. */
  for ( sector = 0 ; sector < ( sizeof( adf_update_ctr ) / sizeof( adf_update_ctr[0] ) ) ; sector++ )
  {
    adf_update_ctr[sector] = ( adf_u8 ) - 1;
  }

  /* These will be set by recover too, but anyway we set them to a known value. */
#if ADF_2_MAP_SECTOR

  /*lint -e{506, 648} */
  adf_curr_mgm_pg[1] = ADF_MGM_END / 2;
  adf_map_lock_seq[1] = 0u;
#endif

  /*lint -e{506, 778} */
  adf_curr_mgm_pg[0] = ADF_MGM_START;
  adf_map_lock_seq[0] = 0u;

  ch = adf_recover();
  return ch;
} /* adf_init */


/****************************************************************************
 *
 * adf_read
 *
 * Will read size bytes from the flash chip into the buffer pointed by
 * ramaddr.
 *
 * INPUT:
 *
 * ramaddr: pointer to destination buffer
 * flashaddr: source address
 * size: number of bytes to read
 *
 * OUTPUT:
 *
 * 0: if all ok
 * 1: othervise
 ***************************************************************************/
#if USE_ECC
unsigned char adf_read  ( void * ramaddr, adf_address_t flashaddr, unsigned int size, unsigned char use_ecc )
#else
unsigned char adf_read  ( void * ramaddr, adf_address_t flashaddr, unsigned int size )
#endif
{
  adf_page_address_t  page_addr;
  adf_byte_address_t  byte_addr;
  adf_byte_address_t  read;
  unsigned char     * dst = (unsigned char *)ramaddr;

  page_addr = (adf_page_address_t)( flashaddr / ADF_PAGE_SIZE );
  byte_addr = (adf_byte_address_t)( flashaddr - ( page_addr * ADF_PAGE_SIZE ) );

  while ( size )
  {
    /* Calculate how many bytes can we read from the current page. */
    read = ADF_PAGE_SIZE - byte_addr;
    if ( read > size )
    {
      read = size;
    }

    /* Read in the current page into buffer one. */
    if ( adf_transfer_buff_1( page_addr ) )
    {
      return 1;
    }

#if USE_ECC
    if ( use_ecc )
    {
      if ( adf_calc_ecc() )
      {
        return 1;                   /* ECC error */
      }
    }

#endif

    /* Read the needed bytes from buffer one. */
    adf_cmd( ADF_READ_BUF1, byte_addr );
    psp_spi_tx1( 0, 0xff );
    psp_spi_rx( 0, dst, read );
    SPI_CS_HI;

    /* Advance further. */
    dst += read;
    size -= read;
    ++page_addr;
    byte_addr = 0;
  }

  return 0;
} /* adf_read */


/****************************************************************************
*
* adf_erase
*
* Erase one or more pages. NOTE: size shall be the intheger multiple of the
* page size. If size specifyes a partial page, the whole page will be erased.
*
* INPUT:
*
* flashaddr: address that selects the first page to be erased
* size: size in bytes.
*
****************************************************************************/
unsigned char adf_erase ( adf_address_t flashaddr, long size )
{
  adf_page_address_t  page_addr;

  page_addr = (adf_page_address_t)( flashaddr / ADF_PAGE_SIZE );

  if ( adf_program_buff_1() )
  {
    return 1;
  }

  while ( size )
  {
    if ( page_addr == adf_buf1_info.page )
    {
      adf_buf1_info.page = (adf_page_address_t)-1;
    }

    adf_cmd( ADF_ERASE_PAGE, PGA2FLA( page_addr ) );
    SPI_CS_HI;
    if ( adf_wait_ready() )
    {
      return 1;
    }

    if ( update_bmp( page_addr ) )
    {
      return 1;
    }

    size -= ADF_PAGE_SIZE;
    page_addr++;
  }

  return 0;
} /* adf_erase */


/****************************************************************************
*
* adf_erase_safe
*
* Erase one page. If the eare is interrupted by a power failure, the driver
* will resume the operation at the next startup. Note: for any value of
* size except 0 the function will erase only one page. If size is zero
* nothing will be done.
*
* INPUT:
*
* flashaddr: address that selects the page to be erased
* size: ignored. The function will erase one full page.
*
 ***************************************************************************/
unsigned char adf_erase_safe ( adf_address_t flashaddr, unsigned int size )
{
  adf_page_address_t  page_addr;
  adf_page_address_t  safe;

  (void)size;
  page_addr = (adf_page_address_t)( flashaddr / ADF_PAGE_SIZE );

  /* First save data from buf1. */
  if ( adf_program_buff_1() )
  {
    return 1;
  }

  /*lint -e{506} */
  safe = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( adf_curr_mgm_pg[adf_mgm_sel] ) );
  if ( adf_lock_page( safe, (adf_page_address_t)( PAGE_ADDRESS_MSB | ( PAGE_ADDRESS_MSB >> 1 ) | page_addr ) ) )
  {
    return 1;
  }

  if ( page_addr == adf_buf1_info.page )
  {
    adf_buf1_info.page = (adf_page_address_t)-1;
  }

  adf_cmd( ADF_ERASE_PAGE, PGA2FLA( page_addr ) );
  SPI_CS_HI;
  if ( adf_wait_ready() )
  {
    return 1;
  }

  if ( update_bmp( page_addr ) )
  {
    return 1;
  }

  if ( adf_unlock_page( safe ) )
  {
    return 1;
  }

#if ADF_2_MAP_SECTOR
  if ( inc_mgm_wr_ctr( 2 ) )
  {
    return 1;
  }

#endif
  return 0;
} /* adf_erase_safe */


/****************************************************************************
 *
 * adf_write
 *
 * Will write size bytes to the flash chip from the buffer pointed by
 * ramaddr.
 *
 * INPUT:
 *
 * ramaddr: pointer to source buffer
 * flashaddr: destination address
 * size: number of bytes to write
 *
 * OUTPUT:
 *
 * 0: if all ok
 * 1: othervise
 ***************************************************************************/
#if USE_ECC
unsigned char adf_write ( adf_address_t flashaddr, _PTRQ void * ramaddr, unsigned int size, unsigned char use_ecc )
#else
unsigned char adf_write ( adf_address_t flashaddr, _PTRQ void * ramaddr, unsigned int size )
#endif
{
  adf_page_address_t    page_addr;
  adf_byte_address_t    byte_addr;
  adf_byte_address_t    write;
  _PTRQ unsigned char * src = (_PTRQ unsigned char *)ramaddr;

  page_addr = (adf_page_address_t)( flashaddr / ADF_PAGE_SIZE );
  byte_addr = (adf_byte_address_t)( flashaddr - ( page_addr * ADF_PAGE_SIZE ) );

  while ( size )
  {
    /* Build new data block in buffer one. */


    /* If buffer does not contains the contents of the page to be modified
       reda the page in. */
    if ( adf_transfer_buff_1( page_addr ) )
    {
      return 1;
    }

#if USE_ECC
    if ( use_ecc )
    {
      if ( adf_calc_ecc() )
      {
        return 1;                   /* ECC error */
      }
    }

#endif

    /* Overwrite data in the buffer */
    write = ADF_PAGE_SIZE - byte_addr;
    if ( write > size )
    {
      write = size;
    }

#if USE_ECC
    if ( use_ecc )
    {
      adf_clear_ecc( byte_addr, write );
      adf_add_ecc( byte_addr, src, write );
    }

#endif
    adf_cmd( ADF_WRITE_BUF1, byte_addr );
    psp_spi_tx( 0, src, write );
    SPI_CS_HI;
    adf_buf1_info.changed = 1;

    src += write;
    size -= write;
    ++page_addr;
    byte_addr = 0;
  }

  return 0;
} /* adf_write */


/*******************************************************************************
 *
 * adf_write_safe
 *
 * Will write size bytes to the flash chip from the buffer pointed by
 * ramaddr. only one pagemay be accessed at the same time.
 * Automatically uses ECC if it is enabled, because write safe will always be
 * called for ECC protected pages.
 *
 * INPUT:
 *
 * ramaddr: pointer to source buffer
 * flashaddr: destination address
 * size: number of bytes to write
 *
 * OUTPUT:
 *
 * 0: if all ok
 * 1: othervise
 *******************************************************************************/
unsigned char adf_write_safe ( adf_address_t flashaddr, _PTRQ void * ramaddr, unsigned int size )
{
  adf_page_address_t          page_addr;
  adf_byte_address_t          byte_addr;
  const _PTRQ unsigned char * src = (_PTRQ unsigned char *)ramaddr;

  page_addr = (adf_page_address_t)( flashaddr / ADF_PAGE_SIZE );
  byte_addr = (adf_byte_address_t)( flashaddr - ( page_addr * ADF_PAGE_SIZE ) );

  if ( size )
  {
    unsigned int        i;
    adf_page_address_t  safe;

    /* Read the page to be modified into buf 1. */
    if ( adf_transfer_buff_1( page_addr ) )
    {
      return 1;
    }

#if USE_ECC
    if ( adf_calc_ecc() )
    {
      return 1;                     /* ECC error */
    }

    adf_clear_ecc( byte_addr, size );
    adf_add_ecc( byte_addr, src, size );
#endif

    /* Write data to buf 1 */
    adf_cmd( ADF_WRITE_BUF1, byte_addr );
    for ( i = 0 ; i < size ; i++ )
    {
      psp_spi_tx1( 0, *src++ );
    }

    SPI_CS_HI;

    /* Program buffer one into the FLASH.*/
    /* First write data into safe. */
    /*lint -e{506} */
    safe = (adf_page_address_t)( ADF_NEXT_MGM_BUFFER( adf_curr_mgm_pg[adf_mgm_sel] ) );
    adf_buf1_info.page = (adf_page_address_t)( safe + 1 );
    adf_buf1_info.changed = 1;
    if ( adf_program_buff_1() )
    {
      return 1;
    }

    /* Log transatcion start. */
    if ( adf_lock_page( safe, (adf_page_address_t)( PAGE_ADDRESS_MSB | page_addr ) ) )
    {
      return 1;
    }

    /* Do the transaction. */
    if ( adf_transfer_buff_1( (adf_page_address_t)( safe + 1 ) ) )
    {
      return 1;
    }

    adf_buf1_info.page = page_addr;
    adf_buf1_info.changed = 0;
    adf_cmd( ADF_PROGERASE_BUF1, PGA2FLA( page_addr ) );
    SPI_CS_HI;
    if ( adf_wait_ready() )
    {
      return 1;
    }

    /* Log transaction success. */
    /* TODO: update_bmp may already clear the lock, so this second erase may be unneccesary. */
    if ( adf_unlock_page( safe ) )
    {
      return 1;
    }

    if ( update_bmp( page_addr ) )
    {
      return 1;
    }

#if ADF_2_MAP_SECTOR
    if ( inc_mgm_wr_ctr( 3 ) )
    {
      return 1;
    }

#endif
  }

  return 0;
} /* adf_write_safe */


/****************************************************************************
 *
 * adf_copy
 *
 * Will copy whole pages from the src to the destination address. NOTE: size
 * shall be the integer multiple of the page size. Less that page size will
 * nothing, more than it will copy an additional whole page.
 *
 * INPUT:
 *
 * dstflashaddr: address where the data will be copied.
 * srcflashaddr: address from where where the data will be copied.
 * size: number of bytes to copy. NOTE: Must be integer multiple of the page
 *       size.
 *
 * OUTPUT:
 *
 * 0: if all ok
 * 1: othervise
 ***************************************************************************/
unsigned char adf_copy ( adf_address_t dstflashaddr, adf_address_t srcflashaddr, unsigned int size )
{
  adf_page_address_t  src_page_addr;
  adf_page_address_t  dst_page_addr;

  src_page_addr = (adf_page_address_t)( srcflashaddr / ADF_PAGE_SIZE );
  dst_page_addr = (adf_page_address_t)( dstflashaddr / ADF_PAGE_SIZE );

  if ( adf_buf1_info.page == src_page_addr )
  {
    if ( adf_program_buff_1() )
    {
      return 1;
    }
  }

  while ( size )
  {
    if ( adf_transfer_buff_1( src_page_addr ) )
    {
      return 1;
    }

    adf_buf1_info.page = dst_page_addr;
    adf_buf1_info.changed = 1;

    size -= ADF_PAGE_SIZE;
    ++src_page_addr;
    ++dst_page_addr;
  }

  return 0;
} /* adf_copy */


