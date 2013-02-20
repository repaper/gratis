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
#include "pwm.h"
#include "array.h"
#include "delay.h"
#include "epd.h"


void EPD_initialise(EPD_type *cog, EPD_size size) {
	cog->size = size;
	cog->lines_per_display = 96;
	cog->dots_per_line = 128;
	cog->bytes_per_line = 128 / 8;
	cog->bytes_per_scan = 96 / 4;
	cog->filler = false;

	// display size dependant items
	const uint8_t *channel_select = CU8(0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00);
	uint16_t channel_select_length = 9;
	cog->gate_source = CU8(0x72, 0x03);
	cog->gate_source_length = 2;

	// set up size structure
	switch (size) {
	default:
	case EPD_1_44:  // default so no change
		break;
	case EPD_2_0:
		cog->lines_per_display = 96;
		cog->dots_per_line = 200;
		cog->bytes_per_line = 200 / 8;
		cog->bytes_per_scan = 96 / 4;
		cog->filler = true;
		channel_select = CU8(0x72, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00);
		cog->gate_source = CU8(0x72, 0x03);

		break;
	case EPD_2_7:
		cog->lines_per_display = 176;
		cog->dots_per_line = 264;
		cog->bytes_per_line = 264 / 8;
		cog->bytes_per_scan = 176 / 4;
		cog->filler = true;
		channel_select = CU8(0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00);
		cog->gate_source = CU8(0x72, 0x00);
		break;
	}

	// power up sequence
	SPI_put(0x00);
	P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
	PWM_start();
	Delay_ms(5);
	P2OUT |= PORT2_PANEL_ON;
	Delay_ms(10);
	P2OUT |= (PORT2_RESET | PORT2_BORDER | PORT2_EPD_CS | PORT2_FLASH_CS);
	Delay_ms(5);
	P2OUT &= ~PORT2_RESET;
	Delay_ms(5);
	P2OUT |= PORT2_RESET;
	Delay_ms(5);

	// wait for COG to become ready
	while (0 != (P2IN & PORT2_BUSY)) {
	}

	// channel select
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x01), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, channel_select, channel_select_length);

	// DC/DC frequency
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x06), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0xff), 2);

	// high power mode osc
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x9d), 2);


	// disable ADC
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x08), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x00), 2);

	// Vcom level
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x09), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0xd0, 0x00), 3);

	// gate and source voltage levels
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, cog->gate_source, cog->gate_source_length);

	Delay_ms(5);  //???

	// driver latch on
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x01), 2);

	// driver latch off
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x00), 2);

	Delay_ms(5);

	// charge pump positive voltage on
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x01), 2);

	// final delay before PWM off
	Delay_ms(30);
	PWM_stop();

	// charge pump negative voltage on
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x03), 2);

	Delay_ms(30);

	// Vcom driver on
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x0f), 2);

	Delay_ms(30);

	// output enable to disable
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x24), 2);

}


//One frame of data is the number of lines * rows. For example:
//The 1.44” frame of data is 96 lines * 128 dots.
//The 2” frame of data is 96 lines * 200 dots.
//The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value) {
	for (uint8_t line = 0; line < cog->lines_per_display ; ++line) {
		EPD_line(cog, line, 0, fixed_value, EPD_normal);
	}
}

void EPD_frame_data(EPD_type *cog, const uint8_t *image, EPD_stage stage) {
	for (uint8_t line = 0; line < cog->lines_per_display; ++line) {
		EPD_line(cog, line, &image[line * cog->bytes_per_line], 0, stage);
	}
}

void EPD_frame_cb(EPD_type *cog, uint32_t address, EPD_reader *reader, EPD_stage stage) {
	static uint8_t buffer[264 / 8];
	for (uint8_t line = 0; line < cog->lines_per_display; ++line) {
		reader(buffer, address + line * cog->bytes_per_line, cog->bytes_per_line);
		EPD_line(cog, line, buffer, 0, stage);
	}
}


void EPD_line(EPD_type *cog, uint16_t line, const uint8_t *data, uint8_t fixed_value, EPD_stage stage) {
	// charge pump voltage levels
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	//SPI_send(PORT2_EPD_CS, CU8(0x72, 0x03), 2);
	SPI_send(PORT2_EPD_CS, cog->gate_source, cog->gate_source_length);

	// send data
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x0a), 2);
	Delay_us(10);

	// CS low
	P2OUT &= ~PORT2_EPD_CS;
	SPI_put_wait(0x72);

	// even pixels
	for (uint16_t b = cog->bytes_per_line; b > 0; --b) {
		if (0 != data) {
			uint8_t pixels = data[b - 1] & 0xaa;
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | (pixels ^ 0xaa);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | (pixels >> 1);
				break;
			}
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	// scan line
	for (uint16_t b = 0; b < cog->bytes_per_scan; ++b) {
		if (line / 4 == b) {
			SPI_put_wait(0xc0 >> (2 * (line & 0x03)));
		} else {
			SPI_put_wait(0x00);
		}
	}

	// odd pixels
	for (uint16_t b = 0; b < cog->bytes_per_line; ++b) {
		if (0 != data) {
			uint8_t pixels = data[b] & 0x55;
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | (pixels ^ 0x55);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + (pixels ^ 0x55);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | ((pixels ^ 0x55) << 1);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | pixels;
				break;
			}
			uint8_t p1 = (pixels >> 6) & 0x03;
			uint8_t p2 = (pixels >> 4) & 0x03;
			uint8_t p3 = (pixels >> 2) & 0x03;
			uint8_t p4 = (pixels >> 0) & 0x03;
			pixels = (p1 << 0) | (p2 << 2) | (p3 << 4) | (p4 << 6);
			SPI_put_wait(pixels);
		} else {
			SPI_put_wait(fixed_value);
		}
	}

	if (cog->filler) {
		SPI_put_wait(0x00);
	}

	// CS high

	P2OUT |= PORT2_EPD_CS;

	// output data to panel
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x2f), 2);
}


void EPD_finalise(EPD_type *cog) {

	EPD_frame_fixed(cog, 0x55); // dummy frame
	//EPD_frame_fixed(cog, 0x00); // dummy frame
	EPD_line(cog, 0x7fffu, 0, 0x55, EPD_normal); // dummy_line
	//EPD_line(cog, 0x7fffu, 0, 0x00); // dummy_line

	Delay_ms(25);

	P2OUT &= ~PORT2_BORDER;
	Delay_ms(30);
	P2OUT |= PORT2_BORDER;

	// latch reset turn on
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x01), 2);

	// output enable off
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x05), 2);

	// Vcom power off
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x0e), 2);

	// power off negative charge pump
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x02), 2);

	// discharge
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x0c), 2);

	Delay_ms(120);

	// all charge pumps off
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of osc
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x0d), 2);

	// discharge internal - 1
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x50), 2);

	Delay_ms(40);

	// discharge internal - 2
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0xA0), 2);

	Delay_ms(40);

	// discharge internal - 3
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(PORT2_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of power and all signals
	P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
	P2OUT |= PORT2_DISCHARGE;
	SPI_put(0x00);

	Delay_ms(150);
	P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
}
