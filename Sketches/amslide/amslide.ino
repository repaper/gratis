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


// This program is to illustrate the display operation as described in
// the datasheets.  The code is in a simple linear fashion and all the
// delays are set to maximum, but the SPI clock is set lower than its
// limit.  Therfore the display sequence will be much slower than
// normal and all of the individual display stages be clearly visible.

// Written in collaboration with justin@wyolum.com
// and developed on a Wyolum AlaMode http://wyolum.com/

// Operation from reset:
// * display version
// * display compiled-in display setting
// * display FLASH detected or not
// * display temperature (displayed before every image is changed)
// * clear screen
// * delay 5 seconds
// * display an image
// * delay ? seconds
// * back to image display


#include <inttypes.h>
#include <ctype.h>

#include <SPI.h>
#include <SD.h>
#include <FLASH.h>
#include <EPD.h>
#include <S5813A.h>
#include <FLASH.h>


// Change this for different display size
// supported sizes: 1_44 2_0 2_7
#define EPD_SIZE EPD_2_0
// #define EPD_SIZE EPD_2_7


// current version number
#define DEMO_VERSION "1"


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
const int Pin_SD_CS = 10;

#endif

#define INDEX_FILE "index.txt"
// the maximum number of characters in an image path
#define MAXIMUM_PATH_LENGTH 32

// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)


// define the E-Ink display
EPD_Class EPD(EPD_SIZE, Pin_PANEL_ON, Pin_BORDER, Pin_DISCHARGE, Pin_PWM, Pin_RESET, Pin_BUSY, Pin_EPD_CS);


File index_file;
File current_image;
File next_image;


// EPD use of SPI totally disrupts SPI settings
// this attempts to replicate the SPI settings for SD card
// Check it against your particular SD interface
void set_spi_for_sdcard(void) {
//	SPI.end();
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV128);
}


void open_index() {
	index_file.close();
	index_file = SD.open(INDEX_FILE);
	if (!index_file) {
		Serial.println(INDEX_FILE " not found");
		for (;;) {
			delay(1000);
		}
	}
}


// open up the next image an return the delay
int get_image() {
	current_image.close();
	current_image = next_image;

	// get seconds, first non-digit ends number
	int seconds = 0;
	int c = 0;
	for (;;) {
		c = index_file.read();
		if (-1 == c) {
			open_index();
			seconds = 0;
			continue;
		}
		if (isdigit(c)) {
			seconds = 10 * seconds + c - '0';
		} else {
			break;
		}
	}
	if (seconds <= 0) {
		seconds = 1;
	}
	// skip spaces
	while (isspace(c)) {
		c = index_file.read();
	}

	// read the filename
	char filename[MAXIMUM_PATH_LENGTH + 1];  // remember the '\0'
	memset(filename, 0, sizeof(filename));
	for (int i = 0; i < sizeof(filename) - 1; ++i) {
		if (-1 == c) {
			break;
		} else if (isprint(c)) {
			filename[i] = c;
		} else {
			break;
		}
		c = index_file.read();
	}
	// skip to next line
	while (0x0a != c) {
		c = index_file.read();
	}

	Serial.print(seconds);
	Serial.print(" ");
	Serial.print("image: ");
	Serial.println(filename);

	next_image = SD.open(filename);
	if (!next_image) {
		Serial.println("error: not found");
		for (;;) {
			delay(1000);
		}
	}
	return seconds;
}

// read a block from the current image
void next_image_reader(void *buffer, uint32_t address, uint16_t length) {
	set_spi_for_sdcard();
	next_image.seek(address);
	next_image.read(buffer, length);
}
void current_image_reader(void *buffer, uint32_t address, uint16_t length){
	set_spi_for_sdcard();
	current_image.seek(address);
	current_image.read(buffer, length);
}


// I/O setup
void setup() {

	pinMode(Pin_TEMPERATURE, INPUT);
	pinMode(Pin_PWM, OUTPUT);
	pinMode(Pin_BUSY, INPUT);
	pinMode(Pin_RESET, OUTPUT);
	pinMode(Pin_PANEL_ON, OUTPUT);
	pinMode(Pin_DISCHARGE, OUTPUT);
	pinMode(Pin_BORDER, OUTPUT);
	pinMode(Pin_EPD_CS, OUTPUT);
	pinMode(Pin_FLASH_CS, OUTPUT);

	digitalWrite(Pin_PWM, LOW);
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_DISCHARGE, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);
	digitalWrite(Pin_FLASH_CS, HIGH);

#if !defined(__MSP430_CPU__)
	// wait for USB CDC serial port to connect.  Arduino Leonardo only
	while (!Serial) {
	}
#endif
	Serial.println();
	Serial.println();
	Serial.println("Demo version: " DEMO_VERSION);
	Serial.println("Display: " MAKE_STRING(EPD_SIZE));
	Serial.println();

	FLASH.begin(Pin_FLASH_CS);
	if (FLASH.available()) {
		Serial.println("FLASH chip detected OK");
	} else {
		Serial.println("unsupported FLASH chip");
	}

	// configure temperature sensor
	S5813A.begin(Pin_TEMPERATURE);
	Serial.begin(9600);
	Serial.print("Initializing SD card...");
	// On the Ethernet Shield, CS is pin 4. It's set as an output by default.
	// Note that even if it's not used as the CS pin, the hardware SS pin
	// (10 on most Arduino boards, 53 on the Mega) must be left as an output
	// or the SD library functions will not work.
	pinMode(Pin_SD_CS, OUTPUT);

	if (!SD.begin(Pin_SD_CS)) {
		Serial.println("initialization failed!");
		for (;;) {
			delay(1000);
		}
	}
	Serial.println("initialization done.");
	open_index();
}


static int state = 0;

// main loop
unsigned long int loop_count = 0;
void loop() {
	int temperature = S5813A.read();
	Serial.print("Temperature = ");
	Serial.print(temperature);
	Serial.println(" Celcius");

	// get an image
	int seconds = get_image();

	//*** maybe need to ensure clock is ok for EPD

	EPD.begin(); // power up the EPD panel
	EPD.setFactor(temperature); // adjust for current temperature

	switch(state) {
	default:
	case 0:
		// clear the screen
		EPD.clear();
		// clear -> image1
		EPD.frame_fixed_repeat(0, EPD_compensate);
		EPD.frame_fixed_repeat(0, EPD_white);
		EPD.frame_cb_repeat(0, next_image_reader, EPD_inverse);
		EPD.frame_cb_repeat(0, next_image_reader, EPD_normal);
		++state;
		break;
	case 1:        // swap images
		EPD.frame_cb_repeat(0, current_image_reader, EPD_compensate);
		EPD.frame_cb_repeat(0, current_image_reader, EPD_white);
		EPD.frame_cb_repeat(0, next_image_reader, EPD_inverse);
		EPD.frame_cb_repeat(0, next_image_reader, EPD_normal);
		break;
	}
	EPD.end();   // power down the EPD panel

	set_spi_for_sdcard();

	// wait for 5 seconds
	for (int x = 0; x < seconds; ++x) {
		delay(1000);
	}
}
