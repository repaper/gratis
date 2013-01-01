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
#include "button.h"


void Button_initialise(void) {
	// enable pull-up on button pin
	// as button pulls to ground
	P1REN = PORT1_SW2;
	P1OUT |= PORT1_SW2; // pull up
}


void Button_wait_state(int loop, bool state) {
	for (;;) {
		for (int i = 0; i < loop; ++i) {
			if (state == (0 == (P1IN & PORT1_SW2))) {
				Delay_ms(10);  // debounce
				if (state == (0 == (P1IN & PORT1_SW2))) {
					P1OUT &= ~PORT1_RED_LED;
					return;
				}
			}
			Delay_ms(20);
		}
		P1OUT ^= PORT1_RED_LED;
	}
}


void Button_wait_click(void) {
	Button_wait_state(30, true); // wait for press
	Button_wait_state(5, false); // wait for release
}
