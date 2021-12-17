//=============================================================================
// GSM.H                                                           20060615 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#ifndef _GSM_
#define _GSM_
#include "System.h"

//-----------------------------------------------------------------------------
// Constants...
//-----------------------------------------------------------------------------
// Defined here also as UART1 needs to manipulate the RTS pin !!! 
// (a little dirty....)
#define IO_SET_GSM IOSET0
#define IO_CLR_GSM IOCLR0

#define GSM_RTS    (1<<22) // P0.22 is RTS to GSM

//-----------------------------------------------------------------------------
// List of all the answers/URC from the module we recognize
//-----------------------------------------------------------------------------
typedef enum {
  GSMRESP_OK,            // OK

  GSMRESP_REVISION,          // REVISION 04.00

  // TCP/IP Handling
  GSMRESP_SIS,           // ^SIS
  GSMRESP_SISW,          // ^SISW
  GSMRESP_SISR,          // ^SISR

  // Outgoing SMS handling
  GSMRESP_CMGS,          // +CMGS <mr>
  GSMRESP_SMS_PROLOG,    // Not passed on (you won't see it)

  // Incoming SMS handling
  GSMRESP_CMTI,          // +CMTI: <mem>, <index>
  GSMRESP_CMGL,          // +CMGL <index>,<stat>,[<alpha>],<length> <CR> <LF> <text>
  GSMRESP_CMGR,          // +CMGR 

  // SIM card handling
  GSMRESP_CPIN,          // +CPIN: <text>
  GSMRESP_SCKS,          // ^SCKS: <m>
  GSMRESP_SCID,          // ^SCID: <text>
  GSMRESP_CPBR,          // +CPBR: 1,"+12345678",145,"Yawsers"

  // Network specific
  GSMRESP_COPS,          // +COPS
  GSMRESP_CSQ,           // +CSQ: <rssi>, <ber>
  GSMRESP_CREG,          // +CREG: <stat>, <lac>, <ci>
  GSMRESP_SMONG,         // 
  GSMRESP_SMOND,         //
  GSMRESP_SNMON,
  
  GSMRESP_SHUTDOWN,      // ^SHUTDOWN
  GSMRESP_EXIT,          // "EXIT: "

  // Voice communication
  GSMRESP_CRING,         // +CRING: <text>
  GSMRESP_SCNI,          // ^SCNI: <id>, <cs>, <number>, <type>
  GSMRESP_BUSY,          // BUSY
  GSMRESP_NO_DIALTONE,   // NO DIALTONE
  GSMRESP_NO_CARRIER,    // NO CARRIER
  GSMRESP_CLIP,          // +CLIP: <number>, <type>, , [, <alpha>][, <CLI validity>]

  // Errors
  GSMRESP_ERROR,         // ERROR 
  GSMRESP_CMS_ERROR,     // +CMS ERROR: <num>
  GSMRESP_CME_ERROR,     // +CME ERROR: <num>

  // Internal messages
  GSMRESP_NOT_SUPPORTED  // Response from GSM is not supported
} enumGSMresp;




//----------------------------------------------------------------------------
// Type of SMS received
//----------------------------------------------------------------------------
typedef enum {
  SMS_TEXT=0,
  SMS_BINARY,
  SMS_UNKNOWN
} enumSMSType;


//----------------------------------------------------------------------------
// State of PIN code
//----------------------------------------------------------------------------
typedef enum {
  SIM_READY,  // PIN code is ready
  SIM_PIN,    // PIN code is required
  SIM_PUK,    // PUK code is required
  SIM_PIN2,   // PIN2 code is required
  SIM_PUK2,   // PIN code is required
  SIM_UNKNOWN // Unknown state
} enumSIMState;


//----------------------------------------------------------------------------
// Structures that holds data for all the responses from the GSM module
//----------------------------------------------------------------------------

// Revision 04.00
typedef struct {
  int revision;
} tdefRevision;

// TCP/IP handling
// ^SIS: <srvProfileId>, <urcCause>[, [<urcInfoId>][, <urcInfoText>]]
typedef struct {
  int profile;
  int cause;
  int id;
  char Text[128];
} tdefSIS;

// ^SISW
// URC: ^SISW: <srvProfileId>, <urcCauseId>
// Response: ^SISW: <srvProfileId>, <cnfWriteLength>, <unackData>
typedef struct {
  int URC;      // Set to true if it's a URC that arrived, false if it's a response to a at^sisw command
  int profile;  // Always present
  int cause;    // only present in a URC
  int writelen; // only present in a response
  int unackdata;// only present in a response
} tdefSISW;


// ^SISR
// URC: ^SISR: <srvProfileId>, <urcCauseId>
// Response: ^SISR: <srvProfileId>, <cnfReadLength>[, <remainUdpPacketLength>]
typedef struct {
  int profile;
  int parm2; // Can either be cause or readlen...
  int udplen;
} tdefSISR;


