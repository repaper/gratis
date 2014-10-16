// -*- mode: c++ -*-
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

// Simple demo with two functions:
//
// 1. Copy an included iname to a specified FLSH sector and display it
// 2. Display a list of already FLASHed images
//
// Note: only one function is available at a time.

#include <Arduino.h>
#include <inttypes.h>
#include <ctype.h>

// required libraries
#include <SPI.h>
#include <FLASH.h>
#include <EPD2.h>
#include <S5813A.h>


// Change this for different display size
// supported sizes: 144 200 270
#define SCREEN_SIZE 0

// select image from:  text_image text-hello cat aphrodite venus saturn
// select a suitable sector, (size 270 will take two sectors)
#define IMAGE        cat
#define FLASH_SECTOR 0

// if the display list is defined it will take priority over the flashing
// (and FLASH code is disbled)
// define a list of {sector, milliseconds}
// #define DISPLAY_LIST {0, 5000}, {1, 5000}

// no futher changed below this point

// set up images from screen size2
#if (SCREEN_SIZE == 144)
#define EPD_SIZE EPD_1_44
#define FILE_SUFFIX _1_44.xbm
#define NAME_SUFFIX _1_44_bits
#define SECTORS_USED 1

#elif (SCREEN_SIZE == 200)
#define EPD_SIZE EPD_2_0
#define FILE_SUFFIX _2_0.xbm
#define NAME_SUFFIX _2_0_bits
#define SECTORS_USED 1

#elif (SCREEN_SIZE == 270)
#define EPD_SIZE EPD_2_7
#define FILE_SUFFIX _2_7.xbm
#define NAME_SUFFIX _2_7_bits
#define SECTORS_USED 2

#else
#error "Unknown EPB size: Change the #define SCREEN_SIZE to a supported value"
#endif


// program version
#define FLASH_LOADER_VERSION "1"

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
#define IMAGE_FILE MAKE_JOIN(IMAGE,FILE_SUFFIX)
#define IMAGE_BITS MAKE_NAME(IMAGE,NAME_SUFFIX)


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

// definition of I/O pins LaunchPad and Arduino are different

#if defined(__MSP430_CPU__)

// TI LaunchPad IO layout
const int Pin_TEMPERATURE = A4;
const int Pin_PANEL_ON = P2_3;
const int Pin_BORDER = P2_5;
const int Pin_DISCHARGE = P2_4;
const int Pin_PWM = P2_1;
const int Pin_RESET = P2_2;
const int Pin_BUSY = P2_0;
const int Pin_EPD_CS = P2_6;
const int Pin_FLASH_CS = P2_7;
const int Pin_SW2 = P1_3;
const int Pin_RED_LED = P1_0;

#else

// Arduino IO layout
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = 2;
const int Pin_BORDER = 3;
const int Pin_DISCHARGE = 4;
const int Pin_PWM = 5;
const int Pin_RESET = 6;
const int Pin_BUSY = 7;
const int Pin_EPD_CS = 8;
const int Pin_FLASH_CS = 9;
const int Pin_SW2 = 12;
const int Pin_RED_LED = 13;

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
static void epd_power_flash_access(bool state);

#if !defined(DISPLAY_LIST)
static void flash_program(uint16_t sector, const void *buffer, uint16_t length);
#endif

// define the E-Ink display
EPD_Class EPD(EPD_SIZE, Pin_PANEL_ON, Pin_BORDER, Pin_DISCHARGE, Pin_RESET, Pin_BUSY, Pin_EPD_CS);


// I/O setup
void setup() {
	pinMode(Pin_RED_LED, OUTPUT);
	pinMode(Pin_SW2, INPUT);
	pinMode(Pin_TEMPERATURE, INPUT);
	pinMode(Pin_PWM, OUTPUT);
	pinMode(Pin_BUSY, INPUT);
	pinMode(Pin_RESET, OUTPUT);
	pinMode(Pin_PANEL_ON, OUTPUT);
	pinMode(Pin_DISCHARGE, OUTPUT);
	pinMode(Pin_BORDER, OUTPUT);
	pinMode(Pin_EPD_CS, OUTPUT);
	pinMode(Pin_FLASH_CS, OUTPUT);

	digitalWrite(Pin_RED_LED, LOW);
	digitalWrite(Pin_PWM, LOW);  // not actually used - set low so can use current eval board unmodified
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_DISCHARGE, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);
	digitalWrite(Pin_FLASH_CS, HIGH);

	Serial.begin(9600);
