// -*- mode: c++ -*-
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


// This program is to illustrate the display operation
// as described in the datasheets.  It has no temperature compensation
// so the display should be at nominal 20..25 Celsius.  The code is in
// a simple linear fashion and all the delays are set to maximum, but the
// SPI clock is set lower than its limit.  Therfore the display sequence
// will be much slower than normal and all of the individual display stages
// be clearly visible.

// Operation from reset:
// * clear screen
// * delay 5 seconds (flash LED)
// * display text image
// * delay 5 seconds (flash LED)
// * display picture
// * delay 5 seconds (flash LED)
// * repeat from start


#include <inttypes.h>
#include <ctype.h>

#if defined(__MSP430_CPU__)
#define PROGMEM
#define LAUNCHPAD 1
#else
#include <avr/pgmspace.h>
#endif

#define HANDS_OFF(x) x

// Arduino libraries
#include <SPI.h>

// Add Images library to compiler path
#include <Images.h>  // this is just an empty file

// configure display size
#define TEXT_IMAGE "text_image_2_0.xbm"
#define TEXT_BITS text_image_2_0_bits

#define PICTURE "cat_2_0.xbm"
#define PICTURE_BITS cat_2_0_bits

#define EPD_SIZE EPD_2_0

#define prog_uint8_t uint8_t

// images
PROGMEM const
#define unsigned
#define char prog_uint8_t
#include TEXT_IMAGE
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char prog_uint8_t
#include PICTURE
#undef char
#undef unsigned


// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)


#if defined (LAUNCHPAD)

// TI LaunchPad IO layout
const int Pin_TEMPERATURE = A4;
const int Pin_PANEL_ON = P2_3;
const int Pin_BORDER = P2_5;
const int Pin_DISCHARGE = P2_4;
const int Pin_PWM = P2_1;
const int Pin_RESET = P2_2;
const int Pin_BUSY = P2_0;
const int Pin_EPD_CS = P2_6;
const int Pin_FLASH_CS = P2_7;
const int Pin_SW2 = P1_3;
const int Pin_RED_LED = P1_0;

#else

// Arduino IO layout
const int Pin_TEMPERATURE = A0;
const int Pin_PANEL_ON = 2;
const int Pin_BORDER = 3;
const int Pin_DISCHARGE = 4;
const int Pin_PWM = 5;
const int Pin_RESET = 6;
const int Pin_BUSY = 7;
const int Pin_EPD_CS = 8;
const int Pin_FLASH_CS = 9;
const int Pin_SW2 = 12;
const int Pin_RED_LED = 13;
#endif

// LED anode through resistor to I/O pin
// LED cathode to Ground
#define LED_ON  HIGH
#define LED_OFF LOW


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

typedef struct {
	EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;
	const uint8_t *gate_source;
	uint16_t gate_source_length;
	bool filler;
} EPD_type;


#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))


void EPD_initialise(EPD_type *cog, EPD_size size);
void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value);
void EPD_frame_data(EPD_type *cog, PROGMEM const prog_uint8_t *image, EPD_stage stage);
void EPD_line(EPD_type *cog, uint16_t line, PROGMEM const prog_uint8_t *data, uint8_t fixed_value, EPD_stage stage);
void EPD_finalise(EPD_type *cog);

void SPI_put(uint8_t c);
void SPI_put_wait(uint8_t c);
void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);

void PWM_start(void);
void PWM_stop(void);


// configure I/O port direction
void setup() {
  pinMode(Pin_RED_LED, OUTPUT);
  pinMode(Pin_SW2, INPUT_PULLUP);
  pinMode(Pin_TEMPERATURE, INPUT);
  pinMode(Pin_PWM, OUTPUT);
  pinMode(Pin_BUSY, INPUT);
  pinMode(Pin_RESET, OUTPUT);
  pinMode(Pin_PANEL_ON, OUTPUT);
  pinMode(Pin_DISCHARGE, OUTPUT);
  pinMode(Pin_BORDER, OUTPUT);
  pinMode(Pin_EPD_CS, OUTPUT);
  pinMode(Pin_FLASH_CS, OUTPUT);

  digitalWrite(Pin_RED_LED, LED_OFF);
  digitalWrite(Pin_PWM, LOW);
  digitalWrite(Pin_RESET, LOW);
  digitalWrite(Pin_PANEL_ON, LOW);
  digitalWrite(Pin_DISCHARGE, LOW);
  digitalWrite(Pin_BORDER, LOW);
  digitalWrite(Pin_EPD_CS, LOW);
  digitalWrite(Pin_FLASH_CS, HIGH);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
}


