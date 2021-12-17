//=============================================================================
// GSM.C                                                           20060615 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "System.h"
//#include "Queue.h"
//#include "UART1.h"
//#include "UART0.h"
#include "LEUART0.h"
#include "GSM.h"
#include "Debug.h"
//#include "FileSystem.h"
//#include "VSMS.h"
//#include "RTXMonitor.h"
//#include "sysman.h"

#include "efm32.h"

//-----------------------------------------------------------------------------
// Define I/O pins for the device
//-----------------------------------------------------------------------------
// A couple of defines that also must be copied to the GSM.h file !!!!
#define IO_DIR IODIR0 // The GPIO registers used for accessing the device's normal functions
#define IO_SET IOSET0
#define IO_CLR IOCLR0
#define IO_PIN IOPIN0

#define GSM_RTS    (1<<22) // P0.22 is RTS to GSM
#define GSM_CTS    (1<<17) // P0.17 is CTS from GSM
#define GSM_DSR    (1<<19) // P0.19 is DSR from GSM
#define GSM_DTR    (1<<20) // P0.20 is DTR to GSM
#define GSM_DCD    (1<<18) // P0.18 is DCD from GSM
#define GSM_RING   (1<<21) // P0.21 is RING from GSM


#define IO_DIR1 IODIR1 // The GPIO registers used for accessing the device's special functions
#define IO_SET1 IOSET1
#define IO_CLR1 IOCLR1
#define IO_PIN1 IOPIN1

#define GSM_EMR    (1<<0)  // P1.00 is emergency reset
#define GSM_VDD    (1<<1)  // P1.01 is VDD from GSM module (indicates it is on)
#define GSM_IGT    (1<<4)  // P1.04 is Ignition

static unsigned char gsminit=FALSE; // Set to TRUE when initGSM has been called
static MUTEX hmutGSM;         // Mutex that controls access to the GSM
static SEM hsemGSM;           // Signalled when we get an answer from the module..

static MUTEX hMutGSMInfo;    // Acquire this mutex before reading/writing the parametes in the global GSMInfo structure
static tdefGSMInfo GSMInfo={0};   // Current set of (global) data from the GSM Module

static enumunlockGSM releaseReason; // Set by thGSM() when it receives one of OK,ERROR,CME ERROR or CMS ERROR

static U64 thGSMStack[200];

static int globalLock=FALSE;

static int dummyTXfunc(char ch) {return 0;}
static int dummyRXfunc(char *ch) {return 1;}

static GSMTXCbFunc cbTX=dummyTXfunc; // Stores the function we use to send characters to the GSM module
static GSMRXCbFunc cbRX=dummyRXfunc; // Stores the function we use to read characters from the GSM module

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static const struct {
   char text[15];
   enumGSMresp gsmResponse;
} GSMResp[] = {
   "OK",          GSMRESP_OK,
   "^SHUTDOWN",   GSMRESP_SHUTDOWN,
   "^SMSO:",      GSMRESP_SHUTDOWN,

   "REVISION ",   GSMRESP_REVISION,

   // TCP/IP Handling
   "^SIS:",      GSMRESP_SIS,
   "^SISW:",     GSMRESP_SISW,
   "^SISR:",     GSMRESP_SISR,

  // Outgoing SMS handling
   "+CMGS:",      GSMRESP_CMGS,
   "> ",          GSMRESP_SMS_PROLOG,

   // Incoming SMS handling
   "+CMTI:",      GSMRESP_CMTI,
   "+CMGL:",      GSMRESP_CMGL,
   "+CMGR:",      GSMRESP_CMGR,

   // SIM card handling
   "+CPIN: ",     GSMRESP_CPIN,
   "^SCKS:",      GSMRESP_SCKS,
   "^SCID: ",     GSMRESP_SCID,
   "+CPBR: ",     GSMRESP_CPBR,  

   // Network specific
   "+COPS:",      GSMRESP_COPS,
   "+CSQ:",       GSMRESP_CSQ,
   "+CREG:",      GSMRESP_CREG,
   "BCCH  G  PBCCH", GSMRESP_SMONG,
   "^SMOND:",     GSMRESP_SMOND,
   "^SNMON: \"INS\",",     GSMRESP_SNMON,
   
   // Voice communication
   "+CRING: ",    GSMRESP_CRING,
   "^SCNI:",      GSMRESP_SCNI,
   "BUSY",        GSMRESP_BUSY,
   "NO DIALTONE", GSMRESP_NO_DIALTONE,
   "NO CARRIER",  GSMRESP_NO_CARRIER,
   "+CLIP: ",     GSMRESP_CLIP,

   // Errors
   "ERROR",       GSMRESP_ERROR,
   "+CMS ERROR:", GSMRESP_CMS_ERROR,
   "+CME ERROR:", GSMRESP_CME_ERROR,
   ":EXIT ",      GSMRESP_EXIT,
};





