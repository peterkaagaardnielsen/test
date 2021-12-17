#include <stdio.h>
#include <string.h>
#include "efm32.h"
#include "system.h"
#include "SPI2.h"
#include "si446x_B0_defs.h"
#include "si4461.h"
#include "GPIOIRQ.h"
//#include "RTXMonitor.h"
#include "RTC.h"

#define EVT_SI4461_INT      0x01

static int power=TRUE;

static OS_TID mTID = NULL;
static int init=FALSE;

extern int doodle[4];


#define STATE_NO_CHANGE     0
#define STATE_SLEEP         1
#define STATE_SPI_ACTIVE    2
#define STATE_READY         3
#define STATE_READY2        4
#define STATE_TX_TUNE       5
#define STATE_RX_TUNE       6
#define STATE_TX            7
#define STATE_RX            8

static RFParametersType* MyParameters;
static uint8_t abApi_Write[70];
static uint8_t resp[16];
static U64 thSI4461Stk[200];

RFStatusType RFStatus;

static si4461_callback_t MyCb = NULL;

//********************************
static uint8_t WaitForCTS(void)
{
    #define MAX_RETRY 2500
    int Retry = 0;
    int i;
    uint8_t rx;

    while(++Retry < MAX_RETRY)
    {
        setCSSPI2(0);
        rx = sendCharSPI2(CMD_CTS_READ);
        rx = receiveCharSPI2();
        setCSSPI2(1);
        if(rx == 0xFF)
        {
            return 1;
        }
        for(i = 4000; i > 0; i--) ; // ~500us
    }
    return 0;
}

//********************************************
static void SendCommand(uint8_t* Cmd, uint8_t Len)
{
    setCSSPI2(0);
    sendSPI2(Cmd, Len);            
    setCSSPI2(1);
}

//***************************************************
static uint8_t GetResponse(uint8_t* Resp, uint8_t Len)
{
    #define MAX_RETRY 2500
    int Retry = 0;
    int i;
    uint8_t rx;

    while(++Retry < MAX_RETRY)
    {
        setCSSPI2(0);
        sendCharSPI2(CMD_CTS_READ);
        rx = receiveCharSPI2();
        if(rx == 0xFF)
        {
            receiveSPI2(Resp, Len);            
            setCSSPI2(1);
            return 1;
        }
        setCSSPI2(1);
        for(i = 4000; i > 0; i--) ; // ~500us
    }
    return 0;
}

//***************************************
static uint8_t SendPowerOnCmd(void)
{
    const unsigned char Cmd[] = { CMD_POWER_UP, 0x01, 0x00,  0x01, 0x8C, 0xBA, 0x80 }; //matches 26MHz XTAL
    
    SendCommand((U8*)Cmd, sizeof(Cmd));
    return WaitForCTS();
}

//***************************************************
/*static void SetPAMode(void)
{
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PA_MODE;
    abApi_Write[4] = 0x21; 
    abApi_Write[5] = 0x7F; //0x7F = +20dbm (see data sheet)
    abApi_Write[6] = 0x00;
    abApi_Write[7] = 0x01;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
}*/

//***********************************
static void ConfigGPIO(void)
{
    abApi_Write[0] = CMD_GPIO_PIN_CFG; 
    abApi_Write[1] = 0x44; //input 0 (pull up)
    abApi_Write[2] = 0x44; //input 1 (pull up) 
    abApi_Write[3] = 0x44; //input 2 (pull up)
    abApi_Write[4] = 0x04; //input 3 (no pull up)
    SendCommand(abApi_Write, 5); 
    GetResponse(resp, 7); 
}
//***********************************
static void ClearPendingTxInt(void)
{
    abApi_Write[0] = CMD_GET_INT_STATUS; 
    abApi_Write[1] = 0x10; // Clear all but Rx-int
    abApi_Write[2] = 0; 
    abApi_Write[3] = 0; 
    SendCommand(abApi_Write, 4); 
    GetResponse(resp, 8); 
}
//***********************************
static void ClearPendingRxInt(void)
{
    abApi_Write[0] = CMD_GET_INT_STATUS; 
    abApi_Write[1] = 0x20; // Clear all but Tx-int
    abApi_Write[2] = 0; 
    abApi_Write[3] = 0; 
    SendCommand(abApi_Write, 4); 
    GetResponse(resp, 8); 
}