#if defined(__AVR__)
	// wait for USB CDC serial port to connect.  Arduino Leonardo only
	while (!Serial) {
	}
#endif
	Serial.println();
	Serial.println();
	Serial.println("Flash loader G2 version: " FLASH_LOADER_VERSION);
	Serial.println("Display: " MAKE_STRING(EPD_SIZE));
	Serial.println();

	FLASH.begin(Pin_FLASH_CS);
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

		EPD.frame_cb_13(address, flash_read, EPD_inverse);
		EPD.frame_stage2();
		EPD.frame_cb_13(address, flash_read, EPD_normal);

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

	epd_power_flash_access(true);

	if (FLASH.available()) {
		Serial.println("FLASH chip detected OK");
	} else {
		Serial.println("unsupported FLASH chip");
	}

	FLASH.info(&maufacturer, &device);
	Serial.print("FLASH: manufacturer = 0x");
	Serial.print(maufacturer, HEX);
	Serial.print("  device = 0x");
	Serial.print(device, HEX);
	Serial.println();

	epd_power_flash_access(false);
}


// EPD display callback for reading the FLASH
static void flash_read(void *buffer, uint32_t address, uint16_t length) {
	FLASH.read(buffer, address, length);
}


static void epd_power_flash_access(bool state) {
	if (state) {
		// turn on panel, but hold reset to prevent SPI loading
		digitalWrite(Pin_RESET, HIGH);
		digitalWrite(Pin_EPD_CS, HIGH);
		digitalWrite(Pin_PANEL_ON, HIGH);
	} else {
		// turn off panel, after FLASH access
		digitalWrite(Pin_RESET, LOW);
		digitalWrite(Pin_EPD_CS, LOW);
		digitalWrite(Pin_PANEL_ON, LOW);
	}
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

	uint32_t address = (uint32_t)(sector) << FLASH_SECTOR_SHIFT;

	epd_power_flash_access(true);
	// erase required sectors
	for (unsigned int i = 0; i < length; i += FLASH_SECTOR_SIZE, address += FLASH_SECTOR_SIZE) {
		Serial.print("FLASH: erase = 0x");
		Serial.println(address, HEX);
		FLASH.write_enable();
		FLASH.sector_erase(address);
	}

	// writable pages are FLASH_PAGE_SIZE bytes
	PROGMEM const uint8_t *p = (PROGMEM const uint8_t *)buffer;
	for (address = (uint32_t)(sector) << FLASH_SECTOR_SHIFT; length >= FLASH_PAGE_SIZE;
	     length -= FLASH_PAGE_SIZE, address += FLASH_PAGE_SIZE, p += FLASH_PAGE_SIZE) {
		Serial.print("FLASH: write @ 0x");
		Serial.print(address, HEX);
		Serial.print("  from: 0x");
		Serial.print((uint32_t)p, HEX);
		Serial.print("  bytes: 0x");
		Serial.println(FLASH_PAGE_SIZE, HEX);
		FLASH.write_enable();
		FLASH.write_from_progmem(address, p, FLASH_PAGE_SIZE);
	}
	// write any remaining partial page
	if (length > 0) {
		Serial.print("FLASH: write @ 0x");
		Serial.print(address, HEX);
		Serial.print("  from: 0x");
		Serial.print((uint32_t)p, HEX);
		Serial.print("  bytes: 0x");
		Serial.println(length, HEX);
		FLASH.write_enable();
		FLASH.write_from_progmem(address, p, length);
	}

	// turn off write - just to be safe
	FLASH.write_disable();
	epd_power_flash_access(false);
}
#endif
