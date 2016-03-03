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

// based on the information in
// PDF: SPRUH73I – October 2011 – Revised August 2013
//
// From the  page at:
//   http://www.ti.com/product/am3359
//
// Link title:
//   AM335x ARM Cortex-A8 Microprocessors (MPUs) Technical Reference Manual (Rev. I)
//   (PDF , 20271 KB)   27 Aug 2013

// OS:
//   root@beaglebone:~# uname -a; lsb_release -a
//   Linux beaglebone 3.8.13 #1 SMP Thu Sep 12 10:27:06 CEST 2013 armv7l GNU/Linux
//   Distributor ID: Angstrom
//   Description:    Angstrom GNU/Linux v2012.12 (Core edition)
//   Release:        v2012.12
//   Codename:       Core edition



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <err.h>

#include "gpio.h"


// register addresses in BeagleBone Black
// (Manual Chapter 2)
enum {                                 // End Address  Size
	GPIO0_REGISTERS = 0x44E07000,  // 0x44E0_7FFF  4kB
	GPIO1_REGISTERS = 0x4804C000,  // 0x4804_CFFF  4kB
	GPIO2_REGISTERS = 0x481AC000,  // 0x481A_CFFF  4KB
	GPIO3_REGISTERS = 0x481AE000,  // 0x481A_EFFF  4KB
};


// GPIO module registers (all registers are 32 bit)
// (Manual Chapter 25)
enum {
	GPIO_REVISION = 0x000 / 4,
	GPIO_SYSCONFIG = 0x010 / 4,
	GPIO_EOI = 0x020 / 4,
	GPIO_IRQSTATUS_RAW_0 = 0x024 / 4,
	GPIO_IRQSTATUS_RAW_1 = 0x028 / 4,
	GPIO_IRQSTATUS_0 = 0x02C / 4,
	GPIO_IRQSTATUS_1 = 0x030 / 4,
	GPIO_IRQSTATUS_SET_0 = 0x034 / 4,
	GPIO_IRQSTATUS_SET_1 = 0x038 / 4,
	GPIO_IRQSTATUS_CLR_0 = 0x03C / 4,
	GPIO_IRQSTATUS_CLR_1 = 0x040 / 4,
	GPIO_IRQWAKEN_0 = 0x044 / 4,
	GPIO_IRQWAKEN_1 = 0x048 / 4,
	GPIO_SYSSTATUS = 0x114 / 4,
	GPIO_CTRL = 0x130 / 4,
	GPIO_OE = 0x134 / 4,
	GPIO_DATAIN = 0x138 / 4,
	GPIO_DATAOUT = 0x13C / 4,
	GPIO_LEVELDETECT0 = 0x140 / 4,
	GPIO_LEVELDETECT1 = 0x144 / 4,
	GPIO_RISINGDETECT = 0x148 / 4,
	GPIO_FALLINGDETECT = 0x14C / 4,
	GPIO_DEBOUNCENABLE = 0x150 / 4,
	GPIO_DEBOUNCINGTIME = 0x154 / 4,
	GPIO_CLEARDATAOUT = 0x190 / 4,
	GPIO_SETDATAOUT = 0x194 / 4
};

// GPIO_OE bits
// (Manual Chapter 25)
enum {
	GPIO_OE_OUTPUT = 0x00,
	GPIO_OE_INPUT  = 0x01
};


// cape manager info
static char *slots = NULL;
static char *ocp = NULL;

// access to peripherals
volatile uint32_t *gpio_map[4];  // GPIO 0..3

static struct {
	char *name;
	int fd;
	uint32_t period;
} pwm[4];

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof((a)[0]))

// local function prototypes;
static bool create_rw_map(volatile uint32_t **map, int fd, uint32_t offset);
static bool delete_map(volatile uint32_t *address);
static bool load_firmware(const char *pin_name);
static bool PWM_enable(int channel, const char *pin_name);
static void PWM_set_duty(int channel, int16_t value);


