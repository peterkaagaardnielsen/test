//=============================================================================
// PMONDef.h                                                       20060814 CHG
//
//=============================================================================
//
//-----------------------------------------------------------------------------
#ifndef _PMONDEF_
#define _PMONDEF_

#define PMON_FRAMESIZE 64

//-----------------------------------------------------------------------------
// All request/response messages
//-----------------------------------------------------------------------------
#define PMON_VERSION               0x01
#define PMON_RESET                 0x02
#define PMON_DEBUG                 0x03


#define PMON_DATAFLASH_SIZE        0x05
#define PMON_DATAFLASH_READ        0x06
#define PMON_DATAFLASH_WRITE       0x07

#define PMON_FRAM_SIZE             0x08
#define PMON_FRAM_READ             0x09
#define PMON_FRAM_WRITE            0x0A

#define PMON_IMEI                  0x0B
#define PMON_ICCID                 0x0C

#define PMON_GENERIC_READ          0x0D
#define PMON_GENERIC_WRITE         0x0E

#define PMON_SET_DEVICE_PROFILE    0x0F
#define PMON_GET_DEVICE_PROFILE    0x10

#define PMON_SAVE_DEVICE_PROFILE   0x11
#define PMON_RELOAD_DEVICE_PROFILE 0x12

#define PMON_GET_MAXNUM_PERSISTENT 0x13
#define PMON_GET_PERSISTENT        0x14
#define PMON_SET_PERSISTENT        0x15

#define PMON_SEND_SMS              0x16
#define PMON_RESET_DEVICE_PROFILE  0x17

#define PMON_FLUSH_DEBUG_BUFFER    0x18


#define PMON_KEYFILE_VALID         0x7D
#define PMON_DEVICEID              0x7E
#define PMON_FALAPP_VERSION        0x7F
#define PMON_FS_FINDDIR            0x80
#define PMON_FS_CHDIR              0x81
#define PMON_FS_MKDIR              0x82
#define PMON_FS_RMDIR              0x83
#define PMON_FS_DELETE             0x84
#define PMON_FS_READ               0x85
#define PMON_FS_WRITE              0x86
#define PMON_FS_SPACE              0x87

#define PMON_VSMS_SEND             0x90
#define PMON_VSMS_RECEIVE          0x91

#define PMON_TERM_CMD              0xA0

// Datatypes...
#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned int
#define S8  signed char
#define S16 signed short
#define S32 signed int

//-----------------------------------------------------------------------------
// Structures for all request/response messages
//-----------------------------------------------------------------------------

#ifdef _WINDOWS_
#define __packed
#pragma pack(1)
#endif

// General command/response structure
typedef __packed struct  {
  U8 cmd;
} req_pmon;

typedef __packed struct  {
  U8 rc;
} resp_pmon;

typedef __packed struct  {
  U8 cmd;
} req_pmonVersion;

typedef __packed struct  {
  U32 version;         // 1.00 => 100
  U32 platform;
  U32 bootmode;
} resp_pmonVersion;


 



typedef __packed struct  {
  U8 cmd;
} req_pmonReset;

typedef __packed struct  {
  U8 rc;              // 0=going to reset
} resp_pmonReset;
 
typedef __packed struct  {
  U8 cmd;
  U8 status;          // 0=disable, 1=enable, 2=get status
} req_pmonDebug;

typedef __packed struct  {
  U8 rc;              // 0=disabled, 1=enabled
} resp_pmonDebug;


typedef __packed struct  {
  U8 cmd;
} req_pmonDataFlashSize;

typedef __packed struct  {
  U32 size;
} resp_pmonDataFlashSize;

typedef __packed struct  {
  U8 cmd;
  U32 address;
  U8  length;
} req_pmonDataFlashRead;

typedef __packed struct  {
  U8 rc;
  U8 length;
  U8 data[PMON_FRAMESIZE-2];
} resp_pmonDataFlashRead;

