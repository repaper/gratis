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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
//#include <sys/mman.h>
#include <unistd.h>
#include <err.h>

#include "gpio.h"


// GPIO

#define MAKE_PIN(_name, _bank, _pin) \
	[GPIO_PIN(_bank, _pin)] = {  \
		.name = _name,       \
		.mode = Mode_GPIO,   \
		.active = Mode_NONE, \
		.pwm_chip = 0,       \
		.pwm_channel = 0,    \
		.pwm_state = 0,     \
		.fd = -1             \
	}

// if the pin can also be a pwm
#define MAKE_PWM(_name, _bank, _pin, _chip, _channel, _state)  \
	[GPIO_PIN(_bank, _pin)] = {  \
		.name = _name,       \
		.mode = Mode_PWM,    \
		.active = Mode_NONE, \
		.pwm_chip = _chip,   \
		.pwm_channel = _channel, \
		.pwm_state = _state, \
		.fd = -1             \
	}

// GPIO mode
typedef enum {
	Mode_NONE,
	Mode_GPIO,
	Mode_PWM
} internal_mode_t;

// GPIO control information
static struct {
	const char *name;       // e.g. "P8.15"
	internal_mode_t mode;   // highest Mode_XXX possible
	internal_mode_t active; // Mode_XXX current
	int pwm_chip;           // pwmchipN numeric value
	int pwm_channel;        // channel number
	int pwm_state;          // index of a state file for multiplexor control
	int fd;                 // open fd to value/duty_cycle file for fast access
} gpio_info[] = {
	// Connector P8
	MAKE_PIN("P8.03", 1, 6),   //  GPIO1_6
	MAKE_PIN("P8.04", 1, 7),   //  GPIO1_7
	MAKE_PIN("P8.05", 1, 2),   //  GPIO1_2
	MAKE_PIN("P8.06", 1, 3),   //  GPIO1_3
	MAKE_PIN("P8.07", 2, 2),   //  GPIO2_2   TIMER4
	MAKE_PIN("P8.08", 2, 3),   //  GPIO2_3   TIMER7
	MAKE_PIN("P8.09", 2, 5),   //  GPIO2_5   TIMER5
	MAKE_PIN("P8.10", 2, 4),   //  GPIO2_4   TIMER6
	MAKE_PIN("P8.11", 1, 13),  //  GPIO1_13
	MAKE_PIN("P8.12", 1, 12),  //  GPIO1_12
	MAKE_PIN("P8.13", 0, 23),  //  GPIO0_23  EHRPWM2B
	MAKE_PIN("P8.14", 0, 26),  //  GPIO0_26
	MAKE_PIN("P8.15", 1, 15),  //  GPIO1_15
	MAKE_PIN("P8.16", 1, 14),  //  GPIO1_14
	MAKE_PIN("P8.17", 0, 27),  //  GPIO0_27
	MAKE_PIN("P8.18", 2, 1),   //  GPIO2_1
	MAKE_PIN("P8.19", 0, 22),  //  GPIO0_22  EHRPWM2A
	MAKE_PIN("P8.20", 1, 31),  //  GPIO1_31
	MAKE_PIN("P8.21", 1, 30),  //  GPIO1_30
	MAKE_PIN("P8.22", 1, 5),   //  GPIO1_5
	MAKE_PIN("P8.23", 1, 4),   //  GPIO1_4
	MAKE_PIN("P8.24", 1, 1),   //  GPIO1_1
	MAKE_PIN("P8.25", 1, 0),   //  GPIO1_0
	MAKE_PIN("P8.26", 1, 29),  //  GPIO1_29
	MAKE_PIN("P8.27", 2, 22),  //  GPIO2_22
	MAKE_PIN("P8.28", 2, 24),  //  GPIO2_24
	MAKE_PIN("P8.29", 2, 23),  //  GPIO2_23
	MAKE_PIN("P8.30", 2, 25),  //  GPIO2_25
	MAKE_PIN("P8.31", 0, 10),  //  GPIO0_10  UART5_CTSN
	MAKE_PIN("P8.32", 0, 11),  //  GPIO0_11  UART5_RTSN
	MAKE_PIN("P8.33", 0, 9),   //  GPIO0_9   UART4_RTSN
	MAKE_PIN("P8.34", 2, 17),  //  GPIO2_17  UART3_RTSN
	MAKE_PIN("P8.35", 0, 8),   //  GPIO0_8   UART4_CTSN
	MAKE_PIN("P8.36", 2, 16),  //  GPIO2_16  UART3_CTSN
	MAKE_PIN("P8.37", 2, 14),  //  GPIO2_14  UART5_TXD
	MAKE_PIN("P8.38", 2, 15),  //  GPIO2_15  UART5_RXD
	MAKE_PIN("P8.39", 2, 12),  //  GPIO2_12
	MAKE_PIN("P8.40", 2, 13),  //  GPIO2_13
	MAKE_PIN("P8.41", 2, 10),  //  GPIO2_10
	MAKE_PIN("P8.42", 2, 11),  //  GPIO2_11
	MAKE_PIN("P8.43", 2, 8),   //  GPIO2_8
	MAKE_PIN("P8.44", 2, 9),   //  GPIO2_9
	MAKE_PIN("P8.45", 2, 6),   //  GPIO2_6
	MAKE_PIN("P8.46", 2, 7),   //  GPIO2_7

	// Connector P9
	MAKE_PIN("P9.11", 0, 30),  //  GPIO0_30  UART4_RXD
	MAKE_PIN("P9.12", 1, 28),  //  GPIO1_28
	MAKE_PIN("P9.13", 0, 31),  //  GPIO0_31  UART4_TXD
	MAKE_PWM("P9.14", 1, 18, 2, 0, 0),  //  GPIO1_18  EHRPWM1A
	MAKE_PIN("P9.15", 1, 16),           //  GPIO1_16
	MAKE_PWM("P9.16", 1, 19, 2, 1, 1),  //  GPIO1_19  EHRPWM1B
	MAKE_PIN("P9.17", 0, 5),   //  GPIO0_5   I2C1_SCL
	MAKE_PIN("P9.18", 0, 4),   //  GPIO0_4   I2C1_SDA
	MAKE_PIN("P9.19", 0, 13),  //  GPIO0_13  I2C2_SCL
	MAKE_PIN("P9.20", 0, 12),  //  GPIO0_12  I2C2_SDA
	MAKE_PIN("P9.21", 0, 3),   //  GPIO0_3   UART2_TXD
	MAKE_PIN("P9.22", 0, 2),   //  GPIO0_2   UART2_RXD
	MAKE_PIN("P9.23", 1, 17),  //  GPIO1_17
	MAKE_PIN("P9.24", 0, 15),  //  GPIO0_15  UART1_TXD
	MAKE_PIN("P9.25", 3, 21),  //  GPIO3_21
	MAKE_PIN("P9.26", 0, 14),  //  GPIO0_14  UART1_RXD
	MAKE_PIN("P9.27", 3, 19),  //  GPIO3_19
	MAKE_PIN("P9.28", 3, 17),  //  GPIO3_17  SPI1_CS0
	MAKE_PIN("P9.29", 3, 15),  //  GPIO3_15  SPI1_D0
	MAKE_PIN("P9.30", 3, 16),  //  GPIO3_16  SPI1_D1
	MAKE_PIN("P9.31", 3, 14),  //  GPIO3_14  SPI1_SCLK
};


