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


#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "io.h"
#include "spi.h"
#include "flash.h"


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
	FLASH_DP = 0xB9,
	FLASH_RDP = 0xAB,
	FLASH_REMS = 0x90,
	FLASH_NOP = 0xFF,

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


// return true if the chip is supported
bool FLASH_initialise(void) {

	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(0xff); // flush the SPI buffers
	SPI_transfer(0xff); // ..
	SPI_transfer(0xff); // ..

	uint8_t maufacturer;
	uint16_t device;
	FLASH_info(&maufacturer, &device);

	return (FLASH_MFG == maufacturer) && (FLASH_ID == device);
}


void FLASH_info(uint8_t *maufacturer, uint16_t *device) {

	P2OUT &= ~PORT2_FLASH_CS; // CS low
	SPI_transfer(FLASH_RDID);
	*maufacturer = SPI_transfer(FLASH_NOP);
	uint8_t id_high = SPI_transfer(FLASH_NOP);
	uint8_t id_low = SPI_transfer(FLASH_NOP);
	*device = (id_high << 8) | id_low;
	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}



void FLASH_read(void *buffer, uint32_t address, uint16_t length) {
	P2OUT &= ~PORT2_FLASH_CS; // CS low

	SPI_transfer(FLASH_FAST_READ);
	SPI_transfer(address >> 16);
	SPI_transfer(address >> 8);
	SPI_transfer(address);
	SPI_transfer(FLASH_NOP); // read dummy byte

	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		*p++ = SPI_transfer(FLASH_NOP);
	}

	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}


bool FLASH_is_busy(void) {
	P2OUT &= ~PORT2_FLASH_CS; // CS low
	SPI_transfer(FLASH_RDSR);
	bool busy = 0 != (FLASH_WIP & SPI_transfer(0xff));
	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
	return busy;
}


void FLASH_write_enable(void) {
	while (FLASH_is_busy()) {
	}
	P2OUT &= ~PORT2_FLASH_CS; // CS low
	SPI_transfer(FLASH_WREN);
	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}



void FLASH_write_disable(void) {
	while (FLASH_is_busy()) {
	}
	P2OUT &= ~PORT2_FLASH_CS; // CS low
	SPI_transfer(FLASH_WRDI);
	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}


void FLASH_write(uint32_t address, void *buffer, uint16_t length) {
	while (FLASH_is_busy()) {
	}

	P2OUT &= ~PORT2_FLASH_CS; // CS low

	SPI_transfer(FLASH_PP);
	SPI_transfer(address >> 16);
	SPI_transfer(address >> 8);
	SPI_transfer(address);
	for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
		SPI_transfer(*p++);
	}

	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}


void FLASH_sector_erase(uint32_t address) {
	while (FLASH_is_busy()) {
	}

	P2OUT &= ~PORT2_FLASH_CS; // CS low
	SPI_transfer(FLASH_SE);
	SPI_transfer(address >> 16);
	SPI_transfer(address >> 8);
	SPI_transfer(address);
	P2OUT |= PORT2_FLASH_CS; // CS high
	SPI_transfer(FLASH_NOP);
}
