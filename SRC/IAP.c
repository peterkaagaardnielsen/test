#include "efm32.h"
#include "IAP.h"
#include "debug.h"
//**********************************************************************************
__attribute__((used)) static void feedWatchdog (void) //dont use the WDOG module one, as its not placed in ram
{
    if (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CMD)
        return;
    WDOG->CMD = WDOG_CMD_CLEAR;
}

//*********************************************************************************
__attribute__((used)) unsigned int eraseIAP (unsigned int start, unsigned int end)
{
    #define MSC_PROGRAM_TIMEOUT     10000000ul
    int PageCnt;
    int i;
    unsigned int Addr;
    int Timeout;
    
    __disable_irq();

    PageCnt = ((end - start) / FLASH_PAGE_SIZE) + 1;

    Addr = start;
    for(i = 0; i<PageCnt; i++)
    {
        feedWatchdog();
        //Enable writing to the MSC
        MSC->WRITECTRL |= MSC_WRITECTRL_WREN;
        
        //Load address
        MSC->ADDRB    = Addr;
        MSC->WRITECMD = MSC_WRITECMD_LADDRIM;

        //Check for invalid address
        if (MSC->STATUS & MSC_STATUS_INVADDR)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 1;//mscReturnInvalidAddr;
        }

        //Check for write protected page
        if (MSC->STATUS & MSC_STATUS_LOCKED)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 2;//mscReturnLocked;
        }

        //Send erase page command
        MSC->WRITECMD = MSC_WRITECMD_ERASEPAGE;

        //Wait for the erase to complete
        Timeout = MSC_PROGRAM_TIMEOUT;
        while ((MSC->STATUS & MSC_STATUS_BUSY) && (Timeout != 0))
        {
            Timeout--;
        }

        if (Timeout == 0)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 3;//mscReturnTimeOut;
        }
        //Disable writing to the MSC
        MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
        
        Addr += FLASH_PAGE_SIZE;
    }    

    __enable_irq();
    return 0;
}

//******************************************************************************************************
__attribute__((used)) unsigned int programIAP(unsigned int Address, void *Data, unsigned int NumBytes)
{
    int Timeout;
    int WordCount;
    int NumWords;

    //Check alignment (Must be aligned to words)
    if((Address & 0x3) != 0) {
      messageDebug(DBG_INFO, __MODULE__, __LINE__, "Misaligned buffer!");
      return 1;
    }

    //Check number of bytes. Must be divisable by four
    if((NumBytes & 0x3) != 0) {
      messageDebug(DBG_INFO, __MODULE__, __LINE__, "Misaligned buffer!");
      return 2;
    }

    //Convert bytes to words
    NumWords = NumBytes >> 2;
    
    __disable_irq();

    for (WordCount = 0; WordCount < NumWords; WordCount++)
    {
        feedWatchdog();

        //Enable writing to the MSC
        MSC->WRITECTRL |= MSC_WRITECTRL_WREN;
        
        //Load address
        MSC->ADDRB    = Address + (WordCount*4);
        MSC->WRITECMD = MSC_WRITECMD_LADDRIM;

        //Check for invalid address
        if (MSC->STATUS & MSC_STATUS_INVADDR)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 3;//mscReturnInvalidAddr;
        }

        //Check for write protected page
        if (MSC->STATUS & MSC_STATUS_LOCKED)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 4;//mscReturnLocked;
        }

        //Wait for the MSC to be ready for a new data word
        //Due to the timing of this function, the MSC should already by ready
        Timeout = MSC_PROGRAM_TIMEOUT;
        while (((MSC->STATUS & MSC_STATUS_WDATAREADY) == 0) && (Timeout != 0))
        {
            Timeout--;
        }

        //Check for timeout
        if (Timeout == 0)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 5;//mscReturnTimeOut;
        }

        //Load data into write data register
        MSC->WDATA = *(((unsigned int *) Data) + WordCount);

        //Trigger write once
        MSC->WRITECMD = MSC_WRITECMD_WRITEONCE;

        //Wait for the write to complete
        Timeout = MSC_PROGRAM_TIMEOUT;
        while ((MSC->STATUS & MSC_STATUS_BUSY) && (Timeout != 0))
        {
            Timeout--;
        }

        //Check for timeout
        if (Timeout == 0)
        {
            //Disable writing to the MSC
            MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
            __enable_irq();
            return 6;//mscReturnTimeOut;
        }
        
        //Disable writing to the MSC
        MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
    }
    
    __enable_irq();
    return 0;//mscReturnOk;
}
