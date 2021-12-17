//=============================================================================
// DESP.h                                                          CHG 20071113
//
//=============================================================================
//
//-----------------------------------------------------------------------------
#ifndef _DESP_H
#define _DESP_H

//-----------------------------------------------------------------------------
// Extracted and decoded data from the "Top Secret" Keyfile area in FLASH..
//-----------------------------------------------------------------------------
typedef struct {
  char IMEI[15+1];      // IMEI number of the GSM module (if the module does not match this, hell will be loose :o)
  int Level;            // Bitmap of security level, Bit 0 is level 1, Bit 1 is level 2, bit 2 is level 3
  // List of allowed base stations
  struct {
    short MCC; // Mobile Country Code (0 is don't care)
    short MNC; // Mobile Network Code (0 is don't care)
    unsigned short LAC; // Location Area Code  (0 is don't care)
    unsigned short CI;  // Cell ID (0 is don't care)
  } CellInfo[16];
  unsigned char MiscData[64]; // Misc data
} tdefDESP;


#define DEVPROFILE_COOKIE 0xBABE1957

//-----------------------------------------------------------------------------
// Deviceinfo data, these are coming just behind the tdefDESP structure
//-----------------------------------------------------------------------------
// The struct must occupy exactly 256 bytes
typedef __packed struct {
  unsigned int cookie; // 4 bytes
  char serial[10+1]; // 11 bytes
  char decryptKey[32+1]; // 33 bytes

  unsigned int dummy[52]; // 208 bytes
} tdefDevProfile;


//-----------------------------------------------------------------------------
// checkLevelDESP()
// Check the security level of the DESP record
// Return:
//        0x10 invalid cookie (image is not valid)
//        Bitmap of security level, Bit 0 is level 1, Bit 1 is level 2, bit 2 is level 3
//-----------------------------------------------------------------------------
int checkLevelDESP(int dumpdata);

//-----------------------------------------------------------------------------
// getDeviceIDDESP()
// return deviceid (last 10 digits of IMEI in desp record
//-----------------------------------------------------------------------------
int getDeviceIDDESP(char *deviceID);


#endif


