//=============================================================================
// DESP.c                                                          CHG 20071113
//
//=============================================================================
//
// Keyfile is 4 KB long
// It's divided in 4 equal chunks, each 1024 bytes long. The first chunc is encoded very simple, the 3 upper chunks are first XOR coded and then xtea encrypted with seperate keys.
// The 4 sections are decoded, and then compared to see if they are equal, thats the "level" field that holds the info, 0x07 if all are correct.
// 
// 
// Field    Length  Position
// Cookie        4  0x0000..0x0003 (0xCAFEB00B)
// IMEI         30  0x0004..0x0021 (IMEI, 1 dummy byte between the 15 digits)
// CellInfo    256  0x0022..0x0121 (Cellinfo, 16 entries, each 16 bytes: MCC, MNC, LAC, CI)
// Misc data    64  0x0122..0x0161 (misc data, 64 bytes)
// 
// 
// 
// 
// 
//-----------------------------------------------------------------------------

//#define _DEBUG_


#include "System.h"
#include "FW.h"
#include "Debug.h"
#include "hcc/src/api/api_tiny.h"
#include "IAP.h"
#include "GSM.h"
//#include "gps.h"
#include "DESP.h"
#include "RTC.h"
//#include "diagnose.h"
//#include "RTXMonitor.h"




static unsigned char *rawdata=((unsigned char*)DEVICE_KEYFILE);
static unsigned char desp_buffer[4096];//__attribute__ ((section ("SDRAM")));

// The cryptmsg must be '0' terminated (length can be anything)
static unsigned char despcryptmsg[]="gz!8%&=21&/%%/&#6754+&?&//)(=";
static tdefDESP desp;
static tdefDevProfile devProfile;
static int despvalid=FALSE;
static char userDeviceID[10+1];

//-----------------------------------------------------------------------------
// Encode/Decode a chunk of memory.
// Set init to true on the first chunk of memory to be en/decoded, false on next chunk
// (If en/decode one large piece of memory, just let init be true on the single call)
//-----------------------------------------------------------------------------
static void despencode(unsigned char* data,int length, int init) {
  static unsigned char* cur;
  static unsigned char* cc;

  if (init) {
    cc=despcryptmsg;
  }

  for (cur=data;cur<&data[length];cur++) {
    *cur = ~*cur; 
    *cur=*cur ^ *cc++;
    if (!*cc) cc=despcryptmsg;
  }
}


#define delta 0x9E3779B9
                                                                    // Chunc 0 is only "lightly" encoded ;-)
static const unsigned int k0[4] = {0x9F4E, 0x3E5D,0x0903,0x0509};		//XTEA key for chunc 1
static const unsigned int k1[4] = {0x3E5D, 0x9F4E,0x0509,0x0903};		//XTEA key for chunc 2
static const unsigned int k2[4] = {0xDEAD, 0xFACE,0xDEAD,0xBABE};		//XTEA key for chunc 3

//********************************************************************************************
// XTEA decrypt
//********************************************************************************************
static void decrypt(unsigned int * v,unsigned int * w, const unsigned int* k) {
  unsigned int y,z,n,sum;
  y=v[0];
  z=v[1];
  sum=0xC6EF3720;
  n=32;

   /* sum = delta<<5, in general sum = delta * n */

   while(n-->0)
      {
      z -= (y << 4 ^ y >> 5) + y ^ sum + k[sum>>11 & 3];
      sum -= delta;
      y -= (z << 4 ^ z >> 5) + z ^ sum + k[sum&3];
      }
   
   w[0]=y; w[1]=z;
}

//-----------------------------------------------------------------------------
// getDeviceIDDESP()
// return deviceid (last 10 digits of IMEI in desp record
//-----------------------------------------------------------------------------
int getDeviceIDDESP(char *deviceID) {
  if (userDeviceID[0]) {
    strcpy(deviceID, userDeviceID);
    return 2;
  }
 
  if (despvalid==FALSE) {
    strcpy(deviceID, "1234567890");
    return 1;
  } else {
    // If the devProfile data is valid, use the serial number defined..
    if (devProfile.cookie == DEVPROFILE_COOKIE && strlen(devProfile.serial))  
      strcpy(deviceID, devProfile.serial);
    // If devProfile data NOT valid, use the last 10 digits of the IMEI from the GSM module
    else
      strcpy(deviceID, &desp.IMEI[5]); 
    return 0;
  }
}

