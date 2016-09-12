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


// For the BCM SOC Preipheral Manual register layout see:
//   http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// also see: http://elinux.org/RPi_Low-level_peripherals
//
// Other items are difficult to determine accurately. Google search "Gert Doms" for some samples


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <err.h>

#include "gpio.h"

// register addresses in Rasberry PI
enum {
	V1_BCM_PERIPHERALS_ADDRESS = 0x20000000,
	V2_BCM_PERIPHERALS_ADDRESS = 0x3F000000,

	GPIO_REGISTERS         = 0x00200000,
	PWM_REGISTERS          = 0x0020C000,
	CLOCK_REGISTERS        = 0x00101000,
	TIMER_REGISTERS        = 0x0000B000
};

// GPIO module registers (all registers are 32 bit
// so simple numbering from 0 in steps of 1 is OK
// (Manual Chapter 6)
enum {                     // byte offset     function   bits  R/W
	GPFSEL0   = 0x00,  // 0x0000  GPIO Function Select 0   32 R/W
	GPFSEL1   = 0x01,  // 0x0004  GPIO Function Select 1   32 R/W
	GPFSEL2   = 0x02,  // 0x0008  GPIO Function Select 2   32 R/W
	GPFSEL3   = 0x03,  // 0x000C  GPIO Function Select 3   32 R/W

	GPFSEL4   = 0x04,  // 0x0010  GPIO Function Select 4   32 R/W
	GPFSEL5   = 0x05,  // 0x0014  GPIO Function Select 5   32 R/W
	//        = 0x06,  // 0x0018  (Reserved)
	GPSET0    = 0x07,  // 0x001C  GPIO Pin Output Set 0 32 W

	GPSET1    = 0x08,  // 0x0020  GPIO Pin Output Set 1 32 W
	//        = 0x09,  // 0x0024  (Reserved)
	GPCLR0    = 0x0a,  // 0x0028  GPIO Pin Output Clear 0 32 W
	GPCLR1    = 0x0b,  // 0x002C  GPIO Pin Output Clear 1 32 W

	//        = 0x0c,  // 0x0030  (Reserved)
	GPLEV0    = 0x0d,  // 0x0034  GPIO Pin Level 0 32 R
	GPLEV1    = 0x0e,  // 0x0038  GPIO Pin Level 1 32 R
	//        = 0x0f,  // 0x003C  (Reserved)

	GPEDS0    = 0x10,  // 0x0040  GPIO Pin Event Detect Status 0 32 R/W
	GPEDS1    = 0x11,  // 0x0044  GPIO Pin Event Detect Status 1 32 R/W
	//        = 0x12,  // 0x0048  (Reserved)
	GPREN0    = 0x13,  // 0x004C  GPIO Pin Rising Edge Detect Enable 0 32 R/W
	GPREN1    = 0x14,  // 0x0050  GPIO Pin Rising Edge Detect Enable 1 32 R/W
	//        = 0x15,  // 0x0054  (Reserved)
	GPFEN0    = 0x16,  // 0x0058  GPIO Pin Falling Edge Detect Enable 0 32 R/W
	GPFEN1    = 0x17,  // 0x005C  GPIO Pin Falling Edge Detect Enable 1 32 R/W

	//        = 0x18,  // 0x0060  (Reserved)
	GPHEN0    = 0x19,  // 0x0064  GPIO Pin High Detect Enable 0 32 R/W
	GPHEN1    = 0x1a,  // 0x0068  GPIO Pin High Detect Enable 1 32 R/W
	//        = 0x1b,  // 0x006C  (Reserved)

	GPLEN0    = 0x1c,  // 0x0070  GPIO Pin Low Detect Enable 0 32 R/W
	GPLEN1    = 0x1d,  // 0x0074  GPIO Pin Low Detect Enable 1 32 R/W
	//        = 0x1e,  // 0x0078  (Reserved)
	GPAREN0   = 0x1f,  // 0x007C  GPIO Pin Async. Rising Edge Detect 0 32 R/W

