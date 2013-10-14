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


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "gpio.h"

// the platform specific I/O pins
#include "gpio_test.h"

#define ENABLE_PWM 1


int main(int argc, char *argv[]) {

	int i = 0;
	int j = 0;

	if (!GPIO_setup()) {
		err(1, "GPIO_setup failed");
	}

	GPIO_mode(LED_BLUE, GPIO_OUTPUT);
	GPIO_mode(LED_WHITE, GPIO_OUTPUT);
	GPIO_mode(LED_PWM, GPIO_PWM);

#if ENABLE_PWM
	// test with pwm
	for (j = 0; j < 20; ++j) {
		GPIO_write(LED_WHITE, 1);
		for (i = 0; i < 1024; ++i) {
			GPIO_pwm_write(LED_PWM, i);
			usleep(1000);
		}
		GPIO_write(LED_WHITE, 0);
		GPIO_write(LED_BLUE, 1);
		for (i = 1023; i >= 0; --i) {
			GPIO_pwm_write(LED_PWM, i);
			usleep(1000);
		}
		GPIO_write(LED_BLUE, 0);
	}

#else
	// simple non-pwm test
	for (i = 0; i < 200; ++i) {
		GPIO_write(LED_WHITE, 1);
		GPIO_write(LED_BLUE, 0);
		usleep(200000);
		GPIO_write(LED_BLUE, 1);
		GPIO_write(LED_WHITE, 0);
		usleep(200000);
	}
#endif

	GPIO_pwm_write(LED_PWM, 0);
	GPIO_write(LED_BLUE, 0);
	GPIO_write(LED_WHITE, 0);
	GPIO_teardown();
	return 0;
}
