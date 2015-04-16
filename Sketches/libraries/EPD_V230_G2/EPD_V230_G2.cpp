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


#include <Arduino.h>
#include <limits.h>

#include <SPI.h>

#include "EPD_V230_G2.h"

// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)

// inline arrays
#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))

// values for border byte
#define BORDER_BYTE_BLACK 0xff
#define BORDER_BYTE_WHITE 0xaa
#define BORDER_BYTE_NULL  0x00

static void SPI_on(void);
static void SPI_off(void);
static void SPI_put(uint8_t c);
static void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);
static uint8_t SPI_read(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);


EPD_Class::EPD_Class(EPD_size _size,
		     uint8_t panel_on_pin,
		     uint8_t border_pin,
		     uint8_t discharge_pin,
		     uint8_t reset_pin,
		     uint8_t busy_pin,
		     uint8_t chip_select_pin) :
	EPD_Pin_PANEL_ON(panel_on_pin),
	EPD_Pin_BORDER(border_pin),
	EPD_Pin_DISCHARGE(discharge_pin),
	EPD_Pin_RESET(reset_pin),
	EPD_Pin_BUSY(busy_pin),
	EPD_Pin_EPD_CS(chip_select_pin),
	size(_size) {

	this->lines_per_display = 96;
	this->dots_per_line = 128;
	this->bytes_per_line = 128 / 8;
	this->bytes_per_scan = 96 / 4;
	this->voltage_level = 0x03;

	this->setFactor(); // ensure default temperature

	// display size dependant items
	{
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
	}

	// set up size structure
	switch (size) {
	default:
	case EPD_1_44:  // default so no change
		break;

	case EPD_2_0: {
		this->lines_per_display = 96;
		this->dots_per_line = 200;
		this->bytes_per_line = 200 / 8;
		this->bytes_per_scan = 96 / 4;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
		this->voltage_level = 0x03;
		break;
	}

	case EPD_2_7: {
		this->lines_per_display = 176;
		this->dots_per_line = 264;
		this->bytes_per_line = 264 / 8;
		this->bytes_per_scan = 176 / 4;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
		this->voltage_level = 0x00;
		break;
	}
	}

}


void  EPD_Class::setFactor(int temperature) {
	// stage1: repeat, step, block
	// stage2: repeat, t1, t2
	// stage3: repeat, step, block

	static const EPD_Class::compensation_type compensation_144[3] = {
		{ 2, 6, 42,   4, 392, 392,   2, 6, 42 },  //  0 ... 10 Celcius
		{ 4, 2, 16,   4, 155, 155,   4, 2, 16 },  // 10 ... 40 Celcius
		{ 4, 2, 16,   4, 155, 155,   4, 2, 16 }   // 40 ... 50 Celcius
	};

	static const EPD_Class::compensation_type compensation_200[3] = {
		{ 2, 6, 42,   4, 392, 392,   2, 6, 42 },  //  0 ... 10 Celcius
		{ 2, 2, 48,   4, 196, 196,   2, 2, 48 },  // 10 ... 40 Celcius
		{ 4, 2, 48,   4, 196, 196,   4, 2, 48 }   // 40 ... 50 Celcius
	};

	static const EPD_Class::compensation_type compensation_270[3] = {
		{ 2, 8, 64,   4, 392, 392,   2, 8, 64 },  //  0 ... 10 Celcius
		{ 2, 4, 32,   4, 196, 196,   2, 4, 32 },  // 10 ... 40 Celcius
		{ 4, 8, 64,   4, 196, 196,   4, 8, 64 }   // 40 ... 50 Celcius
	};

	if (temperature < 10) {
		this->temperature_offset = 0;
	} else if (temperature > 40) {
		this->temperature_offset = 2;
	} else {
		this->temperature_offset = 1;
	}
	switch (this->size) {
	default:
	case EPD_1_44:
		this->compensation = &compensation_144[this->temperature_offset];
		break;

	case EPD_2_0: {
		this->compensation = &compensation_200[this->temperature_offset];
		break;
	}

	case EPD_2_7: {
		this->compensation = &compensation_270[this->temperature_offset];
		break;
	}
	}
}