// set up access to the GPIO and PWM
bool GPIO_setup() {
	const char *memory_device = "/dev/mem";

	memset(gpio_map, 0, sizeof(gpio_map));

	int mem_fd = open(memory_device, O_RDWR | O_SYNC | O_CLOEXEC);

	if (mem_fd < 0) {
		warn("cannot open: %s", memory_device);
		return false;
	}

	// memory map entry to access the various peripheral registers

	if (!create_rw_map(&gpio_map[0], mem_fd, GPIO0_REGISTERS)) {
		warn("failed to mmap gpio0");
		goto fail;
	}
	if (!create_rw_map(&gpio_map[1], mem_fd, GPIO1_REGISTERS)) {
		warn("failed to mmap gpio1");
		goto fail;
	}
	if (!create_rw_map(&gpio_map[2], mem_fd, GPIO2_REGISTERS)) {
		warn("failed to mmap gpio2");
		goto fail;
	}
	if (!create_rw_map(&gpio_map[3], mem_fd, GPIO3_REGISTERS)) {
		warn("failed to mmap gpio3");
		goto fail;
	}

	// close memory device
	close(mem_fd);

	// ensure the base firmware is setup
	if (!load_firmware(NULL)) {
		goto fail;
	}


	// return success
	return true;

fail:
	// failure case delete items already created
	close(mem_fd);
	GPIO_teardown();
	return false;
}


// revoke access to GPIO and PWM
bool GPIO_teardown() {
	for (size_t i = 0; i < SIZE_OF_ARRAY(gpio_map); ++i) {
		delete_map(gpio_map[i]);
	}

	for (size_t i = 0; i < SIZE_OF_ARRAY(pwm); ++i) {
		if (NULL != pwm[i].name) {
			close(pwm[i].fd);
			free(pwm[i].name);
		}
	}

	if (NULL != slots) {
		// should unload the loaded firmware here,
		// but any attempt to do this crashes the process
		// probably a kernel bug, which may cause system instablility
		free(slots);
	}

	if (NULL != ocp) {
		free(ocp);
	}

	// clear all pointers so calling setup again will work
	memset(gpio_map, 0, sizeof(gpio_map));
	memset(pwm, 0, sizeof(pwm));
	slots = NULL;
	ocp = NULL;

	return true;
}


void GPIO_mode(GPIO_pin_type pin, GPIO_mode_type mode) {
	int bank = (pin >> 8) & 0xff;
	int pin32 = pin & 0xff;
	if (bank < 0 || bank > 3 || pin32 < 0 || pin32 > 31) {
		return;
	}
	if (NULL == gpio_map[bank]) {
		return;
	}

	volatile uint32_t *oe = &gpio_map[bank][GPIO_OE];

	switch (mode) {
	default:
	case GPIO_INPUT:
		*oe |= 1 << pin32;
		break;
	case GPIO_OUTPUT:
		*oe &= ~(1 << pin32);
		break;
	case GPIO_PWM:  // only certain pins allowed
		switch (pin) {
		case GPIO_P9_14:    // EHRPWM1A
			PWM_enable(0, "P9_14");
			break;
		case GPIO_P9_16:    // EHRPWM1B
			PWM_enable(1, "P9_16");
			break;
		case GPIO_P8_19:    // EHRPWM2A
			PWM_enable(2, "P8_19");
			break;
		case GPIO_P8_13:    // EHRPWM2B
			PWM_enable(3, "P8_13");
			break;
		default:
			break;
		}
		break;
	}
}


int GPIO_read(int pin) {
	int bank = (pin >> 8) & 0xff;
	pin &= 0xff;
	if (bank < 0 || bank > 3 || pin < 0 || pin > 31) {
		return 0;
	}
	if (0 == (gpio_map[bank][GPIO_DATAIN] & (1 << pin))) {
		return 0;
	} else {
		return 1;
	}
}


void GPIO_write(GPIO_pin_type pin, int value) {
	int bank = (pin >> 8) & 0xff;
	pin &= 0xff;
	if (bank < 0 || bank > 3 || pin < 0 || pin > 31) {
		return;
	}
	uint32_t offset = (value != 0) ? GPIO_SETDATAOUT : GPIO_CLEARDATAOUT;
	uint32_t bit = 1 << pin;
	gpio_map[bank][offset] = bit;
}


// only affect PWM if correct pin is addressed
void GPIO_pwm_write(int pin, uint32_t value) {
	if (value > 1023) {
		value = 1023;
	}
	switch (pin) {
	case GPIO_P9_14:    // EHRPWM1A
		PWM_set_duty(0, value);
		break;
	case GPIO_P9_16:    // EHRPWM1B
		PWM_set_duty(1, value);
		break;
	case GPIO_P8_19:    // EHRPWM2A
		PWM_set_duty(2, value);
		break;
	case GPIO_P8_13:    // EHRPWM2B
		PWM_set_duty(3, value);
		break;
	default:
		break;
	}
}


