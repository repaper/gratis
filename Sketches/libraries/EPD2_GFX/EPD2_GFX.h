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

#if !defined(EPD2_GFX_H)
#define EPD2_GFX_H 1

#include <Arduino.h>
#include <EPD2.h>
#include <S5813A.h>
#include <Adafruit_GFX.h>


class EPD_GFX : public Adafruit_GFX {

private:
	EPD_Class &EPD;
	S5813A_Class &S5813A;

	// FIXME: Make width/height parameters
	// FIXME: Have to change these constants to suit panel (128 x 96, 200 x 96, 264 x 176)
	static const int pixel_width = 200;  // must be a multiple of 8
	static const int pixel_height = 96;

	uint8_t image[pixel_width * pixel_height / 8];

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

	void begin(void);
	void end(void);

	// set a single pixel in new_image
	void drawPixel(int16_t x, int16_t y, uint16_t colour) {
		int bit = x & 0x07;
		int byte = x / 8 + y * (pixel_width / 8);
		int mask = 0x01 << bit;
		if (BLACK == colour) {
			this->image[byte] |= mask;
		} else {
			this->image[byte] &= ~mask;
		}
	}

	// update panel
	void display(void);
};


#endif
