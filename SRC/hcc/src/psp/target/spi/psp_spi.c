/***************************************************************************
 *
 *            Copyright (c) 2011-2013 by HCC Embedded
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
#include "../../include/psp_types.h"
#include "../../include/psp_spi.h"

#include "uart0.h"

#include "efm32.h"

#include "../../../version/ver_psp_spi.h"
#if VER_PSP_SPI_MAJOR != 2 || VER_PSP_SPI_MINOR != 3
 #error Incompatible PSP_SPI version number!
#endif

static uint32_t  g_baudrate;     /* baudrate */
static uint16_t  g_baudrate_div; /* baudrate divider */


/*
** psp_spi_tx1
**
** Transmit 1 byte
**
** Input:
**   uid - unit ID
**   val - value to send
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_tx1 ( uint8_t uid, uint8_t val )
{
  volatile uint8_t  dummy;

  (void)uid;

  USART0->TXDATA = val;
  while ( !( USART0->STATUS & USART_STATUS_TXC ) )
  {
  }

  dummy = USART0->RXDATA;

  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_tx
**
** Transmit 'len' number of bytes
**
** Input:
**   uid   - unit ID
**   p_src - pointer to source buffer
**   len   - number of bytes to transmit
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_tx ( uint8_t uid, uint8_t * p_src, uint32_t len )
{
  uint32_t  idx;

  idx = 0;
  while ( idx < len )
  {
    psp_spi_tx1( uid, p_src[idx] );
    ++idx;
  }

  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_txrx
**
** transmit and receive 'len' number of bytes
**
** Input:
**   uid   - unit ID
**   p_src - pointer to source buffer
**   len   - number of bytes to receive
** Output:
**   p_dst - pointer to destination buffer
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_txrx ( uint8_t uid, uint8_t * p_src, uint8_t * p_dst, uint32_t len )
{
  uint32_t  idx;

  (void)uid;

  idx = 0;
  while ( idx < len )
  {
    USART0->TXDATA = p_src[idx];
    while ( !( USART0->STATUS & USART_STATUS_TXC ) )
    {
    }

    if ( p_dst != NULL )
    {
      p_dst[idx] = USART0->RXDATA;
    }

    ++idx;
  }

  return PSP_SPI_SUCCESS;
} /* psp_spi_txrx */


/*
** psp_spi_rx
**
** receive 'len' number of bytes
**
** Input:
**   uid   - unit ID
**   p_dst - pointer to destination buffer
**   len   - number of bytes to receive
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_rx ( uint8_t uid, uint8_t * p_dst, uint32_t len )
{
  uint32_t  idx;

  (void)uid;

  idx = 0;
  while ( idx < len )
  {
    USART0->TXDATA = 0xFFu; /* send dummy byte */
    while ( !( USART0->STATUS & USART_STATUS_TXC ) )
    {
    }

    if ( p_dst != NULL )
    {
      p_dst[idx] = USART0->RXDATA;
    }

    ++idx;
  }

  return PSP_SPI_SUCCESS;
} /* psp_spi_rx */


/*
** psp_spi_cs_lo
**
** Set chip select low.
**
** Input:
**   uid - unit ID
*/
void psp_spi_cs_lo ( uint8_t uid )
{
  GPIO->P[2].DOUTCLR = 0x100;  /* PC8 is CS */
}


/*
** psp_spi_cs_hi
**
** Set chip select high.
**
** Input:
**   uid - unit ID
*/
void psp_spi_cs_hi ( uint8_t uid )
{
  GPIO->P[2].DOUTSET = 0x100; /* PC8 is CS */
}