// Outgoing SMS handling
// +CMGS <mr>
typedef  struct {
  unsigned short mr;
} tdefCMGS;

// Incoming SMS handling
// +CMTI <mem>, <index>
typedef struct {
  char mem[3];   
  unsigned char Index;
} tdefCMTI;

// +CMGL: <index>, <stat>, <oa>/<da>, [<alpha>], [<scts>] <data>
typedef struct {
  unsigned char Index;
  unsigned char Status;
  unsigned short Year;  // 2000..99
  unsigned char Month;  // 01..12
  unsigned char Date;   // 01..31
  unsigned char Hour;   // 00..23
  unsigned char Minute; // 00..59
  unsigned char Second; // 00..59
} tdefCMGL;

// +CMGR: <stat>, <oa>, [<alpha>], <scts> <data>
typedef struct {
  char oa[21];          // The sender
  enumSMSType Type;     // 0=Text, 1=Binary
  unsigned short Year;  // 2000..99
  unsigned char Month;  // 01..12
  unsigned char Date;   // 01..31
  unsigned char Hour;   // 00..23
  unsigned char Minute; // 00..59
  unsigned char Second; // 00..59
  unsigned char Length; // Length of message
  char text[161];       // The message
} tdefCMGR;

// SIM card handling
// +CPIN:
typedef  struct {
  enumSIMState State;
} tdefCPIN;

// ^SCKS <m>
typedef struct {
  unsigned char m;       
} tdefSCKS;

// ^SCID <text>
typedef struct {
  char CID[32];
} tdefSCID;

// +CPBR: 1,"+12345678",145,"Yawsers"
typedef struct {
  char Number[21];
  unsigned int type;
  char Name[32];
} tdefCPBR;


// Network specific
// +COPS: (2,\"D2\",,\"26202\"),(1,\"E-Plus\",,\"26203\"),(1,\"TD1\",,\"26201\"),(3,\"Interkom\",,\"26207\"),,(0-4),(0,2)
typedef struct {
  unsigned short Current;
  unsigned char Status;
  unsigned short Operators[16];
} tdefCOPS;

// +CSQ <rssi>, <ber>
typedef struct {
  unsigned char RSSI;   
  unsigned char BER;
} tdefCSQ;

// +CREG <stat>, <lac>, <ci>
typedef struct {
  unsigned char Stat;   
  unsigned short LAC;
  unsigned short CI;
} tdefCREG;

// ^SMOND
typedef struct {
  unsigned short ServingMCC;   
  unsigned short ServingMNC;   
  unsigned short ServingLAC;
  unsigned short ServingCI;
  unsigned short ServingBSIC;
  unsigned short ServingChannel;
  unsigned short ServingRxLev;
  struct {
    unsigned short MCC;
    unsigned short MNC;
    unsigned short LAC;
    unsigned short CI;
    unsigned short BSIC;
    unsigned short Channel;
    unsigned short RxLev;
  } Neighbour[6];
  int TA;
} tdefSMOND;


// ^SMOND
typedef struct {
  unsigned char numValid;
  struct {
    unsigned short PLMN; // MCC+MNC of the provider
    unsigned char  Band; // 0=GSM 900, 1=GSM 1800, 4=GSM 850, 8=GSM 1900
    unsigned char  RAT;  // 0=GSM, 3= GSM with GPRS
    unsigned short LAC;  // Location Area Code
    unsigned short CI;   // CellID
    unsigned short ARFCN;// Absolute Radio Frequency Channel Number 
    unsigned short RSSI; // RSSI level
  } CellInfo[32];
} tdefSNMON;



//GPRS Monitor
//BCCH  G  PBCCH  PAT MCC  MNC  NOM  TA      RAC                              # Cell #
//  88  1  -      4   238   02  1    -       9F           
typedef struct {
  unsigned char G;   
} tdefSMONG;


// Voice communication
// +CRING <text>
typedef struct {
  char text[21];  
} tdefCRING;
// +SCNI <id>, <cs>, <number>, <type>
typedef struct {
  unsigned char ID;      
  unsigned char CS;
  char Number[21];
  unsigned char Type;
} tdefSCNI;

// +CLIP: <number>, <type>, , [, <alpha>][, <CLI validity>]
typedef struct {
  char Number[21];
  unsigned int Type;
} tdefCLIP;

// Errors
// +CMS <ERROR> <error> 
typedef struct {
  unsigned short Error; 
} tdefCMS_ERROR;

// +CME <ERROR> <error> 
typedef struct {
  unsigned short Error; 
} tdefCME_ERROR;

// All other
typedef struct {
  char *Text;
} tdefGSM_RESP_NOT_SUPPORTED;


