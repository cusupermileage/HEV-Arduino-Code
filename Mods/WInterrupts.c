/* From TEENSY */

/* Interrupt functions for the Teensy and Teensy++
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2008-2010 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#include "wiring.h"
#include "wiring_private.h"

*/



/* ORIGINAL: */ 

/* -*- mode: jde; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the Wiring project - http://wiring.uniandes.edu.co

  Copyright (c) 2004-05 Hernando Barragan

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
  
  Modified 24 November 2006 by David A. Mellis
  Modified 1 August 2010 by Mark Sproul
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "wiring_private.h"





//m32u4
#define NUM_INTERRUPT 4


volatile static voidFuncPtr intFunc[NUM_INTERRUPT];

static const uint8_t PROGMEM interrupt_mode_mask[] = {0xFC, 0xF3, 0xCF, 0x3F};

static uint8_t pin2int(uint8_t pin)
{
	switch (pin) {
		case CORE_INT0_PIN: return 0;
		case CORE_INT1_PIN: return 1;
		case CORE_INT2_PIN: return 2;
		case CORE_INT3_PIN: return 3;
		default: return 255;
	}
}

// m32u4
void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
	uint8_t mask;

	if (interruptNum >= NUM_INTERRUPT) {
		interruptNum = pin2int(interruptNum);
		if (interruptNum >= NUM_INTERRUPT) return;
	}
	intFunc[interruptNum] = userFunc;
	mask = pgm_read_byte(interrupt_mode_mask + interruptNum);
	mode &= 0x03;
	EICRA = (EICRA & mask) | (mode << (interruptNum * 2));
	EIMSK |= (1 << interruptNum);
}

// m32u4
void detachInterrupt(uint8_t interruptNum)
{
	if (interruptNum >= NUM_INTERRUPT) {
		interruptNum = pin2int(interruptNum);
		if (interruptNum >= NUM_INTERRUPT) return;
	}
	EIMSK &= ~(1 << interruptNum);
	intFunc[interruptNum] = 0;
}
SIGNAL(INT0_vect) {
	if (intFunc[0]) intFunc[0]();	// INT0 is pin 0 (PD0)
}
SIGNAL(INT1_vect) {
	if (intFunc[1]) intFunc[1]();	// INT1 is pin 1 (PD1)
}
SIGNAL(INT2_vect) {
	if (intFunc[2]) intFunc[2]();	// INT2 is pin 2 (PD2) (also Serial RX)
}
SIGNAL(INT3_vect) {
	if (intFunc[3]) intFunc[3]();	// INT3 is pin 3 (PD3) (also Serial TX)
}



















/*
volatile static voidFuncPtr intFunc[EXTERNAL_NUM_INTERRUPTS];
// volatile static voidFuncPtr twiIntFunc;

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
  if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
    intFunc[interruptNum] = userFunc;
    
    // Configure the interrupt mode (trigger on low input, any change, rising
    // edge, or falling edge).  The mode constants were chosen to correspond
    // to the configuration bits in the hardware register, so we simply shift
    // the mode into place.
      
    // Enable the interrupt.
      
    switch (interruptNum) {
#if defined(EICRA) && defined(EICRB) && defined(EIMSK)
    case 2:
      EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      EIMSK |= (1 << INT0);
      break;
    case 3:
      EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      EIMSK |= (1 << INT1);
      break;
    case 4:
      EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (mode << ISC20);
      EIMSK |= (1 << INT2);
      break;
    case 5:
      EICRA = (EICRA & ~((1 << ISC30) | (1 << ISC31))) | (mode << ISC30);
      EIMSK |= (1 << INT3);
      break;
    case 0:
      EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41))) | (mode << ISC40);
      EIMSK |= (1 << INT4);
      break;
    case 1:
      EICRB = (EICRB & ~((1 << ISC50) | (1 << ISC51))) | (mode << ISC50);
      EIMSK |= (1 << INT5);
      break;
    case 6:
      EICRB = (EICRB & ~((1 << ISC60) | (1 << ISC61))) | (mode << ISC60);
      EIMSK |= (1 << INT6);
      break;
    case 7:
      EICRB = (EICRB & ~((1 << ISC70) | (1 << ISC71))) | (mode << ISC70);
      EIMSK |= (1 << INT7);
      break;
#else
    case 0:
    #if defined(EICRA) && defined(ISC00) && defined(EIMSK)
      EICRA = (EICRA & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      EIMSK |= (1 << INT0);
    #elif defined(MCUCR) && defined(ISC00) && defined(GICR)
      MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      GICR |= (1 << INT0);
    #elif defined(MCUCR) && defined(ISC00) && defined(GIMSK)
      MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
      GIMSK |= (1 << INT0);
    #else
      #error attachInterrupt not finished for this CPU (case 0)
    #endif
      break;

    case 1:
    #if defined(EICRA) && defined(ISC10) && defined(ISC11) && defined(EIMSK)
      EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      EIMSK |= (1 << INT1);
    #elif defined(MCUCR) && defined(ISC10) && defined(ISC11) && defined(GICR)
      MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      GICR |= (1 << INT1);
    #elif defined(MCUCR) && defined(ISC10) && defined(GIMSK) && defined(GIMSK)
      MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
      GIMSK |= (1 << INT1);
    #else
      #warning attachInterrupt may need some more work for this cpu (case 1)
    #endif
      break;
#endif
    }
  }
}

void detachInterrupt(uint8_t interruptNum) {
  if(interruptNum < EXTERNAL_NUM_INTERRUPTS) {
    // Disable the interrupt.  (We can't assume that interruptNum is equal
    // to the number of the EIMSK bit to clear, as this isn't true on the 
    // ATmega8.  There, INT0 is 6 and INT1 is 7.)
    switch (interruptNum) {
#if defined(EICRA) && defined(EICRB) && defined(EIMSK)
    case 2:
      EIMSK &= ~(1 << INT0);
      break;
    case 3:
      EIMSK &= ~(1 << INT1);
      break;
    case 4:
      EIMSK &= ~(1 << INT2);
      break;
    case 5:
      EIMSK &= ~(1 << INT3);
      break;
    case 0:
      EIMSK &= ~(1 << INT4);
      break;
    case 1:
      EIMSK &= ~(1 << INT5);
      break;
    case 6:
      EIMSK &= ~(1 << INT6);
      break;
    case 7:
      EIMSK &= ~(1 << INT7);
      break;
#else
    case 0:
    #if defined(EIMSK) && defined(INT0)
      EIMSK &= ~(1 << INT0);
    #elif defined(GICR) && defined(ISC00)
      GICR &= ~(1 << INT0); // atmega32
    #elif defined(GIMSK) && defined(INT0)
      GIMSK &= ~(1 << INT0);
    #else
      #error detachInterrupt not finished for this cpu
    #endif
      break;

    case 1:
    #if defined(EIMSK) && defined(INT1)
      EIMSK &= ~(1 << INT1);
    #elif defined(GICR) && defined(INT1)
      GICR &= ~(1 << INT1); // atmega32
    #elif defined(GIMSK) && defined(INT1)
      GIMSK &= ~(1 << INT1);
    #else
      #warning detachInterrupt may need some more work for this cpu (case 1)
    #endif
      break;
#endif
    }
      
    intFunc[interruptNum] = 0;
  }
}
*/
/*
void attachInterruptTwi(void (*userFunc)(void) ) {
  twiIntFunc = userFunc;
}
*/




