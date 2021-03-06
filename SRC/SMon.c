#include "efm32.h"
#include "system.h"
#include "uart0.h"
#include "SMon.h"
#include "crc16.h"
#include "pmondef.h"
#include "pmon.h"
#include "debug.h"


#define STX 0x02
#define ETX 0x03
#define ESC 0x1B

//Channels:
#define CH_CMD   'A'
#define CH_LOG   'B' 
#define CH_TERM  'C' 

#define EVT_SMON_TERM      1
#define EVT_SMON_RX_PACKET 2

static MUTEX SMonMutex;
static OS_TID mTID = NULL;

#define RX_BUF_SIZE 150 //theoretical worst case for one full packet will be 134
static uint8_t RxBuf[RX_BUF_SIZE];
static uint16_t RxCrc = 0;
static uint16_t CalcCrc = 0;
static uint16_t TxCrc = 0;

enum EStates { RXSTATE_IDLE, RXSTATE_STX, RXSTATE_CHANNEL, RXSTATE_CNT, RXSTATE_DATA, RXSTATE_DATA_ESC, RXSTATE_CRCLO, RXSTATE_CRCHI, RXSTATE_CRCLO_ESC, RXSTATE_CRCHI_ESC };
static U8 RxState;

#define PACKET_DATA_SIZE 64
typedef __packed struct 
{
    U8 Channel;
    U8 Cnt; 
    U8 DataLen;
    U8 Data[PACKET_DATA_SIZE];
} SerialPacketType;
static U8 PrevCnt = 0;

static char TermData[100];
static int TermLen=0;

static SerialPacketType RxP;
static SerialPacketType TxP;
static U64 thLogStk[300];

//******************************************************************
static void PutTxByte(uint8_t Val, uint8_t Encode, uint16_t* CRC)
{
    if(Encode && (Val == ESC))
    {
        if(CRC != NULL)
        {
            *CRC = calc_crc16(*CRC, ESC);
            *CRC = calc_crc16(*CRC, Val);
        }
        putcharUART0(ESC);
        putcharUART0(Val);
    }
    else
    {
        if(CRC != NULL)
        {
            *CRC = calc_crc16(*CRC, Val);
        }
        putcharUART0(Val);
    }
}

//************************************************
static void SendPacket(SerialPacketType* P)
{
    uint8_t i;
    
    MUTEX_ACQUIRE(SMonMutex);

    TxCrc = CRC_INIT;
    PutTxByte(ESC, 0, NULL);
    PutTxByte(STX, 0, NULL);

    PutTxByte(P->Channel, 1, &TxCrc);
    PutTxByte(P->Cnt, 1, &TxCrc);
    for (i = 0; i < P->DataLen; i++)
    {
        PutTxByte(P->Data[i], 1, &TxCrc);
    }

    PutTxByte(ESC, 0, NULL);
    PutTxByte(ETX, 0, NULL);
    PutTxByte((uint8_t)(TxCrc & 0xFF), 1, NULL);
    PutTxByte((uint8_t)(TxCrc / 0x100), 1, NULL);

    MUTEX_RELEASE(SMonMutex);
}

//****************************************
static void MySerialCB(uint16_t Len)
{
    int i;
    uint8_t rx;

  
    for(i=0; i<Len; i++)
    {
        rx = RxBuf[i];
        switch(RxState)
        {
            case RXSTATE_IDLE:
                if (rx == ESC)
                {
                    RxState = RXSTATE_STX;
                }
                break;
    
            case RXSTATE_STX:
                if (rx == STX)
                {
                    CalcCrc = CRC_INIT;
                    RxP.DataLen = 0;
                    RxState = RXSTATE_CHANNEL;
                }
                else if(rx != ESC)
                {
                    RxState = RXSTATE_IDLE;
                }
                break;
    
            case RXSTATE_CHANNEL: 
                CalcCrc = calc_crc16(CalcCrc, rx);
                RxP.Channel = rx;
                RxState = RXSTATE_CNT;
                break;
            
            case RXSTATE_CNT:
                CalcCrc = calc_crc16(CalcCrc, rx);
                RxP.Cnt = rx;
                RxState = RXSTATE_DATA;
                break;

            case RXSTATE_DATA:
                if (rx == ESC)
                {
                    RxState = RXSTATE_DATA_ESC;
                }
                else 
                {
                    CalcCrc = calc_crc16(CalcCrc, rx);
                    if(RxP.DataLen >= PACKET_DATA_SIZE)
                    {
                        RxState = RXSTATE_IDLE;
                    } 
                    else 
                    {
                        RxP.Data[RxP.DataLen] = rx;
                    }
                    RxP.DataLen++;
                }
                break;
                
            case RXSTATE_DATA_ESC:
                if (rx == STX) //unexpected STX inside data
                {
                    CalcCrc = CRC_INIT;
                    RxP.DataLen = 0;
                    RxState = RXSTATE_CHANNEL;
                }
                else if (rx == ETX)
                {
                    RxState = RXSTATE_CRCLO;
                }
                else 
                {
                    CalcCrc = calc_crc16(CalcCrc, ESC);
                    CalcCrc = calc_crc16(CalcCrc, rx);
                    if(RxP.DataLen >= PACKET_DATA_SIZE)
                    {
                        RxState = RXSTATE_IDLE;
                    } 
                    else 
                    {
                        RxP.Data[RxP.DataLen] = rx;
                    }
                    RxP.DataLen++;
                    RxState = RXSTATE_DATA;
                }
                break;
               
            case RXSTATE_CRCLO:
                if(rx == ESC)
                {
                    RxState = RXSTATE_CRCLO_ESC;
                }
                else
                {
                    RxCrc = rx;
                    RxState = RXSTATE_CRCHI;
                }
                break;

            case RXSTATE_CRCHI:
                if(rx == ESC)
                {
                    RxState = RXSTATE_CRCHI_ESC;
                }
                else
                {
                    RxCrc += (uint16_t)(rx * 0x100);
                    if (RxCrc == CalcCrc)
                    {
                        if(((RxP.Channel == CH_CMD) || (RxP.Channel == CH_TERM))&& (RxP.DataLen > 0))
                        {
                            os_evt_set(EVT_SMON_RX_PACKET, mTID);
                        }
                    } 
                    else
                    {
                        //messageDebug(DBG_INFO, __MODULE__, __LINE__, "Checksum error!");
                    }
                    RxState = RXSTATE_IDLE;
                }
                break;

            case RXSTATE_CRCLO_ESC:
                RxCrc = rx;
                RxState = RXSTATE_CRCHI;
                break;
                
            case RXSTATE_CRCHI_ESC:
                RxCrc += (uint16_t)(rx * 0x100);
                if (RxCrc == CalcCrc)
                {
                    if(((RxP.Channel == CH_CMD) || (RxP.Channel == CH_TERM))&& (RxP.DataLen > 0))
                    {
                        os_evt_set(EVT_SMON_RX_PACKET, mTID);
                    }
                } 
                else
                {
                    //messageDebug(DBG_INFO, __MODULE__, __LINE__, "Checksum error!");
                }
                RxState = RXSTATE_IDLE;
                break;
        }
    }
}

