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


#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <time.h>
#include <signal.h>

#include "gpio.h"
#include "spi.h"
#include "epd.h"

// delays - more consistent naming
#define Delay_ms(ms) usleep(1000 * (ms))
#define Delay_us(us) usleep(us)
#define LOW 0
#define HIGH 1
#define digitalRead(pin) GPIO_read(pin)
#define digitalWrite(pin, value) GPIO_write(pin, value)


// inline arrays
#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))


// types
typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;


// function prototypes
static void power_off(EPD_type *epd);
static void PWM_start(int pin);
static void PWM_stop(int pin);
static int temperature_to_factor_10x(int temperature);
static void frame_fixed(EPD_type *epd, uint8_t fixed_value, EPD_stage stage);
static void frame_data(EPD_type *epd, const uint8_t *image, const uint8_t *mask, EPD_stage stage);
static void frame_fixed_repeat(EPD_type *epd, uint8_t fixed_value, EPD_stage stage);
static void frame_data_repeat(EPD_type *epd, const uint8_t *image, const uint8_t *mask, EPD_stage stage);
static void line(EPD_type *epd, uint16_t line, const uint8_t *data, uint8_t fixed_value, const uint8_t *mask, EPD_stage stage);

// panel configuration
struct EPD_struct {
	int EPD_Pin_PANEL_ON;
	int EPD_Pin_BORDER;
	int EPD_Pin_DISCHARGE;
	int EPD_Pin_PWM;
	int EPD_Pin_RESET;
	int EPD_Pin_BUSY;

	EPD_size size;
	int stage_time;
	int factored_stage_time;
	int lines_per_display;
	int dots_per_line;
	int bytes_per_line;
	int bytes_per_scan;
	bool filler;

	EPD_error status;

	const uint8_t *channel_select;
	size_t channel_select_length;
	const uint8_t *gate_source;
	size_t gate_source_length;

	uint8_t *line_buffer;
	size_t line_buffer_size;

	timer_t timer;
	SPI_type *spi;
};


EPD_type *EPD_create(EPD_size size,
		     int panel_on_pin,
		     int border_pin,
		     int discharge_pin,
		     int pwm_pin,
		     int reset_pin,
		     int busy_pin,
		     SPI_type *spi) {

	// create a polled timer
	timer_t timer;
	struct sigevent event;
	event.sigev_notify = SIGEV_NONE;

	if (-1 == timer_create(CLOCK_REALTIME, &event, &timer)) {
		warn("falled to create timer");
		return NULL;
	}

	// allocate memory
	EPD_type *epd = malloc(sizeof(EPD_type));
	if (NULL == epd) {
		warn("falled to allocate EPD structure");
		return NULL;
	}

	epd->spi = spi;
	epd->timer = timer;

	epd->EPD_Pin_PANEL_ON = panel_on_pin;
	epd->EPD_Pin_BORDER = border_pin;
	epd->EPD_Pin_DISCHARGE = discharge_pin;
	epd->EPD_Pin_PWM = pwm_pin;
	epd->EPD_Pin_RESET = reset_pin;
	epd->EPD_Pin_BUSY = busy_pin;

	epd->size = size;
	epd->stage_time = 480; // milliseconds
	epd->lines_per_display = 96;
	epd->dots_per_line = 128;
	epd->bytes_per_line = 128 / 8;
	epd->bytes_per_scan = 96 / 4;
	epd->filler = false;


	// display size dependent items
	{
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};
		static uint8_t gs[] = {0x72, 0x03};
		epd->channel_select = cs;
		epd->channel_select_length = sizeof(cs);
		epd->gate_source = gs;
		epd->gate_source_length = sizeof(gs);
	}

	// set up size structure
	switch (size) {
	default:
	case EPD_1_44:  // default so no change
		break;

	case EPD_2_0: {
		epd->lines_per_display = 96;
		epd->dots_per_line = 200;
		epd->bytes_per_line = 200 / 8;
		epd->bytes_per_scan = 96 / 4;
		epd->filler = true;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00};
		static uint8_t gs[] = {0x72, 0x03};
		epd->channel_select = cs;
		epd->channel_select_length = sizeof(cs);
		epd->gate_source = gs;
		epd->gate_source_length = sizeof(gs);
		break;
	}

	case EPD_2_7: {
		epd->stage_time = 630; // milliseconds
		epd->lines_per_display = 176;
		epd->dots_per_line = 264;
		epd->bytes_per_line = 264 / 8;
		epd->bytes_per_scan = 176 / 4;
		epd->filler = true;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00};
		static uint8_t gs[] = {0x72, 0x00};
		epd->channel_select = cs;
		epd->channel_select_length = sizeof(cs);
		epd->gate_source = gs;
		epd->gate_source_length = sizeof(gs);
		break;
	}
	}

	epd->factored_stage_time = epd->stage_time;

	// buffer for frame line
	epd->line_buffer_size = 2 * epd->bytes_per_line + epd->bytes_per_scan
		+ 3; // command byte, border byte and filler byte

	epd->line_buffer = malloc(epd->line_buffer_size);
	if (NULL == epd->line_buffer) {
		free(epd);
		warn("falled to allocate EPD line buffer");
		return NULL;
	}

	// ensure I/O is all set to ZERO
	power_off(epd);

	return epd;
}


