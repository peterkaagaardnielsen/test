#include "efm32.h"
#include "System.h"
#include "RTC.h"
#include "WDT.h"
#include "clock.h"
#include "debug.h"


#define CurrentTimeInt BURTC->RET[127].REG

/* Declare variables for time keeping */

/* Clock defines */
#define LFXO_FREQUENCY 32768
#define BURTC_PRESCALING 128
#define UPDATE_INTERVAL 1
#define COUNTS_PER_SEC (LFXO_FREQUENCY/BURTC_PRESCALING)
#define COUNTS_BETWEEN_UPDATE (UPDATE_INTERVAL*COUNTS_PER_SEC)




int RTCTick=0;


//******************************
void BURTC_IRQHandler(void) {
  int i;
  
  if ( BURTC->IF & BURTC_IF_COMP0 ) {
    BURTC->COMP0 += COUNTS_BETWEEN_UPDATE;
    // Next compare value
    RTCTick ++;
    CurrentTimeInt ++;
    BURTC->IFC = BURTC_IF_COMP0;
  }
} 

//-----------------------------------------------------------------------------
// setTimeRTC()
//   Sets the current time
// Input:
//   Time: structure with time/date to set the RTC with
//-----------------------------------------------------------------------------
void setTimeRTC(tdefRTCTime Time) 
{
    CurrentTimeInt = timeToSecondsClock(Time.Year, Time.Month, Time.Date, Time.Hour, Time.Min, Time.Sec);
}


//-----------------------------------------------------------------------------
// getTimeRTC()
//   Returns the current time
//-----------------------------------------------------------------------------
tdefRTCTime getTimeRTC(void) {
  tdefRTCTime DT; 
  secondsToTimeClock(CurrentTimeInt, &DT.Year, &DT.Month, &DT.Date, &DT.Hour, &DT.Min, &DT.Sec);
  // Set the day of week and day of year (using calculated values)
  DT.Wday=dayOfWeekClock(DT.Year, DT.Month, DT.Date);
  DT.Yday=dayOfYearClock(DT.Year, DT.Month, DT.Date);
  return DT;    
}

//-----------------------------------------------------------------------------
// initRTC()
//   Initialize RTC
//-----------------------------------------------------------------------------
void initRTC(void) 
{
    tdefRTCTime Time;
    int i;

    
    CMU->OSCENCMD |= CMU_OSCENCMD_LFXOEN;
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
    
    RMU->CTRL &= ~RMU_CTRL_BURSTEN;

    
    i=BURTC->CNT;
   
 //   messageDebug(DBG_INFO, __MODULE__, __LINE__, "currentTimeInt=%i, cnt=%i, comp0=%i", CurrentTimeInt, BURTC->CNT, BURTC->COMP0);

    BURTC->CTRL = BURTC_CTRL_RSTEN;
    if (i>COUNTS_BETWEEN_UPDATE)
      BURTC->COMP0=COUNTS_BETWEEN_UPDATE;
    else  
      BURTC->COMP0=COUNTS_BETWEEN_UPDATE-i;

    BURTC->CTRL |= (/*BURTC_CTRL_COMP0TOP |*/ BURTC_CTRL_PRESC_DIV128 | BURTC_CTRL_CLKSEL_LFXO | BURTC_CTRL_MODE_EM2EN);
    while (RTC->SYNCBUSY & RTC_SYNCBUSY_CTRL);
    BURTC->IEN = (BURTC_IEN_COMP0);
    BURTC->CTRL &= ~BURTC_CTRL_RSTEN;


    NVIC_EnableIRQ(BURTC_IRQn);

    Time = getTimeRTC();
    // Sanity check of the system time..
    if ((Time.Year<2013) || (Time.Year>2050)) 
    {
        Time.Year = 2013;
        Time.Month = 1;
        Time.Date = 1;
        Time.Hour = 0;
        Time.Min = 0;
        Time.Sec = 0;
        Time.Wday = 1;
        Time.Yday = 1;
        setTimeRTC(Time);
    }
}