//-----------------------------------------------------------------------------
// checkLevelDESP()
// Check the security level of the DESP record
// Return:
//        0x10 invalid cookie (image is not valid)
//        Bitmap of security level, Bit 0 is level 1, Bit 1 is level 2, bit 2 is level 3
//-----------------------------------------------------------------------------
int checkLevelDESP(int dumpdata) {
  int i, valid=0;
  unsigned short *pRaw;
  int base;

  // Make a copy of the data in flash to some RAM (as we need to decode in-place)
  memcpy(desp_buffer, rawdata, sizeof(desp_buffer));

  // Check the cookie value
  if (*(int*)desp_buffer!=DESP_COOKIE) {
    return 0x10;
  }

  // Past the cookie..
  base=4;

  // First do an XOR decoding of the 3 "upper" sections
  despencode(&desp_buffer[1024], 3072, TRUE);
  // XTEA decrypt the 3 "upper" sections
  for (i=0; i<128; i++)
    decrypt((unsigned int *)&desp_buffer[1024+(i*8)],(unsigned int *)&desp_buffer[1024+(i*8)],k0);   
  for (i=0; i<128; i++)
    decrypt((unsigned int *)&desp_buffer[2048+(i*8)],(unsigned int *)&desp_buffer[2048+(i*8)],k1);   
  for (i=0; i<128; i++)
    decrypt((unsigned int *)&desp_buffer[3072+(i*8)],(unsigned int *)&desp_buffer[3072+(i*8)],k2);   


  // Get the imei from the first 30 chars in the lowest section (one dummy byte between each digit), and invert each digit..
  for (i=0; i<15; i++) 
    desp.IMEI[i]=~desp_buffer[base+(i*2)];
  // Make a zero terminator
  desp.IMEI[15]='\0';
  
  // We are now past the Cookie and IMEI fields  
  base += (15*2);

  // Get the cellinfo, there is a ushort random value between each member of the struct that we just discard
  pRaw=(unsigned short*)&desp_buffer[base];

  for (i=0; i<16; i++) {
    desp.CellInfo[i].MCC=*pRaw++;
    pRaw++;
    desp.CellInfo[i].MNC=*pRaw++;
    pRaw++;
    desp.CellInfo[i].LAC=*pRaw++;
    pRaw++;
    desp.CellInfo[i].CI=*pRaw++;
    pRaw++;
  }

  // Move past Cookie, IMEI and Cell info information
  base += (16*16);
  
  // Do an xor coding of the (up to) 64 bytes misc data
  despencode(&desp_buffer[base], 64, TRUE);

  // Decode the misc data in the 3 last sections (we need to do that in order to do the 3 memcmp at the end)
  despencode(&desp_buffer[base+1024], 64, TRUE);
  despencode(&desp_buffer[base+2048], 64, TRUE);
  despencode(&desp_buffer[base+3072], 64, TRUE);

  // Copy the misc data from the first section
  memcpy(desp.MiscData, &desp_buffer[base] , sizeof(desp.MiscData));

  // Move to the end of all the data (cookie, imei, cellinfo and misc data) 
  // base becomes the length of the real data..
  base += 64;

  // Do an xor coding of the devProfile data
  despencode(&desp_buffer[base], 256, TRUE);

  // Decode the devProfile data in the 3 last sections (we need to do that in order to do the 3 memcmp at the end)
  despencode(&desp_buffer[base+1024], 256, TRUE);
  despencode(&desp_buffer[base+2048], 256, TRUE);
  despencode(&desp_buffer[base+3072], 256, TRUE);
 
  // Copy the devProfile data from the first section (the cookie value will reveal if the data are correct
  memcpy(&devProfile, &desp_buffer[base] , sizeof(devProfile));
  // If there is devProfile data available, then adjust the base accordingly, else we ignore the 256 bytes (old keyfile format)
  if (devProfile.cookie == DEVPROFILE_COOKIE) {
    // Move to the end of the devProfile data (cookie, serial number etc) 
    // base becomes the length of the real data..
    base += 256;
    MESSAGEDEBUG(DBG_INFO, __MODULE__, __LINE__, "devProfile data: Serial=<%s>, decryptKey=<%s>",devProfile.serial, devProfile.decryptKey);             

  } else {
    MESSAGEDEBUG(DBG_INFO, __MODULE__, __LINE__, "devProfile data not defined in keyfile data",NULL);             
  }

  // Check all 3 copies of the data to see if they match the "level 0" data..
  if (memcmp(&desp_buffer[0], &desp_buffer[1024], base)==0)
    valid |= 0x01;
  if (memcmp(&desp_buffer[0], &desp_buffer[2048], base)==0)
    valid |= 0x02;
  if (memcmp(&desp_buffer[0], &desp_buffer[3072], base)==0)
    valid |= 0x04;

  desp.Level=valid;

  if (dumpdata) {
    //MESSAGEDEBUG(DBG_INFO, __MODULE__, __LINE__,"IMEI=%s, level=%02X", desp.IMEI,desp.Level);
    messageDebug(DBG_INFO, __MODULE__, __LINE__,"IMEI=%s, level=%02X", desp.IMEI,desp.Level);
    i=0;
    while (i<16 && desp.CellInfo[i].MCC!=-1) {
      MESSAGEDEBUG(DBG_INFO, __MODULE__, __LINE__,"MCC=%i, MNC=%i, LAC=%04X, CI=%04X", desp.CellInfo[i].MCC, desp.CellInfo[i].MNC, desp.CellInfo[i].LAC, desp.CellInfo[i].CI); 
      i++;
    }
    HEXDUMPDEBUG(DBG_INFO, __MODULE__, __LINE__, "Misc data", desp.MiscData, 64);
    HEXDUMPDEBUG(DBG_INFO, __MODULE__, __LINE__, "devProfile data", &devProfile, 256);
  }

  despvalid=TRUE;
  return valid;
}