typedef __packed struct  {
  U8 cmd;
  U32 address;
  U8 length;
  U8 data[PMON_FRAMESIZE-6];
} req_pmonDataFlashWrite;

typedef __packed struct  {
  U8 rc;
} resp_pmonDataFlashWrite;

typedef __packed struct  {
  U8 cmd;
} req_pmonFramSize;

typedef __packed struct  {
  U32 size;
} resp_pmonFramSize;

typedef __packed struct  {
  U8 cmd;
  U32 address;
  U8  length;
} req_pmonFramRead;

typedef __packed struct  {
  U8 rc;
  U8 length;
  U8 data[PMON_FRAMESIZE-2];
} resp_pmonFramRead;

typedef __packed struct  {
  U8 cmd;
  U32 address;
  U8 length;
  U8 data[PMON_FRAMESIZE-6];
} req_pmonFramWrite;

typedef __packed struct  {
  U8 rc;
} resp_pmonFramWrite;

typedef __packed struct  {
  U8 cmd;
} req_pmonIMEI;

typedef __packed struct  {
  U8 rc;
  U8 str[15+1];
} resp_pmonIMEI;

typedef __packed struct  {
  U8 cmd;
} req_pmonICCID;

typedef __packed struct  {
  U8 rc;
  U8 str[32+1];
} resp_pmonICCID;

typedef __packed struct  {
  U8 cmd;
  U8 slot;
  U8 length;
} req_pmonGenericRead;

typedef __packed struct  {
  U8 rc;
  U8 slot;
  U8 length;
  U8 data[PMON_FRAMESIZE-3];
} resp_pmonGenericRead;

typedef __packed struct  {
  U8 cmd;
  U8 slot;
  U8 length;
  U8 data[PMON_FRAMESIZE-3];
} req_pmonGenericWrite;

typedef __packed struct  {
  U8 rc;
  U8 slot;
} resp_pmonGenericWrite;

typedef __packed struct  {
  U8 cmd;
  U8 state;
  U8 data[PMON_FRAMESIZE-2];
} req_pmonSetDeviceProfile;

typedef __packed struct  {
  U8 rc;
} resp_pmonSetDeviceProfile;

typedef __packed struct  {
  U8 cmd;
  U8 data[PMON_FRAMESIZE-1];
} req_pmonGetDeviceProfile;

typedef __packed struct  {
  S8 more;
  U8 data[PMON_FRAMESIZE-1];
} resp_pmonGetDeviceProfile;

typedef __packed struct  {
  U8 cmd;
} req_pmonSaveProfile;

typedef __packed struct  {
  U8 rc;
} resp_pmonSaveProfile;

typedef __packed struct  {
  U8 cmd;
} req_pmonReloadProfile;

typedef __packed struct  {
  U8 rc;
} resp_pmonReloadProfile;

typedef __packed struct  {
  U8 cmd;
} req_pmonResetProfile;

typedef __packed struct  {
  U8 rc;
} resp_pmonResetProfile;


typedef __packed struct  {
  U8 cmd;
  U8 type;
} req_pmonGetNumPersistent;

typedef __packed struct  {
  S8 more;
  U32 num;
  U32 size;
} resp_pmonGetNumPersistent;


typedef __packed struct  {
  U8 cmd;
  U8 state;
  U32 type;
  U32 index;
  U32 length;
  U8 data[PMON_FRAMESIZE-14];
} req_pmonSetPersistent;

typedef __packed struct  {
  U8 rc;
} resp_pmonSetPersistent;


typedef __packed struct  {
  U8 cmd;
  U32 type;
  U32 index;
} req_pmonGetPersistent;

typedef __packed struct  {
  S8 more;
  U16 length;
  U8 data[PMON_FRAMESIZE-3];
} resp_pmonGetPersistent;


typedef __packed struct  {
  U8 cmd;
  U8 state;
  U8 data[PMON_FRAMESIZE-2];
} req_pmonSendSMS;

typedef __packed struct  {
  U8 rc;
} resp_pmonSendSMS;