void EPD_Class::begin(void) {

	// assume ok
	this->status = EPD_OK;

	// power up sequence
	digitalWrite(this->EPD_Pin_RESET, LOW);
	digitalWrite(this->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(this->EPD_Pin_DISCHARGE, LOW);
	digitalWrite(this->EPD_Pin_BORDER, LOW);
	digitalWrite(this->EPD_Pin_EPD_CS, LOW);

	SPI_on();

	Delay_ms(5);
	digitalWrite(this->EPD_Pin_PANEL_ON, HIGH);
	Delay_ms(10);

	digitalWrite(this->EPD_Pin_RESET, HIGH);
	digitalWrite(this->EPD_Pin_BORDER, HIGH);
	digitalWrite(this->EPD_Pin_EPD_CS, HIGH);
	Delay_ms(5);

	digitalWrite(this->EPD_Pin_RESET, LOW);
	Delay_ms(5);

	digitalWrite(this->EPD_Pin_RESET, HIGH);
	Delay_ms(5);

	// wait for COG to become ready
	while (HIGH == digitalRead(this->EPD_Pin_BUSY)) {
		Delay_us(10);
	}

	// read the COG ID
	int cog_id = SPI_read(this->EPD_Pin_EPD_CS, CU8(0x71, 0x00), 2);
	cog_id = SPI_read(this->EPD_Pin_EPD_CS, CU8(0x71, 0x00), 2);

	if (0x02 != (0x0f & cog_id)) {
		this->status = EPD_UNSUPPORTED_COG;
		this->power_off();
		return;
	}

	// Disable OE
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x40), 2);

	// check breakage
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
	int broken_panel = SPI_read(this->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
	if (0x00 == (0x80 & broken_panel)) {
		this->status = EPD_PANEL_BROKEN;
		this->power_off();
		return;
	}

	// power saving mode
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0b), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x02), 2);

	// channel select
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x01), 2);
	SPI_send(this->EPD_Pin_EPD_CS, this->channel_select, this->channel_select_length);

	// high power mode osc
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0xd1), 2);

	// power setting
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x08), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x02), 2);

	// Vcom level
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x09), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0xc2), 2);

	// power setting
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

	// driver latch on
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// driver latch off
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	Delay_ms(5);

	bool dc_ok = false;

	for (int i = 0; i < 4; ++i) {
		// charge pump positive voltage on - VGH/VDL on
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

		Delay_ms(240);

		// charge pump negative voltage on - VGL/VDL on
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

		Delay_ms(40);

		// charge pump Vcom on - Vcom driver on
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x0f), 2);

		Delay_ms(40);

		// check DC/DC
		SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
		int dc_state = SPI_read(this->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
		if (0x40 == (0x40 & dc_state)) {
			dc_ok = true;
			break;
		}
	}
	if (!dc_ok) {
		this->status = EPD_DC_FAILED;
		this->power_off();
		return;
	}

	// output enable to disable
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x40), 2);

	SPI_off();
}


void EPD_Class::end(void) {

	this->nothing_frame();

	if (EPD_1_44 == this->size || EPD_2_0 == this->size) {
		this->border_dummy_line();
	}

	this->dummy_line();

	if (EPD_2_7 == this->size) {
		// only pulse border pin for 2.70" EPD
		digitalWrite(this->EPD_Pin_BORDER, LOW);
		Delay_ms(200);
		digitalWrite(this->EPD_Pin_BORDER, HIGH);
	}

	SPI_on();

	// check DC/DC
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0f), 2);
	int dc_state = SPI_read(this->EPD_Pin_EPD_CS, CU8(0x73, 0x00), 2);
	if (0x40 != (0x40 & dc_state)) {
		this->status = EPD_DC_FAILED;
		this->power_off();
		return;
	}

	// latch reset turn on
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// output enable off
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x05), 2);

	// power off charge pump Vcom
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

	// power off charge pump neg voltage
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	Delay_ms(240);

	// power off all charge pumps
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of osc
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// discharge internal on
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x83), 2);

	Delay_ms(30);

	// discharge internal off
	//SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	//SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	power_off();
}

