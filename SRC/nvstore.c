#include "efm32.h"
#include "system.h"
#include "nvstore.h"

#define MAX_PAGES 4096
#define PAGE_SIZE  512

#define RESET_HIGH 
#define RESET_LOW 

//***********************
static void cs_high(void)
{
    GPIO->P[2].DOUTSET = 0x100; // PC8 is CS
}
//***********************
static void cs_low(void)
{
    GPIO->P[2].DOUTCLR = 0x100; // PC8 is CS
}

//********************************************************************
static unsigned char Tx(unsigned char TxVal, unsigned char* RxVal)
{
    unsigned char myval;
    
    USART0->TXDATA = TxVal;
    while (!(USART0->STATUS & USART_STATUS_TXC)) ;

    myval = USART0->RXDATA;

    if(RxVal != 0)
    {
        *RxVal = myval;
    }
    return 1;
}

//***************************************************
static unsigned char Status(unsigned char* Result)
{
    unsigned char ret;

    cs_low();
    ret = Tx(0xD7, 0);
    ret = Tx(0, Result);
    cs_high();

    return ret;
}

//***********************************
static unsigned char WaitReady(void)
{
    unsigned char b;
    int MyTimeCnt = 0;

    while(MyTimeCnt < 10)
    {    
        b = 0;
        Status(&b);
        if(b & 0x80)
        {
            return 1;
        }
        OS_WAIT(10);
        MyTimeCnt++;
    }
    return 0;
}


static openSync(void) {
  acquireUART0();

  // Now we own the USART :)
  cs_high();
  USART0->CLKDIV = 0;
  USART0->CMD = USART_CMD_RXDIS | USART_CMD_TXDIS | USART_CMD_MASTERDIS;
  USART0->CMD = USART_CMD_CLEARRX + USART_CMD_CLEARTX;
  USART0->CTRL = USART_CTRL_TXDELAY_SINGLE /*+ USART_CTRL_AUTOCS*/ + USART_CTRL_SYNC + USART_CTRL_MSBF + USART_CTRL_CLKPOL + USART_CTRL_CLKPHA;
  USART0->ROUTE = USART_ROUTE_LOCATION_LOC2 + USART_ROUTE_CLKPEN /*+ USART_ROUTE_CSPEN*/ + USART_ROUTE_TXPEN + USART_ROUTE_RXPEN;
  USART0->FRAME = USART_FRAME_DATABITS_EIGHT;
  USART0->CMD = USART_CMD_MASTEREN;  
  USART0->CMD = USART_CMD_TXEN + USART_CMD_RXEN;  
}


static releaseSync(void) {
  releaseUART0();
}


//*****************************
void initNVstore(void)
{
  CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_GPIO + CMU_HFPERCLKEN0_USART0);

  RESET_LOW;
  
  /* Pin PC15 (MEM_PON) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE15_MASK) | GPIO_P_MODEH_MODE15_PUSHPULL;
  GPIO->P[2].DOUTCLR = (1<<15); // Power on
  OS_WAIT(10);
  
  
  /* Pin PC8 (CS) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE8_MASK) | GPIO_P_MODEH_MODE8_PUSHPULL;
  /* Pin PC9 (SCK) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE9_MASK) | GPIO_P_MODEH_MODE9_PUSHPULL;
  /* Pin PC10 (MISO) is configured to Input enabled */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE10_MASK) | GPIO_P_MODEH_MODE10_INPUT;
  /* To avoid false start, configure output US0_TX as high on PC11 */
  GPIO->P[2].DOUT |= (1 << 11);
  /* Pin PC11 (MOSI) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE11_MASK) | GPIO_P_MODEH_MODE11_PUSHPULL;

  /* Pin PC8 (CS) is configured to Push-pull */
  GPIO->P[2].MODEH = (GPIO->P[2].MODEH & ~_GPIO_P_MODEH_MODE8_MASK) | GPIO_P_MODEH_MODE8_PUSHPULL;
  
  RESET_HIGH;

  
}

//*********************************************************************
int readNVstore(unsigned char *buffer, int pageNumber)
{
    int i;
    unsigned int adr;

    if(pageNumber > (MAX_PAGES-1))
    {
        return NVSTORE_ERR_PAGE_NO;
    }

    openSync();
    
    if(WaitReady() == 0)
    {
      releaseSync();  
      return NVSTORE_ERR_COMM;
    }

    adr = pageNumber;
    adr <<= 10;
    
    cs_low();
    if(Tx(0x03, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF0000)>>16, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF00)>>8, 0) == 0)
    {
        cs_high();
      releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF), 0) == 0)
    {
        cs_high();
      releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    
    for(i=0; i<PAGE_SIZE; i++)
    {
        if(Tx(0, &buffer[i]) == 0)
        {
            cs_high();
      releaseSync();  
            return NVSTORE_ERR_COMM;
        }
    }
    cs_high();

      releaseSync();  
    return NVSTORE_ERR_OK;
}

//*********************************************************************
int writeNVstore(const unsigned char *buffer, int pageNumber)
{
    int i;
    unsigned int adr;

    if(pageNumber > (MAX_PAGES-1))
    {
        return NVSTORE_ERR_PAGE_NO;
    }
    openSync();

    if(WaitReady() == 0)
    {
      releaseSync();  
        return NVSTORE_ERR_COMM;
    }

    adr = pageNumber;
    adr <<= 10;

    cs_low();
    if(Tx(0x84, 0) == 0) //write to buffer 1
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx(0, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx(0, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx(0, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    
    for(i=0; i<PAGE_SIZE; i++)
    {
        if(Tx(buffer[i], 0) == 0)
        {
            cs_high();
            releaseSync();  
            return NVSTORE_ERR_COMM;
        }
    }
    cs_high();

    if(WaitReady() == 0)
    {
      releaseSync();  
        return NVSTORE_ERR_COMM;
    }

    cs_low();
    if(Tx(0x83, 0) == 0) //latch buffer to page-adr
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF0000)>>16, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF00)>>8, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF), 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    cs_high();

    releaseSync();  
    return NVSTORE_ERR_OK;
}

//***************************************************
int eraseNVstore(int pageNumber)
{
    unsigned int adr;

    if(pageNumber > (MAX_PAGES-1))
    {
        return NVSTORE_ERR_PAGE_NO;
    }
    openSync();

    if(WaitReady() == 0)
    {
      releaseSync();  
      return NVSTORE_ERR_COMM;
    }

    adr = pageNumber;
    adr <<= 10;

    cs_low();
    if(Tx(0x81, 0) == 0) 
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF0000)>>16, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF00)>>8, 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    if(Tx((adr&0xFF), 0) == 0)
    {
        cs_high();
        releaseSync();  
        return NVSTORE_ERR_COMM;
    }
    cs_high();

    releaseSync();  
    return NVSTORE_ERR_OK;
}


