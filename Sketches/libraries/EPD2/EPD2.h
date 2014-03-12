// Copyright 2014 Pervasive Displays, Inc.
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

#if !defined(EPD2_H)
#define EPD2_H 1

#include <Arduino.h>
#include <SPI.h>

#if defined(__MSP430_CPU__)
#define PROGMEM
#else
#include <avr/pgmspace.h>
#endif

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

typedef void EPD_reader(void *buffer, uint32_t address, uint16_t length);

class EPD_Class {
private:
	int EPD_Pin_EPD_CS;
	int EPD_Pin_PANEL_ON;
	int EPD_Pin_BORDER;
	int EPD_Pin_DISCHARGE;
	int EPD_Pin_RESET;
	int EPD_Pin_BUSY;

	EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;

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

	void power_off();

public:
	// power up and power down the EPD panel
	void begin();
	void end();

	void setFactor(int temperature = 25);

	const bool operator!() const {
		return EPD_OK != this->status;
	}

	EPD_error error() const {
		return this->status;
	}

	// clear display (anything -> white)
	void clear() {
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
	void frame_stage2();

	// stages 1/3 functions
	void frame_fixed_13(uint8_t fixed_value, EPD_stage stage);
	void frame_data_13(const uint8_t *image_data, EPD_stage stage, bool read_progmem = true);
	void frame_cb_13(uint32_t address, EPD_reader *reader, EPD_stage stage);

	// single line display - very low-level
	// also has to handle AVR progmem
	void line(uint16_t line, const uint8_t *data, uint8_t fixed_value,
		  bool read_progmem, EPD_stage stage = EPD_normal, uint8_t border_byte = 0x00);

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	EPD_Class(EPD_size size,
		  int panel_on_pin,
		  int border_pin,
		  int discharge_pin,
		  int reset_pin,
		  int busy_pin,
		  int chip_select_pin);

};

#endif
