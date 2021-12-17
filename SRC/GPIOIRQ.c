//-----------------------------------------------------------------------------
// GPIOIRQ.c                                                       20130212 CHG
//-----------------------------------------------------------------------------
#include "efm32.h"
#include "System.h"
#include "GPIOIRQ.h"


// Handler for each GPIO interrupt pin
static tdefCbGPIOIRQ cbIRQ[16];

//-----------------------------------------------------------------------------
// Install handler for interrupt on pin
//-----------------------------------------------------------------------------
void registerHandlerGPIOIRQ(tdefPortInt port, unsigned int pin, int risingEdge, int fallingEdge, tdefCbGPIOIRQ cb) {
  U32 tmp;

  // There are two registers controlling the interrupt configuration:
  // The EXTIPSELL register controls pins 0-7 and EXTIPSELH controls
  // pins 8-15.
  if (pin < 8) {
    GPIO->EXTIPSELL = (GPIO->EXTIPSELL & ~(0xF << (4 * pin))) | (port << (4 * pin));
  } else {
    tmp             = pin - 8;
    GPIO->EXTIPSELH = (GPIO->EXTIPSELH & ~(0xF << (4 * tmp))) | (port << (4 * tmp));
  }

  // Enable/disable rising edge
  if (risingEdge)
    GPIO->EXTIRISE |= (1<<pin);
  else
    GPIO->EXTIRISE &= ~(1<<pin);
    
  // Enable/disable falling edge
  if (fallingEdge)
    GPIO->EXTIFALL |= (1<<pin);
  else
    GPIO->EXTIFALL &= ~(1<<pin);

  // Clear any pending interrupt
  GPIO->IFC = (1<<pin);

  cbIRQ[pin]=cb;
  
}


//-----------------------------------------------------------------------------
// Remove and disable handler for interrupt on pin
//-----------------------------------------------------------------------------
void deRegisterHandlerGPIOIRQ(tdefPortInt port, unsigned int pin) {
  port=port; // port not used
  // Disable interrupt
  GPIO->IEN &= ~(1<<pin);

  // Clear any pending interrupt
  GPIO->IFC = (1<<pin);

  // Kill cb handler
  cbIRQ[pin]=0; 
}


//-----------------------------------------------------------------------------
// Clear any pending interrupt on a pin
//-----------------------------------------------------------------------------
void clearGPIOIRQ(tdefPortInt port, unsigned int pin) {
  port=port; // port not used
  // Clear any pending interrupt
  GPIO->IFC = (1<<pin);
}

//-----------------------------------------------------------------------------
// Enable interrupt for a specific pin (handler must already be installed!)
//-----------------------------------------------------------------------------
void enableGPIOIRQ(tdefPortInt port, unsigned int pin) {
  port=port; // port not used
  // Clear any pending interrupt
  GPIO->IFC = (1<<pin);

  // Enable interrupt
  GPIO->IEN |= (1<<pin);
}

//-----------------------------------------------------------------------------
// Disable interrupt for pin
//-----------------------------------------------------------------------------
void disableGPIOIRQ(tdefPortInt port, unsigned int pin) {
  port=port; // port not used
  // Disable interrupt
  GPIO->IEN &= ~(1<<pin);
  // Clear any pending interrupt (if any should arrive just after disabling)
  GPIO->IFC = (1<<pin);
}

//-----------------------------------------------------------------------------
// Return interrupt enable/disable for pin
//-----------------------------------------------------------------------------
int isEnableGPIOIRQ(tdefPortInt port, unsigned int pin) {
  port=port; // port not used
  return (GPIO->IEN & (1<<pin)) ? 1:0;
}

//-----------------------------------------------------------------------------
// Return interrupt enable/disable for all pins
//-----------------------------------------------------------------------------
int getMaskGPIOIRQ(tdefPortInt port) {
  port=port; // port not used
  return GPIO->IEN;
}

//-----------------------------------------------------------------------------
// Set interrupt enable/disable for all pins
//-----------------------------------------------------------------------------
int setMaskGPIOIRQ(tdefPortInt port, int mask) {
  port=port; // port not used
  GPIO->IEN=mask;
  return 0;
}


//-----------------------------------------------------------------------------
// IRQ handler for even numbered pins
//-----------------------------------------------------------------------------
void GPIO_EVEN_IRQHandler(void) {
  int i;
  for (i=0; i<16; i++) {
    // NOTE: IF flag is set regardless if the interrupt is enabled or not!
    if ((GPIO->IF & (1<<i)) && (GPIO->IEN & (1<<i))) {
      if (cbIRQ[i])
        cbIRQ[i](PortUndef, i);
      GPIO->IFC = (1<<i);
    }
  }
}

//-----------------------------------------------------------------------------
// IRQ handler for odd numbered pins
//-----------------------------------------------------------------------------
void GPIO_ODD_IRQHandler(void) {
  int i;
  for (i=0; i<16; i++) {
    // NOTE: IF flag is set regardless if the interrupt is enabled or not!
    if ((GPIO->IF & (1<<i)) && (GPIO->IEN & (1<<i))) {
      if (cbIRQ[i])
        cbIRQ[i](PortUndef,i);
      GPIO->IFC = (1<<i);
    }
  }
}

//-----------------------------------------------------------------------------
// Init GPIOIRQ module
//-----------------------------------------------------------------------------
void initGPIOInt(void) 
{
    static U8 init = FALSE;
    
    if(init==FALSE)
    {
        init = TRUE;
        memset(cbIRQ,0,sizeof(cbIRQ));
        CMU->HFPERCLKEN0 |= (CMU_HFPERCLKEN0_GPIO);
  
        GPIO->IEN = 0x0000;
        GPIO->IFC = 0xFFFF;
        NVIC_EnableIRQ(GPIO_EVEN_IRQn);
        NVIC_EnableIRQ(GPIO_ODD_IRQn);
    }
}

