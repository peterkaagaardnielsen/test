#include "efm32.h"
#include "System.h"
#include "WDT.h"


//NOTE: This code uses the internal WDT - in reality an external WDT will be used!!

#define WDT_TIMEOUT 12 // = 33 sec

static int init = FALSE;


//************************
int isActiveWDT(void) 
{
    return init;
}

//----------------------------------------------------------------------------
// Force a reboot of the device...
//----------------------------------------------------------------------------
void rebootWDT(void) 
{
    resetDeviceWDT();
}

//********************
void feedWDT (void)
{
    if (init) 
    {
        if (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CMD)
            return;

        WDOG->CMD = WDOG_CMD_CLEAR;
    }
}

//******************************
void initWDT(void) 
{ 
    CMU->OSCENCMD |= CMU_OSCENCMD_LFRCOEN;
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;

    while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL)
        ;

    init = TRUE;
    feedWDT();

    WDOG->CTRL = WDOG_CTRL_CLKSEL_ULFRCO | (WDT_TIMEOUT<<8) | WDOG_CTRL_EN;

    while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL)
        ;
}

//----------------------------------------------------------------------------
// Force an immediate reboot of the device...
//----------------------------------------------------------------------------
void resetDeviceWDT(void) 
{
    int x;
    
    init = FALSE;

    tsk_lock();
    __disable_irq(); // Disable all interrupts..
    for (x=0; x<10000; x++);

    // Do a reset of the CPU
    NVIC_SystemReset();

    while (1);
}