// deallocate memory
void EPD_destroy(EPD_type *epd) {
	if (NULL == epd) {
		return;
	}
	if (NULL != epd->line_buffer) {
		free(epd->line_buffer);
	}
	free(epd);
}


// read current status
EPD_error EPD_status(EPD_type *epd) {
	return epd->status;
}

// starts an EPD sequence
void EPD_begin(EPD_type *epd) {

	// assume OK
	epd->status = EPD_OK;

	// power up sequence
	digitalWrite(epd->EPD_Pin_RESET, LOW);
	digitalWrite(epd->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(epd->EPD_Pin_DISCHARGE, LOW);
	digitalWrite(epd->EPD_Pin_BORDER, LOW);

	SPI_on(epd->spi);

	PWM_start(epd->EPD_Pin_PWM);
	Delay_ms(25);
	digitalWrite(epd->EPD_Pin_PANEL_ON, HIGH);
	Delay_ms(10);

	digitalWrite(epd->EPD_Pin_RESET, HIGH);
	digitalWrite(epd->EPD_Pin_BORDER, HIGH);
	Delay_ms(5);

	digitalWrite(epd->EPD_Pin_RESET, LOW);
	Delay_ms(5);

	digitalWrite(epd->EPD_Pin_RESET, HIGH);
	Delay_ms(5);

	// wait for COG to become ready
	while (HIGH == digitalRead(epd->EPD_Pin_BUSY)) {
		Delay_us(10);
	}

	// channel select
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x01), 2);
	Delay_us(10);
	SPI_send(epd->spi, epd->channel_select, epd->channel_select_length);

	// DC/DC frequency
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x06), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0xff), 2);

	// high power mode osc
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x9d), 2);


	// disable ADC
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x08), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x00), 2);

	// Vcom level
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x09), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0xd0, 0x00), 3);

	// gate and source voltage levels
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, epd->gate_source, epd->gate_source_length);

	Delay_ms(5);  //???

	// driver latch on
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x01), 2);

	// driver latch off
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x00), 2);

	Delay_ms(5);

	// charge pump positive voltage on
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x01), 2);

	// final delay before PWM off
	Delay_ms(30);
	PWM_stop(epd->EPD_Pin_PWM);

	// charge pump negative voltage on
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x03), 2);

	Delay_ms(30);

	// Vcom driver on
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x0f), 2);

	Delay_ms(30);

	// output enable to disable
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x24), 2);

	SPI_off(epd->spi);
}


void EPD_end(EPD_type *epd) {

	// dummy frame
	frame_fixed(epd, 0x55, EPD_normal);

	// dummy line and border
	if (EPD_1_44 == epd->size) {
		// only for 1.44" EPD
		line(epd, 0x7fffu, 0, 0x55, NULL, EPD_normal);

		Delay_ms(250);

	} else {
		// all other display sizes
		line(epd, 0x7fffu, 0, 0x55, NULL, EPD_normal);

		Delay_ms(25);

		digitalWrite(epd->EPD_Pin_BORDER, LOW);
		Delay_ms(250);
		digitalWrite(epd->EPD_Pin_BORDER, HIGH);
	}

	SPI_on(epd->spi);

	// latch reset turn on
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x01), 2);

	// output enable off
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x05), 2);

	// Vcom power off
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x0e), 2);

	// power off negative charge pump
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x02), 2);

	// discharge
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x0c), 2);

	Delay_ms(120);

	// all charge pumps off
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x00), 2);

	// turn of osc
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x0d), 2);

	// discharge internal - 1
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x50), 2);

	Delay_ms(40);

	// discharge internal - 2
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0xA0), 2);

	Delay_ms(40);

	// discharge internal - 3
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x00), 2);
	Delay_us(10);

	power_off(epd);
}


