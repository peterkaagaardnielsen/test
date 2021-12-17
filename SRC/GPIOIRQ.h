//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
#ifndef GPIO_INT
#define GPIO_INT

//-----------------------------------------------------------------------------
// cb function for interrupts on configured GPIO port/pin
//-----------------------------------------------------------------------------
typedef void (*tdefCbGPIOIRQ)(unsigned int port, unsigned int bitno);

//-----------------------------------------------------------------------------
// GPIO ports identificator
// On dragonfly etc port names are "A", "B" etc, on Large cortex etc it is "0", "1" etc
//-----------------------------------------------------------------------------
typedef enum {
  PortA = 0,
  Port0 = 0,
  PortB = 1,
  Port1 = 1,
  PortC = 2,
  Port2 = 2,
  PortD = 3,
  Port3 = 3,
  PortE = 4,
  Port4 = 4,
  PortF = 5,
  Port5 = 5,
  PortUndef = 10,
} tdefPortInt;

//-----------------------------------------------------------------------------
// Install handler for interrupt on pin
//-----------------------------------------------------------------------------
void registerHandlerGPIOIRQ(tdefPortInt port, unsigned int pin, int risingEdge, int fallingEdge, tdefCbGPIOIRQ cb);

//-----------------------------------------------------------------------------
// Remove and disable handler for interrupt on pin
//-----------------------------------------------------------------------------
void deRegisterHandlerGPIOIRQ(tdefPortInt port, unsigned int pin);

//-----------------------------------------------------------------------------
// Return interrupt enable/disable for pin
//-----------------------------------------------------------------------------
int isEnableGPIOIRQ(tdefPortInt port, unsigned int pin);

//-----------------------------------------------------------------------------
// Clear any pending interrupt on a pin
//-----------------------------------------------------------------------------
void clearGPIOIRQ(tdefPortInt port, unsigned int pin);

//-----------------------------------------------------------------------------
// Return interrupt enable/disable for all pins
//-----------------------------------------------------------------------------
int getMaskGPIOIRQ(tdefPortInt port);

//-----------------------------------------------------------------------------
// Set interrupt enable/disable for all pins
//-----------------------------------------------------------------------------
int setMaskGPIOIRQ(tdefPortInt port, int mask);

//-----------------------------------------------------------------------------
// Enable interrupt for a specific pin (handler must already be installed!)
//-----------------------------------------------------------------------------
void enableGPIOIRQ(tdefPortInt port, unsigned int pin);

//-----------------------------------------------------------------------------
// Disable interrupt for pin
//-----------------------------------------------------------------------------
void disableGPIOIRQ(tdefPortInt port, unsigned int pin);

//-----------------------------------------------------------------------------
// Init GPIOIRQ module
//-----------------------------------------------------------------------------
void initGPIOInt(void);

#endif