void EPD_Class::power_off(void) {

	// turn of power and all signals
	digitalWrite(this->EPD_Pin_RESET, LOW);
	digitalWrite(this->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(this->EPD_Pin_BORDER, LOW);

	// ensure SPI MOSI and CLOCK are Low before CS Low
	SPI_off();
	digitalWrite(this->EPD_Pin_EPD_CS, LOW);

	// pulse discharge pin
	digitalWrite(this->EPD_Pin_DISCHARGE, HIGH);
	Delay_ms(150);
	digitalWrite(this->EPD_Pin_DISCHARGE, LOW);
}


// One frame of data is the number of lines * rows. For example:
// The 1.44” frame of data is 96 lines * 128 dots.
// The 2” frame of data is 96 lines * 200 dots.
// The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

void EPD_Class::frame_fixed_timed(uint8_t fixed_value, long stage_time) {
	do {
		unsigned long t_start = millis();
		for (uint8_t line = 0; line < this->lines_per_display ; ++line) {
			this->line(this->lines_per_display - line - 1, 0, fixed_value, false);
		}
		unsigned long t_end = millis();
		if (t_end > t_start) {
			stage_time -= t_end - t_start;
		} else {
			stage_time -= t_start - t_end + 1 + ULONG_MAX;
		}
	} while (stage_time > 0);
}



void EPD_Class::frame_fixed_13(uint8_t value, EPD_stage stage) {

	int repeat;
	int step;
	int block;
	if (EPD_inverse == stage) {  // stage 1
		repeat = this->compensation->stage1_repeat;
		step = this->compensation->stage1_step;
		block = this->compensation->stage1_block;
	} else {                     // stage 3
		repeat = this->compensation->stage3_repeat;
		step = this->compensation->stage3_step;
		block = this->compensation->stage3_block;
	}

	int total_lines = this->lines_per_display;

	for (int n = 0; n < repeat; ++n) {

		int block_begin = 0;
		int block_end = 0;

		while (block_begin < total_lines) {

			block_end += step;
			block_begin = block_end - block;
			if (block_begin < 0) {
				block_begin = 0;
			} else if (block_begin >= total_lines) {
				break;
			}

			bool full_block = (block_end - block_begin == block);

			for (int line = block_begin; line < block_end; ++line) {
				if (line >= total_lines) {
					break;
				}
				if (full_block && (line < (block_begin + step))) {
					this->line(line, 0, 0x00, false, EPD_normal);
				} else {
					this->line(line, 0, value, false, EPD_normal);
				}
			}
		}
	}
}


void EPD_Class::frame_data_13(const uint8_t *image, EPD_stage stage, bool read_progmem) {

	int repeat;
	int step;
	int block;
	if (EPD_inverse == stage) {  // stage 1
		repeat = this->compensation->stage1_repeat;
		step = this->compensation->stage1_step;
		block = this->compensation->stage1_block;
	} else {                     // stage 3
		repeat = this->compensation->stage3_repeat;
		step = this->compensation->stage3_step;
		block = this->compensation->stage3_block;
	}

	int total_lines = this->lines_per_display;

	for (int n = 0; n < repeat; ++n) {

		int block_begin = 0;
		int block_end = 0;

		while (block_begin < total_lines) {

			block_end += step;
			block_begin = block_end - block;
			if (block_begin < 0) {
				block_begin = 0;
			} else if (block_begin >= total_lines) {
				break;
			}

			bool full_block = (block_end - block_begin == block);

			for (int line = block_begin; line < block_end; ++line) {
				if (line >= total_lines) {
					break;
				}
				if (full_block && (line < (block_begin + step))) {
					this->line(line, 0, 0x00, false, EPD_normal);
				} else {
					this->line(line, &image[line * this->bytes_per_line], 0x00, read_progmem, stage);
				}
			}
		}
	}
}


void EPD_Class::frame_cb_13(uint32_t address, EPD_reader *reader, EPD_stage stage) {
	uint8_t buffer[264 / 8]; // allows for 2.70" panel
	int repeat;
	int step;
	int block;
	if (EPD_inverse == stage) {  // stage 1
		repeat = this->compensation->stage1_repeat;
		step = this->compensation->stage1_step;
		block = this->compensation->stage1_block;
	} else {                     // stage 3
		repeat = this->compensation->stage3_repeat;
		step = this->compensation->stage3_step;
		block = this->compensation->stage3_block;
	}

	int total_lines = this->lines_per_display;

	for (int n = 0; n < repeat; ++n) {

		int block_begin = 0;
		int block_end = 0;

		while (block_begin < total_lines) {

			block_end += step;
			block_begin = block_end - block;
			if (block_begin < 0) {
				block_begin = 0;
			} else if (block_begin >= total_lines) {
				break;
			}

			bool full_block = (block_end - block_begin == block);

			for (int line = block_begin; line < block_end; ++line) {
				if (line >= total_lines) {
					break;
				}
				if (full_block && (line < (block_begin + step))) {
					this->line(line, 0, 0x00, false, EPD_normal);
				} else {
					reader(buffer, address + line * this->bytes_per_line, this->bytes_per_line);
					this->line(line, buffer, 0, false, stage);
				}
			}
		}
	}
}


void EPD_Class::frame_stage2(void) {
	for (uint16_t i = 0; i < this->compensation->stage2_repeat; ++i) {
		this->frame_fixed_timed(0xff, this->compensation->stage2_t1);
		this->frame_fixed_timed(0xaa, this->compensation->stage2_t2);
	}
}


void EPD_Class::nothing_frame(void) {
	for (uint16_t line = 0; line < this->lines_per_display; ++line) {
		this->line(line, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_NULL, true);
	}
}


void EPD_Class::dummy_line(void) {
	this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_NULL, true);
}