//***********************************
static uint8_t ClearFifo(void)
{
    abApi_Write[0] = CMD_FIFO_INFO; 
    abApi_Write[1] = 3; // clear both fifo
    SendCommand(abApi_Write, 2); 
    return GetResponse(resp, 2); 
}

//*************************************
static void StartRx(void)
{
    abApi_Write[0] = CMD_START_RX;     
    abApi_Write[1] = 0;          
    abApi_Write[2] = 0x00;        
    abApi_Write[3] = 0x00;        
    abApi_Write[4] = 0x00;         
    
    abApi_Write[5] = STATE_NO_CHANGE;          // rxtimeout
    abApi_Write[6] = STATE_READY;   // rx valid    
    abApi_Write[7] = STATE_READY;   // rx invalid     
//    abApi_Write[6] = STATE_RX;//STATE_READY;   // rx valid    
//    abApi_Write[7] = STATE_RX;//STATE_READY;   // rx invalid     
/*
    abApi_Write[5] = STATE_RX;          
    abApi_Write[6] = STATE_READY;       
    abApi_Write[7] = STATE_READY;        
*/  
    SendCommand(abApi_Write, 8);    
    WaitForCTS();
}
//*****************************
static void StartTx(void)
{
    abApi_Write[0] = CMD_START_TX;      // Use Tx Start command
    abApi_Write[1] = 0;           // Set channel number
    abApi_Write[2] = (STATE_RX << 4);          // state after Tx, start Tx immediately
    abApi_Write[3] = 0x00;          // Packet fields used, do not enter packet length here
    abApi_Write[4] = 0x00;          // Packet fields used, do not enter packet length here
    SendCommand(abApi_Write, 5);    // Send command to the radio IC
    WaitForCTS();
}

//******************************************
static U8 GetProperty(U8 PropGrp, U8 Prop)
{
    U8 result[1];
    abApi_Write[0] = CMD_GET_PROPERTY; 
    abApi_Write[1] = PropGrp;
    abApi_Write[2] = 1; 
    abApi_Write[3] = Prop; 
    SendCommand(abApi_Write, 4); 
    WaitForCTS();
    GetResponse(result, 1); 
    return result[0];
}

