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

#include <SPI.h>

#include "FLASH.h"

// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)


// FLASH MX25V8005 8Mbit flash chip command set (50MHz max clock)
enum {
	FLASH_WREN = 0x06,
	FLASH_WRDI = 0x04,
	FLASH_RDID = 0x9f,
	FLASH_RDSR = 0x05,
	FLASH_WRSR = 0x01,
	FLASH_READ = 0x03,       // read at half frequency
	FLASH_FAST_READ = 0x0b,  // read at full frequency
	FLASH_SE = 0x20,
	FLASH_BE = 0x52,
	FLASH_CE = 0x60,
	FLASH_PP = 0x02,
	FLASH_DP = 0xb9,
	FLASH_RDP = 0xab,
	FLASH_REMS = 0x90,
	FLASH_NOP = 0xff,

	// status register bits
	FLASH_WIP = 0x01,
	FLASH_WEL = 0x02,
	FLASH_BP0 = 0x04,
	FLASH_BP1 = 0x08,
	FLASH_BP2 = 0x10
};


// currently supported chip
#define FLASH_MFG 0xc2
#define FLASH_ID 0x2014


// the default FLASH device
FLASH_Class FLASH(9);


FLASH_Class::FLASH_Class(uint8_t chip_select_pin) : CS(chip_select_pin) {
}


// TODO: Why allow begin to change the chip_select_pin (except for this could make chip_select_pin constant)
void FLASH_Class::begin(uint8_t chip_select_pin) {
	digitalWrite(chip_select_pin, HIGH);
	pinMode(chip_select_pin, OUTPUT);
	this->CS = chip_select_pin;
}


void FLASH_Class::end(void) {
}

// configure the SPI for FLASH access
void FLASH_Class::spi_setup(void) {
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE3);
	SPI.setClockDivider(SPI_CLOCK_DIV4);
	Delay_us(10);

	digitalWrite(this->CS, HIGH);
	SPI.transfer(FLASH_NOP); // flush the SPI buffer
	// SPI.transfer(FLASH_NOP); // ..
	// SPI.transfer(FLASH_NOP); // ..
	Delay_us(50);
}

// shutdown SPI after FLASH access
void FLASH_Class::spi_teardown(void) {
	Delay_us(50);
	digitalWrite(this->CS, HIGH);
	SPI.end();
}

// return true if the chip is supported
bool FLASH_Class::available(void) {
	uint8_t maufacturer;
	uint16_t device;
	this->info(&maufacturer, &device); // initial read to reset the chip
	this->info(&maufacturer, &device); // actual read

	return (FLASH_MFG == maufacturer) && (FLASH_ID == device);
}


void FLASH_Class::info(uint8_t *maufacturer, uint16_t *device) {
	this->spi_setup();
	digitalWrite(this->CS, LOW);
	Delay_us(1000);                     // FLASH wake up delay
	SPI.transfer(FLASH_RDID);
	*maufacturer = SPI.transfer(FLASH_NOP);
	uint8_t id_high = SPI.transfer(FLASH_NOP);
	uint8_t id_low = SPI.transfer(FLASH_NOP);
	*device = (id_high << 8) | id_low;
	this->spi_teardown();
}


void FLASH_Class::read(void *buffer, uint32_t address, uint16_t length) {
	while (this->is_busy()) {           // wait for last action to finish
	}

	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_FAST_READ);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	SPI.transfer(FLASH_NOP); // read dummy byte
	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		*p++ = SPI.transfer(FLASH_NOP);
	}
	this->spi_teardown();
}


bool FLASH_Class::is_busy(void) {
	this->spi_setup();
	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_RDSR);
	bool busy = 0 != (FLASH_WIP & SPI.transfer(0xff));
	digitalWrite(this->CS, HIGH);
	SPI.transfer(FLASH_NOP);
	Delay_us(50);
	return busy;
}


void FLASH_Class::write_enable(void) {
	while (this->is_busy()) {
	}
	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_WREN);
	this->spi_teardown();
}



void FLASH_Class::write_disable(void) {
	while (this->is_busy()) {
	}

	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_WRDI);
	this->spi_teardown();
}


void FLASH_Class::write(uint32_t address, const void *buffer, uint16_t length) {
	while (this->is_busy()) {
	}

	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_PP);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	for (const uint8_t *p = (const uint8_t *)buffer; length != 0; --length) {
		SPI.transfer(*p++);
	}
	this->spi_teardown();
}


#if defined(__AVR__)
void FLASH_Class::write_from_progmem(uint32_t address, PROGMEM const void *buffer, uint16_t length) {
	while (this->is_busy()) {
	}

	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_PP);
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


void FLASH_Class::sector_erase(uint32_t address) {
	while (this->is_busy()) {
	}

	digitalWrite(this->CS, LOW);
	Delay_us(10);
	SPI.transfer(FLASH_SE);
	SPI.transfer(address >> 16);
	SPI.transfer(address >> 8);
	SPI.transfer(address);
	this->spi_teardown();
}