// compute array size at compile time
#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof((a)[0]))

// get the size of a constant string at compile time
// no need for extra strlen at run time
// (note ignore the trailing '\0')
#define CONST_STRLEN(const_string) (sizeof(const_string) - sizeof((char)('\0')))

// turn an argument into a string
#define MAKE_STRING(s) MAKE_STRING_1(s)
#define MAKE_STRING_1(s) #s


#define CAPE_MANAGER_SLOTS "/sys/devices/platform/bone_capemgr/slots"

// GPIO files

#define SYS_GPIO_EXPORT   "/sys/class/gpio/export"
#define SYS_GPIO_UNEXPORT "/sys/class/gpio/unexport"

#define SYS_CLASS_GPIO  "/sys/class/gpio/gpio%d"
#define GPIO_ACTIVE_LOW SYS_CLASS_GPIO "/active_low"
#define GPIO_DIRECTION  SYS_CLASS_GPIO "/direction"
#define GPIO_EDGE       SYS_CLASS_GPIO "/edge"
#define GPIO_VALUE      SYS_CLASS_GPIO "/value"

// GPIO edge / direction
// states were in old system - not needed?
//#define STATE_rxDisable_pullNone "rxDisable_pullNone"
//#define STATE_rxEnable_pullNone  "rxEnable_pullNone"
//#define STATE_rxDisable_pullUp   "rxDisable_pullUp"
//#define STATE_rxEnable_pullUp    "rxEnable_pullUp"
//#define STATE_rxDisable_pullDown "rxDisable_pullDown"
//#define STATE_rxEnable_pullDown  "rxEnable_pullDown"

