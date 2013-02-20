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
#include "serial.h"


#if !defined(UART_BPS)
#define UART_BPS 9600
#endif


#define SERIAL_BR_VALUE (1000000 * CLK_MHz / UART_BPS)
#define SERIAL_BR_HIGH ((SERIAL_BR_VALUE >> (8 + 4)) & 0xff)
#define SERIAL_BR_LOW ((SERIAL_BR_VALUE >> (0 + 4)) & 0xff)
#define SERIAL_BR_FRACTION (SERIAL_BR_VALUE & 0x0f)

#define SERIAL_RXD BIT1
#define SERIAL_TXD BIT2

#define SERIAL_IO_ENABLE (SERIAL_RXD | SERIAL_TXD)


static void Serial_puthex(uint32_t n, uint16_t bits);


void Serial_initialise(void) {
	P1SEL2 |= SERIAL_IO_ENABLE;                         // select pin function...
	P1SEL |= SERIAL_IO_ENABLE;                          // ...as USCI

	UCA0CTL1 |= UCSWRST;                                // reset the module

	UCA0CTL0 = 0;                                       // 8 bit, no parity, async
	UCA0CTL1 = UCSSEL_2 | UCSWRST;                      // CLK = MCLK, reset the module
	UCA0BR0 = SERIAL_BR_LOW;                            // low byte
	UCA0BR1 = SERIAL_BR_HIGH;                           // high byte
	UCA0MCTL = (SERIAL_BR_FRACTION << 4) | UCOS16;      // oversampling mode
	UCA0ABCTL = 0;                                      // auto-detect baud-rate
	UCA0IRTCTL = 0;                                     // disable IRDA transmit
	UCA0IRRCTL = 0;                                     // disable IRDA receive

	IE2 &= UCA0TXIE | UCA0RXIE;                         // diusable interrupts

	UCA0CTL1 &= ~UCSWRST;                               // reset completed
}


uint8_t Serial_getc(void) {
	while (0 == (IFG2 & UCA0RXIFG)) {
	}
	return UCA0RXBUF;
}


uint16_t Serial_gethex(bool echo) {
	uint16_t acc = 0;
	for (;;) {

		uint8_t c = Serial_getc();
		if (c >= '0' && c <= '9') {
			c -= '0';
		} else if (c >= 'a' && c <= 'f') {
			c -= 'a' - 10;
		} else if (c >= 'A' && c <= 'F') {
			c -= 'A' - 10;
		} else {
			return acc;
		}
		if (echo) {
			Serial_puthex(c, 4);
		}
		acc <<= 4;
		acc += c;
	}
}


void Serial_putc(uint8_t c) {
	while (0 == (IFG2 & UCA0TXIFG)) {
	}
	UCA0TXBUF = c;
}


void Serial_puts(const char *s) {
	while ('\0' != *s) {
		uint8_t c = *s++;
		if ('\n' == c) {
			Serial_putc('\r');
			Serial_putc('\n');
		} else {
			Serial_putc(c);
		}
	}
}


static void Serial_puthex(uint32_t n, uint16_t bits) {
	static const char hex[16] = "0123456789abcdef";
	for (int i = bits - 4; i >= 0; i -= 4) {
		Serial_putc(hex[(n >> i) & 0x0f]);
	}
}


void Serial_puthex_byte(uint8_t n) {
	Serial_puthex(n, 8);
}


void Serial_puthex_word(uint16_t n) {
	Serial_puthex(n, 16);
}


void Serial_puthex_double(uint32_t n) {
	Serial_puthex(n, 32);
}


void Serial_puthex_dump(uint32_t address, const void *buffer, uint16_t length) {
	const uint8_t *p = (const uint8_t *)buffer;
	while (0 != length) {
		Serial_puthex_double(address);
		Serial_puts(": ");
		for (int i = 0; i < 16; ++i, ++address) {
			Serial_puthex_byte(*p++);
			Serial_putc(' ');
			if (7 == i) {
				Serial_putc(' ');
			}
			if (--length == 0) {
				break;
			}
		}
		Serial_puts("\n");
	}
}
