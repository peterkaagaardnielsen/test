#include "efm32.h"
#include "fw.h"
#include "smon.h"
#include "system.h"
//#include "Filesystem.h"
#include "iap.h"
#include "debug.h"
#include "rtc.h"
#include "ADC.h"
#include "WDT.h"
#include "hcc/src/api/api_tiny.h"

#include "RF.h"
#include "gsm.h"
#include "pmon.h"

//---------------------------------------------------------------------------
// Minimum Vcc voltage for device to run 
//---------------------------------------------------------------------------
#define VCC_MIN 2800 


//#define BLK_SIZE FLASH_PAGE_SIZE
#define BLK_SIZE 512

#define _MIN(x,y)	((x)<(y)?(x):(y))

extern int FWVersion; // 10 = 0.1

static unsigned char fw_buffer[BLK_SIZE];   
static U64 thMAINStk[300];
static int UserButton;
static int DevelMode=FALSE;
static int tActivity;
extern int os_time;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void restartTimeoutBootloader(void) {
    tActivity=os_time+(100*30);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void SystemInit(void) {
  
  // Set 1 waitstate as we run above 16 MHz
  MSC->READCTRL = MSC_READCTRL_MODE_WS1SCBTP;
  //  CMU->HFRCOCTRL = 0x00000580;
    // Enable HFXO, AUXHFRCO and LFXON
    CMU->OSCENCMD = CMU_OSCENCMD_HFXOEN | /*CMU_OSCENCMD_AUXHFRCOEN |*/ CMU_OSCENCMD_LFXOEN;
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
  
    /* Wait until clock is ready */
    while (!( /*(CMU->STATUS & CMU_STATUS_AUXHFRCORDY) ||*/ (CMU->STATUS & CMU_STATUS_HFXORDY) || (CMU->STATUS & CMU_STATUS_LFXORDY))) ;
    // Select HFXO as clock
    CMU->CMD = 2;
}

//---------------------------------------------------------------------------
// Load and check an FAL image
//----------------------------------------------------------------------------
int loadFW(void) 
{
    int i;
    unsigned int crc;
    F_FILE *file;
    unsigned char* pImage;
    static tdefFWHeader FWHeader;
    unsigned int flashaddr;
    unsigned int togo;

    // Open file
    file=f_open("firmware.sfw","r");
    if (!file) 
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__, "No firmware file found"); 
        return 1;
    }
  
    // Read header info
	  f_read(&FWHeader, 1, sizeof(FWHeader), file);

    // Check cookie and that length is not too big
    if (FWHeader.cookie!=FW_COOKIE || FWHeader.length>FW_MAX_SIZE) 
    {
        messageDebug(DBG_ERR, __MODULE__, __LINE__, "Invalid firmware file"); 
        f_close(file);
        return 2;
    }

    if (FWHeader.target != TARGET) 
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__, "WRONG Firmware type, not intended for this target! Firmware target=%i, this target=%i", FWHeader.target, TARGET); 
        f_close(file);
        return 4;
    }
  
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "Current Firmware, version=%i.%02i size=%i", *((int*)FW_VERSION_CM3)/100, *((int*)FW_VERSION_CM3)%100, *((int*)FW_SIZE_CM3)); 
  
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "Loaded Firmware file, version=%i.%02i size=%i, crc=%08X, Target=%i", FWHeader.version/100, FWHeader.version%100, FWHeader.length, FWHeader.crc, FWHeader.target); 
    
    // If button is NOT pressed, we check if firmware installed is the same as on disk...
    if (UserButton==FALSE) {	
//    if (FALSE) {	
      if (FWHeader.version == *((int*)FW_VERSION_CM3) && (FWHeader.length == *((int*)FW_SIZE_CM3))) 
      {
          messageDebug(DBG_INFO, __MODULE__, __LINE__, "Firmware version matches the version on disk (same version and size), no update needed"); 
          f_close(file);
          return 3;
      }
    } else {
  		messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
      messageDebug(DBG_INFO, __MODULE__, __LINE__,"User button pressed, force flashing of firmware!!!");
  		messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
    }
    
   
    i = eraseIAP(FW_ENTRY_CM3, FW_ENTRY_CM3 + FW_MAX_SIZE-1);

    if (i==0)
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"FLASH erased");
    }
    else 
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"Error %i erasing FLASH", i);
        f_close(file);
        return 5; // Error erasing flash
    }

  
    // Don't program the first block yet..
	  f_read(fw_buffer, BLK_SIZE, 1, file);
    // Need to start the decode from the beginning..		
    fwencode(fw_buffer, BLK_SIZE, TRUE);

    flashaddr = FW_ENTRY_CM3 + BLK_SIZE;
    togo = FWHeader.length - BLK_SIZE;
    
    while (togo) 
    {
      	f_read(fw_buffer, _MIN(togo, BLK_SIZE),1 , file);
        fwencode(fw_buffer, _MIN(togo, BLK_SIZE), FALSE);
			
        if (i=programIAP (flashaddr, fw_buffer, BLK_SIZE)) 
        {
            messageDebug(DBG_INFO, __MODULE__, __LINE__,"Error %i programming image to FLASH", i);
            f_close(file);
            return 6; // Error programming image
        }
  
				flashaddr += BLK_SIZE;
        togo -= _MIN(togo, BLK_SIZE);
    }

    flashaddr = FW_ENTRY_CM3;
    // Now program the first block into FLASH...
    messageDebug(DBG_INFO, __MODULE__, __LINE__,"programming first block");
    f_close(file);
    file=f_open("firmware.sfw","r");
    f_read(&FWHeader, sizeof(FWHeader), 1, file); 
    f_read(fw_buffer, BLK_SIZE, 1, file);
    fwencode(fw_buffer, BLK_SIZE, TRUE);
    f_close(file);

    if (i=programIAP (flashaddr, fw_buffer, BLK_SIZE)) 
    {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"Error %i programming first block", i);
        return 7; // Error programming first block
    }

    // Check the CRC on the loaded image
    pImage = (unsigned char*)FW_ENTRY_CM3;
    crc = 0;
    for (i=0; i<FWHeader.length; i++)
    {
      crc += *(pImage+i);
    }
    crc+=FWHeader.length;

    // If crc error, report
    if (crc != FWHeader.crc) 
    {
        messageDebug(DBG_ERR, __MODULE__, __LINE__, "Invalid CRC after programming, is %08X, should be %08X", crc, FWHeader.crc); 
        //erasing first block to avoid faulty FW getting startet
        messageDebug(DBG_ERR, __MODULE__, __LINE__, "Erasing first fw-block"); 
        eraseIAP(FW_ENTRY_CM3, FW_ENTRY_CM3);
        return 8;
    }
    // All is ok..
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "Firmware image programmed and verified, CRC=0x%08X", crc); 

    return 0;
}