typedef __packed struct  {
  U8 cmd;
} req_pmonDeviceID;

typedef __packed struct  {
  U8 deviceid[10+1];
} resp_pmonDeviceID;


typedef __packed struct  {
  U8 cmd;
} req_pmonKeyfileValid;

typedef __packed struct  {
  U8 valid;
} resp_pmonKeyfileValid;


typedef __packed struct  {
  U8 cmd;
} req_pmonFlushDebug;

typedef __packed struct  {
  U8 rc;      
} resp_pmonFlushDebug;



//----------------------------------------------------------------------------
// Filesystem
//----------------------------------------------------------------------------
typedef __packed struct  {
  U8 cmd;
  U8 first;
  U8 data[PMON_FRAMESIZE-2];
} req_pmonFSFindDir;

typedef __packed struct  {
  S8 more;
  S32 filesize; // -1 if directory
  U8 data[PMON_FRAMESIZE-5];
} resp_pmonFSFindDir;

typedef __packed struct  {
  U8 cmd;
  U8 data[PMON_FRAMESIZE-1];
} req_pmonFSCHDir;

typedef __packed struct  {
  U8 rc;
} resp_pmonFSCHDir;

typedef __packed struct  {
  U8 cmd;
  U8 data[PMON_FRAMESIZE-1];
} req_pmonFSMKDir;

typedef __packed struct  {
  U8 rc;
} resp_pmonFSMKDir;

typedef __packed struct  {
  U8 cmd;
  U8 data[PMON_FRAMESIZE-1];
} req_pmonFSRMDir;

typedef __packed struct  {
  U8 rc;
} resp_pmonFSRMDir;

typedef __packed struct  {
  U8 cmd;
  U8 data[PMON_FRAMESIZE-1];
} req_pmonFSDelete;

typedef __packed struct  {
  U8 rc;
} resp_pmonFSDelete;


typedef __packed struct  {
  U8 cmd;
  U8 state;
  U8 data[PMON_FRAMESIZE-2];
} req_pmonFSRead;

typedef __packed struct  {
  S8 more;
  U32 length;
  U8 data[PMON_FRAMESIZE-5];
} resp_pmonFSRead;


typedef __packed struct  {
  U8 cmd;
  U8 state;
  U8 length;
  U8 data[PMON_FRAMESIZE-3];
} req_pmonFSWrite;

typedef __packed struct  {
  U8 rc;
  int length;
} resp_pmonFSWrite;


typedef __packed struct  {
  U8 cmd;
  U8 drv; // DONT call this "drive", otherwise the fw dont work with falmanager ?!?!?!?!?!?!
} req_pmonFSSpace;

typedef __packed struct  {
  U8 rc;
  U32 total;
  U32 free;
  U32 used;
  U32 bad;
} resp_pmonFSSpace;


//----------------------------------------------------------------------------
// VSMS
//----------------------------------------------------------------------------

typedef __packed struct  {
  U8 cmd;
  U8 state; // 0 start of VSMS, 1 return data data for VSMS
} req_pmonVSMSRead;

typedef __packed struct  {
  S8 more; // 0 no data to receive, 1 more data
  U32 length; // number of databytes in this message
  U8 data[PMON_FRAMESIZE-5];
} resp_pmonVSMSRead;


typedef __packed struct  {
  U8 cmd;
  U8 state; // 0 start of VSMS, 1 data for VSMS, 2 end of message
  U8 length;
  U8 data[PMON_FRAMESIZE-3];
} req_pmonVSMSWrite;

typedef __packed struct  {
  U8 rc;
} resp_pmonVSMSWrite;


typedef __packed struct
{
    U8 cmd;
    U8 data[PMON_FRAMESIZE-1];
} req_pmonTerm;

typedef __packed struct  {
  U8 rc;
} resp_pmonTerm;