//*******************************************
static uint8_t SendSetup(void)
{
    ClearPendingTxInt();
    ClearPendingRxInt();
    ClearFifo();

    //SetPAMode();
    //USER parameters:
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 10;
    abApi_Write[3] = PROP_MODEM_DATA_RATE_2;
    abApi_Write[4] = MyParameters->MODEM_DATA_RATE_2;
    abApi_Write[5] = MyParameters->MODEM_DATA_RATE_1;
    abApi_Write[6] = MyParameters->MODEM_DATA_RATE_0;
    abApi_Write[7] = MyParameters->MODEM_TX_NCO_MODE_3;
    abApi_Write[8] = MyParameters->MODEM_TX_NCO_MODE_2;
    abApi_Write[9] = MyParameters->MODEM_TX_NCO_MODE_1;
    abApi_Write[10] = MyParameters->MODEM_TX_NCO_MODE_0;
    abApi_Write[11] = MyParameters->MODEM_FREQ_DEV_2;
    abApi_Write[12] = MyParameters->MODEM_FREQ_DEV_1;
    abApi_Write[13] = MyParameters->MODEM_FREQ_DEV_0;
    SendCommand(abApi_Write, 14);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_MODEM_TX_RAMP_DELAY;
    abApi_Write[4] = MyParameters->MODEM_TX_RAMP_DELAY;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PA_TC;
    abApi_Write[4] = MyParameters->PA_TC;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 3;
    abApi_Write[3] = PROP_MODEM_MOD_TYPE;
    abApi_Write[4] = MyParameters->MODEM_MOD_TYPE;
    abApi_Write[5] = MyParameters->MODEM_MAP_CONTROL;
    abApi_Write[6] = MyParameters->MODEM_DSM_CONTROL;
    SendCommand(abApi_Write, 7);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_MODEM_CLKGEN_BAND;
    abApi_Write[4] = MyParameters->MODEM_CLKGEN_BAND;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNTH_GROUP;
    abApi_Write[2] = 7;
    abApi_Write[3] = PROP_SYNTH_PFDCP_CPFF;
    abApi_Write[4] = MyParameters->SYNTH_PFDCP_CPFF;
    abApi_Write[5] = MyParameters->SYNTH_PFDCP_CPINT;
    abApi_Write[6] = MyParameters->SYNTH_VCO_KV;
    abApi_Write[7] = MyParameters->SYNTH_LPFILT3;
    abApi_Write[8] = MyParameters->SYNTH_LPFILT2;
    abApi_Write[9] = MyParameters->SYNTH_LPFILT1;
    abApi_Write[10] = MyParameters->SYNTH_LPFILT0;
    SendCommand(abApi_Write, 11);
    WaitForCTS();    
    abApi_Write[0] = CMD_SET_PROPERTY;        
    abApi_Write[1] = PROP_FREQ_CONTROL_GROUP;      
    abApi_Write[2] = 8;               
    abApi_Write[3] = PROP_FREQ_CONTROL_INTE;     
    abApi_Write[4] = MyParameters->FREQ_CONTROL_INTE;              
    abApi_Write[5] = MyParameters->FREQ_CONTROL_FRAC_2;              
    abApi_Write[6] = MyParameters->FREQ_CONTROL_FRAC_1;              
    abApi_Write[7] = MyParameters->FREQ_CONTROL_FRAC_0;              
    abApi_Write[8] = MyParameters->FREQ_CONTROL_CHANNEL_STEP_SIZE_1;              
    abApi_Write[9] = MyParameters->FREQ_CONTROL_CHANNEL_STEP_SIZE_0;              
    abApi_Write[10] = MyParameters->FREQ_CONTROL_W_SIZE;              
    abApi_Write[11] = MyParameters->FREQ_CONTROL_VCOCNT_RX_ADJ;
    SendCommand(abApi_Write, 12);        
    WaitForCTS(); 
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 7;
    abApi_Write[3] = PROP_MODEM_MDM_CTRL;
    abApi_Write[4] = MyParameters->MODEM_MDM_CTRL;
    abApi_Write[5] = MyParameters->MODEM_IF_CONTROL;
    abApi_Write[6] = MyParameters->MODEM_IF_FREQ_2;
    abApi_Write[7] = MyParameters->MODEM_IF_FREQ_1;
    abApi_Write[8] = MyParameters->MODEM_IF_FREQ_0;
    abApi_Write[9] = MyParameters->MODEM_DECIMATION_CFG1;
    abApi_Write[10] = MyParameters->MODEM_DECIMATION_CFG0;
    SendCommand(abApi_Write, 11);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 10;
    abApi_Write[3] = PROP_MODEM_BCR_OSR_1;
    abApi_Write[4] = MyParameters->MODEM_BCR_OSR_1;
    abApi_Write[5] = MyParameters->MODEM_BCR_OSR_0;
    abApi_Write[6] = MyParameters->MODEM_BCR_NCO_OFFSET_2;
    abApi_Write[7] = MyParameters->MODEM_BCR_NCO_OFFSET_1;
    abApi_Write[8] = MyParameters->MODEM_BCR_NCO_OFFSET_0;
    abApi_Write[9] = MyParameters->MODEM_BCR_GAIN_1;
    abApi_Write[10] = MyParameters->MODEM_BCR_GAIN_0;
    abApi_Write[11] = MyParameters->MODEM_BCR_GEAR;
    abApi_Write[12] = MyParameters->MODEM_BCR_MISC1;
    abApi_Write[13] = MyParameters->MODEM_BCR_MISC0;
    SendCommand(abApi_Write, 14);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 8;
    abApi_Write[3] = PROP_MODEM_AFC_GEAR;
    abApi_Write[4] = MyParameters->MODEM_AFC_GEAR;
    abApi_Write[5] = MyParameters->MODEM_AFC_WAIT;
    abApi_Write[6] = MyParameters->MODEM_AFC_GAIN_1;
    abApi_Write[7] = MyParameters->MODEM_AFC_GAIN_0;
    abApi_Write[8] = MyParameters->MODEM_AFC_LIMITER_1;
    abApi_Write[9] = MyParameters->MODEM_AFC_LIMITER_0;
    abApi_Write[10] = MyParameters->MODEM_AFC_MISC;
    abApi_Write[11] = MyParameters->MODEM_AFC_ZIFOFF;
    SendCommand(abApi_Write, 12);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_MODEM_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_MODEM_RSSI_CONTROL;
    abApi_Write[4] = MyParameters->MODEM_RSSI_CONTROL;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PA_MODE;
    abApi_Write[4] = MyParameters->PA_MODE;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PA_PWR_LVL;
    abApi_Write[4] = MyParameters->PA_PWR_LVL;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PA_BIAS_CLKDUTY;
    abApi_Write[4] = MyParameters->PA_BIAS_CLKDUTY; 
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;        
    abApi_Write[1] = PROP_GLOBAL_GROUP;      
    abApi_Write[2] = 1;               
    abApi_Write[3] = PROP_GLOBAL_XO_TUNE;     
    abApi_Write[4] = MyParameters->GLOBAL_XO_TUNE;              
    SendCommand(abApi_Write, 5);        
    WaitForCTS(); 
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_GLOBAL_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_GLOBAL_CLK_CFG;
    abApi_Write[4] = MyParameters->GLOBAL_CLK_CFG;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_GLOBAL_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_GLOBAL_CONFIG;
    abApi_Write[4] = MyParameters->GLOBAL_CONFIG;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_TX_LENGTH;
    abApi_Write[4] = MyParameters->PREAMBLE_TX_LENGTH;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_CONFIG_STD_1;
    abApi_Write[4] = MyParameters->PREAMBLE_CONFIG_STD_1;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_CONFIG_NSTD;
    abApi_Write[4] = MyParameters->PREAMBLE_CONFIG_NSTD;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_CONFIG_STD_2;
    abApi_Write[4] = MyParameters->PREAMBLE_CONFIG_STD_2;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_CONFIG;
    abApi_Write[4] = MyParameters->PREAMBLE_CONFIG;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_PATTERN_31_24;
    abApi_Write[4] = MyParameters->PREAMBLE_PATTERN_31_24;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_PATTERN_23_16;
    abApi_Write[4] = MyParameters->PREAMBLE_PATTERN_23_16;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_PATTERN_15_8;
    abApi_Write[4] = MyParameters->PREAMBLE_PATTERN_15_8;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PREAMBLE_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PREAMBLE_PATTERN_7_0;
    abApi_Write[4] = MyParameters->PREAMBLE_PATTERN_7_0;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNC_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_SYNC_CONFIG;
    abApi_Write[4] = MyParameters->SYNC_CONFIG;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNC_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_SYNC_BITS_31_24;
    abApi_Write[4] = MyParameters->SYNC_BITS_31_24;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNC_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_SYNC_BITS_23_16;
    abApi_Write[4] = MyParameters->SYNC_BITS_23_16;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNC_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_SYNC_BITS_15_8;
    abApi_Write[4] = MyParameters->SYNC_BITS_15_8;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_SYNC_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_SYNC_BITS_7_0;
    abApi_Write[4] = MyParameters->SYNC_BITS_7_0;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PKT_CRC_CONFIG;
    abApi_Write[4] = MyParameters->PKT_CRC_CONFIG;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PKT_CONFIG1;
    abApi_Write[4] = MyParameters->PKT_CONFIG1;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PKT_LEN;
    abApi_Write[4] = MyParameters->PKT_LEN;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PKT_LEN_FIELD_SOURCE;
    abApi_Write[4] = MyParameters->PKT_LEN_FIELD_SOURCE;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PKT_LEN_ADJUST;
    abApi_Write[4] = MyParameters->PKT_LEN_ADJUST;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_FIELD_1_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_FIELD_1_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_FIELD_1_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_FIELD_1_CONFIG;
    abApi_Write[7] = MyParameters->PKT_FIELD_1_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_FIELD_2_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_FIELD_2_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_FIELD_2_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_FIELD_2_CONFIG;
    abApi_Write[7] = MyParameters->PKT_FIELD_2_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_FIELD_3_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_FIELD_3_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_FIELD_3_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_FIELD_3_CONFIG;
    abApi_Write[7] = MyParameters->PKT_FIELD_3_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_FIELD_4_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_FIELD_4_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_FIELD_4_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_FIELD_4_CONFIG;
    abApi_Write[7] = MyParameters->PKT_FIELD_4_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_FIELD_5_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_FIELD_5_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_FIELD_5_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_FIELD_5_CONFIG;
    abApi_Write[7] = MyParameters->PKT_FIELD_5_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_RX_FIELD_1_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_RX_FIELD_1_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_RX_FIELD_1_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_RX_FIELD_1_CONFIG;
    abApi_Write[7] = MyParameters->PKT_RX_FIELD_1_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_RX_FIELD_2_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_RX_FIELD_2_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_RX_FIELD_2_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_RX_FIELD_2_CONFIG;
    abApi_Write[7] = MyParameters->PKT_RX_FIELD_2_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_RX_FIELD_3_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_RX_FIELD_3_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_RX_FIELD_3_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_RX_FIELD_3_CONFIG;
    abApi_Write[7] = MyParameters->PKT_RX_FIELD_3_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_RX_FIELD_4_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_RX_FIELD_4_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_RX_FIELD_4_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_RX_FIELD_4_CONFIG;
    abApi_Write[7] = MyParameters->PKT_RX_FIELD_4_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PKT_GROUP;
    abApi_Write[2] = 4;
    abApi_Write[3] = PROP_PKT_RX_FIELD_5_LENGTH_12_8;
    abApi_Write[4] = MyParameters->PKT_RX_FIELD_5_LENGTH_12_8;
    abApi_Write[5] = MyParameters->PKT_RX_FIELD_5_LENGTH_7_0;
    abApi_Write[6] = MyParameters->PKT_RX_FIELD_5_CONFIG;
    abApi_Write[7] = MyParameters->PKT_RX_FIELD_5_CRC_CONFIG;
    SendCommand(abApi_Write, 8);
    WaitForCTS();
    
    // SYSTEM parameters:
    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_INT_CTL_GROUP;
    abApi_Write[2] = 4;              
    abApi_Write[3] = PROP_INT_CTL_ENABLE;  
    abApi_Write[4] = 0x01;              // INT_CTL: PH enabled
    abApi_Write[5] = 0x18;              // INT_CTL_PH: RX+CRCERR enabled
    abApi_Write[6] = 0x00;              // INT_CTL_MODEM: -
    abApi_Write[7] = 0x00;              // INT_CTL_CHIP_EN: -
    SendCommand(abApi_Write, 8);      
    WaitForCTS();
    abApi_Write[0] = CMD_SET_PROPERTY;       
    abApi_Write[1] = PROP_FRR_CTL_GROUP;    
    abApi_Write[2] = 4;               
    abApi_Write[3] = PROP_FRR_CTL_A_MODE;    
    abApi_Write[4] = 0x03; //ph status             
    abApi_Write[5] = 0x06; //modem int pending             
    abApi_Write[6] = 0x0A; //rssi             
    abApi_Write[7] = 0x09; //curr state           
    SendCommand(abApi_Write, 8);       
    WaitForCTS();    

    StartRx();
    
    return 0;
}

