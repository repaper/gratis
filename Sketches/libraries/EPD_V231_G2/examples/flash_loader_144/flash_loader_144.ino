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


// Notice: ***** Generated file: DO _NOT_ MODIFY, Created on: 2016-01-12 00:11:21 UTC *****


// Simple demo with two functions:
//
// 1. Copy an included iname to a specified FLSH sector and display it
// 2. Display a list of already FLASHed images
//
// Note: only one function is available at a time.

#if defined(ENERGIA)
#include <Energia.h>
#else
#include <Arduino.h>
#endif

#include <inttypes.h>
#include <ctype.h>

// required libraries
#include <SPI.h>
#include <EPD_FLASH.h>
#include <EPD_V231_G2.h>
#define SCREEN_SIZE 144
#include <EPD_PANELS.h>
#include <S5813A.h>
#include <EPD_PINOUT.h>


// Change this for different display size
// supported sizes: 144 200 270 (190 260 - V231_G2 only)
#define SCREEN_SIZE {% DRIVER:panelsize %}

// select image from:  text_image text-hello cat aphrodite venus saturn
// select a suitable sector, (size 260/270 will take two adjacent sectors)
#define IMAGE        cat
#define FLASH_SECTOR 0

// if the display list is defined it will take priority over the flashing
// (and FLASH code is disabled)
// define a list of {sector, milliseconds}
// #define DISPLAY_LIST {0, 5000}, {1, 5000}

// no futher changed below this point

// program version
#define FLASH_LOADER_VERSION "5"

// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)

// other pre-processor magic
// tiken joining and computing the string for #include
#define ID(X) X
#define MAKE_NAME1(X,Y) ID(X##Y)
#define MAKE_NAME(X,Y) MAKE_NAME1(X,Y)
#define MAKE_JOIN(X,Y) MAKE_STRING(MAKE_NAME(X,Y))

// calculate the include name and variable names
#define IMAGE_FILE MAKE_JOIN(IMAGE,EPD_IMAGE_FILE_SUFFIX)
#define IMAGE_BITS MAKE_NAME(IMAGE,EPD_IMAGE_NAME_SUFFIX)


// Add Images library to compiler path
#include <Images.h>  // this is just an empty file

// image
#if !defined(DISPLAY_LIST)
PROGMEM const
#define unsigned
#define char uint8_t
#include IMAGE_FILE
#undef char
#undef unsigned
#endif


// LED anode through resistor to I/O pin
// LED cathode to Ground
#define LED_ON  HIGH
#define LED_OFF LOW

// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)


// function prototypes
static void flash_info(void);
static void flash_read(void *buffer, uint32_t address, uint16_t length);

#if !defined(DISPLAY_LIST)
static void flash_program(uint16_t sector, const void *buffer, uint16_t length);
#endif

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
void setup() {
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
#if defined(__AVR__)
	// // indefinite wait for USB CDC serial port to connect.  Arduino Leonardo only
	// while (!Serial) {
	// }
	// additional delay for USB CDC serial port to connect.  Arduino Leonardo only
	if (!Serial) {       // allows terminal time to sync as long
		delay(500);  // as the serial monitor is opened before
	}                    // upload
#endif
	Serial.println();
	Serial.println();
	Serial.println("Flash loader version: " FLASH_LOADER_VERSION);
	Serial.println("Display size: " MAKE_STRING(EPD_SIZE));
	Serial.println("Film: V" MAKE_STRING(EPD_FILM_VERSION));
	Serial.println("COG: G" MAKE_STRING(EPD_CHIP_VERSION));
	Serial.println();

	EPD_FLASH.begin(Pin_EPD_FLASH_CS);
	flash_info();

	// configure temperature sensor
	S5813A.begin(Pin_TEMPERATURE);

	// if necessary program the flash
#if !defined(DISPLAY_LIST)
	flash_program(FLASH_SECTOR, IMAGE_BITS, sizeof(IMAGE_BITS));
#endif
}


typedef struct {
	int sector;
	int delay_ms;
} display_list_type[];

// list of {FLASH sector, milliseconds} to display
// if not defined then just display the current flash sector
static const display_list_type display_list = {
#if defined(DISPLAY_LIST)
	DISPLAY_LIST
#else
	{FLASH_SECTOR, 5000}
#endif
};

#define DISPLAY_ITEM_COUNT (sizeof(display_list)/sizeof(display_list[0]))

// state counter
static int state = 0;
static unsigned int display_index = 0;

#if EPD_IMAGE_TWO_ARG
static uint32_t old_address = 0xffffffff;
#endif

