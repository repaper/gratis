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
// . Changed CS for EPD_FLASH_CS to avoid conflicts with Energia

#if defined(ENERGIA)
#include <Energia.h>
#else
#include <Arduino.h>
#endif

#include <SPI.h>

#include "EPD_FLASH.h"

// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)


// FLASH MX25V8005 8Mbit flash chip command set (50MHz max clock)
enum {
	EPD_FLASH_WREN = 0x06,
	EPD_FLASH_WRDI = 0x04,
	EPD_FLASH_RDID = 0x9f,
	EPD_FLASH_RDSR = 0x05,
	EPD_FLASH_WRSR = 0x01,
	EPD_FLASH_READ = 0x03,       // read at half frequency
	EPD_FLASH_FAST_READ = 0x0b,  // read at full frequency
	EPD_FLASH_SE = 0x20,
	EPD_FLASH_BE = 0x52,
	EPD_FLASH_CE = 0x60,
	EPD_FLASH_PP = 0x02,
	EPD_FLASH_DP = 0xb9,
	EPD_FLASH_RDP = 0xab,
	EPD_FLASH_REMS = 0x90,
	EPD_FLASH_NOP = 0xff,

	// status register bits
	EPD_FLASH_WIP = 0x01,
	EPD_FLASH_WEL = 0x02,
	EPD_FLASH_BP0 = 0x04,
	EPD_FLASH_BP1 = 0x08,
	EPD_FLASH_BP2 = 0x10
};


// currently supported chip
#define EPD_FLASH_MFG 0xc2
#define EPD_FLASH_ID 0x2014


// the default EPD_FLASH device
EPD_FLASH_Class EPD_FLASH(9);


EPD_FLASH_Class::EPD_FLASH_Class(uint8_t chip_select_pin) : EPD_FLASH_CS(chip_select_pin) {
}


// TODO: Why allow begin to change the chip_select_pin (except for this could make chip_select_pin constant)
void EPD_FLASH_Class::begin(uint8_t chip_select_pin) {
	digitalWrite(chip_select_pin, HIGH);
	pinMode(chip_select_pin, OUTPUT);
	this->EPD_FLASH_CS = chip_select_pin;
}


void EPD_FLASH_Class::end(void) {
}

// configure the SPI for EPD_FLASH access
void EPD_FLASH_Class::spi_setup(void) {
	SPI.begin();
	digitalWrite(this->EPD_FLASH_CS, HIGH);

	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE3);
	SPI.setClockDivider(SPI_CLOCK_DIV4);

	Delay_us(10);

	SPI.transfer(EPD_FLASH_NOP); // flush the SPI buffer
	SPI.transfer(EPD_FLASH_NOP); // ..
	SPI.transfer(EPD_FLASH_NOP); // ..
	Delay_us(10);
}

// shutdown SPI after EPD_FLASH access
void EPD_FLASH_Class::spi_teardown(void) {
	Delay_us(10);
	digitalWrite(this->EPD_FLASH_CS, HIGH);
	SPI.transfer(EPD_FLASH_NOP); // flush the SPI buffer
	SPI.end();
}

// return true if the chip is supported
bool EPD_FLASH_Class::available(void) {
	uint8_t maufacturer;
	uint16_t device;
	this->info(&maufacturer, &device); // initial read to reset the chip
	this->info(&maufacturer, &device); // actual read

	return (EPD_FLASH_MFG == maufacturer) && (EPD_FLASH_ID == device);
}


void EPD_FLASH_Class::info(uint8_t *maufacturer, uint16_t *device) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(1500);                     // FLASH wake up delay
	digitalWrite(this->EPD_FLASH_CS, HIGH);
	Delay_us(50);
	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_RDID);
	*maufacturer = SPI.transfer(EPD_FLASH_NOP);
	uint8_t id_high = SPI.transfer(EPD_FLASH_NOP);
	uint8_t id_low = SPI.transfer(EPD_FLASH_NOP);
	*device = (id_high << 8) | id_low;
	this->spi_teardown();
}


void EPD_FLASH_Class::read(void *buffer, uint32_t address, uint16_t length) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_FAST_READ);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	SPI.transfer(EPD_FLASH_NOP); // read dummy byte
	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		*p++ = SPI.transfer(EPD_FLASH_NOP);
	}
	this->spi_teardown();
}


void EPD_FLASH_Class::wait_for_ready(void) {
	this->spi_setup();
	while (this->is_busy()) {
	}
}

bool EPD_FLASH_Class::is_busy(void) {
	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_RDSR);
	bool busy = 0 != (EPD_FLASH_WIP & SPI.transfer(EPD_FLASH_NOP));
	digitalWrite(this->EPD_FLASH_CS, HIGH);
	SPI.transfer(EPD_FLASH_NOP);
	Delay_us(10);
	return busy;
}


void EPD_FLASH_Class::write_enable(void) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_WREN);
	this->spi_teardown();
}



void EPD_FLASH_Class::write_disable(void) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_WRDI);
	this->spi_teardown();
}


void EPD_FLASH_Class::write(uint32_t address, const void *buffer, uint16_t length) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_PP);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	for (const uint8_t *p = (const uint8_t *)buffer; length != 0; --length) {
		SPI.transfer(*p++);
	}
	this->spi_teardown();
}


#if defined(__AVR__)
void EPD_FLASH_Class::write_from_progmem(uint32_t address, PROGMEM const void *buffer, uint16_t length) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_PP);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	for (PROGMEM const uint8_t *p = (PROGMEM const uint8_t *)buffer; length != 0; ++p, --length) {
		uint8_t the_byte = pgm_read_byte_near(p);
		SPI.transfer(the_byte);
	}
	this->spi_teardown();
}
#endif


void EPD_FLASH_Class::sector_erase(uint32_t address) {
	this->wait_for_ready();

	digitalWrite(this->EPD_FLASH_CS, LOW);
	Delay_us(10);
	SPI.transfer(EPD_FLASH_SE);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	this->spi_teardown();
}