// private functions
// =================

// map page size
#define MAP_SIZE 4096


// setup a map to a peripheral offset
// false => error
static bool create_rw_map(volatile uint32_t **map, int fd, uint32_t offset) {

	void *m = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

	*map = (volatile uint32_t *)(m);

	return m != MAP_FAILED;
}


// remove the map
static bool delete_map(volatile uint32_t *address) {
	munmap((void *)address, MAP_SIZE);
	return true;
}



#include <sys/types.h>
#include <dirent.h>

#define DEVICES "/sys/devices"
#define CAPE_MANAGER "bone_capemgr."
#define OCP "ocp."

#define SLOTS "/slots"

// pwm files
#define PERIOD "/period"
#define DUTY "/duty"
#define RUN "/run"
#define POLARITY "/polarity"

// firmware files (/lib/firmware/NAME.dtbo) to load
#define CAPE_IIO "cape-bone-iio"
#define CAPE_PWM "am33xx_pwm"
#define CAPE_SPI "BB-SPIDEV0"
#define CAPE_PWM_PIN_PREFIX "bone_pwm_"

// ocp files
#define OCP_PWM_PREFIX "pwm_test_"

// get the size of a constant string at compile time
// no need for extra strlen at run time
// (note ignore the trailing '\0')
#define CONST_STRLEN(const_string) (sizeof(const_string) - sizeof((char)('\0')))


static bool find_slots_and_ocp() {
	if (NULL != slots) {
		return true;  // already done
	}

	DIR *dir = opendir(DEVICES);
	if (NULL == dir) {
		return false; // failed
	}

	struct dirent *dp;
	while (NULL != (dp = readdir(dir))) {
		if (0 == strncmp(dp->d_name, CAPE_MANAGER, CONST_STRLEN(CAPE_MANAGER))
		    && NULL == slots) {
			slots = malloc(CONST_STRLEN(DEVICES) + sizeof((char)('/')) + strlen(dp->d_name)
				       + CONST_STRLEN(SLOTS) + sizeof((char)('\0')));
			if (NULL == slots) {
				break;   // failed
			}
			strcpy(slots, DEVICES "/");
			strcat(slots, dp->d_name);
			strcat(slots, SLOTS);
		} else if (0 == strncmp(dp->d_name, OCP, CONST_STRLEN(OCP))
			   && NULL == ocp) {
			ocp = malloc(CONST_STRLEN(DEVICES)
				     + sizeof((char)('/'))
				     + strlen(dp->d_name)
				     + sizeof((char)('\0')));
			if (NULL == ocp) {
				break;   // failed
			}
			strcpy(ocp, DEVICES "/");
			strcat(ocp, dp->d_name);
		}
	}
	(void)closedir(dir);

	if (NULL == slots || NULL == ocp) {
		if (NULL != slots) {
			free(slots);
			slots = NULL;
		}
		if (NULL != ocp) {
			free(ocp);
			ocp = NULL;
		}

		return false;  // failed
	}

	// success
	return true;
}

static bool load_firmware(const char *pin_name) {
	// find slots and ocp paths
	if (!find_slots_and_ocp()) {
		return false;  // failed
	}

	int fd = open(slots, O_RDWR);
	if (fd < 0) {
		return false;  // failed
	}

	char buffer[8192];  // only 4096 is indicated in sysfs, can it be larger?

	memset(buffer, 0, sizeof(buffer));
	read(fd, buffer, sizeof(buffer) - 1);  // allow one nul at end

	// I/O multiplexing
	if (NULL == strstr(buffer, CAPE_IIO)) {
		lseek(fd, 0, SEEK_SET);
		write(fd, CAPE_IIO "\n", CONST_STRLEN(CAPE_IIO "\n"));
	}
	// PWM
	if (NULL == strstr(buffer, CAPE_PWM)) {
		lseek(fd, 0, SEEK_SET);
		write(fd, CAPE_PWM "\n", CONST_STRLEN(CAPE_PWM "\n"));
	}
	// also set up SPI0 -> /dev/spidevX.Y
	if (NULL == strstr(buffer, CAPE_SPI)) {
		lseek(fd, 0, SEEK_SET);
		write(fd, CAPE_SPI "\n", CONST_STRLEN(CAPE_SPI "\n"));
	}

	// The desired PWM pin
	if (NULL != pin_name) {
		char *config = malloc(CONST_STRLEN(CAPE_PWM_PIN_PREFIX)
				      + strlen(pin_name)
				      + sizeof((char)('\n'))
				      + sizeof((char)('\0')));

		if (NULL == config) {
			close(fd);
			return false; // failed
		}
		strcpy(config, CAPE_PWM_PIN_PREFIX);
		strcat(config, pin_name);
		strcat(config, "\n");

		if (NULL == strstr(buffer, config)) {
			lseek(fd, 0, SEEK_SET);
			write(fd, config, strlen(config));
		}
		free(config);
	}
	// finished with the cape manager
	close(fd);
	return true;
}