// main loop
void loop() {
	int temperature = S5813A.read();
	Serial.print("Temperature = ");
	Serial.print(temperature);
	Serial.println(" Celcius");


	int delay_counts = 5;

	EPD.begin(); // power up the EPD panel
	if (!EPD) {
		Serial.print("EPD error = ");
		Serial.print(EPD.error());
		Serial.println("");
		return;
	}

	EPD.setFactor(temperature); // adjust for current temperature

	switch(state) {
	default:
	case 0:         // clear the screen
		EPD.clear();
		state = 1;
		break;

	case 1:         // next image
		uint32_t address = display_list[display_index].sector;
		address <<= 12;
		delay_counts = display_list[display_index].delay_ms;

		Serial.print("address[");
		Serial.print(display_index);
		Serial.print("]  = 0x");
		Serial.print(address, HEX);
		Serial.print(" / ");
		Serial.print(delay_counts);
		Serial.println("ms");

#if EPD_IMAGE_ONE_ARG

		// V230_G2
		EPD.frame_cb_13(address, flash_read, EPD_inverse);
		EPD.frame_stage2();
		EPD.frame_cb_13(address, flash_read, EPD_normal);

#elif EPD_IMAGE_TWO_ARG

		// V110_G1 and V231_G2
		if (0xffffffff != old_address) {
			EPD.frame_cb_repeat(old_address, flash_read, EPD_compensate);
			EPD.frame_cb_repeat(old_address, flash_read, EPD_white);
		}
		EPD.frame_cb_repeat(address, flash_read, EPD_inverse);
		EPD.frame_cb_repeat(address, flash_read, EPD_normal);
		// preserve address for next cycle
		old_address = address;

#else
#error "unsupported image function"
#endif

		// increment list index or reset to start on overflow
		if (++display_index >= DISPLAY_ITEM_COUNT) {
			display_index = 0;
		}
		break;
	}

		EPD.end();   // power down the EPD panel
	delay(delay_counts);
}


// display information about the FLASH chip
static void flash_info(void) {
	uint8_t maufacturer;
	uint16_t device;

	if (EPD_FLASH.available()) {
		Serial.println("EPD FLASH chip detected OK");
	} else {
		Serial.println("unsupported EPD FLASH chip");
	}

	EPD_FLASH.info(&maufacturer, &device);
	Serial.print("EPD_FLASH: manufacturer = 0x");
	Serial.print(maufacturer, HEX);
	Serial.print("  device = 0x");
	Serial.print(device, HEX);
	Serial.println();
}


// EPD display callback for reading the FLASH
static void flash_read(void *buffer, uint32_t address, uint16_t length) {
	EPD_FLASH.read(buffer, address, length);
}

#if !defined(DISPLAY_LIST)
// program image into FLASH
static void flash_program(uint16_t sector, PROGMEM const void *buffer, uint16_t length) {
	Serial.print("FLASH: program sector = ");
	Serial.println(sector, DEC);
	Serial.print("       from memory @ 0x");
	Serial.print((uint32_t)buffer, HEX);
	Serial.print("  total bytes: ");
	Serial.println(length, DEC);

	uint32_t address = (uint32_t)(sector) << EPD_FLASH_SECTOR_SHIFT;

	// erase required sectors
	for (unsigned int i = 0; i < length; i += EPD_FLASH_SECTOR_SIZE, address += EPD_FLASH_SECTOR_SIZE) {
		Serial.print("FLASH: erase = 0x");
		Serial.println(address, HEX);
		EPD_FLASH.write_enable();
		EPD_FLASH.sector_erase(address);
	}

	// writable pages are EPD_FLASH_PAGE_SIZE bytes
	PROGMEM const uint8_t *p = (PROGMEM const uint8_t *)buffer;
	for (address = (uint32_t)(sector) << EPD_FLASH_SECTOR_SHIFT; length >= EPD_FLASH_PAGE_SIZE;
	     length -= EPD_FLASH_PAGE_SIZE, address += EPD_FLASH_PAGE_SIZE, p += EPD_FLASH_PAGE_SIZE) {
		Serial.print("FLASH: write @ 0x");
		Serial.print(address, HEX);
		Serial.print("  from: 0x");
		Serial.print((uint32_t)p, HEX);
		Serial.print("  bytes: 0x");
		Serial.println(EPD_FLASH_PAGE_SIZE, HEX);
		EPD_FLASH.write_enable();
		EPD_FLASH.write_from_progmem(address, p, EPD_FLASH_PAGE_SIZE);
	}
	// write any remaining partial page
	if (length > 0) {
		Serial.print("FLASH: write @ 0x");
		Serial.print(address, HEX);
		Serial.print("  from: 0x");
		Serial.print((uint32_t)p, HEX);
		Serial.print("  bytes: 0x");
		Serial.println(length, HEX);
		EPD_FLASH.write_enable();
		EPD_FLASH.write_from_progmem(address, p, length);
	}

	// turn off write - just to be safe
	EPD_FLASH.write_disable();
}
#endif
