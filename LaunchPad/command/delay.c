// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.


#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "io.h"
#include "delay.h"


void Delay_us(unsigned int delay) {
	TA0CTL |= TACLR;                                    // counter reset

	TA0CCR0 = delay * 2;                                // period
	TA0CCR1 = 0;
	TA0CCR2 = 0;

	TA0CCTL0 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA0CCTL1 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA0CCTL2 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled

	TA0CTL = TASSEL_2 | ID_3 | MC_1;                    // SMCLK, /8, up

	while (0 == (TA0CTL & TAIFG)) {                     // wait for end
		asm volatile ("\tnop");
	}
	TA0CTL = 0;                                         // stop the timer
}


void Delay_ms(unsigned int delay) {
	for (unsigned int i = 0; i < delay; ++i) {
		Delay_us(1000);
	}
}
