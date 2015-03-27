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

#if !defined(EPD_V230_G2_H)
#define EPD_V230_G2_H 1

#include <Arduino.h>
#include <SPI.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#endif

// compile-time #if configuration
#define EPD_CHIP_VERSION      2
#define EPD_FILM_VERSION      230
#define EPD_PWM_REQUIRED      0
#define EPD_IMAGE_ONE_ARG     1
#define EPD_IMAGE_TWO_ARG     0
#define EPD_PARTIAL_AVAILABLE 0

// display panels supported
#define EPD_1_44_SUPPORT      1
#define EPD_1_9_SUPPORT       0
#define EPD_2_0_SUPPORT       1
#define EPD_2_6_SUPPORT       0
#define EPD_2_7_SUPPORT       1


// if more SRAM available (8 kBytes)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define EPD_ENABLE_EXTRA_SRAM 1
#endif

typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // Image pixel -> Display pixel
	EPD_inverse,     // B -> W, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef enum {           // error codes
	EPD_OK,
	EPD_UNSUPPORTED_COG,
	EPD_PANEL_BROKEN,
	EPD_DC_FAILED
} EPD_error;

// values for border byte
const uint8_t EPD_BORDER_BYTE_BLACK = 0xff;
const uint8_t EPD_BORDER_BYTE_WHITE = 0xaa;
const uint8_t EPD_BORDER_BYTE_NULL  = 0x00;

typedef void EPD_reader(void *buffer, uint32_t address, uint16_t length);

class EPD_Class {
private:
	const uint8_t EPD_Pin_PANEL_ON;
	const uint8_t EPD_Pin_BORDER;
	const uint8_t EPD_Pin_DISCHARGE;
	const uint8_t EPD_Pin_RESET;
	const uint8_t EPD_Pin_BUSY;
	const uint8_t EPD_Pin_EPD_CS;

	const EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;

	uint8_t voltage_level;

	EPD_error status;

	PROGMEM const uint8_t *channel_select;
	uint16_t channel_select_length;

	typedef struct {
		uint16_t stage1_repeat;
		uint16_t stage1_step;
		uint16_t stage1_block;
		uint16_t stage2_repeat;
		uint16_t stage2_t1;
		uint16_t stage2_t2;
		uint16_t stage3_repeat;
		uint16_t stage3_step;
		uint16_t stage3_block;
	} compensation_type;

	const compensation_type *compensation;
	uint16_t temperature_offset;

	EPD_Class(const EPD_Class &f);  // prevent copy

	void power_off(void);
	void nothing_frame(void);
	void dummy_line(void);
	void border_dummy_line(void);

public:
	// power up and power down the EPD panel
	void begin(void);
	void end(void);

	void setFactor(int temperature = 25);

	const bool operator!(void) const {
		return EPD_OK != this->status;
	}

	EPD_error error(void) const {
		return this->status;
	}

	// clear display (anything -> white)
	void clear(void) {
		this->frame_fixed_13(0xff, EPD_inverse);
		this->frame_stage2();
		this->frame_fixed_13(0xaa, EPD_normal);
	}

	// output an image (PROGMEM data)
	void image(const uint8_t *image_data) {
		this->frame_data_13(image_data, EPD_inverse);
		this->frame_stage2();
		this->frame_data_13(image_data, EPD_normal);
	}

	// change from old image to new image (SRAM version)
	void image_sram(const uint8_t *image_data) {
		this->frame_data_13(image_data, EPD_inverse, false);
		this->frame_stage2();
		this->frame_data_13(image_data, EPD_normal, false);
	}


	// Low level API calls
	// ===================

	// single frame refresh
	void frame_fixed_timed(uint8_t fixed_value, long stage_time);

	// the B/W toggle stage
	void frame_stage2(void);

	// stages 1/3 functions
	void frame_fixed_13(uint8_t fixed_value, EPD_stage stage);
	void frame_data_13(const uint8_t *image_data, EPD_stage stage, bool read_progmem = true);
	void frame_cb_13(uint32_t address, EPD_reader *reader, EPD_stage stage);

	// single line display - very low-level
	// also has to handle AVR progmem
	void line(uint16_t line, const uint8_t *data, uint8_t fixed_value,
		  bool read_progmem, EPD_stage stage = EPD_normal, uint8_t border_byte = EPD_BORDER_BYTE_NULL, bool set_voltage_limit = false);

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	EPD_Class(EPD_size _size,
		  uint8_t panel_on_pin,
		  uint8_t border_pin,
		  uint8_t discharge_pin,
		  uint8_t reset_pin,
		  uint8_t busy_pin,
		  uint8_t chip_select_pin);

};

#endif
