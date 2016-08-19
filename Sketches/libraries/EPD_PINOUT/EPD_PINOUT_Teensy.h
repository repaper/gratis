// Copyright 2013-2015 Pervasive Displays, Inc.
// Copyright 2016 Wolfgang Astleitner (mrwastl@users.sourceforge.net)
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

#if !defined(EPD_PINOUT_TEENSY_H)
#define EPD_PINOUT_TEENSY_H 1

// pinout is taken from:
//   https://forum.pjrc.com/threads/25793-2-7-inch-ePaper-on-Teensy-does-not-compile-Adafruit_GFX#post53636

// Teensy 3.x IO layout
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = 2;
const int Pin_BORDER = 3;
const int Pin_DISCHARGE = 4;
const int Pin_PWM = 5;
const int Pin_RESET = 6;
const int Pin_BUSY = 7;
const int Pin_EPD_CS = 8;
const int Pin_EPD_FLASH_CS = 9;

// the following are defined for completeness but not used or required
//   for driving the display
const int Pin_SW2 = 0;
const int Pin_RED_LED = 1;

#endif