#define numGSMResp (sizeof(GSMResp)/sizeof(GSMResp[0]))

#define MaxCbList 20 

//----------------------------------------------------------------------------
// List of functions that are to be called whenever a response is received from 
// the GSM module
//----------------------------------------------------------------------------
static struct {
   GSMResponseCbFunc cb;
   void* arg;
} CbList[MaxCbList]; 
static MUTEX hmutCBGSM;       // Mutex that controls access to the list of callback functions

static int debug=FALSE; // If set to true, debug messages from module is enabled


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
unsigned int isRunningGSM(void) {
  return GSMInfo.Initialized;
//  return GSMInfo.Power;
}

//-----------------------------------------------------------------------------
// setDebugGSM()
//   Activate/deactivate debug messages from module
// Returns previous setting of debug flag
//-----------------------------------------------------------------------------
int setDebugGSM(int state) {
  int rc;
/*
  if (state) 
    messageDebug(DBG_INFO,__MODULE__, __LINE__,  "GSM debug on");    
  else
    messageDebug(DBG_INFO,__MODULE__, __LINE__,  "GSM debug off");    
*/
  rc=debug;
  debug=state;
  return rc;
}

//----------------------------------------------------------------------------
// PDU/ASCII helper stuff
//----------------------------------------------------------------------------
#define noc 183 
// non existent character

//----------------------------------------------------------------------------
// Make a hard lock of the GSM
//----------------------------------------------------------------------------
void lockGSM(int state) {
  globalLock=state;
}

//----------------------------------------------------------------------------
// Wait for exclusive access to the GSM module
// Returns: 0 if we have exclusive access, 1 if timeout acquiring access
//----------------------------------------------------------------------------
int enterGSM(void) {
  int tick=0;

  if (!gsminit)
    return 1;

  // Wait (max 50 seconds) until no global lock...
  while (globalLock && (tick++<500)) OS_WAIT(100);

  // Make a failsafe timeout of 40 seconds...
  if (os_mut_wait(&hmutGSM,4000)==OS_R_TMO)
    return 1;

//  MUTEX_ACQUIRE(hmutGSM);
//  SEM_WAIT(hsemGSM, 0); // If the semaphore is already signalled, remove the signal
  while (SEM_WAIT(hsemGSM, 0)==OS_R_OK) OS_WAIT(20); // If the semaphore is already signalled, remove the signal
  return 0;

}

//----------------------------------------------------------------------------
// Wait for answer from the module (with timeout)
// Returns: enumunlockGSM
//          
//----------------------------------------------------------------------------
enumunlockGSM exitGSM(int timeout) {
  enumunlockGSM rc;
  if (!gsminit)
    return GSM_NOT_INIT;

  // Wait for an answer from module (or timeout)
  if (timeout && SEM_WAIT(hsemGSM, timeout)==OS_R_TMO)
    rc=GSM_TIMEOUT;
  else
    rc=releaseReason;
  // Release access to the module
  MUTEX_RELEASE(hmutGSM);
  return rc;
}

//----------------------------------------------------------------------------
// Release the module..
// This will be called by thGSM when it receives OK, ERROR, CME ERROR or CMS ERROR
//----------------------------------------------------------------------------
static void releaseGSM(enumunlockGSM reason) {
  releaseReason=reason;
  SEM_SIGNAL(hsemGSM);
}

