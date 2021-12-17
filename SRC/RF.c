#include <stdio.h>
#include <string.h>
#include "efm32.h"
#include "system.h"
#include "si4461.h"
#include "desp.h"
#include "debug.h"
#include "pmon.h"
#include "pmondef.h"
#include "DataWhitening.h"

unsigned char RF_PARAMETERS_868_STD[] = \
{         \
    0x41,  /*U8 FREQ_CONTROL_INTE */ \
    0x0E,  /*U8 FREQ_CONTROL_FRAC_2 */ \
    0x56,  /*U8 FREQ_CONTROL_FRAC_1 */ \
    0xA5,  /*U8 FREQ_CONTROL_FRAC_0 */ \
    0x00,  /*U8 FREQ_CONTROL_CHANNEL_STEP_SIZE_1 */ \
    0x00,  /*U8 FREQ_CONTROL_CHANNEL_STEP_SIZE_0 */ \
    0x20,  /*U8 FREQ_CONTROL_W_SIZE */ \
    0xFF,  /*U8 FREQ_CONTROL_VCOCNT_RX_ADJ */ \
    66,  /*U8 GLOBAL_XO_TUNE */ \
    0x40,  /*U8 GLOBAL_CLK_CFG */ \
    0x60,  /*U8 GLOBAL_CONFIG */ \
    0x04,  /*U8 PREAMBLE_TX_LENGTH */ \
    0x14,  /*U8 PREAMBLE_CONFIG_STD_1 */ \
    0x00,  /*U8 PREAMBLE_CONFIG_NSTD */ \
    0x0F,  /*U8 PREAMBLE_CONFIG_STD_2 */ \
    0x31,  /*U8 PREAMBLE_CONFIG */ \
    0x00,  /*U8 PREAMBLE_PATTERN_31_24 */ \
    0x00,  /*U8 PREAMBLE_PATTERN_23_16 */ \
    0x00,  /*U8 PREAMBLE_PATTERN_15_8 */ \
    0x00,  /*U8 PREAMBLE_PATTERN_7_0 */ \
    0x03,  /*U8 SYNC_CONFIG */ \
    0xCB,  /*U8 SYNC_BITS_31_24 */ \
    0x89,  /*U8 SYNC_BITS_23_16 */ \
    0xCB,  /*U8 SYNC_BITS_15_8 */ \
    0x89,  /*U8 SYNC_BITS_7_0 */ \
    0x84,  /*U8 PKT_CRC_CONFIG */ \
    0x82,  /*U8 PKT_CONFIG1 */ \
    0x0A,  /*U8 PKT_LEN */ \
    0x01,  /*U8 PKT_LEN_FIELD_SOURCE */ \
    0x00,  /*U8 PKT_LEN_ADJUST */ \
    0x00,  /*U8 PKT_FIELD_1_LENGTH_12_8 */ \
    0x01,  /*U8 PKT_FIELD_1_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_FIELD_1_CONFIG */ \
    0xA2,  /*U8 PKT_FIELD_1_CRC_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_2_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_FIELD_2_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_FIELD_2_CONFIG */ \
    0x0A,  /*U8 PKT_FIELD_2_CRC_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_3_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_FIELD_3_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_FIELD_3_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_3_CRC_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_4_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_FIELD_4_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_FIELD_4_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_4_CRC_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_5_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_FIELD_5_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_FIELD_5_CONFIG */ \
    0x00,  /*U8 PKT_FIELD_5_CRC_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_1_LENGTH_12_8 */ \
    0x01,  /*U8 PKT_RX_FIELD_1_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_RX_FIELD_1_CONFIG */ \
    0x82,  /*U8 PKT_RX_FIELD_1_CRC_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_2_LENGTH_12_8 */ \
      63,  /*U8 PKT_RX_FIELD_2_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_RX_FIELD_2_CONFIG */ \
    0x0A,  /*U8 PKT_RX_FIELD_2_CRC_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_3_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_RX_FIELD_3_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_RX_FIELD_3_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_3_CRC_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_4_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_RX_FIELD_4_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_RX_FIELD_4_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_4_CRC_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_5_LENGTH_12_8 */ \
    0x00,  /*U8 PKT_RX_FIELD_5_LENGTH_7_0 */ \
    0x00,  /*U8 PKT_RX_FIELD_5_CONFIG */ \
    0x00,  /*U8 PKT_RX_FIELD_5_CRC_CONFIG */ \
    0x03,  /*U8 MODEM_MOD_TYPE */ \
    0x00,  /*U8 MODEM_MAP_CONTROL */ \
    0x07,  /*U8 MODEM_DSM_CONTROL */ \
    0x24,  /*U8 MODEM_DATA_RATE_2 */ \
    0x8F,  /*U8 MODEM_DATA_RATE_1 */ \
    0x60,  /*U8 MODEM_DATA_RATE_0 */ \
    0x01,  /*U8 MODEM_TX_NCO_MODE_3 */ \
    0x8C,  /*U8 MODEM_TX_NCO_MODE_2 */ \
    0xBA,  /*U8 MODEM_TX_NCO_MODE_1 */ \
    0x80,  /*U8 MODEM_TX_NCO_MODE_0 */ \
    0x00,  /*U8 MODEM_FREQ_DEV_2 */ \
    0x14,  /*U8 MODEM_FREQ_DEV_1 */ \
    0x02,  /*U8 MODEM_FREQ_DEV_0 */ \
    0x00,  /*U8 MODEM_MDM_CTRL */ \
    0x08,  /*U8 MODEM_IF_CONTROL */ \
    0x03,  /*U8 MODEM_IF_FREQ_2 */ \
    0xC0,  /*U8 MODEM_IF_FREQ_1 */ \
    0x00,  /*U8 MODEM_IF_FREQ_0 */ \
    0x00,  /*U8 MODEM_DECIMATION_CFG1 */ \
    0x30,  /*U8 MODEM_DECIMATION_CFG0 */ \
    0x00,  /*U8 MODEM_AFC_GEAR*/ \
    0x23,  /*U8 MODEM_AFC_WAIT*/ \
    0xC9,  /*U8 MODEM_AFC_GAIN_1*/ \
    0x70,  /*U8 MODEM_AFC_GAIN_0*/ \
    0x00,  /*U8 MODEM_AFC_LIMITER_1*/ \
    0xDC,  /*U8 MODEM_AFC_LIMITER_0*/ \
    0xE0,  /*U8 MODEM_AFC_MISC*/ \
    0x00,  /*U8 MODEM_AFC_ZIFOFF*/ \
    0x08,  /*U8 MODEM_CLKGEN_BAND */ \
    0x12,  /*U8 MODEM_RSSI_CONTROL */ \
    0x01,  /*U8 MODEM_TX_RAMP_DELAY */ \
    0x00,  /*U8 MODEM_BCR_OSR_1*/ \
    0x6D,  /*U8 MODEM_BCR_OSR_0*/ \
    0x04,  /*U8 MODEM_BCR_NCO_OFFSET_2*/ \
    0xB7,  /*U8 MODEM_BCR_NCO_OFFSET_1*/ \
    0xE1,  /*U8 MODEM_BCR_NCO_OFFSET_0*/ \
    0x04,  /*U8 MODEM_BCR_GAIN_1*/ \
    0x6E,  /*U8 MODEM_BCR_GAIN_0*/ \
    0x02,  /*U8 MODEM_BCR_GEAR*/ \
    0x00,  /*U8 MODEM_BCR_MISC1*/ \
    0x00,  /*U8 MODEM_BCR_MISC0*/ \
    0x20,  /*U8 PA_MODE */ \
    0x7F,  /*U8 PA_PWR_LVL */ \
    0x7F,  /*U8 PA_BIAS_CLKDUTY */ \
    0x5D,  /*U8 PA_TC */ \
    0x01,  /*U8 SYNTH_PFDCP_CPFF*/ \
    0x05,  /*U8 SYNTH_PFDCP_CPINT*/ \
    0x0B,  /*U8 SYNTH_VCO_KV*/ \
    0x05,  /*U8 SYNTH_LPFILT3*/ \
    0x02,  /*U8 SYNTH_LPFILT2*/ \
    0x00,  /*U8 SYNTH_LPFILT1*/ \
    0x03,  /*U8 SYNTH_LPFILT0*/ \
    0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00, 0x03, 0x03, /* MODEM_CHFLT_RX1_CHFLT_COE13-0*/ \
    0x15, 0xF0, 0x3F, 0x00,  /* MODEM_CHFLT_RX1_CHFLT_COEM0-3*/ \
    0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00, 0x03, 0x03, /* MODEM_CHFLT_RX2_CHFLT_COE13-0*/ \
    0x15, 0xF0, 0x3F, 0x00  /* MODEM_CHFLT_RX2_CHFLT_COEM0-3*/ \
};