typedef __packed union {
  req_pmon                  reqCmd;            // Just the cmd field..
  resp_pmon                 respRc;            // Just a rc field.. 

  req_pmonVersion           reqVersion;
  resp_pmonVersion          respVersion;
  req_pmonReset             reqReset;
  resp_pmonReset            respReset;
  req_pmonDebug             reqDebug;
  resp_pmonDebug            respDebug;
  req_pmonGetDeviceProfile  reqGetDeviceProfile;
  resp_pmonGetDeviceProfile respGetDeviceProfile;
  req_pmonDataFlashSize     reqDataFlashSize;
  resp_pmonDataFlashSize    respDataFlashSize;
  req_pmonDataFlashRead     reqDataFlashRead;
  resp_pmonDataFlashRead    respDataFlashRead;
  req_pmonDataFlashWrite    reqDataFlashWrite;
  resp_pmonDataFlashWrite   respDataFlashWrite;
  req_pmonFramSize          reqFramSize;
  resp_pmonFramSize         respFramSize;
  req_pmonFramRead          reqFramRead;
  resp_pmonFramRead         respFramRead;
  req_pmonFramWrite         reqFramWrite;
  resp_pmonFramWrite        respFramWrite;
  req_pmonIMEI              reqImei;
  resp_pmonIMEI             respImei;
  req_pmonICCID             reqIccid;
  resp_pmonICCID            respIccid;
  req_pmonGenericRead       reqGenericRead;
  resp_pmonGenericRead      respGenericRead;
  req_pmonGenericWrite      reqGenericWrite;
  resp_pmonGenericWrite     respGenericWrite;
  req_pmonSetDeviceProfile  reqSetDeviceProfile;
  resp_pmonSetDeviceProfile respSetDeviceProfile;
  req_pmonSaveProfile       reqSaveProfile;
  resp_pmonSaveProfile      respSaveProfile;
  req_pmonReloadProfile     reqReloadProfile;
  resp_pmonReloadProfile    respReloadProfile;
  req_pmonResetProfile     reqResetProfile;
  resp_pmonResetProfile    respResetProfile;

  req_pmonGetNumPersistent  reqGetNumPersistent;
  resp_pmonGetNumPersistent respGetNumPersistent;
  req_pmonSetPersistent     reqSetPersistent;
  resp_pmonSetPersistent    respSetPersistent;
  req_pmonGetPersistent     reqGetPersistent;
  resp_pmonGetPersistent    respGetPersistent;

  req_pmonSendSMS  reqSendSMS;
  resp_pmonSendSMS respSendSMS;




  req_pmonFSFindDir  reqFSFindDir;
  resp_pmonFSFindDir respFSFindDir;

  req_pmonFSCHDir  reqFSCHDir;
  resp_pmonFSCHDir respFSCHDir;

  req_pmonFSMKDir  reqFSMKDir;
  resp_pmonFSMKDir respFSMKDir;

  req_pmonFSRMDir  reqFSRMDir;
  resp_pmonFSRMDir respFSRMDir;

  req_pmonFSDelete  reqFSDelete;
  resp_pmonFSDelete respFSDelete;
  
  req_pmonFSRead  reqFSRead;
  resp_pmonFSRead respFSRead;

  req_pmonFSWrite  reqFSWrite;
  resp_pmonFSWrite respFSWrite;

  req_pmonFSSpace  reqFSSpace;
  resp_pmonFSSpace respFSSpace;

  req_pmonDeviceID  reqDeviceID;
  resp_pmonDeviceID respDeviceID;

  req_pmonKeyfileValid reqKeyfileValid;
  resp_pmonKeyfileValid respKeyfileValid;

  req_pmonFlushDebug reqFlushDebug;
	resp_pmonFlushDebug    respFlushDebug;

  req_pmonVSMSRead reqVSMSRead;
  resp_pmonVSMSRead respVSMSRead;

  req_pmonVSMSWrite reqVSMSWrite;
  resp_pmonVSMSWrite respVSMSWrite;

  req_pmonTerm reqTerm;
  resp_pmonTerm respTerm;
  
} pmonFrames;


#ifdef WINDOWS
#pragma pack()
#endif


#endif


