//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#ifndef EFM32G230_UART0_H
#define EFM32G230_UART0_H

typedef void (*uart0_callback_t) (unsigned short Len);

typedef enum EUartParity { uartParityNone, uartParityOdd, uartParityEven } EUartParity;

void initUART0(void);
void enableUART0(unsigned int enable);
void setbaudUART0(unsigned int baudrate);
void setStopBitUART0(unsigned char stopBit);
void setParityUART0(EUartParity parity);
int putcharUART0(char c);
void putstringUART0(char *buf);
void openUART0(unsigned int baudrate, EUartParity parity, unsigned char stopBit, unsigned char *buffer, unsigned short blockSize, unsigned short idleTime, uart0_callback_t callback);
void closeUART0(void);

void setCallbackUART0(unsigned char *buffer, unsigned short blockSize, unsigned short idleTime, uart0_callback_t cb);

#endif