//----------------------------------------------------------------------------
// Return Vbus in mV
//----------------------------------------------------------------------------
static int getVbus(void) {
  // Vbus = 2500 / 4095*cnt)
  return (61035*readADC(ADC_AD4))/50729;
}

//----------------------------------------------------------------------------
// Return CPU Vcc in mV
//----------------------------------------------------------------------------
static int getVcc(void) {
  // Vdd/3 = 1250 / 4095*cnt)
  // 1 cnt = 0.3051 mV input  
  return 3*((3051 * readADC(ADC_AD9))/10000);
}

void mydir() {
  F_FIND find;
  if (!f_findfirst("*",&find)) {
    do {
      messageDebug(DBG_INFO, __MODULE__, __LINE__,"filename:%s",find.filename);
      if (find.attr&F_ATTR_DIR) {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"directory");
      } else {
        messageDebug(DBG_INFO, __MODULE__, __LINE__,"size %d",find.filesize);
      }     
    } while (!f_findnext(&find));
  }
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void thMain(void)
{
    static int i, tick=0;
    static F_FILE* file;

    // Divide coreclock with 16, and adjust SysTick reload so RTOS maintains the tickrate (10 mSec)
  #ifdef DK3750
    CMU->HFCORECLKDIV=1;
  #else
    CMU->HFCORECLKDIV=0;
  #endif
  //CMU->HFCORECLKDIV=0;
  #define OS_TICK 10000 // in uSec
  SysTick->LOAD=((U32)(((double)SystemCoreClockGet()*(double)OS_TICK)/1E6)-1);
  

  // Enable GPIO clock
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

  //power down dataflash asap
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE15_MASK) | GPIO_P_MODEH_MODE15_PUSHPULL;
  GPIO->P[2].DOUTSET = (1<<15); // Power off to dataflash

  // Pin PA0 (red LED) is configured to Push-pull
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_PUSHPULL;
  // Pin PA1 (blue LED) is configured to Push-pull
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_PUSHPULL;
  // Pin PA2 (yellow LED) is configured to Push-pull
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;

  // Pin PC1 is configured to Input enabled with pull-up and filter
  GPIO->P[2].DOUTSET = (1 << 1);
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_INPUTPULLFILTER;

  OS_WAIT(500);
  UserButton=(GPIO->P[2].DIN & (1 << 1)) ? 0:1;

  #ifdef DK3750
  //UserButton=TRUE;
  #endif

  initDebug();
  setOutputDebug(DBG_MON);



  #define CurrentTimeInt_H BURTC->RET[127].REG
  #define CurrentTimeInt_L BURTC->RET[126].REG
  messageDebug(DBG_INFO, __MODULE__, __LINE__, "Raw RTC: H=%08X, L=%08X, CNT=%08X, CTRL=0x%08X", CurrentTimeInt_H, CurrentTimeInt_L, BURTC->CNT,  BURTC->CTRL);
  

//    initRTC();
    initWDT();

  initSMon(115200);

  #ifdef _DRAGONFLY_
  initPMON(0);
  RF_Start();
  OS_WAIT(100);
  #endif
  
