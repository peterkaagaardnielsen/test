//=============================================================================
// FW.h                                                            20060821 CHG
//
//=============================================================================
//
//-----------------------------------------------------------------------------
#ifndef _FW_
#define _FW_
 
#define DEVICE_PROFILE 0x0007D000 // Address of the Device Profile record Flash

#define DEVICE_KEYFILE 0x0007C000 // Address of the Device Key record in Flash
#define DESP_COOKIE 0xCAFEB00B    // The magic cookie that tells us that there is a valid DESP record (Keyfile)


#define FW_ENTRY_CM3  0x00010000 // This is where the Application is stored (it's reset vector)...

#define FW_COOKIE1_CM3 (FW_ENTRY_CM3+0xDC) // 
#define FW_COOKIE2_CM3 (FW_ENTRY_CM3+0xE0) // 

#define FW_VERSION_CM3 (FW_ENTRY_CM3+0xE4) // This is where the fw version info is stored (a 4 byte unsigned int)
#define FW_SIZE_CM3    (FW_ENTRY_CM3+0xE8) // This is where the fw size info is stored (a 4 byte unsigned int)
#define FW_CRC_CM3     (FW_ENTRY_CM3+0xEC) // This is where the fw crc info is stored (a 4 byte unsigned int)
#define FW_TARGET_CM3  (FW_ENTRY_CM3+0xF0) // This is where the fw target info is stored (a 4 byte unsigned int)

#define FW_MAX_SIZE   (184*1024)           // Maximum size of firmware (256 kb - 64 Kb for bootloader -  4KB for USerFlash - 4KB for FALHeader info)

#define FW_COOKIE 0xDEADBEEF               // The magic cookie that tells us it's a valid FW file 
 
// The cryptmsg must be '0' terminated (length can be anything)
static unsigned char fwcryptmsg[]="Big fat asses";
 
 
//-----------------------------------------------------------------------------
// Encode/Decode a chunk of memory.
// Set init to true on the first chunk of memory to be en/decoded, false on next chunk
// (If en/decode one large piece of memory, just let iunit be true on the single call)
//-----------------------------------------------------------------------------
static void fwencode(unsigned char* data,int length, int init) {
  static unsigned char* cur;
  static unsigned char* cc;
 
  if (init) {
    cc=fwcryptmsg;
  }
 
  for (cur=data;cur<&data[length];cur++) {
    *cur = ~*cur;
    *cur=*cur ^ *cc++;
    if (!*cc) cc=fwcryptmsg;
  }
}
 
 
//-----------------------------------------------------------------------------
// Firmware header structure
// Structure must fit exactly 64 bytes              
//-----------------------------------------------------------------------------
typedef struct {
  unsigned int cookie;
  unsigned int length;  // Length of the actual firmware (starting FW_HEADERSIZE from the beginning of this structure)
  unsigned int crc;     // CRC of the FW image (starting FW_HEADERSIZE from the beginning of this structure)
                        // The CRC is calculated as a sum of all bytes of the image, at the end, the length field is added to the crc
  unsigned int version;
  unsigned int target;  // Type of target the firmware is intended for (7=blackbird, 8= robin, 9=dragonfly etc)
  unsigned int reserved[11];
} tdefFWHeader;
 
 
 
 
#endif