static void power_off(EPD_type *epd) {

	// turn of power and all signals
	digitalWrite(epd->EPD_Pin_RESET, LOW);
	digitalWrite(epd->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(epd->EPD_Pin_BORDER, LOW);

	// ensure SPI MOSI and CLOCK are Low before CS Low
	SPI_off(epd->spi);

	// discharge pulse
	digitalWrite(epd->EPD_Pin_DISCHARGE, HIGH);
	Delay_ms(150);
	digitalWrite(epd->EPD_Pin_DISCHARGE, LOW);
}


void EPD_set_temperature(EPD_type *epd, int temperature) {
	epd->factored_stage_time = epd->stage_time * temperature_to_factor_10x(temperature) / 10;
}

// clear display (anything -> white)
void EPD_clear(EPD_type *epd) {
	frame_fixed_repeat(epd, 0xff, EPD_compensate);
	frame_fixed_repeat(epd, 0xff, EPD_white);
	frame_fixed_repeat(epd, 0xaa, EPD_inverse);
	frame_fixed_repeat(epd, 0xaa, EPD_normal);
}

// assuming a clear (white) screen output an image
void EPD_image_0(EPD_type *epd, const uint8_t *image) {
	frame_fixed_repeat(epd, 0xaa, EPD_compensate);
	frame_fixed_repeat(epd, 0xaa, EPD_white);
	frame_data_repeat(epd, image, NULL, EPD_inverse);
	frame_data_repeat(epd, image, NULL, EPD_normal);
}

// change from old image to new image
void EPD_image(EPD_type *epd, const uint8_t *old_image, const uint8_t *new_image) {
	frame_data_repeat(epd, old_image, NULL, EPD_compensate);
	frame_data_repeat(epd, old_image, NULL, EPD_white);
	frame_data_repeat(epd, new_image, NULL, EPD_inverse);
	frame_data_repeat(epd, new_image, NULL, EPD_normal);
}

// change from old image to new image
void EPD_partial_image(EPD_type *epd, const uint8_t *old_image, const uint8_t *new_image) {

	frame_data_repeat(epd, old_image, new_image, EPD_compensate);
	frame_data_repeat(epd, old_image, new_image, EPD_white);
	frame_data_repeat(epd, new_image, old_image, EPD_inverse);
	frame_data_repeat(epd, new_image, old_image, EPD_normal);
}


// internal functions
// ==================

// convert a temperature in Celsius to
// the scale factor for frame_*_repeat methods
static int temperature_to_factor_10x(int temperature) {
	if (temperature <= -10) {
		return 170;
	} else if (temperature <= -5) {
		return 120;
	} else if (temperature <= 5) {
		return 80;
	} else if (temperature <= 10) {
		return 40;
	} else if (temperature <= 15) {
		return 30;
	} else if (temperature <= 20) {
		return 20;
	} else if (temperature <= 40) {
		return 10;
	}
	return 7;
}


// One frame of data is the number of lines * rows. For example:
// The 1.44” frame of data is 96 lines * 128 dots.
// The 2” frame of data is 96 lines * 200 dots.
// The 2.7” frame of data is 176 lines * 264 dots.

// the image is arranged by line which matches the display size
// so smallest would have 96 * 32 bytes

static void frame_fixed(EPD_type *epd, uint8_t fixed_value, EPD_stage stage) {
	for (uint8_t l = 0; l < epd->lines_per_display ; ++l) {
		line(epd, l, NULL, fixed_value, NULL, stage);
	}
}


static void frame_data(EPD_type *epd, const uint8_t *image, const uint8_t *mask, EPD_stage stage) {
	if (NULL == mask) {
		for (uint8_t l = 0; l < epd->lines_per_display ; ++l) {
			line(epd, l, &image[l * epd->bytes_per_line], 0, NULL, stage);
		}
	} else {
		for (uint8_t l = 0; l < epd->lines_per_display ; ++l) {
			size_t n = l * epd->bytes_per_line;
			line(epd, l, &image[n], 0, &mask[n], stage);
		}
	}
}


static void frame_fixed_repeat(EPD_type *epd, uint8_t fixed_value, EPD_stage stage) {
	struct itimerspec its;
	its.it_value.tv_sec = epd->factored_stage_time / 1000;
	its.it_value.tv_nsec = (epd->factored_stage_time % 1000) * 1000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (-1 == timer_settime(epd->timer, 0, &its, NULL)) {
		err(1, "timer_settime failed");
	}
	do {
		frame_fixed(epd, fixed_value, stage);

		if (-1 == timer_gettime(epd->timer, &its)) {
			err(1, "timer_gettime failed");
		}
	} while (its.it_value.tv_sec > 0 || its.it_value.tv_nsec > 0);
}


static void frame_data_repeat(EPD_type *epd, const uint8_t *image, const uint8_t *mask, EPD_stage stage) {
	struct itimerspec its;
	its.it_value.tv_sec = epd->factored_stage_time / 1000;
	its.it_value.tv_nsec = (epd->factored_stage_time % 1000) * 1000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (-1 == timer_settime(epd->timer, 0, &its, NULL)) {
		err(1, "timer_settime failed");
	}
	do {
		frame_data(epd, image, mask, stage);
		if (-1 == timer_gettime(epd->timer, &its)) {
			err(1, "timer_gettime failed");
		}
	} while (its.it_value.tv_sec > 0 || its.it_value.tv_nsec > 0);
}


static void line(EPD_type *epd, uint16_t line, const uint8_t *data, uint8_t fixed_value, const uint8_t *mask, EPD_stage stage) {

	SPI_on(epd->spi);

	// charge pump voltage levels
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(epd->spi, epd->gate_source, epd->gate_source_length);

	// send data
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x0a), 2);
	Delay_us(10);

	// CS low
	uint8_t *p = epd->line_buffer;

	*p++ = 0x72;

	// border byte only necessary for 1.44" EPD
	if (EPD_1_44 == epd->size) {
		*p++ = 0x00;
	}

	// even pixels
	for (uint16_t b = epd->bytes_per_line; b > 0; --b) {
		if (NULL != data) {
			uint8_t pixels = data[b - 1] & 0xaa;
			uint8_t pixel_mask = 0xff;
			if (NULL != mask) {
				pixel_mask = (mask[b - 1] ^ pixels) & 0xaa;
				pixel_mask |= pixel_mask >> 1;
			}
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
			*p++ = (pixels & pixel_mask) | (~pixel_mask & 0x55);
		} else {
			*p++ = fixed_value;
		}
	}

	// scan line
	for (uint16_t b = 0; b < epd->bytes_per_scan; ++b) {
		if (line / 4 == b) {
			*p++ = 0xc0 >> (2 * (line & 0x03));
		} else {
			*p++ = 0x00;
		}
	}

	// odd pixels
	for (uint16_t b = 0; b < epd->bytes_per_line; ++b) {
		if (NULL != data) {
			uint8_t pixels = data[b] & 0x55;
			uint8_t pixel_mask = 0xff;
			if (NULL != mask) {
				pixel_mask = (mask[b] ^ pixels) & 0x55;
				pixel_mask |= pixel_mask << 1;
			}
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
			pixels = (pixels & pixel_mask) | (~pixel_mask & 0x55);

			uint8_t p1 = (pixels >> 6) & 0x03;
			uint8_t p2 = (pixels >> 4) & 0x03;
			uint8_t p3 = (pixels >> 2) & 0x03;
			uint8_t p4 = (pixels >> 0) & 0x03;
			pixels = (p1 << 0) | (p2 << 2) | (p3 << 4) | (p4 << 6);
			*p++ = pixels;
		} else {
			*p++ = fixed_value;
		}
	}

	if (epd->filler) {
		*p++ = 0x00;
	}
	// send the accumulated line buffer
	SPI_send(epd->spi, epd->line_buffer, p - epd->line_buffer);

	// output data to panel
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(epd->spi, CU8(0x72, 0x2f), 2);

	SPI_off(epd->spi);
}


static void PWM_start(int pin) {
	GPIO_pwm_write(pin, 511);
}


static void PWM_stop(int pin) {
	GPIO_pwm_write(pin, 0);
}
