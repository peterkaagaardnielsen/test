/***************************************************************************
 *
 *            Copyright (c) 2011-2012 by HCC Embedded
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
#ifndef _PSP_SPI_H_
#define _PSP_SPI_H_

#include "../../version/ver_psp_spi.h"
#if VER_PSP_SPI_MAJOR != 2 || VER_PSP_SPI_MINOR != 3
 #error Incompatible PSP_SPI version number!
#endif

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define PSP_SPI_SUCCESS 0
#define PSP_SPI_ERROR   1

typedef int  t_spi_ret;

t_spi_ret psp_spi_tx1 ( uint8_t uid, uint8_t val );
t_spi_ret psp_spi_tx ( uint8_t uid, uint8_t * p_src, uint32_t len );
t_spi_ret psp_spi_txrx ( uint8_t uid, uint8_t * p_src, uint8_t * p_dst, uint32_t len );
t_spi_ret psp_spi_rx ( uint8_t uid, uint8_t * p_dst, uint32_t len );
void psp_spi_cs_lo ( uint8_t uid );
void psp_spi_cs_hi ( uint8_t uid );
t_spi_ret psp_spi_lock ( uint8_t unit );
t_spi_ret psp_spi_unlock ( uint8_t unit );
t_spi_ret psp_spi_set_baudrate ( uint8_t uid, uint32_t br );
t_spi_ret psp_spi_get_baudrate ( uint8_t uid, uint32_t * p_br );
t_spi_ret psp_spi_init ( uint8_t uid );
t_spi_ret psp_spi_start ( uint8_t uid );
t_spi_ret psp_spi_stop ( uint8_t uid );
t_spi_ret psp_spi_delete ( uint8_t uid );



#ifdef __cplusplus
}
#endif

#endif /* _PSP_SPI_H_ */