/*
#if defined(EICRA) && defined(EICRB)

SIGNAL(INT0_vect) {
  if(intFunc[EXTERNAL_INT_2])
    intFunc[EXTERNAL_INT_2]();
}

SIGNAL(INT1_vect) {
  if(intFunc[EXTERNAL_INT_3])
    intFunc[EXTERNAL_INT_3]();
}

SIGNAL(INT2_vect) {
  if(intFunc[EXTERNAL_INT_4])
    intFunc[EXTERNAL_INT_4]();
}

SIGNAL(INT3_vect) {
  if(intFunc[EXTERNAL_INT_5])
    intFunc[EXTERNAL_INT_5]();
}

SIGNAL(INT4_vect) {
  if(intFunc[EXTERNAL_INT_0])
    intFunc[EXTERNAL_INT_0]();
}

SIGNAL(INT5_vect) {
  if(intFunc[EXTERNAL_INT_1])
    intFunc[EXTERNAL_INT_1]();
}

SIGNAL(INT6_vect) {
  if(intFunc[EXTERNAL_INT_6])
    intFunc[EXTERNAL_INT_6]();
}

SIGNAL(INT7_vect) {
  if(intFunc[EXTERNAL_INT_7])
    intFunc[EXTERNAL_INT_7]();
}

#else

SIGNAL(INT0_vect) {
  if(intFunc[EXTERNAL_INT_0])
    intFunc[EXTERNAL_INT_0]();
}

SIGNAL(INT1_vect) {
  if(intFunc[EXTERNAL_INT_1])
    intFunc[EXTERNAL_INT_1]();
}

#endif
*/


/*
SIGNAL(SIG_2WIRE_SERIAL) {
  if(twiIntFunc)
    twiIntFunc();
}
*/