/*
** psp_spi_lock
**
** Lock the SPI for the specific unit. This can
** be useful if multiple units are attached to the
** same SPI bus.
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_lock ( uint8_t uid )
{
  acquireUART0();  /* get USART0 mutex, disconnect module from UART functionality */
  psp_spi_cs_hi( uid );

  USART0->CLKDIV = g_baudrate_div;
  USART0->CMD = USART_CMD_RXDIS | USART_CMD_TXDIS | USART_CMD_MASTERDIS;
  USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  USART0->CTRL = USART_CTRL_TXDELAY_SINGLE | USART_CTRL_SYNC | USART_CTRL_MSBF | USART_CTRL_CLKPOL | USART_CTRL_CLKPHA;
  USART0->ROUTE = USART_ROUTE_LOCATION_LOC2 | USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN;
  USART0->FRAME = USART_FRAME_DATABITS_EIGHT;
  USART0->CMD = USART_CMD_MASTEREN;
  USART0->CMD = USART_CMD_TXEN | USART_CMD_RXEN;

  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_unlock
**
** Unlock the SPI for the specific unit. This can
** be useful if multiple units are attached to the
** same SPI bus.
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_unlock ( uint8_t uid )
{
  releaseUART0();

  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_set_baudrate
**
** Set baudrate.
**
** Input:
**   uid - unit ID
**   br  - baudrate in Hz
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_set_baudrate ( uint8_t uid, uint32_t br )
{
  uint32_t  freq;

  (void)uid;

  g_baudrate = br; /* store the baud rate value. */

  freq = SystemHFClockGet();

  /* In SPI master mode the maximal baud rate if the half of HFCLK. */
  if ( g_baudrate > ( freq >> 1 ) )
  {
    g_baudrate = ( freq >> 1 );
  }

  /* Formula from reference manual: USARTn_CLKDIV = 256 x (fHFPERCLK/(2 x brdesired) - 1) */
  g_baudrate_div = ( ( ( freq / ( 2 * g_baudrate ) ) - 1u ) << 8 );

  /* Store actual baudrate */
  g_baudrate = freq / ( 2 * ( 1u + ( g_baudrate_div >> 8 ) ) );

  return PSP_SPI_SUCCESS;
} /* psp_spi_set_baudrate */


/*
** psp_spi_get_baudrate
**
** Get baudrate.
**
** Input:
**   uid  - unit ID
** Output:
**   p_br - baudrate in Hz
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_get_baudrate ( uint8_t uid, uint32_t * p_br )
{
  *p_br = g_baudrate;

  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_init
**
** Init SPI port
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_init ( uint8_t uid )
{
  psp_spi_cs_hi( 0 );

  CMU->HFPERCLKEN0 |= ( CMU_HFPERCLKEN0_GPIO | CMU_HFPERCLKEN0_USART0 );

  /* Pin PC8 (CS) is configured to Push-pull */
  GPIO->P[2].MODEH = ( GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE8_MASK ) | GPIO_P_MODEH_MODE8_PUSHPULL;

  /* Pin PC9 (SCK) is configured to Push-pull */
  GPIO->P[2].MODEH = ( GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE9_MASK ) | GPIO_P_MODEH_MODE9_PUSHPULL;

  /* Pin PC10 (MISO) is configured to Input enabled */
  GPIO->P[2].MODEH = ( GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE10_MASK ) | GPIO_P_MODEH_MODE10_INPUT;

  /* To avoid false start, configure output US0_TX as high on PC11 */
  GPIO->P[2].DOUT |= ( 1 << 11 );

  /* Pin PC11 (MOSI) is configured to Push-pull */
  GPIO->P[2].MODEH = ( GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE11_MASK ) | GPIO_P_MODEH_MODE11_PUSHPULL;

  /* Pin PC8 (CS) is configured to Push-pull */
  GPIO->P[2].MODEH = ( GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE8_MASK ) | GPIO_P_MODEH_MODE8_PUSHPULL;

  return PSP_SPI_SUCCESS;
} /* psp_spi_init */


/*
** psp_spi_start
**
** Start SPI port
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_start ( uint8_t uid )
{
  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_stop
**
** Stop SPI port
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_stop ( uint8_t uid )
{
  return PSP_SPI_SUCCESS;
}


/*
** psp_spi_delete
**
** Delete SPI port
**
** Input:
**   uid - unit ID
** Return:
**   PSP_SPI_...
*/
t_spi_ret psp_spi_delete ( uint8_t uid )
{
  return PSP_SPI_SUCCESS;
}


