// Copyright 2013-2014 Pervasive Displays, Inc.
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "gpio.h"
#include "spi.h"
#if EPD_COG_VERSION == 1
#include "epd_v1.h"
#elif EPD_COG_VERSION == 2
#include "epd_v2.h"
#else
#error "unsupported COG version"
#endif

#include EPD_IO

// 1.44" test images
#include "aphrodite_1_44.xbm"
#include "cat_1_44.xbm"
#include "saturn_1_44.xbm"
#include "text_hello_1_44.xbm"
#include "text_image_1_44.xbm"
#include "venus_1_44.xbm"

// 2.0" test images
#include "aphrodite_2_0.xbm"
#include "cat_2_0.xbm"
#include "saturn_2_0.xbm"
#include "text_hello_2_0.xbm"
#include "text_image_2_0.xbm"
#include "venus_2_0.xbm"

// 2.7" test images
#include "aphrodite_2_7.xbm"
#include "cat_2_7.xbm"
#include "saturn_2_7.xbm"
#include "text_hello_2_7.xbm"
#include "text_image_2_7.xbm"
#include "venus_2_7.xbm"


static const uint8_t *images_1_44[] = {
	aphrodite_1_44_bits,
	cat_1_44_bits,
	saturn_1_44_bits,
	text_hello_1_44_bits,
	text_image_1_44_bits,
	venus_1_44_bits
};

static const uint8_t *images_2_0[] = {
	aphrodite_2_0_bits,
	cat_2_0_bits,
	saturn_2_0_bits,
	text_hello_2_0_bits,
	text_image_2_0_bits,
	venus_2_0_bits
};

static const uint8_t *images_2_7[] = {
	aphrodite_2_7_bits,
	cat_2_7_bits,
	saturn_2_7_bits,
	text_hello_2_7_bits,
	text_image_2_7_bits,
	venus_2_7_bits
};

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof((a)[0]))

// print usage message and exit
static void usage(const char *program_name, const char *message, ...) {

	if (NULL != message) {
		va_list ap;
		va_start(ap, message);
		printf("error: ");
		vprintf(message, ap);
		printf("\n");
		va_end(ap);
	}
	if (NULL != program_name) {
		program_name = "epd_test";
	}

	printf("usage: %s [ 1.44 | 2.0 | 2.7 ]\n", program_name);
	exit(1);
}

// the main test program
int main(int argc, char *argv[]) {

	int rc = 0;
	EPD_size display_size = EPD_1_44;
	const uint8_t *const *images = images_1_44;
	int image_count = SIZE_OF_ARRAY(images_1_44);

	if (argc < 2) {
		usage(argv[0], "missing argument");
	} else if (argc > 2) {
		usage(argv[0], "extraneous extra arguments");
	}

	if (0 == strcmp("1.44", argv[1]) || 0 == strcmp("1_44", argv[1])) {
		display_size = EPD_1_44;
		images = images_1_44;
		image_count = SIZE_OF_ARRAY(images_1_44);
	} else if (0 == strcmp("2.0", argv[1]) || 0 == strcmp("2_0", argv[1])) {
		display_size = EPD_2_0;
		images = images_2_0;
		image_count = SIZE_OF_ARRAY(images_2_0);
	} else if (0 == strcmp("2.7", argv[1]) || 0 == strcmp("2_7", argv[1])) {
		display_size = EPD_2_7;
		images = images_2_7;
		image_count = SIZE_OF_ARRAY(images_2_7);
	} else {
		usage(argv[0], "unknown display size: %s", argv[1]);
	}

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
#if  EPD_COG_VERSION == 1
	GPIO_mode(pwm_pin, GPIO_PWM);
#endif
	GPIO_mode(reset_pin, GPIO_OUTPUT);
	GPIO_mode(busy_pin, GPIO_INPUT);

	EPD_type *epd = EPD_create(display_size,
				   panel_on_pin,
				   border_pin,
				   discharge_pin,
#if  EPD_COG_VERSION == 1
				   pwm_pin,
#endif
				   reset_pin,
				   busy_pin,
				   spi);

	if (NULL == epd) {
		rc = 1;
		warn("EPD_setup failed");
		goto done_spi;
	}

#if 0
	// EPD display
	printf("clear display\n");
	EPD_begin(epd);
	EPD_clear(epd);
	EPD_end(epd);
#endif

	printf("images start\n");
	for (int i = 0; i < image_count; ++i) {
		printf("image = %d\n", i);
		EPD_begin(epd);
#if EPD_COG_VERSION == 1
		if (0 == i) {
			EPD_image_0(epd, images[i]);
		} else {
			EPD_image(epd, images[i - 1], images[i]);
		}
#else
		EPD_image(epd, images[i]);
#endif
		EPD_end(epd);
		if (i < image_count - 1) {
			sleep(5);
		}
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
