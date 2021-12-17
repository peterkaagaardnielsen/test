//=============================================================================
// PMON.h                                                          20060815 CHG
//
//=============================================================================
//
//-----------------------------------------------------------------------------
#ifndef _PMON_
#define _PMON_

#ifdef PMON_EXPORTS
#define PMON_API __declspec(dllexport)
#else
#define PMON_API __declspec(dllimport)
#endif

#include "pmondef.h"

//-----------------------------------------------------------------------------
// Return values from the PMON functions
//-----------------------------------------------------------------------------
typedef enum {
  PMON_OK=0,              // Successful operation
  PMON_ERROR,             // General error
  PMON_DEVICE_NOT_INIT,   // initPMON() has not been called
  PMON_DEVICE_NOT_FOUND,  // The device is not found on the USB bus(ses)
  PMON_DEVICE_NO_COMM,    // Error in talking with the unit
  PMON_FILE_NOT_FOUND,    // File not found
  PMON_FILE_NOT_VALID,    // Firmwarefile not valid
  PMON_FW_VERIFY_ERROR,   // Verify error of uploaded firmware image
  PMON_NO_LISTENER,       // No generic listener for this slot/type of generic data request
  PMON_UNKNOWN_PARAMETER, // Specified parameter is unknown
  PMON_ERROR_SAVE,        // Could not save working set of parameters to FLASH
  PMON_VERIFY_ERROR,      // Verify error for data written
  PMON_ILL_INDEX,         // Specified index is illegal
  PMON_ILL_VALUE,         // Specified value is illegal
  PMON_ERR_SMS,           // Error when sending SMS message
} tdefrcPMON;


//------------------------------------------------------------------------------
// Type of storage for persistent storage functions
//------------------------------------------------------------------------------
typedef enum {
  PERSTYPE_FRAM=0,    // Storage type is FRAM
  PERSTYPE_DFLASH=1,  // Storage type is DataFlash
} tdefEnumPersType;

//----------------------------------------------------------------------------
// Callback function for progress status (for read/write dataflash/fram/fw)
//----------------------------------------------------------------------------
typedef void(*tdefProgressCBFunc)(unsigned int total, unsigned int current, void* arg);


//----------------------------------------------------------------------------
// Callback function for log output from the device
//----------------------------------------------------------------------------
typedef void(*tdefLogCBFunc)(char* msg, void* arg);

//-----------------------------------------------------------------------------
// getICCIDPMON()
//   Returns ICCID number for the SIM card in the device
// Output:
//   ICCID number
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getICCIDPMON(char *iccid);


//-----------------------------------------------------------------------------
// getIMEIPMON()
//   Returns IMEI number for the GSM module in the device
// Output:
//   IMEI number
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getIMEIPMON(char *imei);


//-----------------------------------------------------------------------------
// genericReadPMON()
//   Read a number of bytes from the specified slot
// Input: 
//   slot: slotnumber 0..255
//   length: number of bytes to request (0..61)
//   buffer: buffer for the read data
// Output:
//   length: number of bytes actually read
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON genericReadPMON(unsigned char slot, unsigned char *length, unsigned char *buffer);

//-----------------------------------------------------------------------------
// genericWritePMON()
//   Write a number of bytes to the specified slot
// Input: 
//   slot: slotnumber 0..255
//   length: number of bytes to write (0..61)
//   buffer: buffer for the data
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON genericWritePMON(unsigned char slot, unsigned char length, unsigned char *buffer);

//-----------------------------------------------------------------------------
// resetPMON()
//   Resets the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON resetPMON(void);

//-----------------------------------------------------------------------------
// dataflashReadPMON()
//   Reads a number of bytes from the Dataflash on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON dataflashReadPMON(int address, int length, unsigned char *buffer, tdefProgressCBFunc cb, void* arg);

//-----------------------------------------------------------------------------
// dataflashWritePMON()
//   Write a number of bytes to the Dataflash on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON dataflashWritePMON(int address, int length, unsigned char *buffer, tdefProgressCBFunc cb, void* cbarg);

//-----------------------------------------------------------------------------
// dataflashSizePMON()
//   Gets the size in bytes of the Dataflash mounted on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON dataflashSizePMON(int *size);

//-----------------------------------------------------------------------------
// framReadPMON()
//   Reads a number of bytes from the FRAM on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON framReadPMON(int address, int length, unsigned char *buffer, tdefProgressCBFunc cb, void* cbarg);

//-----------------------------------------------------------------------------
// framWritePMON()
//   Write a number of bytes to the FRAM on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON framWritePMON(int address, int length, unsigned char *buffer, tdefProgressCBFunc cb, void* cbarg);