	GPAREN1   = 0x20,  // 0x0080  GPIO Pin Async. Rising Edge Detect 1 32 R/W
	//        = 0x21,  // 0x0084  (Reserved)
	GPAFEN0   = 0x22,  // 0x0088  GPIO Pin Async. Falling Edge Detect 0 32 R/W
	GPAFEN1   = 0x23,  // 0x008C  GPIO Pin Async. Falling Edge Detect 1 32 R/W

	//        = 0x24,  // 0x0090  (Reserved)
	GPPUD     = 0x25,  // 0x0094  GPIO Pin Pull-up/down Enable 32 R/W
	GPPUDCLK0 = 0x26,  // 0x0098  GPIO Pin Pull-up/down Enable Clock 0 32 R/W
	GPPUDCLK1 = 0x27,  // 0x009C  GPIO Pin Pull-up/down Enable Clock 1 32 R/W

	//        = 0x28,  // 0x00A0  (Reserved)
};

// GPFSEL bits
// (Manual Chapter 6)
enum {
	GPFSEL_INPUT  = 0x00,
	GPFSEL_OUTPUT = 0x01,
	GPFSEL_ALT_0  = 0x04,
	GPFSEL_ALT_1  = 0x05,
	GPFSEL_ALT_2  = 0x06,
	GPFSEL_ALT_3  = 0x07,
	GPFSEL_ALT_4  = 0x03,
	GPFSEL_ALT_5  = 0x02,

	GPFSEL_PINS_PER_REGISTER = 10,
	GPFSEL_MASK  = 0x07,  // 3 bits per pin
	GPFSEL_SHIFT = 3      // 3 bits per pin

};

// PWM peripheral registers
// (Manual Chapter 9)
enum {                     // byte offset     function   bits  R/W
	CTL  = 0x00,       // 0x00  PWM Control  32
	STA  = 0x01,       // 0x04  PWM Status  32
	DMAC = 0x02,       // 0x08  PWM DMA Configuration  32
	//   = 0x03,       // 0x0c  Reserved
	RNG1 = 0x04,       // 0x10  PWM Channel 1 Range  32
	DAT1 = 0x05,       // 0x14  PWM Channel 1 Data  32
	FIF1 = 0x06,       // 0x18  PWM FIFO Input  32
	//   = 0x07,       // 0x1c  Reserved
	RNG2 = 0x08,       // 0x20  PWM Channel 2 Range  32
	DAT2 = 0x09,       // 0x24  PWM Channel 2 Data  32
};

// PWM CTL register bit masks
// (Manual Chapter 9)
enum {
	//                 31:16  Reserved - Write as 0, read as don't care
	MSEN2  = 1 << 15,  // 15  Channel 2 M/S Enable  0: PWM algorithm is used  1: M/S transmission is used.  RW  0x0
	//                 // 14  Reserved -  Write as 0, read as don't care
	USEF2  = 1 << 13,  // 13  Channel 2 Use Fifo  0: Data register is transmitted  1: Fifo is used for transmission  RW  0x0
	POLA2  = 1 << 12,  // 12  Channel 2 Polarity  0 : 0=low 1=high  1: 1=low 0=high  RW  0x0
	SBIT2  = 1 << 11,  // 11  Channel 2 Silence Bit  Defines the state of the output when no  transmission takes place  RW  0x0
	RPTL2  = 1 << 10,  // 10  Channel 2 Repeat Last Data  0: Transmission interrupts when FIFO is empty  1: Last data in FIFO is transmitted repetedly until  FIFO is not empty  RW  0x0
	MODE2  = 1 <<  9,  //  9  Channel 2 Mode  0: PWM mode  1: Serialiser mode  RW  0x0
	PWEN2  = 1 <<  8,  //  8  Channel 2 Enable  0: Channel is disabled  1: Channel is enabled  RW  0x0
	MSEN1  = 1 <<  7,  //  7  Channel 1 M/S Enable  0: PWM algorithm is used  1: M/S transmission is used.  RW  0x0
	CLRF1  = 1 <<  6,  //  6  Clear Fifo  1: Clears FIFO  0: Has no effect  This is a single shot operation. This bit always  reads 0  RO  0x0
	USEF1  = 1 <<  5,  //  5  Channel 1 Use Fifo  0: Data register is transmitted  1: Fifo is used for transmission  RW  0x0
	POLA1  = 1 <<  4,  //  4  Channel 1 Polarity  0 : 0=low 1=high  1: 1=low 0=high  RW  0x0
	SBIT1  = 1 <<  3,  //  3  Channel 1 Silence Bit  Defines the state of the output when no  transmission takes place  RW  0x0
	RPTL1  = 1 <<  2,  //  2  Channel 1 Repeat Last Data  0: Transmission interrupts when FIFO is empty  1: Last data in FIFO is transmitted repetedly until  FIFO is not empty  RW  0x0
	MODE1  = 1 <<  1,  //  1  Channel 1 Mode  0: PWM mode  1: Serialiser mode  RW  0x0
	PWEN1  = 1 <<  0,  //  0  Channel 1 Enable  0: Channel is disabled  1: Channel is enabled  RW  0x0