static int state = 0;


// main loop
void loop() {
	EPD_type cog;

	int frame_cycles = 8;

	EPD_initialise(&cog, EPD_SIZE);

	if (0 == state) {
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_fixed(&cog, 0xff);  // all black
		}
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_fixed(&cog, 0xaa);  // all white
		}
	} else if (1 == state) {
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, TEXT_BITS, EPD_inverse);
		}
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, TEXT_BITS, EPD_normal);
		}
	} else if (2 == state) {
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, TEXT_BITS, EPD_compensate);
		}
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, TEXT_BITS, EPD_white);
		}
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, PICTURE_BITS, EPD_inverse);
		}
		for (int i = 0; i < frame_cycles; ++i) {
			EPD_frame_data(&cog, PICTURE_BITS, EPD_normal);
		}
		state = -1;
	}

	EPD_finalise(&cog);
	++state;

	// flash LED for 5 seconds
	for (int x = 0; x < 50; ++x) {
		  digitalWrite(Pin_RED_LED, LED_ON);
		  Delay_ms(50);
		  digitalWrite(Pin_RED_LED, LED_OFF);
		  Delay_ms(50);
	}
}


HANDS_OFF(void EPD_initialise(EPD_type *cog, EPD_size size)) {
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

	// P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_DISCHARGE, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);

	PWM_start();
	Delay_ms(5);
	// P2OUT |= PORT2_PANEL_ON;
	digitalWrite(Pin_PANEL_ON, HIGH);
	Delay_ms(10);
	//P2OUT |= (PORT2_RESET | PORT2_BORDER | PORT2_EPD_CS | PORT2_FLASH_CS);
	digitalWrite(Pin_RESET, HIGH);
	digitalWrite(Pin_BORDER, HIGH);
	digitalWrite(Pin_EPD_CS, HIGH);
	digitalWrite(Pin_FLASH_CS, HIGH);
	Delay_ms(5);
	//P2OUT &= ~PORT2_RESET;
	digitalWrite(Pin_RESET, LOW);
	Delay_ms(5);
	//P2OUT |= PORT2_RESET;
	digitalWrite(Pin_RESET, HIGH);
	Delay_ms(5);

	// wait for COG to become ready
	//while (0 != (P2IN & PORT2_BUSY)) {
	while (HIGH == digitalRead(Pin_BUSY)) {
	}

	// channel select
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x01), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, channel_select, channel_select_length);

	// DC/DC frequency
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x06), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0xff), 2);

	// high power mode osc
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x9d), 2);


	// disable ADC
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x08), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// Vcom level
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x09), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0xd0, 0x00), 3);

	// gate and source voltage levels
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, cog->gate_source, cog->gate_source_length);

	Delay_ms(5);  //???

	// driver latch on
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// driver latch off
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x00), 2);

	Delay_ms(5);

	// charge pump positive voltage on
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// final delay before PWM off
	Delay_ms(30);
	PWM_stop();

	// charge pump negative voltage on
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x03), 2);

	Delay_ms(30);

	// Vcom driver on
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x0f), 2);

	Delay_ms(30);

	// output enable to disable
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x24), 2);

}


//One frame of data is the number of lines * rows. For example:
//The 1.44” frame of data is 96 lines * 128 dots.
//The 2” frame of data is 96 lines * 200 dots.
//The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

HANDS_OFF(void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value)) {
	for (uint8_t line = 0; line < cog->lines_per_display ; ++line) {
		EPD_line(cog, line, 0, fixed_value, EPD_normal);
	}
}