//----------------------------------------------------------------------------
// Send a string to the module..
//----------------------------------------------------------------------------
void sendStringGSM(char *str) {
  unsigned int temp;  

  while (isEnableLEUART0()==FALSE)
    OS_WAIT(100);

  
  // If PE9/CTS is asserted from the GSM, it is in sleepmode..  
  if (GPIO->P[4].DIN & (1 << 9)) {
    // Remember the state of RTS
    temp=GPIO->P[4].DIN & (1 << 12);
    // Toggle PE12/RTS to wakeup the module
    GPIO->P[4].DOUTSET = (1<<12);
    OS_WAIT(100);
    GPIO->P[4].DOUTCLR = (1<<12);
    OS_WAIT(100);
    // Restore old state of module..
    if (temp)
      GPIO->P[4].DOUTSET = (1<<12);
    else
      GPIO->P[4].DOUTCLR = (1<<12);
  }
  
  if (debug)
    messageDebug(DBG_INFO, __MODULE__, __LINE__, str);

  while (*str)
    cbTX(*(str++));
  
  // !!! HACK
#ifdef _WINDOWS_
  cbTX(0); 
#endif
}


//----------------------------------------------------------------------------
// Send a command to the GSM module
// Returns: enumunlockGSM
//----------------------------------------------------------------------------
enumunlockGSM sendGSM(char* str, int timeout) {
  enumunlockGSM rc;
  // Get access to the module..
  if (!gsminit)
    return GSM_NOT_INIT;

  if (enterGSM())
    return GSM_ERROR;

  sendStringGSM(str);
  rc=exitGSM(timeout);
  OS_WAIT(100);
  return rc;
}

//----------------------------------------------------------------------------
// Returns length of the received string
//          =0 String was too small (less than 2 chars, typically just a CR LF sequence from the GSM)
//          -1 Buffer overflow
//----------------------------------------------------------------------------
static int receiveGSMString(char* buffer, int buflen) {
   char ch;
   int index=0, rc;

   // We continue until we get a string with a LF at the end (or buffer overflow).. 
   while (1) {
     // If no characters is ready, we wait 100 mSec. As soon as there is characters ready, they are
     // handled at maximum speed.
     while (cbRX(&ch)==1) OS_WAIT(100);

     switch (ch) {
     // Ignore any CR...
     case 0x0D:
       break;
     // ..we are looking for a LF as the terminator..
     case 0x0A:
       // The shortest answer we want is a "OK"
       if (index >= 1) {
         buffer[index]='\0';
         rc=(int)strlen(buffer);
       } else 
         rc=0;
       index=0;
       return rc;
     // ..All other characters are stuffed into the buffer, if too much, we abort and report buffer full..
     default:
        buffer[index++]=ch;
        if (index==buflen)
          return -1;
        break;
     }
  }
}

//----------------------------------------------------------------------------
// Enumerate the response from the GSM module
//----------------------------------------------------------------------------
static enumGSMresp GetResponse(char* str, char** text) {

  enumGSMresp resp=GSMRESP_NOT_SUPPORTED;
   unsigned int i;

   // See if the response from the module is known.. 
   for (i=0; i<numGSMResp; i++) {
     if (strncmp(GSMResp[i].text, str, strlen(GSMResp[i].text))==NULL) {
       // Response was found
       resp=GSMResp[i].gsmResponse;
       break;
     }
   }

   // If we did find a response we know of..
   if (resp != GSMRESP_NOT_SUPPORTED)
      // If no data follows the response (f.ex "OK", "ERROR" etc
      if (strlen(GSMResp[i].text) == strlen(str))
        *text = NULL;
      // data followes the response, let text point to what follows the response
      else
        *text=&str[strlen(GSMResp[i].text)];
   // The response from the module is not known, just return the complete response
   else
     *text=str;
   return resp;
}