	PWM2_MASK = 0x0000bf00,   //Mask for all bits on Channel 2
	PWM1_MASK = 0x000000ff    //Mask for all bits on Channel 1
};


// ARM timer registers
// (Manual Chapter 14)
enum {                              // byte offset     function
	TIMER_LOAD        = 0x100,  // 0x400  Load
	TIMER_VALUE       = 0x101,  // 0x404  Value  (Read Only)
	TIMER_CONTROL     = 0x102,  // 0x408  Control
	TIMER_CLEAR_IRQ   = 0x103,  // 0x40C  IRQ Clear/Ack  (Write only)
	TIMER_RAW_IRQ     = 0x104,  // 0x410  RAW IRQ  (Read Only)
	TIMER_MASKED_IRQ  = 0x105,  // 0x414  Masked IRQ  (Read Only)
	TIMER_RELOAD      = 0x106,  // 0x418  Reload
	TIMER_PRE_DIVIDER = 0x107,  // 0x41C  Pre-divider  (Not in real 804!)
	TIMER_COUNTER     = 0x108,  // 0x420  Free running counter (Not in real 804!)
};


// PWM CLOCK control
// random Google searches, no definitive datsheet found
// http://www.raspberrypi.org/phpBB3/viewtopic.php?t=8467&p=236176

// ** The following code snippets are all from the Gertboard example code.
// ** It is too big to put all here. To have a fully working PWM I suggest you get the full
// ** example code and remove the parts you don't need.
// **
// ** #define PWMCLK_CNTL *(clk+40)
// ** #define PWMCLK_DIV *(clk+41)
// **
// ** // Derive PWM clock direct from X-tal
// ** // thus any system auto-slow-down-clock-to-save-power does not effect it
// ** // The values below depends on the X-tal frequency!
// ** PWMCLK_DIV = 0x5A000000 | (32<<12); // set pwm div to 32 (19.2MHz/32 = 600KHz)
// ** PWMCLK_CNTL = 0x5A000011; // Source=osc and enable
// **
// ** So the core is the "(32<<12)". That sets your clock divider.
// ** You can change the '32' to some other number but DON'T PLAY WITH THE OTHER VALUES!
// **
// ** You have to mmap the clk pointer before you can use that code.

// PWM CLOCK control register offsets
enum {
	// top byte of registers appear to be password (0x5a)
	PWMCLK_CNTL = 40,  // 31..24:password, 23..0:? (bit 4 = pwm clk enable?)
	PWMCLK_DIV  = 41,  // 31..24:password, 23..12:divisor, 11..0:?