#define DevTypeUsbDongle      5
#define DevTypeLommyManager 103

#define MSGTYPE_AVAILABLE   0x00
#define MSGTYPE_CONNECT     0x01
#define MSGTYPE_PMON_REQ    0x02
#define MSGTYPE_PMON_RESP   0x03

#define FLAG_FIRST 0x01
#define FLAG_LAST  0x02
#define FLAG_DW    0x04

#define MAX_DATA_LEN 64
#define MAX_DATA_BLOCK_LEN 44

typedef __packed struct {
  U8 totlen;
  U32 dst;        // 4 byte ID
  U32 src;        // 4 byte ID
  U8 port;        // Bit 7..6 encryption info: 00: no encryption, 01: SW encrypt, 10: HW encrypt, 11: Reserved
                  // Bit 0..5 port number: 0x3F is "broadcast port"
  U8 deviceinfo;  // Bit 7..6 00: controlled listen, 01: Sleeps/polls, 10: Reserved, 11: Reserved
                  // Bit 5..4 00: End device, 01: Range extender, 10: Access point, 11: Reserved
                  // Bit 3 Reserved
                  // Bit 2..0: Hop count
  U8 tractid;     // Transaction ID
} FWPHeadType;
typedef __packed struct
{
    U8 DevType;
    U16 VendorID;
    U8 MsgType;
    U8 Flag;
    U8 Tno;
} RFHeadType;
typedef __packed struct
{
    FWPHeadType FWPHead;
    RFHeadType RFHead;
    U8 Data[MAX_DATA_LEN];
} TXPacketType;
typedef __packed struct
{
    FWPHeadType FWPHead;
    U8 Dongle;
    RFHeadType RFHead;
    U8 Data[MAX_DATA_LEN];
} RXPacketType;

