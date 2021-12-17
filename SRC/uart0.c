//------------------------------------------------------------------------------
// UART0                                                        20130108 CHG/SSA
//------------------------------------------------------------------------------
#include "efm32.h"
#include "system.h"
#include <string.h>
#include <limits.h>
#include "uart0.h"
#include "Debug.h"

#define RBUF_SIZE   128
#define TBUF_SIZE   128

static U64 thUART0Stk[300]; // Stack 
static OS_TID mTID = NULL;
static unsigned int iTime;
static MUTEX mutUSART0;
static unsigned int Baudrate, Ien;
static unsigned int bSize;
static unsigned char *pUserBuf = NULL;
static uart0_callback_t mycb = NULL;


//------------------------------------------------------------------------------
// Disable UART interrupt
//------------------------------------------------------------------------------
static void disableINT(void) {
  USART0->IEN &= ~(USART_IEN_RXDATAV);
  NVIC_DisableIRQ(USART0_RX_IRQn);
}

//------------------------------------------------------------------------------
// Enable UART interrupt
//------------------------------------------------------------------------------
static void enableINT(void) {
  USART0->IEN |= (USART_IEN_RXDATAV);
  NVIC_EnableIRQ(USART0_RX_IRQn);
}


//------------------------------------------------------------------------------
// Make some check about the buffersize, must at least be 2, and also power of 2
//------------------------------------------------------------------------------
#if RBUF_SIZE < 2
  #error RBUF_SIZE is too small.  It must be larger than 1.
#elif ((RBUF_SIZE & (RBUF_SIZE-1)) != 0)
  #error RBUF_SIZE must be a power of 2.
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct buf_rx {
  unsigned int idxMarkerInput;          // Next In Index
  unsigned int idxMarkerOutput;         // Next Out Index 
  unsigned int MarkerCount;
  char buf [RBUF_SIZE];     // Buffer
};

static struct buf_rx rbuf = { 0, 0, 0};

//------------------------------------------------------------------------------
// Acquire UART0 (if UART is shared between peripherals)
//------------------------------------------------------------------------------
void acquireUART0(void) {
  MUTEX_ACQUIRE(mutUSART0);
  // Wait for TX to finish
  while (!(USART0->STATUS & USART_STATUS_TXBL)) ;
  // Remember IEN status
  Ien=USART0->IEN;
  USART0->IEN=0;
  // To avoid false start, configure output US0_TX as high on PE10
  GPIO->P[4].DOUT |= (1 << 10);
  // Disconnect TX and RX pins from UART
  USART0->ROUTE = 0;
  // The caller now has access to the UART
}

//------------------------------------------------------------------------------
// Release UART0 (if UART is shared between peripherals)
// After this call, the UART0 is configured for Async
//------------------------------------------------------------------------------
void releaseUART0(void) {
  // Wait for TX to finish
  while (!(USART0->STATUS & USART_STATUS_TXBL)) ;
  // To avoid false start, configure output US0_TX as high on PE10
  GPIO->P[4].DOUT |= (1 << 10);
  // Configure UART to Async use again
  USART0->CMD = USART_CMD_RXDIS | USART_CMD_TXDIS | USART_CMD_MASTERDIS;
  USART0->CTRL = 0;
  USART0->CMD = USART_CMD_CLEARRX + USART_CMD_CLEARTX;
  USART0->FRAME = USART_FRAME_PARITY_NONE + USART_FRAME_DATABITS_EIGHT + USART_FRAME_STOPBITS_ONE;
  // Set the baudrate again
  setbaudUART0(Baudrate);
  // and set the IEN flags again
  USART0->IEN=Ien;
  // Start uart
  USART0->CMD = 5;
  // Route TX and RX to pin PE10 and PE11 again
  USART0->ROUTE = USART_ROUTE_LOCATION_LOC0 | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN;
  
  // Caller no longer owns the UART
  MUTEX_RELEASE(mutUSART0);
}