// enable PWM
static bool PWM_enable(int channel, const char *pin_name) {
	if (NULL != pwm[channel].name) {
		return true;  // already configured
	}

	// find slots and ocp paths
	if (!load_firmware(pin_name)) {
		return false;  // failed
	}

	// wait a bit for the pwm device to appear.
	// is this long enough or should the whole code below be in a retry loop?
	usleep(10000);

	char *config = malloc(CONST_STRLEN(OCP_PWM_PREFIX)
			      + strlen(pin_name)
			      + sizeof((char)('\0')));

	if (NULL == config) {
		return false; // failed
	}

	strcpy(config, OCP_PWM_PREFIX);
	strcat(config, pin_name);
	size_t length = strlen(config);

	// find pwm path
	DIR *dir = opendir(ocp);
	if (NULL == dir) {
		goto done; // failed
	}
	struct dirent *dp;
	while (NULL != (dp = readdir(dir))) {
		if (0 == strncmp(dp->d_name, config, length)) {
			pwm[channel].name = malloc(strlen(ocp)
						   + sizeof((char)('/'))
						   + strlen(dp->d_name) + CONST_STRLEN(DUTY)
						   + sizeof((char)('\0')));
			if (NULL == pwm[channel].name) {
				break;  // failed
			}

			strcpy(pwm[channel].name, ocp);
			strcat(pwm[channel].name, "/");
			strcat(pwm[channel].name, dp->d_name);
			strcat(pwm[channel].name, DUTY);

			// wait up to 5 seconds for the pwm driver to appear.
			// is the device tree populated in the background?
			for (int i = 0; i < 500; ++i) {
				pwm[channel].fd = open(pwm[channel].name, O_RDWR);
				if (pwm[channel].fd >= 0) {
					break;
				}
				usleep(10000);
			}
			if (pwm[channel].fd < 0) {
				fprintf(stderr, "PWM failed to appear\n"); fflush(stderr);
				free(pwm[channel].name);
				pwm[channel].name = NULL;
				break;  // failed
			}

			// set duty = zero
			lseek(pwm[channel].fd, 0, SEEK_SET);
			write(pwm[channel].fd, "0\n", 2);

			char buffer[4096];
			// set zero duty => zero output
			strcpy(buffer, ocp);
			strcat(buffer, "/");
			strcat(buffer, dp->d_name);
			strcat(buffer, POLARITY);
			int fd = open(buffer, O_WRONLY);
			write(fd, "0\n", 2);
			close(fd);

			// read and save period?
			// ???currently seems to be fixed to 500000
			pwm[channel].period = 500000;

			// start
			strcpy(buffer, ocp);
			strcat(buffer, "/");
			strcat(buffer, dp->d_name);
			strcat(buffer, RUN);
			fd = open(buffer, O_WRONLY);
			write(fd, "1\n", 2);
			close(fd);

			break;
		}
	}
	closedir(dir);

done:
	free(config);
	return NULL != pwm[channel].name;
}


static void PWM_set_duty(int channel, int16_t value) {
	if (channel < 0 || channel > 3) {
		return;
	}
	if (value < 0) {
		value = 0;
	} else if (value > 1023) {
		value = 1023;
	}

	uint32_t duty = value * pwm[channel].period / 1023;

	char config[256];
	snprintf(config, sizeof(config), "%d\n", duty),

	lseek(pwm[channel].fd, 0, SEEK_SET);
	write(pwm[channel].fd, config, strlen(config));
}
