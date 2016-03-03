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


// simple serial port driven command system to upload data to flash
// and display images.

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
#include <EPD_V110_G1.h>
#define SCREEN_SIZE 144
#include <EPD_PANELS.h>
#include <S5813A.h>
#include <EPD_PINOUT.h>


// version number
#define COMMAND_VERSION "5"


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

static uint16_t xbm_count;
static bool xbm_parser(uint8_t *b);


static uint8_t Serial_getc();
static uint16_t Serial_gethex(bool echo);
static void Serial_puthex(uint32_t n, int bits);
static void Serial_puthex_byte(uint8_t n);
static void Serial_puthex_word(uint16_t n);
static void Serial_puthex_double(uint32_t n);
static void Serial_hex_dump(uint32_t address, const void *buffer, uint16_t length);


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
	// indefinite wait for USB CDC serial port to connect.  Arduino Leonardo only
	while (!Serial) {
	}
	// // additional delay for USB CDC serial port to connect.  Arduino Leonardo only
	// if (!Serial) {       // allows terminal time to sync as long
	// 	delay(500);  // as the serial monitor is opened before
	// }                    // upload
#endif
	Serial.println();
	Serial.println();
	Serial.println("Command version: " COMMAND_VERSION);
	Serial.println("Display size: " MAKE_STRING(EPD_SIZE));
	Serial.println("Film: V" MAKE_STRING(EPD_FILM_VERSION));
	Serial.println("COG: G" MAKE_STRING(EPD_CHIP_VERSION));
	Serial.println();

	EPD_FLASH.begin(Pin_EPD_FLASH_CS);
	flash_info();

	// configure temperature sensor
	S5813A.begin(Pin_TEMPERATURE);
}

// main loop
void loop() {
	Serial.println();
	Serial.print("Command: ");
	uint8_t c = Serial_getc();
	if (!isprint(c)) {
		return;
	}
	Serial.write(c);
	switch (c) {
	case 'h':
	case 'H':
	case '?':
		Serial.println();
		Serial.println("h          - this command");
		Serial.println("d<ss> <ll> - dump sector in hex, ll * 16 bytes");
		Serial.println("e<ss>      - erase sector to 0xff");
		Serial.println("u<ss>      - upload XBM to sector");
		Serial.println("i<ss>      - display an image on white screen");
#if EPD_IMAGE_TWO_ARG
		Serial.println("r<ss>      - revert an image back to white");
#endif
		Serial.println("l          - search for non-empty sectors");
		Serial.println("w          - clear screen to white");
		Serial.println("f          - dump EPD FLASH identification");
		Serial.println("t          - show temperature");
		break;
	case 'd':
	{
		uint16_t sector = Serial_gethex(true);
		Serial.print(' ');
		uint16_t count = Serial_gethex(true);
		if (0 == count) {
			count = 1;
		}
		Serial.println();
		Serial.print("sector: ");
		Serial_puthex_byte(sector);
		Serial.println();
		uint32_t address = sector;
		address <<= 12;
		for (uint16_t i = 0; i < count; ++i) {
			uint8_t buffer[16];
			EPD_FLASH.read(buffer, address, sizeof(buffer));
			Serial_hex_dump(address, buffer, sizeof(buffer));
			address += sizeof(buffer);
		}
		break;
	}

	case 'e':
	{
		uint32_t sector = Serial_gethex(true);
		EPD_FLASH.write_enable();
		EPD_FLASH.sector_erase(sector << 12);
		EPD_FLASH.write_disable();
		break;
	}

	case 'u':
	{
		uint32_t address = Serial_gethex(true);
		address <<= 12;
		xbm_count = 0;
		Serial.println();
		Serial.println("start upload...");

		for (;;) {
			uint8_t buffer[16];
			uint16_t count = 0;
			for (int i = 0; i < sizeof(buffer); ++i, ++count) {
				if (!xbm_parser(&buffer[i])) {
					break;
				}
			}
			EPD_FLASH.write_enable();
			EPD_FLASH.write(address, buffer, count);
			address += count;
			if (sizeof(buffer) != count) {
				break;
			}
		}

		EPD_FLASH.write_disable();

		Serial.println();
		Serial.print(" read = ");
		Serial_puthex_word(xbm_count);
		if (128 * 96 / 8 == xbm_count) {
			Serial.print(" 128x96 1.44");
		} else if (200 * 96 / 8 == xbm_count) {
			Serial.print(" 200x96 2.0");
		} else if (264L * 176 / 8 == xbm_count) {
			Serial.print(" 264x176 2.7");
		} else {
			Serial.print(" invalid image");
		}
		Serial.println();
		break;
	}

#if EPD_IMAGE_ONE_ARG
	case 'i':
	{
		uint32_t address = Serial_gethex(true);
		address <<= 12;
		EPD.begin();
		if (!EPD) {
			Serial.print("EPD error = ");
			Serial.print(EPD.error());
			Serial.println("");
			break;
		}
		int t = S5813A.read();
		EPD.setFactor(t);
		EPD.frame_cb_13(address, flash_read, EPD_inverse);
		EPD.frame_stage2();
		EPD.frame_cb_13(address, flash_read, EPD_normal);
		EPD.end();
		break;
	}

#elif EPD_IMAGE_TWO_ARG
	case 'i':
	{
		uint32_t address = Serial_gethex(true);
		address <<= 12;
		EPD.begin();
		int t = S5813A.read();
		EPD.setFactor(t);
		EPD.frame_cb_repeat(address, flash_read, EPD_inverse);
		EPD.frame_cb_repeat(address, flash_read, EPD_normal);
		EPD.end();
		break;
	}

	case 'r':
	{
		uint32_t address = Serial_gethex(true);
		address <<= 12;
		EPD.begin();
		int t = S5813A.read();
		EPD.setFactor(t);
		EPD.frame_cb_repeat(address, flash_read, EPD_compensate);
		EPD.frame_cb_repeat(address, flash_read, EPD_white);
		EPD.end();
		break;
	}
#else
#error "unsupported image function"
#endif

	case 'l':
	{
		Serial.println();
		uint16_t per_line = 0;
		for (uint16_t sector = 0; sector <= 0xff; ++sector) {
			for (uint16_t i = 0; i < 4096; i += 16) {
				uint8_t buffer[16];
				uint32_t address = sector;
				address <<= 12;
				address += i;
				EPD_FLASH.read(buffer, address, sizeof(buffer));
				bool flag = false;
				for (uint16_t j = 0; j < sizeof(buffer); ++j) {
					if (0xff != buffer[j]) {
						flag = true;
						break;
					}
				}
				if (flag) {
					Serial_puthex_byte(sector);
					if (++per_line >= 16) {
						Serial.println();
						per_line = 0;
					} else {
						Serial.print(' ');
					}
					break;
				}
			}

		}
		if (0 != per_line) {
			Serial.println();
		}
		break;
	}

	case 'f':
	{
		Serial.println();
		flash_info();
		break;
	}

	case 'w':
	{
		EPD.begin();
		int t = S5813A.read();
		EPD.setFactor(t);
		EPD.clear();
		EPD.end();
		break;
	}

	case 't':
	{
		long vADC = analogRead(Pin_TEMPERATURE);
		Serial.println();
		Serial.print("Temperature ADC = ");
		Serial.print(vADC);
		Serial.print(" (0x");
		Serial_puthex_word(vADC);
		Serial.print(")");
		Serial.println();
		Serial.print("Temperature = ");
		Serial.print(S5813A.read());
		Serial.print(" Celcius");
		Serial.println();

		break;
	}

	default:
		Serial.println();
		Serial.println("error");

	}
}