#define GPIO_EDGE_none     "none"
#define GPIO_DIRECTION_in  "in"
#define GPIO_DIRECTION_out "out"


// PWM files
#define MUX_default "default"
#define MUX_pwm     "pwm"
#define MUX_spi     "spi"

const char *const pwm_state_file[] = {
	"/sys/devices/platform/ocp/ocp:P9_14_pinmux/state",
	"/sys/devices/platform/ocp/ocp:P9_16_pinmux/state"
};

#define SYS_PWM_EXPORT   "/sys/class/pwm/pwmchip%d/export"
#define SYS_PWM_UNEXPORT "/sys/class/pwm/pwmchip%d/unexport"

#define SYS_CLASS_PWM  "/sys/class/pwm/pwmchip%d/pwm%d"
#define PWM_DUTY_CYCLE SYS_CLASS_PWM "/duty_cycle"
#define PWM_ENABLE     SYS_CLASS_PWM "/enable"
#define PWM_PERIOD     SYS_CLASS_PWM "/period"
#define PWM_POLARITY   SYS_CLASS_PWM "/polarity"

#define PWM_POLARITY_normal "normal"
#define PWM_DEFAULT_PERIOD 500000


// SPI initialisation
const char *const spi_state_file[] = {
	"/sys/devices/platform/ocp/ocp:P9_17_pinmux/state",
	"/sys/devices/platform/ocp/ocp:P9_21_pinmux/state",
	"/sys/devices/platform/ocp/ocp:P9_22_pinmux/state",
	"/sys/devices/platform/ocp/ocp:P9_18_pinmux/state"
};


// firmware files (/lib/firmware/NAME-00A0.dtbo) to load
//#define CAPE_IIO "cape-bone-iio"
//#define CAPE_PWM "am33xx_pwm"
//#define CAPE_SPI "BB-SPIDEV0"

// stand-alone file - diable all above firmware files
#define CAPE_UNIVERSAL "cape-universal"

// the universal cape automatically exports GPIOs so do not export/unexport them
#if defined(CAPE_UNIVERSAL)
#define USE_GPIO_EXPORT 0
#else
#define USE_GPIO_EXPORT 1
#endif

// local function prototypes;
static bool load_firmware(const char *pin_name);
static void write_file(const char *file_name, const char *buffer, size_t length);
static void write_pin_file(const char *file_name, int pin, const char *buffer, size_t length);
static void write_pwm_file(const char *file_name, int chip, int channel, const char *buffer, size_t length);
static char *make_formatted_buffer(const char *format, ...);
static void export(int number);
static void unexport(int number);
static void export_pwm(int chip, int number);
static void unexport_pwm(int chip, int number);
static bool GPIO_enable(int pin);
static bool PWM_enable(int pin);
static void PWM_set_duty(int pin, int16_t value);


// set up access to the GPIO and PWM
bool GPIO_setup() {

	// ensure the base firmware is setup
	if (load_firmware(NULL)) {
		// return success
		return true;
	}

	// shutdown anything that was created
	GPIO_teardown();
	return false;
}


