/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>

#include <RTL.h>
#include "efm32.h"

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

/*
int fputc(int c, FILE *f) {
  // Add CR to a LF
  if(c=='\n') {
    while (!(USART0->STATUS & USART_STATUS_TXBL)) ;
    USART0->TXDATA = '\r';
  }

  while (!(USART0->STATUS & USART_STATUS_TXBL)) ;
  return (USART0->TXDATA = c);
}
*/
int fputc(int c, FILE *f) {
 
  
  // Add CR to a LF
  if(c=='\n') {
    //putcharUART0('\r');
  }
//putcharUART0(c);
  return (c);
}


int fgetc(FILE *f) {
  return -1; //(SER_GetChar());
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int c) {
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
