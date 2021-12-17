//------------------------------------------------------------------------------
// LEUART0.c                                                        20130130 CHG
//------------------------------------------------------------------------------
#include "efm32.h"
#include "system.h"
#include "leuart0.h"

#define RBUF_SIZE   1024 // Receive buffer size (multiplum of 2!)

static int isEnabled = 0;
static int currentBaudrate=0;

//------------------------------------------------------------------------------
// Make some check about the buffersize, must at least be 2, and also power of 2
//------------------------------------------------------------------------------
#if RBUF_SIZE < 2
  #error RBUF_SIZE is too small.  It must be larger than 1.
#elif ((RBUF_SIZE & (RBUF_SIZE-1)) != 0)
  #error RBUF_SIZE must be a power of 2.
#endif


//------------------------------------------------------------------------------
// Circular buffers
//------------------------------------------------------------------------------
struct buf_rx {
  unsigned int idxMarkerInput;          // Next In Index
  unsigned int idxMarkerOutput;         // Next Out Index 
  unsigned int MarkerCount;
  char buf [RBUF_SIZE];     // Buffer
};
static struct buf_rx rbuf = { 0, 0, 0};

//------------------------------------------------------------------------------
// Use SYNCBUSY register as the LE clock is async to coreclock
//------------------------------------------------------------------------------
__STATIC_INLINE void LEUART_Sync(unsigned int mask) {
  // Avoid deadlock if modifying the same register twice when freeze mode is activated.
  if (LEUART0->FREEZE & LEUART_FREEZE_REGFREEZE)
    return;
  
  // Wait for any pending previous write operation to have been completed in low frequency domain
  while (LEUART0->SYNCBUSY & mask);
}

//------------------------------------------------------------------------------
// Interrupt handler for LEUART
//------------------------------------------------------------------------------
void LEUART0_IRQHandler(void) {
  volatile unsigned char ch;
  
  // Check if RX data available
  if (LEUART0->STATUS & LEUART_STATUS_RXDATAV) {
    if (rbuf.MarkerCount >= RBUF_SIZE) {
      ch = LEUART0->RXDATA;
    } else {
      rbuf.buf[rbuf.idxMarkerInput] = LEUART0->RXDATA;
      rbuf.idxMarkerInput=(rbuf.idxMarkerInput+1) % RBUF_SIZE;
      rbuf.MarkerCount++;

      // At 80% usage of buffer, remove RTS to GSM module
      if (rbuf.MarkerCount> ((8*RBUF_SIZE)/10)) {
        GPIO->P[4].DOUTSET = (1<<12); // GSM_RTS, remove rts to gsm (GSM can send up to 264 bytes more however !)
      }
    }
  }
}


//------------------------------------------------------------------------------
// Return true if receiver is enabled
//------------------------------------------------------------------------------
int isEnableLEUART0(void) {
  return isEnabled;
}

void enableLEUART0(unsigned int enable) {
  if (enable) {
    LEUART0->IEN |= (LEUART_IEN_RXDATAV);
    NVIC_EnableIRQ(LEUART0_IRQn);
    NVIC_SetPriority(LEUART0_IRQn, 0);

  } else {
    LEUART0->IEN &= ~(LEUART_IEN_RXDATAV);
    NVIC_DisableIRQ(LEUART0_IRQn);
  }
  isEnabled = enable;
}

//------------------------------------------------------------------------------
// Set baudrate
//------------------------------------------------------------------------------
void setbaudLEUART0(unsigned int baudrate) {
  // If baudrate is 0, go set the previous one (using the possibly new HFCoreclk)
  if (baudrate==0)
    baudrate=currentBaudrate;
  currentBaudrate=baudrate;


  LEUART_Sync(LEUART_SYNCBUSY_CLKDIV);
  // Set prescaler for LEUART clock to "div by 4"
  CMU->LFBPRESC0 |= CMU_LFBPRESC0_LEUART0_DIV4;
  // The coreclock  is divided with 2x4 before hitting the LEUART
  LEUART0->CLKDIV = 256*(SystemCoreClockGet()/8/(baudrate)-1);
}

