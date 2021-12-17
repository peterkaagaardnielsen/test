//=============================================================================
// Clock.H                                                         20061026 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#ifndef _CLOCK_
#define _CLOCK_


//-----------------------------------------------------------------------------
// timeToSecondsClock()
//   Calculate the number of seconds since 1/1 1980
//-----------------------------------------------------------------------------
unsigned int timeToSecondsClock(unsigned short Year, unsigned char Month, unsigned char Date, unsigned char Hour, unsigned char Min, unsigned char Sec);

//-----------------------------------------------------------------------------
// secondsToTimeClock()
//   Set the currnet time according to number of seconds since 1/1 1980
//-----------------------------------------------------------------------------
unsigned int secondsToTimeClock(unsigned int linsec, unsigned short *Year, unsigned char *Month, unsigned char *Date, unsigned char *Hour, unsigned char *Min, unsigned char *Sec);

//-----------------------------------------------------------------------------
// secondsToTimeClock()
//   Set the current time according to number of seconds since 1/1 1980
//-----------------------------------------------------------------------------
int dayOfWeekClock(int year, int month, int day);

//-----------------------------------------------------------------------------
// dayOfYearClock()
//   Return day of year
//-----------------------------------------------------------------------------
int dayOfYearClock(int year, int month, int day);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void initClock(void);

#endif