HANDS_OFF(void EPD_frame_data(EPD_type *cog, PROGMEM const prog_uint8_t *image, EPD_stage stage)) {
	for (uint8_t line = 0; line < cog->lines_per_display; ++line) {
		EPD_line(cog, line, &image[line * cog->bytes_per_line], 0, stage);
		//Delay_ms(20);
	}
}


HANDS_OFF(void EPD_line(EPD_type *cog, uint16_t line, PROGMEM const prog_uint8_t *data, uint8_t fixed_value, EPD_stage stage)) {
	// charge pump voltage levels
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	//SPI_send(Pin_EPD_CS, CU8(0x72, 0x03), 2);
	SPI_send(Pin_EPD_CS, cog->gate_source, cog->gate_source_length);

	// send data
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x0a), 2);
	Delay_us(10);

	// CS low
	digitalWrite(Pin_EPD_CS, LOW);
	SPI_put_wait(0x72);

	// even pixels
	for (uint16_t b = cog->bytes_per_line; b > 0; --b) {
		if (0 != data) {
#if defined(LAUNCHPAD)
			uint8_t pixels = data[b - 1] & 0xaa;
#else
			uint8_t pixels = pgm_read_byte_near(data + b - 1) & 0xaa;
#endif
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
		}	}

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
#if defined(LAUNCHPAD)
			uint8_t pixels = data[b] & 0x55;
#else
			uint8_t pixels = pgm_read_byte_near(data + b) & 0x55;
#endif
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
	digitalWrite(Pin_EPD_CS, HIGH);

	// output data to panel
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x2f), 2);
}


HANDS_OFF(void EPD_finalise(EPD_type *cog)) {

	EPD_frame_fixed(cog, 0x55); // dummy frame
	//EPD_frame_fixed(cog, 0x00); // dummy frame
	EPD_line(cog, 0x7fffu, 0, 0x55, EPD_normal); // dummy_line
	//EPD_line(cog, 0x7fffu, 0, 0x00); // dummy_line

	Delay_ms(25);

	// P2OUT &= ~PORT2_BORDER;
	digitalWrite(Pin_BORDER, LOW);
	Delay_ms(30);
	// P2OUT |= PORT2_BORDER;
	digitalWrite(Pin_BORDER, HIGH);

	// latch reset turn on
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// output enable off
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x05), 2);

	// Vcom power off
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x0e), 2);

	// power off negative charge pump
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x02), 2);

	// discharge
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x0c), 2);

	Delay_ms(120);

	// all charge pumps off
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of osc
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x0d), 2);

	// discharge internal - 1
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x50), 2);

	Delay_ms(40);

	// discharge internal - 2
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0xA0), 2);

	Delay_ms(40);

	// discharge internal - 3
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of power and all signals
	//P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
	digitalWrite(Pin_RESET, LOW);
	digitalWrite(Pin_PANEL_ON, LOW);
	digitalWrite(Pin_BORDER, LOW);
	digitalWrite(Pin_EPD_CS, LOW);
	//P2OUT |= PORT2_DISCHARGE;
	digitalWrite(Pin_DISCHARGE, HIGH);

	SPI_put(0x00);

	Delay_ms(150);
	// P2OUT &= ~(PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS);
	digitalWrite(Pin_DISCHARGE, LOW);
}


void SPI_put(uint8_t c) {
	SPI.transfer(c);
}


void SPI_put_wait(uint8_t c) {

	SPI_put(c);

	// wait for COG ready
	while (HIGH == digitalRead(Pin_BUSY)) {
	}
}


void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
	// CS low
	digitalWrite(cs_pin, LOW);

	// send all data
	for (uint16_t i = 0; i < length; ++i) {
		SPI_put(*buffer++);
	}

	// CS high
	digitalWrite(cs_pin, HIGH);
}


void PWM_start(void) {
	analogWrite(Pin_PWM, 128);  // 50% duty cycle
}


void PWM_stop(void) {
	analogWrite(Pin_PWM, 0);
}