//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void USART0_RX_IRQHandler(void) 
{
    volatile unsigned char ch;
  
    if(USART0->STATUS & USART_STATUS_RXDATAV) 
    {
        if (rbuf.MarkerCount >= RBUF_SIZE) 
        {
            ch = USART0->RXDATA;
        } 
        else 
        {
            rbuf.buf[rbuf.idxMarkerInput] = USART0->RXDATA;
            rbuf.idxMarkerInput=(rbuf.idxMarkerInput+1) % RBUF_SIZE;
            rbuf.MarkerCount++;
        }
        if (rbuf.MarkerCount >= bSize  && (mTID !=NULL)) 
        {
            isr_evt_set(1, mTID);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void enableUART0(unsigned int enable) {
  MUTEX_ACQUIRE(mutUSART0);
  if (enable) {
    enableINT();
  } else {
    disableINT();
  }
  MUTEX_RELEASE(mutUSART0);
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void setbaudUART0(unsigned int baudrate) {
  #define _USART_CTRL_OVS_MASK                  0x60UL                                   /**< Bit mask for USART_OVS */
  #define _USART_CLKDIV_MASK                    0x001FFFC0UL                     /**< Mask for USART_CLKDIV */
  #define _USART_CTRL_OVS_X16                   0x00000000UL                             /**< Mode X16 for USART_CTRL */
  #define USART_CTRL_OVS_X16                    (_USART_CTRL_OVS_X16 << 5)               /**< Shifted mode X16 for USART_CTRL */
  unsigned int clkdiv;
  
  MUTEX_ACQUIRE(mutUSART0);
  Baudrate=baudrate;
  // Calculate and set CLKDIV with fractional bits
  clkdiv  = 4 * SystemHFClockGet() + (16 * baudrate) / 2;
  clkdiv /= (16 * baudrate);
  clkdiv -= 4;
  clkdiv *= 64;
  clkdiv &= _USART_CLKDIV_MASK;
  USART0->CTRL  &= ~_USART_CTRL_OVS_MASK;
  USART0->CTRL  |= USART_CTRL_OVS_X16;
  USART0->CLKDIV = clkdiv;
  MUTEX_RELEASE(mutUSART0);
}

//------------------------------------------------------------------------------
// Send a single character to the UART
//------------------------------------------------------------------------------
int putcharUART0(char c) {
  MUTEX_ACQUIRE(mutUSART0);
  while (!(USART0->STATUS & USART_STATUS_TXBL));
  USART0->TXDATA = c;
  while (!(USART0->STATUS & USART_STATUS_TXC));
  MUTEX_RELEASE(mutUSART0);
  return (c);
}

//------------------------------------------------------------------------------
// Send a zero terminated string to the UART
//------------------------------------------------------------------------------
void putstringUART0(char *buf) {
  MUTEX_ACQUIRE(mutUSART0);
  while (*buf) {
    while (!(USART0->STATUS & USART_STATUS_TXBL));
    USART0->TXDATA = *(buf++);
    while (!(USART0->STATUS & USART_STATUS_TXC));
  }
  MUTEX_RELEASE(mutUSART0);
}

//------------------------------------------------------------------------------
// Get a single character from UART0
// Returns 1 if no available, 0 if character is read
//------------------------------------------------------------------------------
int getcharUART0(char *ch) {
  disableINT();
  if (rbuf.MarkerCount == 0) {
    enableINT();
    return 1;
  }

  *ch=rbuf.buf[rbuf.idxMarkerOutput];
  rbuf.idxMarkerOutput=(rbuf.idxMarkerOutput+1) % RBUF_SIZE;
  rbuf.MarkerCount--;

  enableINT();
  return 0;
}

//******************************************************
void getBulkUART0(unsigned int len, struct buf_rx *p)
{
    unsigned int count;

    if(pUserBuf != NULL) 
    {
        memset(pUserBuf,0xCC,bSize);

        disableINT();
        for(count=0; count<len; count++) 
        {
            pUserBuf[count] = rbuf.buf[rbuf.idxMarkerOutput];
            rbuf.idxMarkerOutput = (rbuf.idxMarkerOutput+1) % RBUF_SIZE;
            rbuf.MarkerCount--;
        }
        enableINT();
    }
}

//******************************
__task void thRXUART0(void)
{
    U16 evt;
    int i;
    unsigned short size;

    volatile char dummy;

    rbuf.idxMarkerOutput = 0;
    rbuf.idxMarkerInput = 0;
    rbuf.MarkerCount = 0;
  
    USART0->CMD = USART_CMD_TXEN + USART_CMD_RXEN;
    dummy = USART0->RXDATA;
  
    mTID = os_tsk_self(); 

 
    while(1) 
    {
        if(os_evt_wait_or(0xFFFF, iTime) == OS_R_EVT) 
        {
            // Got an event, check what it is..
            evt = os_evt_get();
            //stop the thread
            if(evt & 0x02) 
            {              
                os_tsk_delete_self();  
            } 
            else if(evt & 0x01) 
            {
                disableINT();
                size = rbuf.MarkerCount; 
                enableINT();

                for(i=0;i<(size/bSize);i++)
                {
                    getBulkUART0(bSize,&rbuf);
                    if (mycb) mycb(bSize);
                }
            }
        } 

        else 
        {
            int l;
            disableINT();
            size = rbuf.MarkerCount; 
            enableINT();
            
            while (size) 
            {
                l = MINIMUM(bSize, size);
                getBulkUART0(l,&rbuf);
                if (mycb) mycb(l);
                size -= l;
            }
        }

    }
}

//********************************************************************************************************************
void setCallbackUART0(unsigned char *buffer, unsigned short blockSize, unsigned short idleTime, uart0_callback_t cb)
{
    mycb = cb;
    bSize = blockSize;
    if(idleTime > 10) 
    {
        iTime = idleTime/10;
    } 
    else 
    {
        iTime = 1;
    }

    pUserBuf = buffer;
    if(pUserBuf)
      START_THREAD_USR(thRXUART0, 0, thUART0Stk, sizeof(thUART0Stk));
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void initUART0(void) {
  volatile char dummy;
  
  MUTEX_INIT(mutUSART0);

  rbuf.idxMarkerOutput = 0;
  rbuf.idxMarkerInput = 0;
  rbuf.MarkerCount = 0;

  CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_GPIO + CMU_HFPERCLKEN0_USART0);

  /* To avoid false start, configure output US0_TX as high on PE10 */
  GPIO->P[4].DOUT |= (1 << 10);
  /* Pin PE10 is configured to Push-pull */
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE10_MASK) | GPIO_P_MODEH_MODE10_PUSHPULL;
  /* Pin PE11 is configured to Input enabled */
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE11_MASK) | GPIO_P_MODEH_MODE11_INPUT;
   
  USART0->CMD = USART_CMD_RXDIS | USART_CMD_TXDIS | USART_CMD_MASTERDIS;
  USART0->CMD = USART_CMD_CLEARRX + USART_CMD_CLEARTX;
  USART0->ROUTE |= USART_ROUTE_LOCATION_LOC0 | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN;
  USART0->FRAME = USART_FRAME_PARITY_NONE + USART_FRAME_DATABITS_EIGHT + USART_FRAME_STOPBITS_ONE;
  USART0->CMD = 5;

  enableUART0(0);
}
