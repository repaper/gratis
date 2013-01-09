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
#include <avr/pgmspace.h>
#include <pins_arduino.h>

#include <SPI.h>

#include "FLASH.h"


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
FLASH_Class FLASH(12, SPI);


FLASH_Class::FLASH_Class(int chip_select_pin, SPIClass &SPI_driver) : SPI(SPI_driver), CS(chip_select_pin) {
}


void FLASH_Class::begin(int chip_select_pin, SPIClass &SPI_driver) {
	digitalWrite(chip_select_pin, HIGH);
	pinMode(chip_select_pin, OUTPUT);
	this->SPI = SPI_driver;
	this->CS = chip_select_pin;
}


void FLASH_Class::end() {
}


// return true if the chip is supported
bool FLASH_Class::available(void) {

	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP); // flush the SPI buffers
	this->SPI.transfer(FLASH_NOP); // ..
	this->SPI.transfer(FLASH_NOP); // ..

	uint8_t maufacturer;
	uint16_t device;
	this->info(&maufacturer, &device); // initial read to reset the chip
	this->info(&maufacturer, &device); // actual read

	return (FLASH_MFG == maufacturer) && (FLASH_ID == device);
}


void FLASH_Class::info(uint8_t *maufacturer, uint16_t *device) {

	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_RDID);
	*maufacturer = this->SPI.transfer(FLASH_NOP);
	uint8_t id_high = this->SPI.transfer(FLASH_NOP);
	uint8_t id_low = this->SPI.transfer(FLASH_NOP);
	*device = (id_high << 8) | id_low;
	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}


void FLASH_Class::read(void *buffer, uint32_t address, uint16_t length) {
	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_FAST_READ);
	this->SPI.transfer(address >> 16);
	this->SPI.transfer(address >> 8);
	this->SPI.transfer(address);
	this->SPI.transfer(FLASH_NOP); // read dummy byte
	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		*p++ = this->SPI.transfer(FLASH_NOP);
	}

	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}


bool FLASH_Class::is_busy(void) {
	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_RDSR);
	bool busy = 0 != (FLASH_WIP & this->SPI.transfer(0xff));
	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
	return busy;
}


void FLASH_Class::write_enable(void) {
	while (this->is_busy()) {
	}
	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_WREN);
	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}



void FLASH_Class::write_disable(void) {
	while (this->is_busy()) {
	}
	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_WRDI);
	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}


void FLASH_Class::write(uint32_t address, void *buffer, uint16_t length) {
	while (this->is_busy()) {
	}
	digitalWrite(this->CS, LOW);

	this->SPI.transfer(FLASH_PP);
	this->SPI.transfer(address >> 16);
	this->SPI.transfer(address >> 8);
	this->SPI.transfer(address);
	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		this->SPI.transfer(*p++);
	}

	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}


void FLASH_Class::sector_erase(uint32_t address) {
	while (this->is_busy()) {
	}

	digitalWrite(this->CS, LOW);
	this->SPI.transfer(FLASH_SE);
	this->SPI.transfer(address >> 16);
	this->SPI.transfer(address >> 8);
	this->SPI.transfer(address);
	digitalWrite(this->CS, HIGH);
	this->SPI.transfer(FLASH_NOP);
}
