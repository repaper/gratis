// Copyright 2013-2015 Pervasive Displays, Inc.
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


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "epd.h"
#include "gpio.h"

// the platform specific I/O pins
#include EPD_IO


// use redefine the EPD pins as LEDs
#define LED_1_PIN    reset_pin
#define LED_2_PIN    discharge_pin
#define LED_3_PIN    panel_on_pin
#define LED_4_PIN    border_pin

#define LED_PWM_PIN  pwm_pin



void scan_leds(void) {
	static int state = 0;
	switch (state) {
	default:
		state = 0;
	case 0:
		GPIO_write(LED_1_PIN, 1);
		GPIO_write(LED_2_PIN, 0);
		GPIO_write(LED_3_PIN, 0);
		GPIO_write(LED_4_PIN, 0);
		break;
	case 1:
		GPIO_write(LED_1_PIN, 0);
		GPIO_write(LED_2_PIN, 1);
		GPIO_write(LED_3_PIN, 0);
		GPIO_write(LED_4_PIN, 0);
		break;
	case 2:
		GPIO_write(LED_1_PIN, 0);
		GPIO_write(LED_2_PIN, 0);
		GPIO_write(LED_3_PIN, 1);
		GPIO_write(LED_4_PIN, 0);
		break;
	case 3:
		GPIO_write(LED_1_PIN, 0);
		GPIO_write(LED_2_PIN, 0);
		GPIO_write(LED_3_PIN, 0);
		GPIO_write(LED_4_PIN, 1);
		break;
	}
	++state;
}


int main(int argc, char *argv[]) {

	if (!GPIO_setup()) {
		err(1, "GPIO_setup failed");
	}

	GPIO_mode(LED_1_PIN, GPIO_OUTPUT);
	GPIO_mode(LED_2_PIN, GPIO_OUTPUT);
	GPIO_mode(LED_3_PIN, GPIO_OUTPUT);
	GPIO_mode(LED_4_PIN, GPIO_OUTPUT);
	GPIO_mode(LED_PWM_PIN, GPIO_PWM);

	GPIO_write(LED_1_PIN, 0);
	GPIO_write(LED_2_PIN, 0);
	GPIO_write(LED_3_PIN, 0);
	GPIO_write(LED_4_PIN, 0);

	GPIO_pwm_write(LED_PWM_PIN, 0);

	usleep(200000);

#if EPD_PWM_REQUIRED
	// test with pwm
	for (int j = 0; j < 20; ++j) {
		scan_leds();
		for (int i = 0; i < 1024; ++i) {
			GPIO_pwm_write(LED_PWM_PIN, i);
			usleep(500);
		}
		for (int i = 1023; i >= 0; --i) {
			GPIO_pwm_write(LED_PWM_PIN, i);
			usleep(500);
		}
	}

#else
	// simple non-pwm test
	for (int i = 0; i < 50; ++i) {
		scan_leds();
		usleep(200000);
	}
#endif

	GPIO_pwm_write(LED_PWM_PIN, 0);

	GPIO_write(LED_1_PIN, 0);
	GPIO_write(LED_2_PIN, 0);
	GPIO_write(LED_3_PIN, 0);
	GPIO_write(LED_4_PIN, 0);

	GPIO_teardown();
	return 0;
}
