// -*- mode: c++ -*-
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

// Updated 2015-08-01 by Rei Vilo
// . Added #include Energia
// . For Energia, changed pin names to pin numbers (see comment below)
// . Works on MSP430F5529, LM4F120, TM4C123
// . Fails on MSP432 and CC3200

// {% SYSTEM:notice %}


// Simple demo to toggle EPD between two images.

// Operation from reset:
// * display version
// * display compiled-in display setting
// * display EPD FLASH detected or not
// * display temperature (displayed before every image is changed)
// * clear screen
// * delay 5 seconds (flash LED)
// * display text image
// * delay 5 seconds (flash LED)
// * display picture
// * delay 5 seconds (flash LED)
// * back to text display


#if defined(ENERGIA)
#   include "Energia.h"
#else
#   include <Arduino.h>
#endif

#include <inttypes.h>
#include <ctype.h>

// required libraries
#include <SPI.h>
#include <EPD_FLASH.h>
#include <{% DRIVER:header %}>
#define SCREEN_SIZE {% PANEL:size %}
#include <EPD_PANELS.h>
#include <S5813A.h>

// select two images from:  text_image text-hello cat aphrodite venus saturn
#define IMAGE_1  text_image
#define IMAGE_2  cat

// Error message for MSP430
#if (SCREEN_SIZE == 270) && defined(__MSP430G2553__)
#error MSP430: not enough memory
#endif

// no futher changed below this point

// current version number
#define DEMO_VERSION "4"


// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)

// other pre-processor magic
// token joining and computing the string for #include
#define ID(X) X
#define MAKE_NAME1(X,Y) ID(X##Y)
#define MAKE_NAME(X,Y) MAKE_NAME1(X,Y)
#define MAKE_JOIN(X,Y) MAKE_STRING(MAKE_NAME(X,Y))

// calculate the include name and variable names
#define IMAGE_1_FILE MAKE_JOIN(IMAGE_1,EPD_IMAGE_FILE_SUFFIX)
#define IMAGE_1_BITS MAKE_NAME(IMAGE_1,EPD_IMAGE_NAME_SUFFIX)
#define IMAGE_2_FILE MAKE_JOIN(IMAGE_2,EPD_IMAGE_FILE_SUFFIX)
#define IMAGE_2_BITS MAKE_NAME(IMAGE_2,EPD_IMAGE_NAME_SUFFIX)


// Add Images library to compiler path
#include <Images.h>  // this is just an empty file

// images
PROGMEM const
#define unsigned
#define char uint8_t
#include IMAGE_1_FILE
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include IMAGE_2_FILE
#undef char
#undef unsigned


#if defined(ENERGIA)

// For Energia, use pin numbers instead of pin names.
// * pin names are going to be deprecated
// * pin numbers ensure consistency across all LaunchPads

const int Pin_TEMPERATURE   = 6;  //  A10 = MSP432,   A6 = F5529, MSP430 = A4;
const int Pin_PANEL_ON      = 11; // P3_6 = MSP432, P8_1 = F5529, MSP430 = P2_3;
const int Pin_BORDER        = 13; // P5_0 = MSP432, P2_6 = F5529, MSP430 = P2_5;
const int Pin_DISCHARGE     = 12; // P5_2 = MSP432, P2_3 = F5529, MSP430 = P2_4;

const int Pin_RESET         = 10; // P6_4 = MSP432, P4_1 = F5529, MSP430 = P2_2;
const int Pin_BUSY          = 8;  // P4_6 = MSP432, P2_7 = F5529, MSP430 = P2_0;
const int Pin_EPD_CS        = 19; // P2_5 = MSP432, P2_0 = F5529, MSP430 = P2_6;
const int Pin_EPD_FLASH_CS  = 18; // P3_0 = MSP432, P2_2 = F5529, MSP430 = P2_7;

const int Pin_SW2           = PUSH2; // PUSH2 = MSP432, PUSH2 = F5529, MSP430 = PUSH2;
const int Pin_RED_LED       = RED_LED; // RED_LED = MSP432, RED_LED = F5529, MSP430 = RED_LED;

#   if defined(__MSP430G2553__)
// MSP430G2553 LaunchPad IO layout

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#   elif defined(__MSP430F5529__)

// __MSP430F5529__
// MSP430F5529 LaunchPad IO layout
// Pin_PWM requires a special macro P4_2_PWM
//
#if EPD_PWM_REQUIRED
// Simulated PWM on port P4_2 linked to port P7_4
// See http://forum.43oh.com/topic/4372-solved-help-software-pwm-for-the-msp430f5529/?p=39372
// Posted 17 September 2013 - 09:53 PM by energia
//
#define PM_P42_SET_MODE ((PM_TB0CCR2A << 8) | PORT_SELECTION0 | OUTPUT)
#define P4_2_PWM P7_4
const int Pin_PWM           = P4_2_PWM; // = F5529, MSP430 = P2_1;
#endif

#   elif defined(__LM4F120H5QR__)

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#   elif defined(__CC3200R1M1RGC__)

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#   elif defined(__MSP432P401R__)
// MSP432P401R LaunchPad IO layout
// Pin_PWM requires a dedicated timer task
//
#if EPD_PWM_REQUIRED
#error No PWM on pin 9 for MSP432
#endif