	// values to load
	PWM_CONTROL_STOP  = 0x5A000001,  // stop PWM clock?
	PWM_CONTROL_START = 0x5A000011,  // Crystal Oscillator Source
	PWM_DIVISOR_VALUE = 0x5A000000 | (((32 * 6) & 0xfff) << 12)   // 100kHz
};

// Chip versions
typedef struct {
	const char *name;
	const uint32_t base;
} chip_version;

const char *cpu_info_file = "/proc/cpuinfo";
const char *cpu_info_key = "\nHardware";  // note '\n' for start of line match
const chip_version chips[] = {
	{"BCM2708", V1_BCM_PERIPHERALS_ADDRESS},
	{"BCM2709", V2_BCM_PERIPHERALS_ADDRESS}
};

#define SIZE_OF_ARRAY(a) (sizeof(a)/ sizeof((a)[0]))


// access to peripherals
volatile uint32_t *gpio_map;
volatile uint32_t *pwm_map;
volatile uint32_t *clock_map;
/* volatile uint32_t *timer_map; */


// local function prototypes;
static bool get_cpu_io_base_address(uint32_t *base);
static bool create_rw_map(volatile uint32_t **map, int fd, uint32_t base_address, uint32_t offset);
static bool delete_map(volatile uint32_t *address);


// set up access to the GPIO and PWM
bool GPIO_setup() {

	uint32_t base_address = 0;

	if (!get_cpu_io_base_address(&base_address)) {
		warn("cannot get the GPIO base address");
		return false;
	}

	const char *memory_device = "/dev/mem";

	int mem_fd = open(memory_device, O_RDWR | O_SYNC | O_CLOEXEC);

	if (mem_fd < 0) {
		warn("cannot open: %s", memory_device);
		return false;
	}

	// memory map entry to access the various peripheral registers

	if (!create_rw_map(&gpio_map, mem_fd, base_address, GPIO_REGISTERS)) {
		warn("failed to mmap gpio");
		goto close_mem;
	}
	if (!create_rw_map(&pwm_map, mem_fd, base_address, PWM_REGISTERS)) {
		warn("failed to mmap pwm");
		goto unmap_gpio;
	}
	if (!create_rw_map(&clock_map, mem_fd, base_address, CLOCK_REGISTERS)) {
		warn("failed to mmap clock");
		goto unmap_pwm;
	}
	/* if (!create_rw_map(&timer_map, mem_fd, base_address, TIMER_REGISTERS)) { */
	/* 	warn("failed to mmap timer"); */
	/* 	goto unmap_clock; */
	/* } */

	// close memory device and return success
	close(mem_fd);
	return true;

	// failure case delete items already created
/* unmap_clock: */
/*         delete_map(clock_map); */
unmap_pwm:
	delete_map(pwm_map);
unmap_gpio:
	delete_map(gpio_map);
close_mem:
	close(mem_fd);
	return false;
}


// revoke access to GPIO and PWM
bool GPIO_teardown() {
//	delete_map(timer_map);
	delete_map(clock_map);
	delete_map(pwm_map);
	delete_map(gpio_map);
	return true;
}