// revoke access to GPIO and PWM
bool GPIO_teardown() {

	// finalise SPI multiplexor
	for (int i = 0; i < SIZE_OF_ARRAY(spi_state_file); ++i) {
		write_file(spi_state_file[i], MUX_default "\n", CONST_STRLEN(MUX_default "\n"));
	}

	for (size_t pin = 0; pin < SIZE_OF_ARRAY(gpio_info); ++pin) {
		if (Mode_NONE == gpio_info[pin].active) {
			continue;
		}
		if (gpio_info[pin].fd >= 0) {
			close(gpio_info[pin].fd);
			gpio_info[pin].fd = -1;
		}

		switch(gpio_info[pin].active) {
		case Mode_NONE:
			break;

		case Mode_GPIO:
			unexport(pin);
			break;

		case Mode_PWM:
		{
			int chip = gpio_info[pin].pwm_chip;
			int channel = gpio_info[pin].pwm_channel;

			write_pwm_file(PWM_DUTY_CYCLE, chip, channel, "0\n", 2);
			write_pwm_file(PWM_ENABLE, chip, channel, "0\n", 2);

			unexport_pwm(chip, channel);
			write_file(pwm_state_file[gpio_info[pin].pwm_state], MUX_default "\n", CONST_STRLEN(MUX_default "\n"));
			//unexport(pin);
			break;
		}
		}

		gpio_info[pin].active = Mode_NONE;
	}

	return true;
}


void GPIO_mode(GPIO_pin_type pin, GPIO_mode_type mode) {

	// ignore unimplemented pins
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name) {
		return;
	}

	switch (mode) {
	default:
	case GPIO_INPUT:
		if (gpio_info[pin].fd < 0 && !GPIO_enable(pin)) {
			return;
		}
		write_pin_file(GPIO_DIRECTION, pin, GPIO_DIRECTION_in "\n", CONST_STRLEN(GPIO_DIRECTION_in "\n"));
		write_pin_file(GPIO_ACTIVE_LOW, pin, "0\n", 2);
		write_pin_file(GPIO_EDGE, pin, GPIO_EDGE_none "\n", CONST_STRLEN(GPIO_EDGE_none "\n"));
		//write_pin_file(GPIO_STATE, pin, STATE_rxEnable_pullNone "\n", CONST_STRLEN(STATE_rxEnable_pullNone "\n"));
		break;

	case GPIO_OUTPUT:
		if (gpio_info[pin].fd < 0 && !GPIO_enable(pin)) {
			return;
		}
		write_pin_file(GPIO_DIRECTION, pin, GPIO_DIRECTION_out "\n", CONST_STRLEN(GPIO_DIRECTION_out "\n"));
		write_pin_file(GPIO_ACTIVE_LOW, pin, "0\n", 2);
		write_pin_file(GPIO_EDGE, pin, GPIO_EDGE_none "\n", CONST_STRLEN(GPIO_EDGE_none "\n"));
		//write_pin_file(GPIO_STATE, pin, STATE_rxDisable_pullNone "\n", CONST_STRLEN(STATE_rxDisable_pullNone "\n"));
		break;

	case GPIO_PWM:  // only certain pins allowed
		if (gpio_info[pin].fd < 0 && !PWM_enable(pin)) {
			return;
		}
		break;
	}
}


int GPIO_read(int pin) {
	// ignore unimplemented or inactive pins
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name || gpio_info[pin].fd < 0) {
		return 0;
	}

	int fd = gpio_info[pin].fd;
	char buffer[2];
	buffer[0] = 0;

	lseek(fd, 0, SEEK_SET);
	read(fd, buffer, sizeof(buffer));

	if ('1' == buffer[0]) {
		return 1;
	} else {
		return 0;
	}
}


void GPIO_write(GPIO_pin_type pin, int value) {
	// ignore unimplemented or inactive pins
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name || gpio_info[pin].fd < 0) {
		return;
	}

	int fd = gpio_info[pin].fd;

	lseek(fd, 0, SEEK_SET);

	if (0 == value) {
		write(fd, "0\n", 2);
	} else {
		write(fd, "1\n", 2);
	}
	fsync(fd);
}


// only affect PWM if correct pin is addressed
void GPIO_pwm_write(int pin, uint32_t value) {
	if (value > 1023) {
		value = 1023;
	}
	PWM_set_duty(pin, value);
}


// private functions
// =================

#include <ctype.h>
#include <sys/types.h>
//#include <dirent.h>
#include <stdarg.h>




