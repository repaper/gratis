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

#if !defined(EPD_GFX_H)
#define EPD_GFX_H 1

// assumes the correct EPD_Vvvv_Gg.h has already been included

#include <Arduino.h>
#include <Adafruit_GFX.h>


class EPD_GFX : public Adafruit_GFX {

private:
	EPD_Class &EPD;
	S5813A_Class &S5813A;

	// FIXME: Make width/height parameters
	// FIXME: Have to change these constants to suit panel (128 x 96, 200 x 96, 264 x 176)
	static const int pixel_width = EPD_PIXEL_WIDTH;  // must be a multiple of 8
	static const int pixel_height = EPD_PIXEL_HEIGHT;

#if EPD_IMAGE_TWO_ARG
	uint8_t old_image[(uint32_t)(pixel_width) * (uint32_t)(pixel_height) / 8];
#endif
	uint8_t new_image[(uint32_t)(pixel_width) * (uint32_t)(pixel_height) / 8];

	EPD_GFX(EPD_Class&);  // disable copy constructor

public:

	enum {
		WHITE = 0,
		BLACK = 1
	};

	// constructor
	EPD_GFX(EPD_Class &epd, S5813A_Class &s5813a) :
	Adafruit_GFX(this->pixel_width, this->pixel_height),
		EPD(epd), S5813A(s5813a) {
	}

	void begin(void) {
		int temperature = this->S5813A.read();

		// erase display
		this->EPD.begin();
		this->EPD.setFactor(temperature);
		this->EPD.clear();
		this->EPD.end();

		// clear buffers to white
#if EPD_IMAGE_TWO_ARG
		memset(this->old_image, 0, sizeof(this->old_image));
#endif
		memset(this->new_image, 0, sizeof(this->new_image));
	}

	void end(void){
	}

	// set a single pixel in new_image
	void drawPixel(int16_t x, int16_t y, uint16_t colour) {
		if (x < 0 || x >= this->pixel_width || y < 0 || y >= this->pixel_height) {
			return; // avoid buffer overwrite
		}
		int bit = x & 0x07;
		int byte = x / 8 + y * (pixel_width / 8);
		int mask = 0x01 << bit;
		if (BLACK == colour) {
			this->new_image[byte] |= mask;
		} else {
			this->new_image[byte] &= ~mask;
		}
	}

	// refresh the display: change from current image to new image
	void display(void) {

		int temperature = this->S5813A.read();

		// erase old, display new
		this->EPD.begin();
		this->EPD.setFactor(temperature);

#if defined(EPD_ENABLE_EXTRA_SRAM)

#if EPD_IMAGE_ONE_ARG
		this->EPD.image_sram(this->image);
#elif EPD_IMAGE_TWO_ARG
		this->EPD.image_sram(this->old_image, this->new_image);
#else
#error "unsupported image function"
#endif

#else
#error EPD_GFX - Needs more RAM: EPD_ENABLE_EXTRA_SRAM
#endif
		this->EPD.end();

#if EPD_IMAGE_TWO_ARG
		// copy new over to old
		memcpy(this->old_image, this->new_image, sizeof(this->old_image));
#endif
	}
};


#endif