void sendTermSMon(char* Text) //max 64 chars including 0-termination 
{
  // Wait for last term message to be sent
  while (termAvailable()) OS_WAIT(10);
  
  strncpy(TermData, Text, 63);
  TermData[63] = 0;
  TermLen = strlen(TermData) + 1;
  os_evt_set(EVT_SMON_TERM, mTID);
}

static int termAvailable(void)
{
    return TermLen;
}

//***************************************************************************
static U8 HandleBulkInLog(void) // Handles logdata TO the PC 
{
    int avail, i;
    SerialPacketType P;

    avail = numCharAvailableDebug();

    if (avail > 0) 
    {  
        i=0;
        while (i < sizeof(P.Data)-1) 
        {
            // If we have more to send, go send the char
            if (avail) 
            {
                avail--;
                removeCharDebug(&P.Data[i+1]);
                i++;
                // We only send one single string in a transmission...
                if (P.Data[i] == 0)
                    break;
            }  
            else
                break;
        }
        P.Channel = CH_LOG;
        P.DataLen = i+1; //len-byte + string-len
        P.Data[0] = i; //string-len
        SendPacket(&P);
        return numCharAvailableDebug(); //more to send?
    } else
      setAllSentDebug();
    return 0;
}

//***************************************************************************
static void HandleBulkInTerm(void) // Handles terminal data TO the PC 
{
    int avail;
    SerialPacketType P;

    avail = termAvailable();

    if (avail) 
    {  
        P.Channel = CH_TERM;
        P.DataLen = avail;
        strncpy(P.Data, TermData, 63);
        P.Data[63] = 0;
        SendPacket(&P);
        TermLen = 0;
  }
}

//****************************
__task void thLog(void)
{
    U16 evt;
    int txlen=0;
    //setNameRTXMON(__FUNCTION__);
    mTID = os_tsk_self(); 
  
    while(1)
    {

      if(os_evt_wait_or(0xFFFF, 20) == OS_R_EVT) 
        {
            evt = os_evt_get();
            
            if(evt & EVT_SMON_TERM)
            {
                HandleBulkInTerm();
            }
            if(evt & EVT_SMON_RX_PACKET)
            {
                if(PrevCnt == RxP.Cnt) //duplicate?
                {
                    SendPacket(&TxP); //send prev packet again
                }
                else
                {
                    PrevCnt = RxP.Cnt;
//         if (RxP.Channel!='A')
  //                hexDumpDebug(DBG_INFO, __MODULE__, __LINE__, "Data", (void*)&RxP, sizeof(SerialPacketType));  

                    txlen = handlePMON((pmonFrames*)RxP.Data);
                    if(txlen)
                    {
                        memcpy(&TxP, &RxP, sizeof(SerialPacketType));
                        TxP.DataLen = txlen;
                        SendPacket(&TxP);
                    }
                }
            }
        }
        while(HandleBulkInLog());
    }
}

//********************************
void initSMon(int baudrate)
{
    MUTEX_INIT(SMonMutex);
    RxState = RXSTATE_IDLE;
    initUART0();
    setbaudUART0(baudrate);
    setCallbackUART0(RxBuf, 5, 10, MySerialCB);
    enableUART0(TRUE);
    START_THREAD_USR(thLog, 2, (void*)thLogStk, sizeof(thLogStk));
}





