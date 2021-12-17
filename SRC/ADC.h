//=============================================================================
// ADC.H                                                           20060414 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#ifndef _ADC_
#define _ADC_


typedef enum {      // Portpin:    LGC1:          LGC2:                     Blackbird:                  Dragonfly:
  ADC_AD0   = 0x00, //(P0.23)      GSM Audio      Analog input 2            Analog input 2              
	ADC_AD1   = 0x01, //(P0.24)      CHG_A_VBUS     GSM audio                 GSM audio
	ADC_AD2   = 0x02, //(P0.25)      CHG_A_TEMP     Charger temperature       Charger temperature
	ADC_AD3   = 0x03, //(P0.26)      Not used       Analog input 1 (ignition) Analog input 1 (ignition)
  ADC_AD4   = 0x04, //(P1.30)      Not used       Not used                  Not used                    PSU_V_CAP
  ADC_AD5   = 0x05, //(P1.31)      Not used       Input voltage(12V)        Input voltage(12V)
  ADC_AD6   = 0x06, //(P0.12)      CHG_A_VBAT     CHG Vbat                  CHG Vbat
	ADC_AD7   = 0x07, //(P0.13)      CHG_A_ISET     CHG_A_ISET                CHG_A_ISET
  ADC_AD8   = 0x08, //                                                                                  Internal temperature
  ADC_AD9   = 0x09  //                                                                                  Internal Vdd/3
} tdefenumADC;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int readADC(tdefenumADC ch);

//-----------------------------------------------------------------------------
// powerADC()
//   Controls power to AD 0 converter
//-----------------------------------------------------------------------------
void powerADC(unsigned char power);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void initADC(void);

#endif