static U64 thRFStk[300];
static U8 Running;
static int init = FALSE;
static unsigned int MyAddr;
static int Connected = 0;
static U32 MyPartner = 0;
static RXPacketType MyRxPacket;
static TXPacketType TxP;
static U8 RxOfs = 0;
static U8 TxTno = 0;
static U8 RxTno = 0;

//****************************
static U8 GetNextTxTno(void) //1-255
{
    if(++TxTno == 0)
    {
        TxTno=1;
    }
    return TxTno;
}
//*********************************
static U8 Tx(U8* buf, U8 len)
{
	int rc;
    int i;

    do 
    {
        rc = sendPacketSI4461(buf, len);
        if (rc) OS_WAIT(10);
            i++;
    } while (i<5 && rc);

    return rc; //0 if ok
}
//********************************************************
static U8 Send(U32 Dest, TXPacketType* P, U8 DataLen)
{
    TXPacketType MyP;
    U8 SplitLen = 0;
    static int TransID = 0;
  
    if(init == FALSE)
    {
        return 1;
    }
    
    memcpy(&MyP, P, sizeof(TXPacketType));

    if(DataLen > MAX_DATA_BLOCK_LEN)
    {
        SplitLen = DataLen - MAX_DATA_BLOCK_LEN;
        DataLen = MAX_DATA_BLOCK_LEN;
    }
    
    MyP.FWPHead.dst = Dest;
    MyP.FWPHead.src = MyAddr;
    MyP.FWPHead.port = 0x3F;
    MyP.FWPHead.deviceinfo = 0; // Hopcount in Bit 2..0
    MyP.FWPHead.tractid = TransID;
    TransID++;

    //packet ready, send it now - use dw and split in two if necessary
    if(SplitLen == 0) 
    {
        //only one packet
        MyP.FWPHead.totlen = sizeof(FWPHeadType) - 1 + sizeof(RFHeadType) + DataLen;
        MyP.RFHead.Flag = FLAG_FIRST + FLAG_LAST;
        MyP.RFHead.Tno = GetNextTxTno();
        if(IsDWRequired(MyP.Data, DataLen))
        {
            DWData(MyP.Data, DataLen);
            MyP.RFHead.Flag |= FLAG_DW;
        }
        return Tx((U8*)&MyP, MyP.FWPHead.totlen + 1);
    }
    else
    {
        //split into two packets
        MyP.FWPHead.totlen = sizeof(FWPHeadType) - 1 + sizeof(RFHeadType) + DataLen;
        MyP.RFHead.Flag = FLAG_FIRST;
        MyP.RFHead.Tno = GetNextTxTno();
        if(IsDWRequired(MyP.Data, DataLen))
        {
            DWData(MyP.Data, DataLen);
            MyP.RFHead.Flag |= FLAG_DW;
        }
        if(Tx((U8*)&MyP, MyP.FWPHead.totlen + 1) == 0) //send first
        {
            MyP.FWPHead.totlen = sizeof(FWPHeadType) - 1 + sizeof(RFHeadType) + SplitLen;
            MyP.RFHead.Flag = FLAG_LAST;
            memmove(MyP.Data, &MyP.Data[DataLen], SplitLen);
            MyP.RFHead.Tno = GetNextTxTno();
            if(IsDWRequired(MyP.Data, SplitLen))
            {
                DWData(MyP.Data, SplitLen);
                MyP.RFHead.Flag |= FLAG_DW;
            }
            return Tx((U8*)&MyP, MyP.FWPHead.totlen + 1); //send last
        }
    }
    return 1;
}