// macro to load a firmware file
#define LOAD_CAPE_FIRMWARE_FILE(cape_name)                                \
	if (NULL == strstr(buffer, cape_name)) {                          \
		lseek(fd, 0, SEEK_SET);                                   \
		write(fd, cape_name "\n", CONST_STRLEN(cape_name "\n"));  \
	}

static bool load_firmware(const char *pin_name) {

	int fd = open(CAPE_MANAGER_SLOTS, O_RDWR);
	if (fd < 0) {
		return false;  // failed
	}

	char buffer[8192];  // only 4096 is indicated in sysfs, can it be larger?

	memset(buffer, 0, sizeof(buffer));
	read(fd, buffer, sizeof(buffer) - 1);  // allow one nul at end

#if defined(CAPE_IIO)
	// I/O multiplexing
	LOAD_CAPE_FIRMWARE_FILE(CAPE_IIO)
#endif

#if defined(CAPE_UNIVERSAL)
	// try universal before other I/Os
	LOAD_CAPE_FIRMWARE_FILE(CAPE_UNIVERSAL)
#endif

#if defined(CAPE_PWM)
	// PWM
	LOAD_CAPE_FIRMWARE_FILE(CAPE_PWM)
#endif

#if defined(CAPE_SPI)
	// also set up SPI0 -> /dev/spidevX.Y
	LOAD_CAPE_FIRMWARE_FILE(CAPE_SPI)
#endif

	// finished with the cape manager
	close(fd);

	// initialise SPI multiplexor
	for (int i = 0; i < SIZE_OF_ARRAY(spi_state_file); ++i) {
		write_file(spi_state_file[i], MUX_spi "\n", CONST_STRLEN(MUX_spi "\n"));
	}

	return true;
}

static void write_file(const char *file_name, const char *buffer, size_t length) {
	if (length <= 0) {
		length = strlen(buffer);
	}
	int fd = open(file_name, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "write_file failed: '%s' <- '%s'\n", file_name, buffer); fflush(stderr);
		return;  // failed
	}
	size_t n = write(fd, buffer, length);
	fsync(fd);
	if (n != length) {
		fprintf(stderr, "write_file only wrote: %d of %d\n", n, length); fflush(stderr);
	}
	close(fd);
}


static void write_pin_file(const char *path, int pin, const char *buffer, size_t length) {
	char *f = make_formatted_buffer(path, pin);
	write_file(f, buffer, length);
	free(f);
}

static void write_pwm_file(const char *path, int chip, int pin, const char *buffer, size_t length) {
	char *f = make_formatted_buffer(path, chip, pin);
	write_file(f, buffer, length);
	free(f);
}


# define INITIAL_SIZE 4096
static char *make_formatted_buffer(const char *format, ...) {

	char *buffer = malloc(INITIAL_SIZE);
	memset(buffer, 0, INITIAL_SIZE);

	va_list ap;

	va_start(ap, format);
	int length = vsnprintf(buffer, INITIAL_SIZE, format, ap);
	va_end(ap);

	if (length >= INITIAL_SIZE) {
		fprintf(stderr, "buffer overflow in make_formatted_buffer from:'%s'  attempting to write %d bytes\n", format, length); fflush(stderr);
		exit(1);
	}

	return buffer;
}


static void export_unexport(const char *path, int number) {
	char *data_line = make_formatted_buffer("%d\n", number);
	write_file(path, data_line, 0);
	free(data_line);
}

static void export(int number) {
#if USE_GPIO_EXPORT
	export_unexport(SYS_GPIO_EXPORT, number);
#else
	(void)number;
#endif
}


static void unexport(int number) {
#if USE_GPIO_EXPORT
	export_unexport(SYS_GPIO_UNEXPORT, number);
#else
	(void)number;
#endif
}

static void pwm_export_unexport(const char *path, int chip, int number) {
	char *buffer = make_formatted_buffer(path, chip);
	export_unexport(buffer, number);
	free(buffer);
}

static void export_pwm(int chip, int number) {
	pwm_export_unexport(SYS_PWM_EXPORT, chip, number);
}
static void unexport_pwm(int chip, int number) {
	pwm_export_unexport(SYS_PWM_UNEXPORT, chip, number);
}