//----------------------------------------------------------------------------
// thGSM
// Receives incoming data from GSM module. Decodes and processes the data..
//----------------------------------------------------------------------------
void thGSM(TH_PARAM) {
  enumGSMresp GSMResp;         // The ID of the received GSM response
  static tdefGSMRespData gsmRespData; // Data for the decoded response

  static char rxbuf[256];      // Receive buffer for incoming data from GSM module
  char *pText;                 // The data following the decoded GSM response
  int i;
  
  gsmRespData.CME_ERROR.Error=0; // To avoid a "not initialized variable" during debug in VC

  //setNameRTXMON("thGSM");

  // Give this thread access (because of RemoteFile)
  //f_enterFS();

  while (1) {
    // See if we can get a string from the module..
    // The OS_WAIT() will only be called if there is either a a too short or too long string received
    while (receiveGSMString(rxbuf, sizeof(rxbuf)) <= 0) {
      OS_WAIT(100);
    }

    if (debug)
      messageDebug(DBG_INFO, __MODULE__, __LINE__, rxbuf);
    
    // Try to enumerate the reponse from the module..
    GSMResp=GetResponse(rxbuf, &pText);

    // Decode each verb, looking at parameters etc
    // put the decoded data into the gsmRespData union
    switch (GSMResp) {

      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      // OK
      //+++++++++++++++++++++++++++++++++++++++++++++++++++
      case GSMRESP_OK:
        releaseGSM(GSM_OK);
        break;

      case GSMRESP_EXIT:
        break;

      case GSMRESP_REVISION:    
        break;


      case GSMRESP_SMOND:    
        break;

      case GSMRESP_SIS:    
        break;

      case GSMRESP_SISW:    
        break;

      case GSMRESP_SISR:    
        break;

      case GSMRESP_CMGS:
        break;

      case GSMRESP_CMTI:    
        break;

      case GSMRESP_CMGL: // 
        break;

      case GSMRESP_CMGR:
        break;

      case GSMRESP_CPIN:
        break;

      case GSMRESP_SCKS:
        break;

      case GSMRESP_SCID:
        break;

      case GSMRESP_CPBR:    
        break;

      case GSMRESP_COPS:
        break;

     case GSMRESP_CSQ:
       break;

     case GSMRESP_SMONG:
       break;

    case GSMRESP_SNMON:
       break; 

      case GSMRESP_CREG:
        break;

      case GSMRESP_CRING:
        break;

      case GSMRESP_NO_DIALTONE:
        break;

      case GSMRESP_NO_CARRIER:
        break;

      case GSMRESP_CLIP:
        break;
           
      case GSMRESP_SCNI:
        break;
        
      case GSMRESP_ERROR:
        break;

       case GSMRESP_CMS_ERROR:
         break;

      case GSMRESP_CME_ERROR:
        break;

      case GSMRESP_NOT_SUPPORTED:
       gsmRespData.GSM_RESP_NOT_SUPPORTED.Text=pText;
       break;
    }
    
    MUTEX_ACQUIRE(hmutCBGSM);
    // Call all the functions that has been registered..
    for (i=0; i<MaxCbList; i++) {
      if (CbList[i].cb) 
        CbList[i].cb(GSMResp, gsmRespData, CbList[i].arg);
    }
    MUTEX_RELEASE(hmutCBGSM);

    OS_WAIT(100);
  }
}

//----------------------------------------------------------------------------
// Register a client function.
// The installed funtion will be called whenever the GSM module sends a response
// Return 0 if ok, 1 if no more room in list
//----------------------------------------------------------------------------
unsigned int registerGSM(GSMResponseCbFunc func, void* arg) {
 int i; 
  if (!gsminit)
    return GSM_NOT_INIT;

 MUTEX_ACQUIRE(hmutCBGSM);
 // Insert the client at the first free entry in the list..
 for (i=0; i<MaxCbList; i++) {
   if (CbList[i].cb==NULL) {
     CbList[i].cb=func;
     CbList[i].arg=arg;
     MUTEX_RELEASE(hmutCBGSM);
     return 0;
   }
 }
 MUTEX_RELEASE(hmutCBGSM);
 return 1;
}


