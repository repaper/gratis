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
#include "spi.h"
#include "epd.h"

#include "epd_io.h"


// test images
#include "aphrodite_2_0.xbm"
#include "cat_2_0.xbm"
#include "saturn_2_0.xbm"
#include "text_hello_2_0.xbm"
#include "text_image_2_0.xbm"
#include "venus_2_0.xbm"


static uint8_t *images[] = {
	aphrodite_2_0_bits,
	cat_2_0_bits,
	saturn_2_0_bits,
	text_hello_2_0_bits,
	text_image_2_0_bits,
	venus_2_0_bits
};

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof((a)[0]))


int main(int argc, char *argv[]) {

	int rc = 0;

	if (!GPIO_setup()) {
		rc = 1;
		warn("GPIO_setup failed");
		goto done;
	}

	SPI_type *spi = SPI_create(SPI_DEVICE, SPI_BPS);
	if (NULL == spi) {
		rc = 1;
		warn("SPI_setup failed");
		goto done_gpio;
	}

	GPIO_mode(panel_on_pin, GPIO_OUTPUT);
	GPIO_mode(border_pin, GPIO_OUTPUT);
	GPIO_mode(discharge_pin, GPIO_OUTPUT);
	GPIO_mode(pwm_pin, GPIO_PWM);
	GPIO_mode(reset_pin, GPIO_OUTPUT);
	GPIO_mode(busy_pin, GPIO_INPUT);

	EPD_type *epd = EPD_create(EPD_2_0,
				   panel_on_pin,
				   border_pin,
				   discharge_pin,
				   pwm_pin,
				   reset_pin,
				   busy_pin,
				   spi);

	if (NULL == epd) {
		rc = 1;
		warn("EPD_setup failed");
		goto done_spi;
	}


	// EPD display
	EPD_begin(epd);
	EPD_clear(epd);
	EPD_end(epd);

	for (int i = 0; i < SIZE_OF_ARRAY(images); ++i) {
		EPD_begin(epd);
		if (0 == i) {
			EPD_image_0(epd, images[i]);
		} else {
			EPD_image(epd, images[i - 1], images[i]);
		}
		EPD_end(epd);
		sleep(5);
	}

	// release resources
//done_epd:
	EPD_destroy(epd);
done_spi:
	SPI_destroy(spi);
done_gpio:
	GPIO_teardown();
done:
	return rc;
}