//----------------------------------------------------------------------------
// Union with all responses
//----------------------------------------------------------------------------
typedef union {

  tdefRevision REVISION;

  // TCP/IP handling
  tdefSIS SIS;
  tdefSISW SISW;
  tdefSISR SISR;

  // Outgoing SMS handling
  tdefCMGS CMGS;  
  // Incoming SMS handling
  tdefCMTI CMTI;
  tdefCMGL CMGL;
  tdefCMGR CMGR;
  // SIM card handling
  tdefCPIN CPIN;
  tdefSCKS SCKS;
  tdefSCID SCID;
  tdefCPBR CPBR;

  // Network specific
  tdefCOPS COPS;
  tdefCSQ  CSQ;
  tdefCREG CREG;
  tdefSMONG SMONG;
  tdefSMOND SMOND;
  tdefSNMON SNMON;
  
  // Voice communication
  tdefCRING CRING;
  tdefSCNI  SCNI;
  tdefCLIP  CLIP;
  // Errors
  tdefCMS_ERROR CMS_ERROR;
  tdefCME_ERROR CME_ERROR;
  // All other
  tdefGSM_RESP_NOT_SUPPORTED GSM_RESP_NOT_SUPPORTED;
} tdefGSMRespData;


//----------------------------------------------------------------------------
// Reason codes for unlockGSM()
//----------------------------------------------------------------------------
typedef enum {
  GSM_OK=0,      // OK received
  GSM_TIMEOUT,   // Timeout waiting for answer 
  GSM_ERROR,     // ERROR received
  GSM_CME_ERROR, // CME ERROR received
  GSM_CMS_ERROR, // CMS ERROR received
  GSM_NO_ANSWER, // Module does not answer
  GSM_INIT_ERR,  // Error in initializing module
  GSM_NO_POWER,  // GSM module is not powered on
  GSM_NO_BASE,   // GSM module is not currently connected to a base station
  GSM_VOICECALL, // A voicecall is incoming
  GSM_NO_VOICECALL, // No voicecall incoming
  GSM_NOT_INIT,  // initGSM() has not been called   
} enumunlockGSM;


//----------------------------------------------------------------------------
// Callback function for receiving response information from GSM module
// 
//----------------------------------------------------------------------------
typedef void (*GSMResponseCbFunc)(enumGSMresp response, tdefGSMRespData responsedata, void* arg);


//----------------------------------------------------------------------------
// Callback function for transmitting characters to the GSM module
//----------------------------------------------------------------------------
typedef int(*GSMTXCbFunc)(char ch);


//----------------------------------------------------------------------------
// Callback function for receiving characters from the GSM module
//----------------------------------------------------------------------------
typedef int(*GSMRXCbFunc)(char* ch);


//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
typedef struct {
  unsigned char Power;         // True if the GSM module is powered up
  unsigned char Initialized;   // True when all init commands has been sent to GSM after a power-up
  unsigned char SIMCard;       // True if a SIM card is detected
  unsigned char Connected;     // True if connected to a basestation
  unsigned char Roaming;       // True if we are roaming
  unsigned char IncomingVoice; // True if we are being called by someone
  unsigned char OffHook;       // True if the "phone" is lifted (off the hook)
  char IncomingNumber[21];     // Caller id (if not suppressed) of the incoming voice call
  
  // CellID and LAC gets updated whenever a change of base station occurs
  unsigned int LAC;            // LAC (Location Area Code) of the base we are currently connected to
  unsigned int CellID;         // CellID of the base we are currently connected to
  
  // The "operator" struct is _only_ updated after a call to getCurrentProviderListGSM() !!
  struct {
    unsigned short plmn[16];   // plmn of operator (decimal, f.ex 23802, Denmark (238), Sonofon(02)) 
    char Name[16][32+1];       // Name of operator
    unsigned char Status[16];  // 0 Unknown, 1 operator available, 2 current operator, 3 operator forbidden
  } Operator;

  unsigned int Revision;       // Version number of the GSM module multiplied with 100 (4.00 is set as 400)
  
  // MCC and MNC gets updated _only_ after a call to powerGSM(TRUE) and getCellInfoGSM()
  unsigned short MCC;          // Mobile Country Code of the current base
  unsigned short MNC;          // Mobile Network Code of the current base
} tdefGSMInfo;


//----------------------------------------------------------------------------
// Query the module for its IMEI number
// Output: IMEI: The modules IMEI code will be copied to this array
// Returns: enumunlockGSM: Result of operation, if GSM_OK then IMEI is defined
//----------------------------------------------------------------------------
enumunlockGSM getIMEIGSM(char* imei);

//-----------------------------------------------------------------------------
// powerGSM()
//   Switch GSM module on or off
// Input:
//   power: true if swithing on, false if powering down
//   PINCode: the 4 ASCII digit pincode to use (if SIM card needs it)
//-----------------------------------------------------------------------------
enumunlockGSM powerGSM(unsigned int power, char* PINCode);

//----------------------------------------------------------------------------
// initGSM()
// Initialize GSM interface
// Input: txcb: Callback function that will send a character to the GSM module
//        rxcb: Callback function that we can receive characters from the GSM module with
//----------------------------------------------------------------------------
void initGSM(/*GSMTXCbFunc txcb, GSMRXCbFunc rxcb*/void);





#endif

