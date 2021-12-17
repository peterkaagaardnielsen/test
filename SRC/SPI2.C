#include "efm32.h"
#include "SPI2.h"


//static MUTEX mutSPI;

//-----------------------------------------------------------------------------
// lockSPI2()
//   Gain exclusive access to the SPI2 module
//-----------------------------------------------------------------------------
void lockSPI2(void) 
{
    // MUTEX_ACQUIRE(mutSPI);
}

//-----------------------------------------------------------------------------
// unlockSPI2()
//   Release exclusive access to the SPI2 module
//-----------------------------------------------------------------------------
void unlockSPI2(void) 
{
    // MUTEX_RELEASE(mutSPI);
}

//-----------------------------------------------------------------------------
// flushSPI2()
//   Flush the RX FIFO
//-----------------------------------------------------------------------------
void flushSPI2(void) 
{
    uint8_t dummy;
   // MUTEX_ACQUIRE(mutSPI);
    if(USART2->STATUS & USART_STATUS_RXDATAV)
    {
        dummy = USART2->RXDATA;
    }
   // MUTEX_RELEASE(mutSPI);
}

//------------------------------------------------------------------------------
// sendCharSPI2()
// Send a single byte to the SPI port
//------------------------------------------------------------------------------
unsigned char sendCharSPI2(unsigned char ch) 
{
   // MUTEX_ACQUIRE(mutSPI);
    USART2->TXDATA = ch;
    while (!(USART2->STATUS & USART_STATUS_TXC)) ;
//    while (!(USART2->STATUS & USART_STATUS_RXDATAV)) ;
   // MUTEX_RELEASE(mutSPI);
    return USART2->RXDATA;
}

//------------------------------------------------------------------------------
// sendSPI2()
// Send a buffer to the SPI port
// Input: buf, data to send
//        length, number of bytes to send
//------------------------------------------------------------------------------
void sendSPI2(unsigned char *buf, unsigned int length) 
{
    if ( length == 0 )
        return;

    //  MUTEX_ACQUIRE(mutSPI);

   while (length > 0) 
    {
        sendCharSPI2(*buf);
  	    length--;
        buf++;
    }
    // MUTEX_RELEASE(mutSPI);
}


//------------------------------------------------------------------------------
// receiveCharSPI2()
// Receive one byte from SPI port
//------------------------------------------------------------------------------
unsigned char receiveCharSPI2(void) 
{
   // MUTEX_ACQUIRE(mutSPI);
    USART2->TXDATA = 0xFF;
    while (!(USART2->STATUS & USART_STATUS_TXC)) ;
//    while (!(USART2->STATUS & USART_STATUS_RXDATAV)) ;
   // MUTEX_RELEASE(mutSPI);
    return USART2->RXDATA;
}

//------------------------------------------------------------------------------
// receiveCharSPI2Int()
// Receive one byte from SPI port (no mutex protection !)
//------------------------------------------------------------------------------
static unsigned char receiveCharSPI2Int(void) 
{
    USART2->TXDATA = 0xFF;
    while (!(USART2->STATUS & USART_STATUS_TXC)) ;
//    while (!(USART2->STATUS & USART_STATUS_RXDATAV)) ;
    return USART2->RXDATA;
}

//------------------------------------------------------------------------------
// receiveSPI2()
// Receives a number of bytes from the SPI port
// Input: buf, buffer where data will be placed in
//        length, number of bytes to receive
//------------------------------------------------------------------------------
void receiveSPI2(unsigned char *buf, unsigned int length) 
{
    unsigned int i;
    // MUTEX_ACQUIRE(mutSPI);
    for (i = 0; i < length; i++)
    buf[i] = receiveCharSPI2Int();
    //MUTEX_RELEASE(mutSPI);
}

//------------------------------------------------------------------------------
// setCSSPI2()
// Set CS (SSET) pin of the SPI port
// Input: hi, new state of the CS pin
//------------------------------------------------------------------------------
void setCSSPI2(unsigned char hi)
{
  volatile int i;
//  for (i=0;i<10; i++);
  
    if(hi)
    {
        GPIO->P[2].DOUTSET = (1<<5);
    }
    else
    {
        GPIO->P[2].DOUTCLR = (1<<5);
    }    
}

//-----------------------------------------------------------------------------
// initSPI2()
// Initializes the SPI2 port 
//-----------------------------------------------------------------------------
void initSPI2(void) 
{
  CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_GPIO + CMU_HFPERCLKEN0_USART2);
  
  /* RF_MOSI, Pin PC2 is configured to Push-pull */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;
  /* RF_MISO, Pin PC3 is configured to Input enabled with pull-down */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_INPUT;
  /* RF_SCK, Pin PC4 is configured to Push-pull */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE4_MASK) | GPIO_P_MODEL_MODE4_PUSHPULL;
  /* RF_SSEL, Pin PC5 is configured to Push-pull */
  GPIO->P[2].MODEL = (GPIO->P[2].MODEL & ~_GPIO_P_MODEL_MODE5_MASK) | GPIO_P_MODEL_MODE5_PUSHPULL;
  
  setCSSPI2(1);

  USART2->CLKDIV = 0;
  USART2->CTRL = USART_CTRL_SYNC + USART_CTRL_MSBF;
  USART2->CMD = USART_CMD_CLEARRX + USART_CMD_CLEARTX;
  USART2->IEN = 0; 
  USART2->ROUTE = USART_ROUTE_LOCATION_LOC0 + USART_ROUTE_CLKPEN + USART_ROUTE_TXPEN + USART_ROUTE_RXPEN;
  USART2->FRAME = USART_FRAME_DATABITS_EIGHT;
  USART2->CMD = USART_CMD_MASTEREN + USART_CMD_TXEN + USART_CMD_RXEN;  

  flushSPI2();
}