//********************************
static void Reset(void)
{
    GPIO->P[1].DOUTSET = (1<<11);
    OS_WAIT(10);
    GPIO->P[1].DOUTCLR = (1<<11);
    OS_WAIT(50);
}

//***************************************
static uint8_t InitRF(void)
{
    if(SendPowerOnCmd() == 0) //is si4461 mounted?
    {
        return 0;
    }
    ClearPendingTxInt();
    ClearPendingRxInt();
    ClearFifo();
    ConfigGPIO();
    
    abApi_Write[0] = CMD_PART_INFO;
    SendCommand(abApi_Write, 1);
    GetResponse(resp, 8);

    SendSetup();
    return 1;
}

//**********************************************
static uint8_t GetFastResponseRegister(uint8_t reg)
{
    uint8_t res;
    
    setCSSPI2(0);
    sendCharSPI2(reg);
    res = receiveCharSPI2();
    setCSSPI2(1);
    return res;
}

//**********************************************
static uint8_t ReadRxDataBuffer(uint8_t* Data, uint8_t Len)
{
    setCSSPI2(0);
    sendCharSPI2(CMD_RX_FIFO_READ);
    receiveSPI2(Data, Len);
    setCSSPI2(1);
    return 0;
}

//******************************
void calibrateSI4461(void)
{
    //chip will autocal on evry transition from STATE_READY to STATE_RX/TX
}

