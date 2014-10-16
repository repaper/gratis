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

#if !defined(EPD_FLASH_H)
#define EPD_FLASH_H 1

#include <Arduino.h>


// maximum bytes that can be written by one write command
#define FLASH_PAGE_SIZE 128

// to shift sector number (0..FLASH_SECTOR_COUNT) to an address for erase
#define FLASH_SECTOR_SHIFT 12

// erase size is one sector
#define FLASH_SECTOR_SIZE 4096

// total available sectors
#define FLASH_SECTOR_COUNT 256


class FLASH_Class {
private:
	uint8_t CS;

	void spi_setup(void);
	void spi_teardown(void);
	bool is_busy(void);
	FLASH_Class(const FLASH_Class &f);  // prevent copy

public:
	bool available(void);
	void info(uint8_t *maufacturer, uint16_t *device);
	void read(void *buffer, uint32_t address, uint16_t length);
	void write_enable(void);
	void write_disable(void);
	void write(uint32_t address, const void *buffer, uint16_t length);

	// Arduino has separate memory spaces, but MSP430, ARM do not
#if !defined(__AVR__)
	// just alias the function name
	inline void write_from_progmem(uint32_t address, const void *buffer, uint16_t length) {
		this->write(address, buffer, length);
	}
#else
	void write_from_progmem(uint32_t address, PROGMEM const void *buffer, uint16_t length);
#endif

	void sector_erase(uint32_t address);

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	void begin(uint8_t chip_select_pin);
	void end(void);

	FLASH_Class(uint8_t chip_select_pin);

};

extern FLASH_Class FLASH;

#endif