#   else
#       error LaunchPad for Energia not supported
#   endif

#else

// Arduino IO layout
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = 2;
const int Pin_BORDER = 3;
const int Pin_DISCHARGE = 4;
#if EPD_PWM_REQUIRED
const int Pin_PWM = 5;
#endif
const int Pin_RESET = 6;
const int Pin_BUSY = 7;
const int Pin_EPD_CS = 8;
const int Pin_EPD_FLASH_CS = 9;
const int Pin_SW2 = 12;
const int Pin_RED_LED = 13;

#endif


// LED anode through resistor to I/O pin
// LED cathode to Ground
#define LED_ON  HIGH
#define LED_OFF LOW


// define the E-Ink display
EPD_Class EPD(EPD_SIZE,
	      Pin_PANEL_ON,
	      Pin_BORDER,
	      Pin_DISCHARGE,
#if EPD_PWM_REQUIRED
	      Pin_PWM,
#endif
	      Pin_RESET,
	      Pin_BUSY,
	      Pin_EPD_CS);


// I/O setup
void setup()
{
	pinMode(Pin_RED_LED, OUTPUT);
	pinMode(Pin_SW2, INPUT);
	pinMode(Pin_TEMPERATURE, INPUT);
#if EPD_PWM_REQUIRED
	pinMode(Pin_PWM, OUTPUT);
#endif
	pinMode(Pin_BUSY, INPUT);
	pinMode(Pin_RESET, OUTPUT);
	pinMode(Pin_PANEL_ON, OUTPUT);
	pinMode(Pin_DISCHARGE, OUTPUT);
	pinMode(Pin_BORDER, OUTPUT);
	pinMode(Pin_EPD_CS, OUTPUT);
	pinMode(Pin_EPD_FLASH_CS, OUTPUT);

	digitalWrite(Pin_RED_LED, LOW);
#if EPD_PWM_REQUIRED
	digitalWrite(Pin_PWM, LOW);
#endif
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_DISCHARGE, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);
	digitalWrite(Pin_EPD_FLASH_CS, HIGH);

	Serial.begin(9600);
    delay(500);
    
#if defined(__AVR__)
	// wait for USB CDC serial port to connect.  Arduino Leonardo only
    while (!Serial);
	delay(20);  // allows terminal time to sync
#endif
    
	Serial.println();
	Serial.println();
	Serial.println("Demo version: " DEMO_VERSION);
	Serial.println("Display size: " MAKE_STRING(EPD_SIZE));
	Serial.println("Film: V" MAKE_STRING(EPD_FILM_VERSION));
	Serial.println("COG: G" MAKE_STRING(EPD_CHIP_VERSION));
    Serial.println();
    
    Serial.println("Image 1: " IMAGE_1_FILE);
    Serial.println("Image 2: " IMAGE_2_FILE);
	Serial.println();

	EPD_FLASH.begin(Pin_EPD_FLASH_CS);
    if (EPD_FLASH.available())
    {
		Serial.println("EPD FLASH chip detected OK");
    }
    else
    {
		uint8_t maufacturer;
		uint16_t device;
		EPD_FLASH.info(&maufacturer, &device);
		Serial.print("unsupported EPD FLASH chip: MFG: 0x");
		Serial.print(maufacturer, HEX);
		Serial.print("  device: 0x");
		Serial.print(device, HEX);
		Serial.println();
	}

	// configure temperature sensor
	S5813A.begin(Pin_TEMPERATURE);
}


static int state = 0;


// main loop
void loop()
{
	int temperature = S5813A.read();
	Serial.print("Temperature = ");
    Serial.print(temperature, DEC);
    Serial.println(" Celsius");

	EPD.begin(); // power up the EPD panel
	EPD.setFactor(temperature); // adjust for current temperature

	int delay_counts = 50;
    switch(state)
    {
	default:
	case 0:         // clear the screen
		EPD.clear();
		state = 1;
		delay_counts = 5;  // reduce delay so first image come up quickly
		break;

	case 1:         // clear -> text
#if EPD_IMAGE_ONE_ARG
		EPD.image(IMAGE_1_BITS);
#elif EPD_IMAGE_TWO_ARG
		EPD.image_0(IMAGE_1_BITS);
#else
#error "unsupported image function"
#endif
		++state;
		break;

	case 2:         // text -> picture
#if EPD_IMAGE_ONE_ARG
		EPD.image(IMAGE_2_BITS);
#elif EPD_IMAGE_TWO_ARG
		EPD.image(IMAGE_1_BITS, IMAGE_2_BITS);
#else
#error "unsupported image function"
#endif
		++state;
		break;

	case 3:        // picture -> text
#if EPD_IMAGE_ONE_ARG
		EPD.image(IMAGE_1_BITS);
#elif EPD_IMAGE_TWO_ARG
		EPD.image(IMAGE_2_BITS, IMAGE_1_BITS);
#else
#error "unsupported image function"
#endif
		state = 2;  // back to picture next time
		break;
	}
	EPD.end();   // power down the EPD panel

	// flash LED for 5 seconds
    for (int x = 0; x < delay_counts; ++x)
    {
		digitalWrite(Pin_RED_LED, LED_ON);
		delay(50);
		digitalWrite(Pin_RED_LED, LED_OFF);
		delay(50);
	}
}