//*************************************
uint8_t getRxStatusSI4461(uint8_t* status)
{
    *status = 0; //not supported (or implemented later!)
    return 1;
}

//****************************
void powerdownSI4461(void)
{
  if (init==FALSE) return; 
  disableGPIOIRQ(PortUndef, 15);
  power=FALSE;
  abApi_Write[0] = CMD_CHANGE_STATE;
  abApi_Write[1] = STATE_SLEEP;
  SendCommand(abApi_Write, 2);
}

//**************************
void powerupSI4461(void)
{
    if (init==FALSE) return; 
    power=TRUE;
    abApi_Write[0] = CMD_CHANGE_STATE;
    abApi_Write[1] = STATE_RX;
    SendCommand(abApi_Write, 2);
    WaitForCTS(); 
    enableGPIOIRQ(PortUndef, 15);
} 

//********************************
uint8_t getRssiSI4461(int16_t* rssi)
{
    *rssi = GetFastResponseRegister(CMD_FAST_RESPONSE_REG_C)/2 - GetProperty(PROP_MODEM_GROUP, PROP_MODEM_RSSI_COMP) - 70;
    //*rssi = (GetFastResponseRegister(CMD_FAST_RESPONSE_REG_C) / 2) + -130;
    return 0;
}

//*******************************************************
uint8_t sendPacketSI4461(uint8_t* Payload, uint8_t Len)
{
    uint8_t stat;
    int i;
    int retry;
    if (init==FALSE) return 1; 
    
    ClearPendingTxInt();

    abApi_Write[0] = CMD_CHANGE_STATE;
    abApi_Write[1] = STATE_READY;
    SendCommand(abApi_Write, 2);
    WaitForCTS();

    abApi_Write[0] = CMD_TX_FIFO_WRITE; 
    memcpy(&abApi_Write[1], Payload, Len); 
    SendCommand(abApi_Write, Len+1); 
    WaitForCTS();

    // Set TX packet length
    abApi_Write[0] = CMD_SET_PROPERTY;        // Use property command
    abApi_Write[1] = PROP_PKT_GROUP;        // Select property group
    abApi_Write[2] = 1;               // Number of properties to be written
    abApi_Write[3] = PROP_PKT_FIELD_1_LENGTH_7_0; // Specify first property
    abApi_Write[4] = Len;           // Field length (variable packet length info)
    SendCommand(abApi_Write, 5);        // Send command to the radio IC
    WaitForCTS();

    StartTx();

    stat = GetFastResponseRegister(CMD_FAST_RESPONSE_REG_A);
    retry = 10;
    while(!(stat & 0x20) && retry)
    {
        stat = GetFastResponseRegister(CMD_FAST_RESPONSE_REG_A); //get ph_status and wait until packet is sent
        if(!(stat&0x20))
        {
          for(i = 50000; i > 0; i--) ; // ~5ms
          retry--;
        }
    }
    
    if(retry <= 0)
    {
        return 1;
    }

    RFStatus.TotTx++;
    return 0; //ok
}

