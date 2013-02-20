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
#include "pwm.h"


//#define TIMER1_OUT0 BIT0
#define TIMER1_OUT1 BIT1
//#define TIMER1_OUT2 BIT2
#define TIMER1_IO_ENABLE (TIMER1_OUT1)

#define TIMER1_COUNTS 4000


void PWM_start(void) {
	P2DIR |= TIMER1_IO_ENABLE;                          // Timer_A3 outputs
	P2OUT &= ~TIMER1_IO_ENABLE;                         // set low (may need changing)

	P2SEL2 &= ~TIMER1_IO_ENABLE;                        // select pin function...
	P2SEL |= TIMER1_IO_ENABLE;                          // ...as timer output

	TA1CTL |= TACLR;                                    // counter reset

	TA1CTL = TASSEL_2 | ID_0 | MC_3;                    // SMCLK, /1, up/down
	TA1CCR0 = TIMER1_COUNTS;                            // period square wave on TA1.0
	TA1CCR1 = TIMER1_COUNTS / 2;                        // toggle point (subtract interlock delay) TA1.1
	TA1CCR2 = TIMER1_COUNTS / 2;                        // toggle point TA1.2

	TA1CCTL0 = OUTMOD_4 | CCIS_2 | CM_0;                // Toggle, Capture GND, Capture disabled
	TA1CCTL1 = OUTMOD_3 | CCIS_2 | CM_0;                // PWM toggle/reset, Capture GND, Capture disabled
	TA1CCTL2 = OUTMOD_6 | CCIS_2 | CM_0;                // PWM toggle/set, Capture GND, Capture disabled
}


void PWM_stop(void) {
	P2DIR |= TIMER1_IO_ENABLE;                          // Timer_A3 outputs
	P2OUT &= ~TIMER1_IO_ENABLE;                         // set low (may need changing)

	P2SEL2 &= ~TIMER1_IO_ENABLE;                        // select pin function...
	P2SEL &= ~TIMER1_IO_ENABLE;                         // ...as normal output

	TA1CTL = TACLR;                                     // counter stop
}
