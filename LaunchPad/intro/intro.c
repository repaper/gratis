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
// * wait for S2 button to be pressed and released
// * clear the display
// * wait for S2 button to be pressed and released
// * display the first image (assuming screen is already white)
// * wait for S2 button to be pressed and released
// * remove the first image and display second image
// * repeat from start
//
// Notes: LED1 will flash while the program waits for a button press
//        LED1 will be off while display is updating

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

const
#include TEXT_IMAGE

const
#include PICTURE

#if !defined(CLK_MHz)
#define CLK_MHz 16
#endif

#define SPI_BPS 10000000


// IO layout
// Port 1
#define PORT1_RED_LED     BIT0
#define PORT1_RXD         BIT1
#define PORT1_TXD         BIT2
#define PORT1_SW2         BIT3
#define PORT1_TEMPERATURE BIT4
#define PORT1_SPI_CLK     BIT5
#define PORT1_SPI_MISO    BIT6
#define PORT1_SPI_MOSI    BIT7

// Port 2
#define PORT2_PWM         BIT0
#define PORT2_BUSY        BIT1
#define PORT2_RESET       BIT2
#define PORT2_PANEL_ON    BIT3
#define PORT2_DISCHARGE   BIT4
#define PORT2_BORDER      BIT5
#define PORT2_EPD_CS      BIT6
#define PORT2_FLASH_CS    BIT7

#define PORT1_DIR   (PORT1_RED_LED)
#define PORT2_DIR   (PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS | PORT2_FLASH_CS)

#define PORT1_INIT  0
#define PORT2_INIT  (PORT2_FLASH_CS)

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

void Power_on(void);
void EPD_initialise(EPD_type *cog, EPD_size size);
void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value);
void EPD_frame_data(EPD_type *cog, const uint8_t *image, EPD_stage stage);
void EPD_line(EPD_type *cog, uint16_t line, const uint8_t *data, uint8_t fixed_value, EPD_stage stage);
void EPD_finalise(EPD_type *cog);

void SPI_initialise(void);
void SPI_put(uint8_t c);
void SPI_put_wait(uint8_t c);
void SPI_send(uint8_t port2_cs_pin, const uint8_t *buffer, uint16_t length);

void Delay_us(unsigned int delay);
void Delay_ms(unsigned int delay);

void PWM_start(void);
void PWM_stop(void);

void Button_initialise(void);
void Button_wait_click(void);



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
	SPI_initialise();

	EPD_type cog;

	for (int state = 0; ; ++state) {
		int frame_cycles = 8;
		Button_wait_click();

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
		} else {
			state = -1;
		}

		EPD_finalise(&cog);
	}
}


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
		//Delay_ms(20);
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


void Delay_us(unsigned int delay) {
	TA0CTL |= TACLR;                                    // counter reset

	TA0CCR0 = delay * 2;                                // period
	TA0CCR1 = 0;
	TA0CCR2 = 0;

	TA0CCTL0 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA0CCTL1 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled
	TA0CCTL2 = OUTMOD_0 | CCIS_2 | CM_0;                // OUT, Capture GND, Capture disabled

	TA0CTL = TASSEL_2 | ID_3 | MC_1;                    // SMCLK, /8, up

	while (0 == (TA0CTL & TAIFG)) {                     // wait for end
		asm volatile ("\tnop");
	}
	TA0CTL = 0;                                         // stop the timer
}


void Delay_ms(unsigned int delay) {
	for (unsigned int i = 0; i < delay; ++i) {
		Delay_us(1000);
	}
}


#define TIMER1_OUT0 BIT0
//#define TIMER1_OUT1 BIT1
//#define TIMER1_OUT2 BIT2
#define TIMER1_IO_ENABLE (TIMER1_OUT0)

#define TIMER1_COUNTS 4000

void PWM_start(void) {
	P2DIR |= TIMER1_IO_ENABLE;                          // Timer_A3 outputs
	P2OUT &= ~TIMER1_IO_ENABLE;                         // set low (may need changing)

	P2SEL2 &= ~TIMER1_IO_ENABLE;                        // select pin function...
	P2SEL |= TIMER1_IO_ENABLE;                          // ...as timer output

	TA1CTL |= TACLR;                                    // counter reset

	TA1CTL = TASSEL_2 | ID_0 | MC_3;                    // SMCLK, /1, up/down
	TA1CCR0 = TIMER1_COUNTS;                            // period square wave on TA1.0
	TA1CCR1 = TIMER1_COUNTS / 2;                        // toggle point (subtract interlock delay) TA1.1
	TA1CCR2 = TIMER1_COUNTS / 2;                        // toggle point TA1.2

	TA1CCTL0 = OUTMOD_4 | CCIS_2 | CM_0;                // Toggle, Capture GND, Capture disabled
	TA1CCTL1 = OUTMOD_3 | CCIS_2 | CM_0;                // PWM toggle/reset, Capture GND, Capture disabled
	TA1CCTL2 = OUTMOD_6 | CCIS_2 | CM_0;                // PWM toggle/set, Capture GND, Capture disabled
}


void PWM_stop(void) {
	P2DIR |= TIMER1_IO_ENABLE;                          // Timer_A3 outputs
	P2OUT &= ~TIMER1_IO_ENABLE;                         // set low (may need changing)

	P2SEL2 &= ~TIMER1_IO_ENABLE;                        // select pin function...
	P2SEL &= ~TIMER1_IO_ENABLE;                         // ...as normal output

	TA1CTL = TACLR;                                     // counter stop
}



void Button_initialise(void) {
	// enable pull-up on button pin
	// as button pulls to ground
	P1REN = PORT1_SW2;
	P1OUT |= PORT1_SW2; // pull up
}


void Button_wait_state(int loop, bool state) {
	for (;;) {
		for (int i = 0; i < loop; ++i) {
			if (state == (0 == (P1IN & PORT1_SW2))) {
				Delay_ms(10);  // debounce
				if (state == (0 == (P1IN & PORT1_SW2))) {
					P1OUT &= ~PORT1_RED_LED;
					return;
				}
			}
			Delay_ms(20);
		}
		P1OUT ^= PORT1_RED_LED;
	}
}


void Button_wait_click(void) {
	Button_wait_state(30, true); // wait for press
	Button_wait_state(5, false); // wait for release
}
