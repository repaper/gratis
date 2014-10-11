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


#include <Arduino.h>
#include <limits.h>

//#include <Adafruit_GFX.h>
#include <EPD2.h>
#include <S5813A.h>

#include "EPD2_GFX.h"


void EPD_GFX::begin(void) {
	int temperature = this->S5813A.read();

	// erase display
	this->EPD.begin();
	this->EPD.setFactor(temperature);
	this->EPD.clear();
	this->EPD.end();

	// clear buffers to white
	memset(this->image, 0, sizeof(this->image));
}


void EPD_GFX::display(void) {
	int temperature = this->S5813A.read();

	// erase old, display new
	this->EPD.begin();
	this->EPD.setFactor(temperature);
#if defined(EPD_ENABLE_EXTRA_SRAM)
	this->EPD.image_sram(this->image);
#else
#error EPD_GFX - Needs more RAM: EPD_ENABLE_EXTRA_SRAM
#endif
	this->EPD.end();
}