// enable GPIO
static bool GPIO_enable(int pin) {
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name) {
		return false; // invalid pin
	}
	if (Mode_GPIO == gpio_info[pin].active) {
		return true;  // already a GPIO
	}
	if (Mode_PWM == gpio_info[pin].active) {
		return false;  // already a PWM
	}

	// as the kernel to allocate the pin
	export(pin);

	// open a file handle to the value - to speed
	// up access assumes most read/write go to
	// this as other items (like direction) are
	// only changed occasionally.
	char *f = make_formatted_buffer(GPIO_VALUE, pin);
	gpio_info[pin].fd = open(f, O_RDWR | O_EXCL);
	free(f);

	if (gpio_info[pin].fd < 0) {

		unexport(pin);

		return false; // failed
	}

	gpio_info[pin].active = Mode_GPIO;

	return true;
}


// enable PWM
static bool PWM_enable(int pin) {
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name) {
		return false; // invalid pin
	}
	if (Mode_GPIO == gpio_info[pin].active) {
		return false; // already a GPIO
	}
	if (Mode_PWM == gpio_info[pin].active) {
		return true; // already a PWM
	}

	if (Mode_PWM != gpio_info[pin].mode) {
		return false; // no PWN for this pin
	}

	// make gpio
	//export(pin);
	//write_pin_file(GPIO_DIRECTION, pin, GPIO_DIRECTION_out "\n", CONST_STRLEN(GPIO_DIRECTION_out "\n"));

	int chip = gpio_info[pin].pwm_chip;
	int channel = gpio_info[pin].pwm_channel;

	write_file(pwm_state_file[gpio_info[pin].pwm_state], MUX_pwm "\n", CONST_STRLEN(MUX_pwm "\n"));
	export_pwm(chip, channel);
	//usleep(50000)

	char *duty_cycle = make_formatted_buffer(PWM_DUTY_CYCLE, chip, channel);
	gpio_info[pin].fd = open(duty_cycle, O_RDWR);
	free(duty_cycle);

	if (gpio_info[pin].fd < 0) {
		fprintf(stderr, "PWM failed to open\n"); fflush(stderr);
		return false;  // failed
	}

	gpio_info[pin].active = Mode_PWM;

	write_pwm_file(PWM_ENABLE, chip, channel, "0\n", 2);

	// set duty = zero
	lseek(gpio_info[pin].fd, 0, SEEK_SET);
	write(gpio_info[pin].fd, "0\n", 2);

	// set default period, polarity and enable the PWM
	write_pwm_file(PWM_PERIOD, chip, channel, MAKE_STRING(PWM_DEFAULT_PERIOD) "\n", CONST_STRLEN(MAKE_STRING(PWM_DEFAULT_PERIOD) "\n"));
	write_pwm_file(PWM_POLARITY, chip, channel, PWM_POLARITY_normal "\n", CONST_STRLEN(PWM_POLARITY_normal "\n"));
	write_pwm_file(PWM_ENABLE, chip, channel, "1\n", 2);

	return true;
}


static void PWM_set_duty(int pin, int16_t value) {
	if (pin < 0 || pin >= SIZE_OF_ARRAY(gpio_info) || NULL == gpio_info[pin].name) {
		return; // invalid pin
	}
	if (Mode_PWM != gpio_info[pin].active) {
		return;  // not a PWM
	}

	// limit to same range as Arduino
	if (value < 0) {
		value = 0;
	} else if (value > 1023) {
		value = 1023;
	}

	uint32_t duty = value * PWM_DEFAULT_PERIOD / 1023;

	char config[256];
	memset(config, 0, sizeof(config));
	int n0 = snprintf(config, sizeof(config), "%d\n", duty);

	lseek(gpio_info[pin].fd, 0, SEEK_SET);
	int n = write(gpio_info[pin].fd, config, strlen(config));
	if (n != n0) {
		fprintf(stderr, "PWM failed to write duty wrote: %d of %d\n", n, n0); fflush(stderr);
	}
	fsync(gpio_info[pin].fd);
}