void GPIO_mode(GPIO_pin_type pin, GPIO_mode_type mode) {
	if (NULL == gpio_map) {
		return;
	}
	uint32_t offset = pin / GPFSEL_PINS_PER_REGISTER;
	uint32_t shift = (pin % GPFSEL_PINS_PER_REGISTER) * GPFSEL_SHIFT;
	uint32_t mask = ~(GPFSEL_MASK << shift);
	uint32_t pin_function = GPFSEL_INPUT;

	switch (mode) {
	default:
	case GPIO_INPUT:
		pin_function = GPFSEL_INPUT;
		break;
	case GPIO_OUTPUT:
		pin_function = GPFSEL_OUTPUT;
		break;
	case GPIO_PWM:  // only certain pins allowed
		if (GPIO_P1_12 == pin) {
			pin_function = GPFSEL_ALT_5;
			uint32_t  saved_pwm_control = pwm_map[CTL];
			pwm_map[CTL] = 0; //disable while setting clocks
			usleep(1000);

			// set up the clock source and divisor
			clock_map[PWMCLK_CNTL] = PWM_CONTROL_STOP;
			usleep(1000);
			clock_map[PWMCLK_DIV] = PWM_DIVISOR_VALUE;
			usleep(1000);
			clock_map[PWMCLK_CNTL] = PWM_CONTROL_START;
			usleep(1000);

			uint32_t pwm1_mode = CLRF1 | PWEN1;
			// keep channel 2 and setup channel 1
			pwm_map[CTL] = (saved_pwm_control & PWM2_MASK) | pwm1_mode;
			pwm_map[RNG1] = 1024;  // 10 bit range 0 .. 1023 (like Arduino)
			pwm_map[DAT1] = 0;     // initially zero
		}
		break;
	}

	pin_function <<= shift;

	gpio_map[GPFSEL0 + offset] &= mask;
	gpio_map[GPFSEL0 + offset] |= pin_function;
}


int GPIO_read(GPIO_pin_type pin) {
	if ((unsigned)(pin) > 63) {
		return 0;
	}
	uint32_t offset = GPLEV0;
	if ((unsigned)(pin) >= 32) {
		offset = GPLEV1;
		pin -= 32;
	}
	if (0 == (gpio_map[offset] & (1 << pin))) {
		return 0;
	} else {
		return 1;
	}
}


void GPIO_write(GPIO_pin_type pin, int value) {
	if ((unsigned)(pin) > 63) {
		return;
	}
	uint32_t offset = (value != 0) ? GPSET0 : GPCLR0;
	if ((unsigned)(pin) >= 32) {
		offset = (value != 0) ? GPSET1 : GPCLR1;
		pin -= 32;
	}
	uint32_t bit = 1 << pin;
	gpio_map[offset] = bit;
}


// only affetct PWM if correct pin is addressed
void GPIO_pwm_write(GPIO_pin_type pin, uint32_t value) {
	if (GPIO_P1_12 == pin) {
		pwm_map[DAT1] = value;
	}
}


// private functions
// =================

// map page size
#define MAP_SIZE 4096

bool get_cpu_io_base_address(uint32_t *base) {

	int info_fd = open(cpu_info_file, O_RDONLY);

	if (info_fd < 0) {
		warn("cannot open: %s", cpu_info_file);
		return false;
	}

	char buffer[8192];  // cpuinfo is not very big (on Rpi B2: "cat /proc/cpuinfo | wc -c" -> 1112 bytes)
	memset(buffer, 0, sizeof(buffer)); // ensure nul('\0') terminated

	ssize_t n = read(info_fd, buffer, sizeof(buffer) - 1);
	close(info_fd);

	if (n < 0) {
		return false;
	}

	// locate the key
	char *p = strstr(buffer, cpu_info_key);
	if (NULL == p) {
		return false;
	}

	p += strlen(cpu_info_key);

	while (isspace(*p) || ':' == *p) {
		++p;
	}
	char *p1 = p;
	while (isalnum(*p1)) {
		++p1;
	}

	for (int i = 0; i < SIZE_OF_ARRAY(chips); ++i) {
		size_t l = p1 - p;
		if (strlen(chips[i].name) == l && 0 == strncmp(chips[i].name, p, l)) {
			*base = chips[i].base;
			return true;
		}
	}

	// failed
	return false;
}

// setup a map to a peripheral offset
// false => error
static bool create_rw_map(volatile uint32_t **map, int fd, uint32_t base_address, uint32_t offset) {

	void *m = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base_address + offset);

	*map = (volatile uint32_t *)(m);

	return m != MAP_FAILED;
}


// remove the map
static bool delete_map(volatile uint32_t *address) {
	munmap((void *)address, MAP_SIZE);
	return true;
}
