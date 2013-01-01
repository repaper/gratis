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
#include <ctype.h>

#include "io.h"
#include "serial.h"
#include "spi.h"
#include "flash.h"
#include "button.h"
#include "delay.h"
#include "epd.h"


static void flash_info(void);

static uint16_t xbm_count;
static bool xbm_parser(uint8_t *b);


int main(void) {
	// stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// set up system clock
#if 16 == CLK_MHz
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;
#else
#error Unsupported clock frequency
#endif
	BCSCTL2 = 0;

	// initialise I/O
	P1DIR = PORT1_DIR;
	P1OUT = PORT1_INIT;
	P1SEL = 0;
	P1SEL2 = 0;

	P2DIR = PORT2_DIR;
	P2OUT = PORT2_INIT;
	P2SEL = 0;
	P2SEL2 = 0;


	Button_initialise();
	Serial_initialise();
	SPI_initialise();

	// CS high
	P2OUT |= PORT2_FLASH_CS;
	SPI_put(0xff);

	// pause to allow starting of the com program
	// as immediate send seems to cause USB lock-up
	Button_wait_click();

	// send a title
	Serial_puts("\n\nStarting\n\n");
	if (FLASH_initialise()) {
		Serial_puts("FLASH chip detected OK\n");
	} else {
		Serial_puts("unsupported FLASH chip\n");
		flash_info();
		for (;;) {
		}
	}

	for (;;) {
		Serial_puts("\nCommand: ");
		uint8_t c = Serial_getc();
		if (!isprint(c)) {
			continue;
		}
		Serial_putc(c);
		switch (c) {
		case 'h':
		case 'H':
		case '?':
			Serial_puts("\n"
				    "h          - this command\n"
				    "d<ss> <ll> - dump sector in hex, ll * 16 bytes\n"
				    "e<ss>      - erase sector to 0xff\n"
				    "u<ss>      - upload XBM to sector\n"
				    "i<ss>      - display an image on white screen\n"
				    "r<ss>      - revert an image back to white\n"
				    "l          - search for non-empty sectors\n"
				    "w          - clear screen to white\n"
				    "f          - dump FLASH identification\n");
			break;

		case 'd':
		{
			uint16_t sector = Serial_gethex(true);
			Serial_putc(' ');
			uint16_t count = Serial_gethex(true);
			if (0 == count) {
				count = 1;
			}
			Serial_puts("\nsector: ");
			Serial_puthex_word(sector);
			Serial_puts("\n");
			uint32_t address = sector;
			address <<= 12;
			for (uint16_t i = 0; i < count; ++i) {
				uint8_t buffer[16];
				FLASH_read(buffer, address, sizeof(buffer));
				Serial_puthex_dump(address, buffer, sizeof(buffer));
				address += sizeof(buffer);
			}
			break;
		}

		case 'e':
		{
			uint32_t sector = Serial_gethex(true);
			FLASH_write_enable();
			FLASH_sector_erase(sector << 12);
			while (FLASH_is_busy()) {
			}
			FLASH_write_disable();
			break;
		}

		case 'u':
		{
			uint32_t address = Serial_gethex(true);
			address <<= 12;
			xbm_count = 0;
			Serial_puts("\nstart upload...\n");

			for (;;) {
				uint8_t buffer[16];
				uint16_t count = 0;
				for (int i = 0; i < sizeof(buffer); ++i, ++count) {
					if (!xbm_parser(&buffer[i])) {
						break;
					}
				}
				FLASH_write_enable();
				FLASH_write(address, buffer, count);
				address += count;
				if (sizeof(buffer) != count) {
					break;
				}
			}

			FLASH_write_disable();


			Serial_puts("\n read = ");
			Serial_puthex_word(xbm_count);
			if (128 * 96 / 8 == xbm_count) {
				Serial_puts(" 128x96 1.44");
			} else if (200 * 96 / 8 == xbm_count) {
				Serial_puts(" 200x96 2.0");
			} else if (264L * 176 / 8 == xbm_count) {
				Serial_puts(" 264x176 2.7");
			} else {
				Serial_puts(" invalid image");
			}
			break;
		}

		case 'i':
		{
			uint32_t address = Serial_gethex(true);
			address <<= 12;
			EPD_type cog;
			const int frame_cycles = 5;
			EPD_initialise(&cog, EPD_SIZE);
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_cb(&cog, address, FLASH_read, EPD_inverse);
			}
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_cb(&cog, address, FLASH_read, EPD_normal);
			}
			EPD_finalise(&cog);
			break;
		}

		case 'r':
		{
			uint32_t address = Serial_gethex(true);
			address <<= 12;
			EPD_type cog;
			const int frame_cycles = 5;
			EPD_initialise(&cog, EPD_SIZE);
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_cb(&cog, address, FLASH_read, EPD_compensate);
			}
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_cb(&cog, address, FLASH_read, EPD_white);
			}
			EPD_finalise(&cog);
			break;
		}

		case 'l':
		{
			Serial_puts("\n");
			uint16_t per_line = 0;
			for (uint16_t sector = 0; sector <= 0xff; ++sector) {
				for (uint16_t i = 0; i < 4096; i += 16) {
					uint8_t buffer[16];
					uint32_t address = sector;
					address <<= 12;
					address += i;
					FLASH_read(buffer, address, sizeof(buffer));
					bool flag = false;
					for (uint16_t j = 0; j < sizeof(buffer); ++j) {
						if (0xff != buffer[j]) {
							flag = true;
							break;
						}
					}
					if (flag) {
						Serial_puthex_byte(sector);
						if (++per_line >= 16) {
							Serial_puts("\n");
							per_line = 0;
						} else {
							Serial_putc(' ');
						}
						break;
					}
				}

			}
			if (0 != per_line) {
				Serial_puts("\n");
			}
			break;
		}

		case 'f':
		{
			Serial_puts("\n");
			flash_info();
			break;
		}

		case 'w':
		{
			EPD_type cog;
			const int frame_cycles = 16;
			EPD_initialise(&cog, EPD_SIZE);
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_fixed(&cog, 0xff);  // all black
			}
			for (int i = 0; i < frame_cycles; ++i) {
				EPD_frame_fixed(&cog, 0xaa);  // all white
			}
			EPD_finalise(&cog);
			break;
		}

		default:
			Serial_puts("\nerror\n");

		}
	}
}



static void flash_info(void) {
	uint8_t maufacturer;
	uint16_t device;
	FLASH_info(&maufacturer, &device);
	Serial_puts("FLASH: manufacturer = ");
	Serial_puthex_byte(maufacturer);
	Serial_puts(" device = ");
	Serial_puthex_word(device);
	Serial_puts("\n");
}


static bool xbm_parser(uint8_t *b) {
	for (;;) {
		uint8_t c = Serial_getc();
		if ('0' == c && 'x' == Serial_getc()) {
			*b = Serial_gethex(false);
			++xbm_count;
			if (0 == (xbm_count & 0x07)) {
				P1OUT ^= PORT1_RED_LED;
			}
			return true;
		} else if (';' == c) {
			break;
		}
	}
	P1OUT &= ~PORT1_RED_LED;
	return false;
}