//***********************************************************************************
uint8_t receivePacketSI4461(uint8_t* buf, uint8_t* len, uint8_t* lqi, int16_t* rssi)
{
    U8 packetlen;
    U8 fifolen;
    
    abApi_Write[0] = CMD_FIFO_INFO;
    abApi_Write[1] = 0;
    SendCommand(abApi_Write, 2); 
    GetResponse(resp, 2);
    fifolen = resp[0];
    if(fifolen == 0)
    {
        return 1;
    }
    else 
    {
        ReadRxDataBuffer(&packetlen, 1);
        if(packetlen == 0) //no data
        {
            ClearFifo();
            return 2;
        }
        if(packetlen > 64) //not legal
        {
            ClearFifo();
            return 3;
        }
        if(packetlen > fifolen) //mismatch
        {
            ClearFifo();
            return 4;
        }
        ReadRxDataBuffer(buf, packetlen);
        *len = packetlen;
        *lqi = 0; //todo
        getRssiSI4461(rssi);
        return 0; //ok
    }
}

//**************************
void si4461_isr(void)
{
  isr_evt_set(EVT_SI4461_INT, mTID);
  //setWakeupCauseRTC(WAKEUP_RF);
}


//****************************************
void setCallbackSI4461(si4461_callback_t cb)
{
    MyCb = cb;
}

