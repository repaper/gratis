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

#if !defined(EPD_H)
#define EPD_H 1

#include "spi.h"

// compile-time #if configuration
#define EPD_CHIP_VERSION      2
#define EPD_FILM_VERSION      230
#define EPD_PWM_REQUIRED      0
#define EPD_IMAGE_ONE_ARG     1
#define EPD_IMAGE_TWO_ARG     0
#define EPD_PARTIAL_AVAILABLE 0

// display panels supported
#define EPD_1_44_SUPPORT      1
#define EPD_1_9_SUPPORT       0
#define EPD_2_0_SUPPORT       1
#define EPD_2_6_SUPPORT       0
#define EPD_2_7_SUPPORT       1


// possible panel sizes
typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // error codes
	EPD_OK,
	EPD_UNSUPPORTED_COG,
	EPD_PANEL_BROKEN,
	EPD_DC_FAILED
} EPD_error;

typedef struct EPD_struct EPD_type;


// functions
// =========

// allocate memory
EPD_type *EPD_create(EPD_size size,
		     int panel_on_pin,
		     int border_pin,
		     int discharge_pin,
		     int reset_pin,
		     int busy_pin,
		     SPI_type *spi);

// release memory
void EPD_destroy(EPD_type *epd);

// set the temperature compensation (call before begin)
void EPD_set_temperature(EPD_type *epd, int temperature);

// sequence start/end
void EPD_begin(EPD_type *epd);
void EPD_end(EPD_type *epd);

// ok/error status
EPD_error EPD_status(EPD_type *epd);

// items below must be bracketed by begin/end
// ==========================================

// clear the screen
void EPD_clear(EPD_type *epd);

// change from old image to new image
void EPD_image(EPD_type *epd, const uint8_t *mage);


#endif
