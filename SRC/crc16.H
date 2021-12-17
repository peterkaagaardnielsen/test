#ifndef __CRC16_H__
#define __CRC16_H__

#include "stdbool.h"

#define CRC_INIT 0xFFFF    
   
unsigned short calc_crc16(unsigned short cur_crc,unsigned char val);


#endif //__CRC16_H__