//----------------------------------------------------------------------------
// Deregister a client function.
// Returns 0 if found, 1 if not found
//----------------------------------------------------------------------------
unsigned int deregisterGSM(GSMResponseCbFunc func) {
  int i; 
  if (!gsminit)
    return GSM_NOT_INIT;

  MUTEX_ACQUIRE(hmutCBGSM);
  // Search for the client, if found, delete it..
  for (i=0; i<MaxCbList; i++) {
    if (CbList[i].cb==func) {
      CbList[i].cb=NULL;
      MUTEX_RELEASE(hmutCBGSM);
      return 0;
    }
  }
  MUTEX_RELEASE(hmutCBGSM);
  return 1;
}

//----------------------------------------------------------------------------
// cb helperfunction 
//----------------------------------------------------------------------------
static void cbgetIMEI(enumGSMresp response, tdefGSMRespData responsedata, void* arg) 
{
  // If we get a "unsopported" answer, and the length is 15, then we assume its a IMEI code
  if ((response==GSMRESP_NOT_SUPPORTED) && (strlen(responsedata.GSM_RESP_NOT_SUPPORTED.Text)==15)) {
    if (arg) {
      memcpy(arg, responsedata.GSM_RESP_NOT_SUPPORTED.Text, 15);
      ((char*)arg)[15]='\0';
    }
  }
}

//----------------------------------------------------------------------------
// Query the module for its IMEI number
// Output: IMEI: The modules IMEI code will be copied to this array
// Returns: enumunlockGSM: Result of operation, if GSM_OK then IMEI is defined
//----------------------------------------------------------------------------
enumunlockGSM getIMEIGSM(char* imei) {
  enumunlockGSM lrc;
  int tick=0;
 
  if (!gsminit)
    return GSM_NOT_INIT;

  if (!GSMInfo.Power)
    return GSM_NO_POWER;
  
 // if (!GSMInfo.Initialized)
   // return GSM_NO_POWER;
  
  
  // Get access to the module
  if (enterGSM())
    return GSM_ERROR;
  
  // Register our helper function (the helper will modify the "arg" parameter)
  registerGSM(cbgetIMEI, (void*)imei);
  imei[0]=0;
  // Issue command to the module
  sendStringGSM("\rAT+CGSN\r");
  // the cb function above will now be called by the response to the command..
  // 
  // Wait for the "OK" after the response message
  lrc=exitGSM(12000);
  while ((tick++ < 50) && (imei[0]==0)) OS_WAIT(100);

  deregisterGSM(cbgetIMEI);
  return lrc;
}

//--------------------------------------------------------------------------
// Wait a number of uSec's
// Heavily platform dependent :-)
// On Merlin, every byte read or write gets the core speed and this function adjusts the delay accordingly
//--------------------------------------------------------------------------
static void delay(unsigned int i) {
  volatile unsigned char j;
  // Running at CORECLOCK_HIGH
  while (i--) {
    // Running on LPC1778 at 96 MHz
    for (j=0; j<3; j++);
    __nop(); __nop();
    __nop(); __nop();
  }
}