//------------------------------------------------------------------------------
// Send a single character to the circualr TX buffer
//------------------------------------------------------------------------------
int putcharLEUART0(char c) {
  while (!(LEUART0->STATUS & LEUART_STATUS_TXBL));
  LEUART_Sync(LEUART_SYNCBUSY_TXDATA);
  LEUART0->TXDATA = c;
  return 0;
}

//------------------------------------------------------------------------------
// Send a zero terminated string to UART
//------------------------------------------------------------------------------
void putstringLEUART0(char *buf) {
  while (*buf) {
    while (!(LEUART0->STATUS & LEUART_STATUS_TXBL));
    LEUART_Sync(LEUART_SYNCBUSY_TXDATA);
    LEUART0->TXDATA = *(buf++);
  }
}


//------------------------------------------------------------------------------
// Get a single character from UART
// Returns 1 if no available, 0 if character is read
//------------------------------------------------------------------------------
int getcharLEUART0(char *ch) {

  // If no data available, just return
  if (rbuf.MarkerCount == 0) {
    return 1;
  }
  
  // Disable RX interrupt
  enableLEUART0(FALSE);
  // Extract character from circular buffer
  *ch=rbuf.buf[rbuf.idxMarkerOutput];
  rbuf.idxMarkerOutput=(rbuf.idxMarkerOutput+1) % RBUF_SIZE;
  rbuf.MarkerCount--;
  // Enable RX interrupt
  enableLEUART0(TRUE);

  // At 50% usage of buffer, assert RTS to GSM module
  if (rbuf.MarkerCount < ((5*RBUF_SIZE)/10)) {
    GPIO->P[4].DOUTCLR = (1<<12); // GSM_RTS, assert rts to gsm
  }
  
  return 0;
}

//------------------------------------------------------------------------------
// Init LEUART 
//------------------------------------------------------------------------------
void initLEUART0(unsigned int baudrate) {

  // Set prescaler for LEUART clock to "div by 4"
  CMU->LFBPRESC0 |= CMU_LFBPRESC0_LEUART0_DIV4;
  // Set LFB to HF Coreclock/2
  CMU->LFCLKSEL |= CMU_LFCLKSEL_LFB_HFCORECLKLEDIV2;
  // Enable clock to LEUART
  CMU->LFBCLKEN0 |= CMU_LFBCLKEN0_LEUART0;

  // To avoid false start, configure output LEU0_TX as high on PE14
  GPIO->P[4].DOUT |= (1 << 14);
  // Pin PE14 is configured to Push-pull
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE14_MASK) | GPIO_P_MODEH_MODE14_PUSHPULL;
  // Pin PE15 is configured to Input enabled
  GPIO->P[4].MODEH = (GPIO->P[4].MODEH & ~_GPIO_P_MODEH_MODE15_MASK) | GPIO_P_MODEH_MODE15_INPUT;
  
  // Module LEUART is configured to location 2
  LEUART0->ROUTE = (LEUART0->ROUTE & ~_LEUART_ROUTE_LOCATION_MASK) | LEUART_ROUTE_LOCATION_LOC2;
  // Enable signals TX, RX
  LEUART0->ROUTE |= LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN;
 
  setbaudLEUART0(baudrate);
  // Clear RX and TX
  LEUART_Sync(LEUART_SYNCBUSY_CMD);
  LEUART0->CMD = LEUART_CMD_CLEARRX + LEUART_CMD_CLEARTX;
  // Get transmitter roling
  //no parity, 8 databits, one stopbit
  LEUART0->CTRL = 0; 
  // Disable all interrupts
  LEUART0->IEN = 0;
  // Enable transmitter and receiver
  LEUART_Sync(LEUART_SYNCBUSY_CMD);
  LEUART0->CMD = LEUART_CMD_TXEN + LEUART_CMD_RXEN;

// Disable receive interrupt
  enableLEUART0(FALSE);
  
}