//-----------------------------------------------------------------------------
// framSizePMON()
//   Gets the size in bytes of the FRAM mounted on the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON framSizePMON(int *size);

//-----------------------------------------------------------------------------
// getFWVersionPMON()
//   Gets the firmware version of the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getFWVersionPMON(int *version);

//-----------------------------------------------------------------------------
// getDLLVersionPMON()
//   Gets the version of the PMON driver
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getDLLVersionPMON(int *version);

//-----------------------------------------------------------------------------
// uploadFirmwarePMON()
//   Uploads a new firmware to the device
//   Returns then version and length of the firmware uploaded
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON uploadFirmwarePMON(char *filename, int* version, int* length, tdefProgressCBFunc cb, void* cbarg);


//-----------------------------------------------------------------------------
// getDeviceProfileValuePMON()
//   Get the value of a specific parameter
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getDeviceProfileValuePMON(char *name, char *value);

//-----------------------------------------------------------------------------
// setDeviceProfileValuePMON()
//   Set the value of a specific parameter
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON setDeviceProfileValuePMON(char *name, char *value);


//-----------------------------------------------------------------------------
// reloadDeviceProfilePMON()
//   Reload parameters from FLASH to the current working set 
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON reloadDeviceProfilePMON(void);

//-----------------------------------------------------------------------------
// saveDeviceProfilePMON()
//   Save the current working set of parameters to FLASH
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON saveDeviceProfilePMON(void);
  

//-----------------------------------------------------------------------------
// closePMON()
//   Close the PMON system
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON closePMON(void);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON installLogCBPMON(tdefLogCBFunc cb, void* arg);

//-----------------------------------------------------------------------------
// findNumberOfDevicesPMON()
//   Return the number of Devices connected currently
//-----------------------------------------------------------------------------
PMON_API int findNumberOfDevicesPMON(void);

//-----------------------------------------------------------------------------
// downloadVoicePMON()
//   Download a set of voice messages to the device
//   This function erases all present messages in the device, and downloads a complete
//   new set of messages
// Input: config: a list of voice numbers and filenames to compose the messages from.
//        format: "1,d:\xyz\name.wav" one message per line
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON downloadVoicePMON(char *config, tdefProgressCBFunc cb,void* cbarg);


//-----------------------------------------------------------------------------
// persistentSizePMON()
//   Gets the number of persistent entries for the specified type (FRAM/DATAFLASH)
//   and the maximum number of bytes that type can hold
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON persistentSizePMON(tdefEnumPersType type, int *numentries, int *maxdatasize);

//-----------------------------------------------------------------------------
// getPersistentValuePMON()
//   Get the content of a specific persistent entry
// Input: type: Type of persistent storage (PERSTYPE_DFLASH etc)
//        num: Index number of the persistent entry
//        maxlength: Maximum number of bytes we are interested in
// Output:
//        length: number of bytes stored in the persistent entry (can be higher than maxlength)
//        value: the data from the entry
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON getPersistentValuePMON(tdefEnumPersType type, int num, int maxlength, int *length, unsigned char *value);

//-----------------------------------------------------------------------------
// setPersistentValuePMON()
//   Set the content of a specific persistent entry
// Input: type: Type of persistent storage (PERSTYPE_DFLASH etc)
//        num: Index number of the persistent entry
//        length: Length of data..
//        value: the data to write to the entry
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON setPersistentValuePMON(tdefEnumPersType type, int num, int length, unsigned char *value);

//-----------------------------------------------------------------------------
// setPersistentValuePMON()
//   Set the content of a specific persistent entry
// Input: type: Type of persistent storage (PERSTYPE_DFLASH etc)
//        num: Index number of the persistent entry
//        length: Length of data..
//        value: the data to write to the entry
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON setPersistentValuePMON(tdefEnumPersType type, int num, int length, unsigned char *value);


//-----------------------------------------------------------------------------
// sendSMSPMON()
//   Send a SMS message using the device
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON sendSMSPMON(char *name, char *value);

//-----------------------------------------------------------------------------
// resetDeviceProfilePMON()
//   Reset parameters to defaults 
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON resetDeviceProfilePMON(void);

int handlePMON(pmonFrames *pmon);


//-----------------------------------------------------------------------------
// initPMON()
//   Initialize the PMON system, establishes connection to the device with number devnum
//-----------------------------------------------------------------------------
PMON_API tdefrcPMON initPMON(int devnum);

#endif
