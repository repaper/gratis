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


#if !defined(SPI_BPS)
#define SPI_BPS 10000000
#endif


#define SPI_BR_VALUE (1000000 * CLK_MHz / SPI_BPS)
#define SPI_BR_HIGH ((SPI_BR_VALUE >> 8) & 0xff)
#define SPI_BR_LOW ((SPI_BR_VALUE >> 0) & 0xff)

#if SPI_BR_VALUE <= 0
#error SPI_BPS is to high
#endif

#define SPI_CLK BIT5
#define SPI_SOMI BIT6
#define SPI_SIMO BIT7

#define SPI_IO_ENABLE (SPI_CLK | SPI_SOMI | SPI_SIMO)


void SPI_initialise(void) {
	P1SEL2 |= SPI_IO_ENABLE;                            // select pin function...
	P1SEL |= SPI_IO_ENABLE;                             // ...as SPI

	UCB0CTL1 |= UCSWRST;                                // reset the module

	UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC;  // 8 bit, 3-wire, sync
	UCB0CTL1 = UCSSEL_2 | UCSWRST;                      // CLK = MCLK, reset the module
	UCB0BR0 = SPI_BR_LOW;                               // low byte
	UCB0BR1 = SPI_BR_HIGH;                              // high byte

	IE2 &= UCB0TXIE | UCB0RXIE;                         // disable interrupts

	UCB0CTL1 &= ~UCSWRST;                               // reset completed
}


void SPI_put(uint8_t c) {
	while (0 == (IFG2 & UCB0TXIFG)) {
	}
	UCB0TXBUF = c;
}


uint8_t SPI_get(void) {
	while (0 == (IFG2 & UCB0RXIFG)) {
	}
	return UCB0RXBUF;
}


uint8_t SPI_transfer(uint8_t c) {
	SPI_put(c);
	return SPI_get();
}


// only for EPD use
void SPI_put_wait(uint8_t c) {
	while (0 == (IFG2 & UCB0TXIFG)) {
	}
	UCB0TXBUF = c;

	// wait for last byte to clear SPI
	while (0 != (UCB0STAT & UCBUSY)) {
	}
	// wait for COG ready
	while (0 != (P2IN & PORT2_BUSY)) {
	}
}


void SPI_send(uint8_t port2_cs_pin, const uint8_t *buffer, uint16_t length) {
	// CS low
	P2OUT &= ~port2_cs_pin;

	// send all data
	for (uint16_t i = 0; i < length; ++i) {
		while (0 == (IFG2 & UCB0TXIFG)) {
		}
		UCB0TXBUF = *buffer++;
	}

	// wait for last byte to clear SPI
	while (0 != (UCB0STAT & UCBUSY)) {
	}

	// CS high
	P2OUT |= port2_cs_pin;
}
