//=============================================================================
// ADC.C                                                           20130107 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#include "efm32.h"
#include "System.h"
#include "ADC.h"



/*
// *****************************************
void analog_getCPUTemperature(int* Temp)
{
    uint32_t sample;
    ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
    ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

    CMU_ClockEnable(cmuClock_ADC0, true);
    ADC_Reset(ADC0);
    
    init.timebase = ADC_TimebaseCalc(0);
    init.prescale = ADC_PrescaleCalc(7000000, 0);

    ADC_Init(ADC0, &init);

    singleInit.reference  = (VREF == 2500.0 ? adcRef2V5 : adcRef1V25);
    singleInit.input      = adcSingleInpTemp;
    singleInit.resolution = adcRes12Bit;

    singleInit.acqTime = adcAcqTime32;

    ADC_InitSingle(ADC0, &singleInit);

    ADC_Start(ADC0, adcStartSingle);
    while (ADC0->STATUS & ADC_STATUS_SINGLEACT) ;
    sample = ADC_DataSingleGet(ADC0);
    *Temp = (sample * VREF) / (4096 * 1.92); //1.92 is the TGRAD_ADCTH for the device(see datasheet)
    
    ADC_Reset(ADC0);
    CMU_ClockEnable(cmuClock_ADC0, false);
}
*/


//-----------------------------------------------------------------------------
// return raw value of selected AD channel 
//-----------------------------------------------------------------------------
int readADC(tdefenumADC ch) {
  int rc;
  unsigned int cal;
  powerADC(TRUE);

  // If we are reading internal temperature, use 1.25V ref

  if (ch==_ADC_SINGLECTRL_INPUTSEL_TEMP || ch==_ADC_SINGLECTRL_INPUTSEL_VDDDIV3) {
    cal  = ADC0->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK | _ADC_CAL_SINGLEGAIN_MASK);
    cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_GAIN_MASK) >>
            _DEVINFO_ADC0CAL0_1V25_GAIN_SHIFT) << _ADC_CAL_SINGLEGAIN_SHIFT;
    cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_OFFSET_MASK) >>
            _DEVINFO_ADC0CAL0_1V25_OFFSET_SHIFT) << _ADC_CAL_SINGLEOFFSET_SHIFT;
    ADC0->CAL = cal;
  } else {
    // Load calibration factor for 2,5 V reference
    cal  = ADC0->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK | _ADC_CAL_SINGLEGAIN_MASK);
    cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_2V5_GAIN_MASK) >>
            _DEVINFO_ADC0CAL0_2V5_GAIN_SHIFT) << _ADC_CAL_SINGLEGAIN_SHIFT;
    cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_2V5_OFFSET_MASK) >>
            _DEVINFO_ADC0CAL0_2V5_OFFSET_SHIFT) << _ADC_CAL_SINGLEOFFSET_SHIFT;
    ADC0->CAL = cal;
  }
  
  // Make sure conversion is not in progress
  ADC0->CMD = ADC_CMD_SINGLESTOP;
  
  ADC0->CTRL = (ADC_CTRL_OVSRSEL_X4096) | // X4096 samples for each conversion result
               (31<<16)   | // Timebase = 31 => 31+1 HFCLOCK cycles for warmup sequence
               (0x07<<8)  | // Set PRESC to 7+1 HFPERCLK cycles (divide HFPERCLOCK with 8). Resulting clock must be below 13 MHz)
               (ADC_CTRL_LPFMODE_RCFILT)   |  // On chip RC filter selected   
               (ADC_CTRL_WARMUPMODE_NORMAL);
  
  // If we are reading internal temperature or VDD/3, use 1.25V ref
  if (ch==_ADC_SINGLECTRL_INPUTSEL_TEMP || ch==_ADC_SINGLECTRL_INPUTSEL_VDDDIV3) 
    ADC0->SINGLECTRL = (ADC_SINGLECTRL_AT_256CYCLES) |
                       (ADC_SINGLECTRL_REF_1V25) |      // Use 1.25> reference
                       (ch<<_ADC_SINGLECTRL_INPUTSEL_SHIFT);  // Select temp as input
  else 
    ADC0->SINGLECTRL =  (ADC_SINGLECTRL_AT_256CYCLES) |
                        (ADC_SINGLECTRL_REF_2V5) |             // Use 2.5V reference
                        (ch<<_ADC_SINGLECTRL_INPUTSEL_SHIFT);  // Select "ch" as input (0 is ADC0.0, 1 is ADC0.1 etc)

  ADC0->CMD = ADC_CMD_SINGLESTART; // Single conversion start
  while ((ADC0->STATUS & ADC_STATUS_SINGLEDV)==0); // Wait for single sample data valid
  rc=ADC0->SINGLEDATA;
  powerADC(FALSE);
  return rc;
}


//-----------------------------------------------------------------------------
// powerADC()
//   Controls power to AD 0 converter
//-----------------------------------------------------------------------------
void powerADC(unsigned char power) {
  if (power) {
    CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_ADC0);
  } else {
    CMU->HFPERCLKEN0 &= ~(CMU_HFPERCLKEN0_ADC0);
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void initADC(void) {

  
}