//****************************
__task void thSI4461(void) 
{
    U16 evt;
    U8 buf[100];
    U8 len;
    U8 lqi;
    int16_t rssi;
    U8 stat;
  
    //setNameRTXMON(__FUNCTION__);
    mTID = os_tsk_self(); 

    RFStatus.TotRx = 0;
    RFStatus.TotTx = 0;
    RFStatus.TotCrcErr = 0;
    
    initGPIOInt();
    registerHandlerGPIOIRQ(PortA, 15, FALSE, TRUE, (tdefCbGPIOIRQ)si4461_isr);
    enableGPIOIRQ(PortUndef, 15);
    ClearPendingRxInt();

    StartRx();
  
    while(1) 
    {
        while (!power) OS_WAIT(100);
    
        if (os_evt_wait_or(0xFFFF, 100) == OS_R_EVT) 
        {
            evt = os_evt_get();
            if(evt & EVT_SI4461_INT) 
            {
                buf[0] = CMD_GET_PH_STATUS;
                buf[1] = 0x00;        
                buf[2] = 0x00;        
                buf[3] = 0x00;        
                SendCommand(buf, 2/*4*/);  
                if(GetResponse(buf, 2 /*8*/)) 
                {
                    stat = buf[1];
                    //stat = buf[3];
                    // Check CRC error
                    if(stat & 0x08) 
                    {
                        RFStatus.TotCrcErr++;
                        ClearFifo();
                    }                
                    // Check if packet received
                    if(stat & 0x10) 
                    {
                        if(MyCb != NULL) 
                        {
                            while(receivePacketSI4461(buf, &len, &lqi, &rssi) == 0)
                            {
                                MyCb(buf, len, lqi, rssi);
                                RFStatus.TotRx++;
                            }
                        }
                    }
                }
             //   ClearPendingRxInt();
                StartRx();
            }
        }
        else
        {
           if (power) {
             StartRx();
             ClearPendingRxInt();
           }
        }
    }
}

//*********************************
int setTransmitPowerSI4461(int dbm)
{
    int sel;
    U8 regval;
    
    sel = dbm;
    
    if(sel < -25) 
    {
        sel = -30;
        regval = 2;
    }
    else if(sel < -17) 
    {
        sel = -20;
        regval = 5;
    }
    else if(sel < -12) 
    {
        sel = -15;
        regval = 7;
    }
    else if(sel < -8) 
    {
        sel = -10;
        regval = 9;
    }
    else if(sel < -3) 
    {
        sel = -6;
        regval = 13;
    }
    else if(sel < 3) 
    {
        sel = 0;
        regval = 24;
    }
    else if(sel < 6) 
    {
        sel = 5;
        regval = 40;
    }
    else if(sel < 9) 
    {
        sel = 7;
        regval = 47;
    }
    else if(sel < 11) 
    {
        sel = 10;
        regval = 70;
    }
    else 
    {
        sel = 15;
        regval = 127;
    }

    abApi_Write[0] = CMD_SET_PROPERTY;
    abApi_Write[1] = PROP_PA_GROUP;
    abApi_Write[2] = 1;
    abApi_Write[3] = PROP_PA_PWR_LVL;
    abApi_Write[4] = regval;
    SendCommand(abApi_Write, 5);
    WaitForCTS();
    
    return sel;
}

//***************************************************
uint8_t initSI4461(void* parameters)
{
    MyParameters = parameters;

    if(init) //is task already running?
    {
        Reset();    
        InitRF();
    }
    else
    {
        CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_GPIO);
        initSPI2();

        //RF INT on PA15
        GPIO->P[0].DOUTSET = (1 << 15);
        GPIO->P[0].MODEH = (GPIO->P[0].MODEH & ~_GPIO_P_MODEH_MODE15_MASK) | GPIO_P_MODEH_MODE15_INPUTPULLFILTER;

        Reset();    
        if(InitRF() == 0)
        {
            return 0;
        }
        power = TRUE;
        START_THREAD_USR(thSI4461, 2, (void*)thSI4461Stk, sizeof(thSI4461Stk));
        init = TRUE;
    }
    return 1;
}

         
//***************************************************
void setupSI4461(void)
{
    // RF_SDN, Pin PB11 is configured to Push-pull 
    // set shutdown
    GPIO->P[1].DOUTSET = (1 << 11);
    GPIO->P[1].MODEH = (GPIO->P[1].MODEH & ~_GPIO_P_MODEH_MODE11_MASK) | GPIO_P_MODEH_MODE11_PUSHPULL;

}

           