void EPD_Class::border_dummy_line(void) {
	this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_BLACK);
	Delay_ms(40);
	this->line(0x7fffu, 0, 0x00, false, EPD_normal, EPD_BORDER_BYTE_WHITE);
	Delay_ms(200);
}


void EPD_Class::line(uint16_t line, const uint8_t *data, uint8_t fixed_value,
		     bool read_progmem, EPD_stage stage, uint8_t border_byte,
		     bool set_voltage_limit) {

       SPI_on();

       if (set_voltage_limit) {
	       // charge pump voltage level reduce voltage shift
	       SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	       const uint8_t b[2] = {0x72, this->voltage_level};
	       SPI_send(this->EPD_Pin_EPD_CS, b, sizeof(b));
       }

       // send data
       SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0a), 2);

       Delay_us(10);

       // CS low
       digitalWrite(this->EPD_Pin_EPD_CS, LOW);
       SPI_put(0x72);

       // border byte
       SPI_put(border_byte);

       // odd pixels
       for (uint16_t b = this->bytes_per_line; b > 0; --b) {
	       if (0 != data) {
#if !defined(__AVR__)
		       uint8_t pixels = data[b - 1];
#else
		       // AVR has multiple memory spaces
		       uint8_t pixels;
		       if (read_progmem) {
			       pixels = pgm_read_byte_near(data + b - 1);
		       } else {
			       pixels = data[b - 1];
		       }
#endif
		       switch(stage) {
		       case EPD_inverse:      // B -> W, W -> B
			       pixels ^= 0xff;
			       break;
		       case EPD_normal:       // B -> B, W -> W
			       break;
		       }
		       pixels = 0xaa | pixels;
		       SPI_put(pixels);
	       } else {
		       SPI_put(fixed_value);
	       }
       }

       // scan line
       int scan_pos = (this->lines_per_display - line - 1) >> 2;
       int scan_shift = (line & 0x03) << 1;
       for (unsigned int b = 0; b < this->bytes_per_scan; ++b) {
	       if (scan_pos == (int) b) {
		       SPI_put(0x03 << scan_shift);
	       } else {
		       SPI_put(0x00);
	       }
       }

       // even pixels
       for (uint16_t b = 0; b < this->bytes_per_line; ++b) {
	       if (0 != data) {
#if !defined(__AVR__)
		       uint8_t pixels = data[b];
#else
		       // AVR has multiple memory spaces
		       uint8_t pixels;
		       if (read_progmem) {
			       pixels = pgm_read_byte_near(data + b);
		       } else {
			       pixels = data[b];
		       }
#endif
		       switch(stage) {
		       case EPD_inverse:      // B -> W, W -> B (Current Image)
			       pixels ^= 0xff;
			       break;
		       case EPD_normal:       // B -> B, W -> W (New Image)
			       break;
		       }
		       pixels >>= 1;
		       pixels |= 0xaa;

		       pixels = ((pixels & 0xc0) >> 6)
			       | ((pixels & 0x30) >> 2)
			       | ((pixels & 0x0c) << 2)
			       | ((pixels & 0x03) << 6);
		       SPI_put(pixels);
	       } else {
		       SPI_put(fixed_value);
	       }
       }

       // CS high
       digitalWrite(this->EPD_Pin_EPD_CS, HIGH);

       // output data to panel
       SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
       SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x07), 2);

       SPI_off();
}


static void SPI_on(void) {
	SPI.end();
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI_put(0x00);
	SPI_put(0x00);
	Delay_us(10);
}


static void SPI_off(void) {
	// SPI.begin();
	// SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	// SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI_put(0x00);
	SPI_put(0x00);
	Delay_us(10);
	SPI.end();
}


static void SPI_put(uint8_t c) {
	SPI.transfer(c);
}


static void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
	// CS low
	digitalWrite(cs_pin, LOW);

	// send all data
	for (uint16_t i = 0; i < length; ++i) {
		SPI_put(*buffer++);
	}

	// CS high
	digitalWrite(cs_pin, HIGH);
}

// FIXME: What is the purpose of rbuffer?  It is set, but never used.
static uint8_t SPI_read(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
	// CS low
	digitalWrite(cs_pin, LOW);

	uint8_t rbuffer[4];
	uint8_t result = 0;

	// send all data
	for (uint16_t i = 0; i < length; ++i) {
		result = SPI.transfer(*buffer++);
		if (i < 4) {
			rbuffer[i] = result;
		}
	}

	// CS high
	digitalWrite(cs_pin, HIGH);
	return result;
}
