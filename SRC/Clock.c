//=============================================================================
// Clock.C                                                         20061026 CHG
//
//=============================================================================
// 
//-----------------------------------------------------------------------------
#include "clock.h"

typedef  signed     char    Z_char;
typedef  signed     char    Z_byte;
typedef  signed     short   Z_short;
typedef  signed     short   Z_shortword;
typedef  signed     int     Z_int;
typedef  signed     int     Z_word;
typedef  signed     long    Z_long;
typedef  signed     long    Z_longword;
typedef int boolean;
#define and         &&      /* logical (boolean) operators: lower case */
#define or          ||
#define not         !
#define true 1
#define false 0

static const Z_int DateCalc_Days_in_Year_[2][14] =
{
    { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

static const Z_int DateCalc_Days_in_Month_[2][13] =
{
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static void DateCalc_Normalize_Time(Z_long *Dd, Z_long *Dh, Z_long *Dm, Z_long *Ds)
{
    Z_long quot;

    quot = (Z_long) (*Ds / 60L);
    *Ds -= quot * 60L;
    *Dm += quot;
    quot = (Z_long) (*Dm / 60L);
    *Dm -= quot * 60L;
    *Dh += quot;
    quot = (Z_long) (*Dh / 24L);
    *Dh -= quot * 24L;
    *Dd += quot;
}

static void DateCalc_Normalize_Ranges(Z_long *Dd, Z_long *Dh, Z_long *Dm, Z_long *Ds)
{
    Z_long quot;

    /* Prevent overflow errors on systems */
    /* with short "long"s (e.g. 32 bits): */

    quot = (Z_long) (*Dh / 24L);
    *Dh -= quot * 24L;
    *Dd += quot;
    quot = (Z_long) (*Dm / 60L);
    *Dm -= quot * 60L;
    *Dh += quot;
    DateCalc_Normalize_Time(Dd,Dh,Dm,Ds);
}


static void DateCalc_Normalize_Signs(Z_long *Dd, Z_long *Dh, Z_long *Dm, Z_long *Ds)
{
    Z_long quot;

    quot = (Z_long) (*Ds / 86400L);
    *Ds -= quot * 86400L;
    *Dd += quot;
    if (*Dd != 0L)
    {
        if (*Dd > 0L)
        {
            if (*Ds < 0L)
            {
                *Ds += 86400L;
                (*Dd)--;
            }
        }
        else
        {
            if (*Ds > 0L)
            {
                *Ds -= 86400L;
                (*Dd)++;
            }
        }
    }
    *Dh = 0L;
    *Dm = 0L;
    if (*Ds != 0L) DateCalc_Normalize_Time(Dd,Dh,Dm,Ds);
}


static Z_long DateCalc_Year_to_Days(Z_int year)
{
    Z_long days;

    days = year * 365L;
    days += year >>= 2;
    days -= year /= 25;
    days += year >>  2;
    return(days);
}

static boolean DateCalc_leap_year(Z_int year)
{
    Z_int yy;

    return( ((year and 0x03) == 0) and
            ( (((yy = (Z_int) (year / 100)) * 100) != year) or
                ((yy and 0x03) == 0) ) );
}



static Z_long DateCalc_Date_to_Days(Z_int year, Z_int month, Z_int day)
{
    boolean leap;

    if ((year >= 1) and
        (month >= 1) and (month <= 12) and
        (day >= 1) and
        (day <= DateCalc_Days_in_Month_[leap=DateCalc_leap_year(year)][month]))
            return( DateCalc_Year_to_Days(--year) +
                    DateCalc_Days_in_Year_[leap][month] + day );
    return(0L);
}

static Z_int DateCalc_Day_of_Week(Z_int year, Z_int month, Z_int day)
{
    Z_long days;

    days = DateCalc_Date_to_Days(year,month,day);
    if (days > 0L)
    {
        days--;
        days %= 7L;
        days++;
    }
    return( (Z_int) days );
}

Z_int DateCalc_Day_of_Year(Z_int year, Z_int month, Z_int day)
{
    boolean leap;

    if ((year >= 1) and
        (month >= 1) and (month <= 12) and
        (day >= 1) and
        (day <= DateCalc_Days_in_Month_[leap=DateCalc_leap_year(year)][month]))
            return( DateCalc_Days_in_Year_[leap][month] + day );
    return(0);
}



static boolean DateCalc_check_date(Z_int year, Z_int month, Z_int day)
{
    if ((year >= 1) and
        (month >= 1) and (month <= 12) and
        (day >= 1) and
        (day <= DateCalc_Days_in_Month_[DateCalc_leap_year(year)][month]))
            return(true);
    return(false);
}

static boolean DateCalc_check_time(Z_int hour, Z_int min, Z_int sec)
{
    if ((hour >= 0) and (min >= 0) and (sec >= 0) and
        (hour < 24) and (min < 60) and (sec < 60))
            return(true);
    return(false);
}


//
//Returns number of seconds since 1980-1-1 00:00:00
//
static Z_long DateCalc_DT_to_linsec(Z_int year,Z_int month, Z_int day,   /* I */
                             Z_int hour,Z_int min, Z_int sec)     /* I */
{
   static Z_long Dd;
   static Z_long HH;
   static Z_long MM;
   static Z_long SS;
   static Z_long Dd_base=0;

   Dd= 0;
   if (!DateCalc_check_date(year,month,day) || !DateCalc_check_time(hour,min,sec)) {
      //bad date and/or time:
      return 0;
   }

   if (!Dd_base) {
      Dd_base= DateCalc_Date_to_Days(1980,1,1);
   }

   Dd =  DateCalc_Date_to_Days(year,month,day) - Dd_base;
   SS = ((((hour * 60L) + min) * 60L) + sec);
   DateCalc_Normalize_Signs(&Dd,&HH,&MM,&SS);
   return (Dd*24L*60L*60L)+(HH*60L*60L)+(MM*60L)+SS;
}



//
// Returns a specific date/time converted from a number of sec. since 1980-1-1 00:00:00
//
static boolean  DateCalc_linsec_to_DT(Z_int* year,Z_int* month, Z_int* day, 
                            Z_int* hour,Z_int* min, Z_int* sec,   /* O */
                            Z_long seconds)                       /* I */
{
   static Z_long Dd,Dh,Dm,Ds;
   static Z_long  days;
   static boolean leap;


   Dd=seconds/(24L*60L*60L);
   seconds-=Dd*(24L*60L*60L);
   Dh=seconds/(60L*60L);
   seconds-=Dh*(60L*60L);
   Dm=seconds/60L;
   seconds-=Dm*60L;
   Ds=seconds;

   *year=1980;
   *month=1;
   *day=1;
   *hour=0;
   *min=0;
   *sec=0;
   
   
   //The code below is actually:
   //DateCalc_add_delta_dhms(year,month,day,hour,min,sec,dd,hh,nn,ss);
   //
   //
   if (DateCalc_check_date(*year,*month,*day) and
       DateCalc_check_time(*hour,*min,*sec))
   {
       DateCalc_Normalize_Ranges(&Dd,&Dh,&Dm,&Ds);
       Ds += ((((*hour * 60L) + *min) * 60L) + *sec) +
              ((( Dh   * 60L) +  Dm)  * 60L);
       while (Ds < 0L)
       {
           Ds += 86400L;
           Dd--;
       }
       if (Ds > 0L)
       {
           Dh = 0L;
           Dm = 0L;
           DateCalc_Normalize_Time(&Dd,&Dh,&Dm,&Ds);
           *hour = (Z_int) Dh;
           *min  = (Z_int) Dm;
           *sec  = (Z_int) Ds;
       }
       else *hour = *min = *sec = 0;


       //The code below does the following:
       //return( DateCalc_add_delta_days(year,month,day,Dd) );
       //

       if (((days = DateCalc_Date_to_Days(*year,*month,*day)) > 0L) and
           ((days += Dd) > 0L))
       {
           *year = (Z_int) ( days / 365.2425 );
           *day  = (Z_int) ( days - DateCalc_Year_to_Days(*year) );
           if (*day < 1)
           {
               *day = (Z_int) ( days - DateCalc_Year_to_Days(*year-1) );
           }
           else (*year)++;
           leap = DateCalc_leap_year(*year);
           if (*day > DateCalc_Days_in_Year_[leap][13])
           {
               *day -= DateCalc_Days_in_Year_[leap][13];
               leap  = DateCalc_leap_year(++(*year));
           }
           for ( *month = 12; *month >= 1; (*month)-- )
           {
               if (*day > DateCalc_Days_in_Year_[leap][*month])
               {
                   *day -= DateCalc_Days_in_Year_[leap][*month];
                   break;
               }
           }
           return(true);
       }
       return(false);
   }
   return false;
}

//-----------------------------------------------------------------------------
// timeToSecondsClock()
//   Calculate the number of seconds since 1/1 1980
//-----------------------------------------------------------------------------
unsigned int timeToSecondsClock(unsigned short Year, unsigned char Month, unsigned char Date, unsigned char Hour, unsigned char Min, unsigned char Sec) {
  unsigned int i;
//  MUTEX_ACQUIRE(hMut);
  i=DateCalc_DT_to_linsec(Year, Month, Date, Hour, Min, Sec);
//  MUTEX_RELEASE(hMut);
  return i;
}

//-----------------------------------------------------------------------------
// secondsToTimeClock()
//   Set the current time according to number of seconds since 1/1 1980
//-----------------------------------------------------------------------------
unsigned int secondsToTimeClock(unsigned int linsec, unsigned short *Year, unsigned char *Month, unsigned char *Date, unsigned char *Hour, unsigned char *Min, unsigned char *Sec) {
  Z_int yy,mm,dd,hh,nn,ss;

 // MUTEX_ACQUIRE(hMut);
  if (DateCalc_linsec_to_DT(&yy,&mm,&dd,&hh,&nn,&ss,linsec)) {
     if (Year)  *Year=yy;
     if (Month) *Month=mm;
     if (Date)  *Date=dd;
     if (Hour)  *Hour=hh;
     if (Min)   *Min=nn;
     if (Sec)   *Sec=ss;
 //    MUTEX_RELEASE(hMut);
     return 0;
  } else {
 //    MUTEX_RELEASE(hMut);
     return 1;
  }
}


//-----------------------------------------------------------------------------
// dayOfWeekClock()
//   Retun day of week
//-----------------------------------------------------------------------------
int dayOfWeekClock(int year, int month, int day) {
  return DateCalc_Day_of_Week(year, month, day);
}

//-----------------------------------------------------------------------------
// dayOfYearClock()
//   Return day of year
//-----------------------------------------------------------------------------
int dayOfYearClock(int year, int month, int day) {
  return DateCalc_Day_of_Year(year, month, day);
}
