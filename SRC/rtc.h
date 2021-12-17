#ifndef _RTC_
#define _RTC_

//-----------------------------------------------------------------------------
// Structure for all information from RTC
//-----------------------------------------------------------------------------
typedef struct {
  signed char  Sec;   // Second value - [0,59]
  signed char  Min;   // Minute value - [0,59]
  signed char  Hour;  // Hour value - [0,23]
  signed char  Date;  // Day of the month value - [1,31]
  signed char  Month; // Month value - [1,12]
  signed short Year;  // Year value - [0,4095]
  signed char  Wday;  // Day of week value - [0,6]
  signed short  Yday;  // Day of year value - [1,365]
} tdefRTCTime;

extern int RTCTick;

//-----------------------------------------------------------------------------
// setTimeRTC()
//   Sets the current time
//-----------------------------------------------------------------------------
void setTimeRTC(tdefRTCTime Time);

//-----------------------------------------------------------------------------
// getTimeRTC()
//   Returns the current time
//-----------------------------------------------------------------------------
tdefRTCTime getTimeRTC(void);

//-----------------------------------------------------------------------------
// setAlarmTimeRTC()
//   Sets the alarm time
// Input:
//   Alarm: structure with time/date to set the RTC alarm with
//   DoReset: If 1, ccpu will be reset after alarm timeout
//-----------------------------------------------------------------------------
void RTCSetAlarm(tdefRTCTime Alarm, unsigned char DoReset);

//----------------------------------------------------------------------------
// Callback function for RTC second interrupts
//----------------------------------------------------------------------------
typedef void(*tdefRTCCBFunc)(void* arg);

//-----------------------------------------------------------------------------
// registerRTC()
//   Register a callback function for the RTC seconds interrupt.
// WARNING: When the callback function is called, it executes in interrupt scope
// so keep the functions VERY small and lightweight !!
// Input:
//   cb: Function to call
//   arg: general argument that will be passed to the cb when called
// Returns 0 if ok, 1 if no room for more cb functions in table
//-----------------------------------------------------------------------------
int registerRTC(tdefRTCCBFunc cb, void *arg);

//-----------------------------------------------------------------------------
// initRTC()
//   Initialize RTC
//-----------------------------------------------------------------------------
void initRTC(void);



#endif