//TEST!!!!!!!!!!!!!!
//UserButton=TRUE;

    if (UserButton) {
      messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
      messageDebug(DBG_INFO, __MODULE__, __LINE__,"User buttton pressed !beep");
      messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
    }

    messageDebug(DBG_WAR, __MODULE__, __LINE__, "******************** Bootloader ******************* ");
    //messageDebug(DBG_INFO, __MODULE__, __LINE__,"SSA RF TEST VERSION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "Lommy platform 9 (DRAGONFLY) ");
    messageDebug(DBG_WAR, __MODULE__, __LINE__, "Bootloader %i.%02i, build: %s %s", FWVersion/100, FWVersion%100, __DATE__, __TIME__);
   
    
    messageDebug(getVcc() < VCC_MIN ? DBG_ERR:DBG_INFO, __MODULE__, __LINE__, "Vcc=%i mV", getVcc());
    while (getVcc() < VCC_MIN) {
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "+++++++++++++++++++++++++++++++++++++++++++++++++++");
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "LOW WOLTAGE DETECTED! Vcc=%i mV", getVcc());
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "Resetting unit in 10 seconds");
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "+++++++++++++++++++++++++++++++++++++++++++++++++++");
      OS_WAIT(10000);
      resetDeviceWDT();
    }

	/* Pin PC15 (MEM_PON) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE15_MASK) | GPIO_P_MODEH_MODE15_PUSHPULL;
  GPIO->P[2].DOUTCLR = (1<<15); // Power on to dataflash
	OS_WAIT(500);

  f_init();
  i=f_initvolume();
  if (i!=F_NO_ERROR) {
    messageDebug(DBG_ERR, __MODULE__, __LINE__, "Disk not formatted, doing a format!");
    i=f_format();
    if (i!=F_NO_ERROR)
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "COULD NOT FORMAT DISK!!!");
  }

//  mydir();
  
    
  //  i=initFileSystem();
    while (!isEmptyDebug()) OS_WAIT(10);

    file=f_open("HOOTERS","r");
    if (file) {
      f_close(file);
      messageDebug(DBG_ERR, __MODULE__, __LINE__, "HOOTERS file found, NOT loading new firmware file !beep"); 
      DevelMode=TRUE;
    }
  

    
    if(i == 0)
    {
     // f_enterFS();
      // Look for new firmware, if found, go program it
    	if (DevelMode==FALSE) loadFW();
    } else {
      messageDebug(DBG_INFO, __MODULE__, __LINE__, "Could NOT intialize filesystem, rc=%i !", i);
    }
    
    // If user button pressed, do NOT execute firmware but stay in bootloader
    if (UserButton) 
    {	
  		messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
	    messageDebug(DBG_INFO, __MODULE__, __LINE__,"User buttton pressed, staying in bootloader");
  		messageDebug(DBG_INFO, __MODULE__, __LINE__,"+++++++++++++++++++++++++++++++++++++++++++++++");
    } 
    else 
    {
        while(RF_IsConnected())
        {
            OS_WAIT(1000);
            feedWDT();
            GPIO->P[0].DOUTTGL = 0x01;//toggle red LED
        }

        if (*((int*)FW_ENTRY_CM3) == 0xFFFFFFFF) 
        {
            messageDebug(DBG_ERR, __MODULE__, __LINE__,"***********************************************");
            messageDebug(DBG_ERR, __MODULE__, __LINE__,"******** No Firmware detected in memory*********");
            messageDebug(DBG_ERR, __MODULE__, __LINE__,"***********************************************");
        } 
        else 
        {
            messageDebug(DBG_INFO, __MODULE__, __LINE__,"**************** Executing Firmware ***************");
            while (!isEmptyDebug()) OS_WAIT(10);
        
            if(*(unsigned int*)FW_ENTRY_CM3 != 0xFFFFFFFF) 
            {
                NVIC_DisableIRQ(USART0_RX_IRQn);
                tsk_lock();
                // The application reset vector is at BASE+4 (BASE+0 is stack address)
                ((void(*)())*((volatile uint32_t *) (FW_ENTRY_CM3+4)))();
            }
        }  
    }		

/*    
  // Pin PA1 is configured to Push-pull 
  GPIO->P[0].MODEL = (GPIO->P[0].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_PUSHPULL;
    
  // Enable LFXO signal CLKOUT1 (same pin as the switch)
  CMU->ROUTE |= CMU_ROUTE_CLKOUT1PEN;
  CMU->CTRL &= ~(7<<23); 
  CMU->CTRL |= (3<<23); 
*/    
    i=0;

    /*
        setDebugGSM(TRUE);
        initGSM();
        powerGSM(1, "");
        sendStringGSM("ATI1\r");
*/
    
   	restartTimeoutBootloader();

    while(1) {
		// If inactivity for too long from PC, reset
		if (os_time > tActivity) {
			messageDebug(DBG_INFO, __MODULE__, __LINE__, "No activity, resetting");
			OS_WAIT(100);
			NVIC_SystemReset();
		}
		
        OS_WAIT(1000);
        feedWDT();
        tick++;
        messageDebug(DBG_INFO, __MODULE__, __LINE__, "Bootloader active, cnt=%i", tick);
        GPIO->P[0].DOUTTGL = 0x01;//toggle red LED
    }
}

//*****************
int main(void)
{
    INIT_THREAD_USR(thMain, 0, (void*)thMAINStk, sizeof(thMAINStk));
    return 0;
}