static void flash_info(void) {
	uint8_t maufacturer;
	uint16_t device;

	if (EPD_FLASH.available()) {
		Serial.println("EPD FLASH chip detected OK");
	} else {
		Serial.println("unsupported EPD FLASH chip");
	}

	EPD_FLASH.info(&maufacturer, &device);
	Serial.print("EPD_FLASH: manufacturer = ");
	Serial_puthex_byte(maufacturer);
	Serial.print(" device = ");
	Serial_puthex_word(device);
	Serial.println();
}


static void flash_read(void *buffer, uint32_t address, uint16_t length) {
	EPD_FLASH.read(buffer, address, length);
}


static bool xbm_parser(uint8_t *b) {
	for (;;) {
		uint8_t c = Serial_getc();
		if ('0' == c && 'x' == Serial_getc()) {
			*b = Serial_gethex(false);
			++xbm_count;
			if (0 == (xbm_count & 0x07)) {
				digitalWrite(Pin_RED_LED, !digitalRead(Pin_RED_LED));
			}
			return true;
		} else if (';' == c) {
			break;
		}
	}
	digitalWrite(Pin_RED_LED, LED_OFF);
	return false;
}


// miscellaneous serial interface routines

static uint8_t Serial_getc() {
	while (0 == Serial.available()) {
	}
	return Serial.read();
}


static uint16_t Serial_gethex(bool echo) {
	uint16_t acc = 0;
	for (;;) {

		uint8_t c = Serial_getc();

		if (c >= '0' && c <= '9') {
			c -= '0';
		} else if (c >= 'a' && c <= 'f') {
			c -= 'a' - 10;
		} else if (c >= 'A' && c <= 'F') {
			c -= 'A' - 10;
		} else {
			return acc;
		}
		if (echo) {
			Serial_puthex(c, 4);
		}
		acc <<= 4;
		acc += c;
	}
}


static void Serial_puthex(uint32_t n, int bits) {
	for (int i = bits - 4; i >= 0; i -= 4) {
		char nibble = ((n >> i) & 0x0f) + '0';
		if (nibble > '9') {
			nibble += 'a' - '9' - 1;
		}
		Serial.print(nibble);
	}
}


static void Serial_puthex_byte(uint8_t n) {
	Serial_puthex(n, 8);
}


static void Serial_puthex_word(uint16_t n) {
	Serial_puthex(n, 16);
}


static void Serial_puthex_double(uint32_t n) {
	Serial_puthex(n, 32);
}


static void Serial_hex_dump(uint32_t address, const void *buffer, uint16_t length) {
	const uint8_t *p = (const uint8_t *)buffer;
	while (0 != length) {
		Serial_puthex_double(address);
		Serial.print(": ");
		for (int i = 0; i < 16; ++i, ++address) {
			Serial_puthex_byte(*p++);
			Serial.print(' ');
			if (7 == i) {
				Serial.print(' ');
			}
			if (--length == 0) {
				break;
			}
		}
		Serial.println();
	}
}
