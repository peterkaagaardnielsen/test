#ifndef _SPI2_
#define _SPI2_

//-----------------------------------------------------------------------------
// lockSPI2()
//   Gain exclusive access to the SPI2 module
//-----------------------------------------------------------------------------
void lockSPI2(void);

//-----------------------------------------------------------------------------
// unlockSPI2()
//   Release exclusive access to the SPI2 module
//-----------------------------------------------------------------------------
void unlockSPI2(void);

//------------------------------------------------------------------------------
// sendCharSPI2()
// Send a single byte to the SPI port
//------------------------------------------------------------------------------
unsigned char sendCharSPI2(unsigned char ch);

//------------------------------------------------------------------------------
// sendSPI2()
// Send a buffer to the SPI port
// Input: buf, data to send
//        length, number of bytes to send
//------------------------------------------------------------------------------
void sendSPI2(unsigned char *buf, unsigned int length);

//------------------------------------------------------------------------------
// receiveCharSPI2()
// Receive one byte from SPI port
//------------------------------------------------------------------------------
unsigned char receiveCharSPI2(void);

//------------------------------------------------------------------------------
// receiveSPI2()
// Receives a number of bytes from the SPI port
// Input: buf, buffer where data will be placed in
//        length, number of bytes to receive
//------------------------------------------------------------------------------
void receiveSPI2(unsigned char *buf, unsigned int length);

//-----------------------------------------------------------------------------
// flushSPI2()
//   Flush the RX FIFO
//-----------------------------------------------------------------------------
void flushSPI2(void);

//------------------------------------------------------------------------------
// setCSSPI2()
// Set CS (SSET) pin of the SPI port
// Input: hi, new state of the CS pin
//------------------------------------------------------------------------------
void setCSSPI2(unsigned char hi);

//-----------------------------------------------------------------------------
// initSPI2()
// Initializes the SPI2 port 
//-----------------------------------------------------------------------------
void initSPI2(void);

#endif
