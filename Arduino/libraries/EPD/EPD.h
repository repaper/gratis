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

#if !defined(EPD_H)
#define EPD_H 1

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <SPI.h>


typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef void EPD_reader(void *buffer, uint32_t address, uint16_t length);

class EPD_Class {
private:
	SPIClass &SPI;
	int EPD_Pin_EPD_CS;
	int EPD_Pin_PANEL_ON;
	int EPD_Pin_BORDER;
	int EPD_Pin_DISCHARGE;
	int EPD_Pin_PWM;
	int EPD_Pin_RESET;
	int EPD_Pin_BUSY;

	EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;
	PROGMEM const prog_uint8_t *gate_source;
	uint16_t gate_source_length;
	PROGMEM const prog_uint8_t *channel_select;
	uint16_t channel_select_length;

	bool filler;

public:
	void frame_fixed(uint8_t fixed_value);
	//void frame_data_p(PROGMEM const prog_uint8_t *image, EPD_stage stage);
	void frame_cb(uint32_t address, EPD_reader *reader, EPD_stage stage);

	void line(uint16_t line, const uint8_t *data, uint8_t fixed_value, EPD_stage stage);

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	void begin();
	void end();

	EPD_Class(EPD_size size,
		  int panel_on_pin,
		  int border_pin,
		  int discharge_pin,
		  int pwm_pin,
		  int reset_pin,
		  int busy_pin,
		  int chip_select_pin,
		  SPIClass &SPI_driver);

};

#endif


