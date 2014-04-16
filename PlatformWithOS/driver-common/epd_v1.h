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

#if !defined(EPD_V1_H)
#define EPD_V1_H 1

#include "spi.h"

typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // error codes
	EPD_OK,
} EPD_error;

typedef struct EPD_struct EPD_type;



// functions
// =========

// allocate memory
EPD_type *EPD_create(EPD_size size,
		     int panel_on_pin,
		     int border_pin,
		     int discharge_pin,
		     int pwm_pin,
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

// assuming a clear (white) screen output an image
void EPD_image_0(EPD_type *epd, const uint8_t *image);

// change from old image to new image
void EPD_image(EPD_type *epd, const uint8_t *old_image, const uint8_t *new_image);

// change from old image to new image
// only updating changed pixels
void EPD_partial_image(EPD_type *epd, const uint8_t *old_image, const uint8_t *new_image);


#endif
