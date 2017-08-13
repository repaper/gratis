// Copyright 2013-2015 Pervasive Displays, Inc.
// Copyright 2017 Wolfgang Astleitner (mrwastl@users.sourceforge.net)
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

#if !defined(EPD_PINOUT_ESP32_H)
#define EPD_PINOUT_ESP32_H 1

// ESP32 IO layout
const int Pin_TEMPERATURE  = A6 /* A6 == 34 */;
const int Pin_PANEL_ON     = 21;
const int Pin_BORDER       = 27;
const int Pin_DISCHARGE    = 22;
const int Pin_RESET        = 25;
const int Pin_BUSY         = 33;
const int Pin_EPD_CS       = 32;
const int Pin_EPD_FLASH_CS = 17;

// PWM: for V110 only
const int Pin_PWM          = 26;
const int Pin_PWM_Channel  = 0;

// customisable SPI
const int Pin_SPI_MISO     = 12;
const int Pin_SPI_MOSI     = 13;
const int Pin_SPI_SCK      = 14;
const int Pin_SPI_Channel  = 2;
// set SPI frequency to 40 MHz (max. freq. for COG driver)
// will be reduced to 20 MHz because of SPI_CLOCK_DIV2 (see SPI_on())
const uint32_t ESP32_SPI_Frequency = 40000000;


// the following are defined for completeness but not used or required
//   for driving the display
const int Pin_SW2 = 2;
const int Pin_RED_LED = 4;

// SPI object for both EPD_FLASH and EPD_V???_G?-classes
static SPIClass esp32_spi(Pin_SPI_Channel);

#endif