//-----------------------------------------------------------------------------
// powerGSM()
//   Switch GSM module on or off
// Input:
//   power: true if swithing on, false if powering down
//   PINCode: the 4 ASCII digit pincode to use (if SIM card needs it)
//-----------------------------------------------------------------------------
enumunlockGSM powerGSM(unsigned int power, char* PINCode) 
{
    int i;

    if (!gsminit)
        return GSM_NOT_INIT;

    // Check if we are already powered on/off
    MUTEX_ACQUIRE(hMutGSMInfo);
    i=GSMInfo.Power;
    MUTEX_RELEASE(hMutGSMInfo);
    if (i==power)
        return GSM_OK;

    if (power) 
    {
        //  initLEUART0(57600); //1!!!!!!!!!!!!!!
        // Clear PE12/RTS
        GPIO->P[4].DOUTCLR = (1<<12);
        // Enable PA9/PSU_BOOSTER
        GPIO->P[0].DOUTSET = (1<<9); // PSU_BOOSTER

			
        OS_WAIT(1000);
        i=0;
 
        // Try 3 times to poweron the module
        for (i=0; i<3; i++) 
        {

			
					// Assert PA3/IGT for 65 uS to module..
					GPIO->P[0].DOUTSET = (1<<3); // GSM_ON
					delay(40);
					GPIO->P[0].DOUTCLR = (1<<3); // GSM_ON

					OS_WAIT(500);

					// Assert PA3/IGT for 1 second to module..
					GPIO->P[0].DOUTSET = (1<<3); // GSM_ON
					OS_WAIT(1000);
					GPIO->P[0].DOUTCLR = (1<<3); // GSM_ON

					enableLEUART0(TRUE);
          OS_WAIT(1000);

	/*
					// Assert PA3/IGT for 1 second to module..
            GPIO->P[0].DOUTSET = (1<<3); // GSM_ON
            OS_WAIT(1000);
            GPIO->P[0].DOUTCLR = (1<<3); // GSM_ON
            enableLEUART0(TRUE);
            OS_WAIT(1000);
*/      
            // See if the module responds to a "AT" with "OK"
            if (sendGSM("AT\r",2000)==GSM_OK)
                break;
        }

        // Did not succeed in getting a OK back on the AT
        if (i>=3) // was: "if (i==2)", changed 20110502
            return GSM_NO_ANSWER;

        GSMInfo.Power=TRUE;
    }
    else
    {
        enableLEUART0(FALSE);
        GPIO->P[4].DOUTCLR = (1<<12); // GSM_RTS
        // Disable PSU_BOOSTER
        GPIO->P[0].DOUTCLR = (1<<9); // PSU_BOOSTER
        GSMInfo.Power=FALSE;
    }
    return GSM_OK;
}

//----------------------------------------------------------------------------
// isPowerGSM()
//   Returns true if the GSM module is powered up
//----------------------------------------------------------------------------
int isPowerGSM(void) {
  return GSMInfo.Power;
}


//----------------------------------------------------------------------------
// initGSM()
// Initialize GSM interface
// Input: txcb: Callback function that will send a character to the GSM module
//        rxcb: Callback function that we can receive characters from the GSM module with
//----------------------------------------------------------------------------
void initGSM(/*GSMTXCbFunc txcb, GSMRXCbFunc rxcb*/void) {
  int i;

    if(gsminit)
        return;
    
  GPIO->P[0].DOUTCLR = (1<<3); // GSM_ON
  GPIO->P[0].DOUTCLR = (1<<9); // PSU_BOOSTER
  GPIO->P[4].DOUTCLR = (1<<12); // GSM_RTS

  GPIO->P[0].DOUTSET = (1<<2); // GSM_RUNNING with pullup
  GPIO->P[4].DOUTCLR = (1<<9); // GSM_CTS with pulldown
  
  /* Pin PA2 (GSM_RUNNING) is configured to Input enabled with pull-up and filter */
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_INPUTPULLFILTER;
  /* Pin PA3 (GSM_ON) is configured to Push-pull */
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;
  /* Pin PE9 (GSM_CTS) is configured to Input enabled with pull-down and filter */
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE9_MASK) | GPIO_P_MODEH_MODE9_INPUTPULLFILTER;
  /* Pin PE12 (GSM_RTS) is configured to Push-pull */
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE12_MASK) | GPIO_P_MODEH_MODE12_PUSHPULL;
  /* Pin PA9 (PSU_BOOSTER) is configured to Push-pull */
  GPIO->P[0].MODEH = (GPIO->P[0].MODEH & ~_GPIO_P_MODEH_MODE9_MASK) | GPIO_P_MODEH_MODE9_PUSHPULL;
    
  //initLEUART0(57600);
  initLEUART0(115200); // changed to 20151103 as the EHS5 is fixed at 115200 baud
  
  memset(&GSMInfo,0,sizeof(GSMInfo));

  MUTEX_INIT(hmutGSM);
  MUTEX_INIT(hMutGSMInfo);
  MUTEX_INIT(hmutCBGSM);
  SEM_INIT(hsemGSM);
  
  for (i=0; i<MaxCbList; i++)
    CbList[i].cb=NULL;

  cbTX=putcharLEUART0;//txcb; // Used for transmitting a character to the GSM module
  cbRX=getcharLEUART0;//rxcb; // Used for receiving a character from the GSM module

  START_THREAD_USR (thGSM, 0,&thGSMStack, sizeof(thGSMStack));

  gsminit=TRUE;
}