//***************************
static void Broadcast(void)
{
    TXPacketType P;
    
    P.RFHead.DevType = DevTypeLommyManager;
    P.RFHead.VendorID = 0xFFFF;
    P.RFHead.MsgType = MSGTYPE_AVAILABLE;
    P.RFHead.Flag = FLAG_FIRST + FLAG_LAST;
    Send(0xFFFFFFFF, &P, 0);
}
//******************************************
static void HandlePacket(RXPacketType* P)
{
    static int prevtxlen = 0;
    int txlen;
    
    /*if(P->RFHead.MsgType == MSGTYPE_CONNECT) //not implemeted...dont know if it ever will be!
    {
        MyPartner = P->FWPHead.src;
        Connected = 50;
    }*/
    
    if(Connected)
    {
        if(P->RFHead.Tno == RxTno) //duplicate?
        {
            //re-transmit last packet
            if(prevtxlen)
            {
                //GPIO->P[0].DOUTSET = 0x04; // TEST LED
                Send(MyPartner, &TxP, prevtxlen);
            }
        }
        else 
        {
            RxTno = P->RFHead.Tno;
            if(P->RFHead.MsgType == MSGTYPE_PMON_REQ)
            {
                txlen = handlePMON((pmonFrames*)P->Data);
                prevtxlen = txlen;
                if(txlen && (txlen <= MAX_DATA_LEN))
                {
                    memcpy(&TxP.FWPHead, &P->FWPHead, sizeof(FWPHeadType));
                    memcpy(&TxP.RFHead, &P->RFHead, sizeof(RFHeadType));
                    memcpy(TxP.Data, P->Data, txlen);
                    TxP.RFHead.MsgType = MSGTYPE_PMON_RESP;
                    Send(MyPartner, &TxP, txlen);
                }
            }
        }
    }
}

//************************************************
void cbRF(U8* buf, U8 len, U8 lqi, S16 rssi)
{
    static U8 GotFirst = FALSE;
    int dlen;
    RXPacketType P;

    P.FWPHead.totlen = len;
    memcpy((U8*)&P.FWPHead.dst, buf, sizeof(RXPacketType));

    if(P.FWPHead.dst == MyAddr)
    {
        if(P.Dongle == DevTypeUsbDongle) 
        {
            if(P.RFHead.DevType == DevTypeLommyManager)
            {    
                Connected = 50;
                MyPartner = P.FWPHead.src;
                dlen = len - (sizeof(FWPHeadType) + sizeof(RFHeadType)); //calculate length of Data[] section
                
                memcpy(&MyRxPacket.RFHead, &P.RFHead, sizeof(RFHeadType)); //copy header section only
                if( (P.RFHead.Flag & FLAG_FIRST) && !(P.RFHead.Flag & FLAG_LAST)) //first but not last packet?
                {
                    memcpy(&MyRxPacket.Data[0], &P.Data, dlen);
                    if(P.RFHead.Flag & FLAG_DW)
                    {
                        DWData(MyRxPacket.Data, dlen);
                    }
                    GotFirst = TRUE;
                    RxOfs = dlen;
                }
                else if((GotFirst || P.RFHead.Flag & FLAG_FIRST) && P.RFHead.Flag & FLAG_LAST) //if first and last
                {
                    memcpy(&MyRxPacket.Data[RxOfs], &P.Data, dlen);
                    if(P.RFHead.Flag & FLAG_DW)
                    {
                        DWData(&MyRxPacket.Data[RxOfs], dlen);
                    }
                    GotFirst = FALSE;
                    RxOfs = 0;
                    HandlePacket(&MyRxPacket);
                }
            }
        }
    }
}

//*************************
static void thRF(void)
{
    RFParametersType rfSettings;
    char deviceID[11];

    checkLevelDESP(FALSE);
    getDeviceIDDESP(deviceID);
    MyAddr = strtoul(deviceID,NULL,10);    
    
    setupSI4461();
    memcpy(&rfSettings, RF_PARAMETERS_868_STD, sizeof(rfSettings));
    if(initSI4461(&rfSettings))
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"si4461 found");
        setCallbackSI4461(cbRF);
    }
    else
    {
        //si4461 is not mounted, so stay in dummy loop
        messageDebug(DBG_WAR, __MODULE__, __LINE__,"si4461 not found");
        while(Running)
        {
            OS_WAIT(400);
        }
    }
    
    init = TRUE;
    
    while(Running)
    {
        if(Connected)
        {
            Connected--;
            if(Connected == 0) //going from connected to unconnected
            {
                
            }
        }
        else
        {
            Broadcast();
        }
        OS_WAIT(400);
    }
}

//***************************
U8 RF_IsConnected(void)
{
    return (Connected > 0);
}

//*********************
void RF_Stop(void)
{
    Running = FALSE;
}

//********************
void RF_Start(void)
{
    Running = TRUE;
    START_THREAD_USR(thRF, 0, (void*)thRFStk, sizeof(thRFStk));
}



