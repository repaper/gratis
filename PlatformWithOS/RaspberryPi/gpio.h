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


#if !defined(GPIO_H)
#define GPIO_H 1

#include <stdbool.h>
#include <stdint.h>

// pin types
typedef enum {
	//                         Rev1    / Rev2
	GPIO_P1_08 = 14,    //     GPIO_14            Boot to Alt 0 -> UART0_TXD  ALT5 = UART1_TXD
	GPIO_P1_10 = 15,    //     GPIO_15            Boot to Alt 0 -> UART0_RXD  ALT5 = UART1_RXD
	GPIO_P1_12 = 18,    //     GPIO_18            PCM_CLK  ALT4 = SPI1_CE0_N  ALT5 = PWM0
	GPIO_P1_16 = 23,    //     GPIO_23            ALT3 = SD1_CMD   ALT4 = ARM_RTCK
	GPIO_P1_18 = 24,    //     GPIO_24            ALT3 = SD1_DAT0  ALT4 = ARM_TDO
	GPIO_P1_22 = 25,    //     GPIO_25            ALT3 = SD1_DAT1  ALT4 = ARM_TCK
	GPIO_P1_24 =  8,    //     GPIO_8             SPI0_CE0_N
	GPIO_P1_26 =  7,    //     GPIO_7             SPI0_CE1_N

	// need to change if rev1
	GPIO_P1_03 =  2,    //     GPIO_0  / GPIO_2   1K8 pull up resistor I2C0_SDA / I2C1_SDA
	GPIO_P1_05 =  3,    //     GPIO_1  / GPIO_3   1K8 pull up resistor I2C0_SCL / I2C1_SCL

	GPIO_P1_07 =  4,    //     GPIO_4             GPCLK0  ALT5 = ARM_TDI

	GPIO_P1_11 = 17,    //     GPIO_17            ALT3 = UART0_RTS ALT4 = SPI1_CE1_N ALT5 = UART1_RTS

	// need to change if rev1
	GPIO_P1_13 = 27,    //     GPIO_21 / GPIO_27  PCM_DOUT / reserved  ALT4 = SPI1_SCLK  ALT5 = GPCLK1  ALT3 = SD1_DAT3  ALT4 = ARM_TMS

	GPIO_P1_15 = 22,    //     GPIO_22            ALT3 = SD1_CLK ALT4 = ARM_TRST

	GPIO_P1_19 = 10,    //     GPIO_10            SPI0_MOSI
	GPIO_P1_21 =  9,    //     GPIO_9             SPI0_MISO
	GPIO_P1_23 = 11,    //     GPIO_11            SPI0_SCLK

} GPIO_pin_type;


// GPIO modes
typedef enum {
	GPIO_INPUT,   // as input
	GPIO_OUTPUT,  // as output
	GPIO_PWM      // as PWM output (only for P1_12
} GPIO_mode_type;


// functions
// =========

// enable GPIO system (maps device registers)
// return false if failure
bool GPIO_setup();

// release mapped device registers
bool GPIO_teardown();

// set a mode for a given GPIO pin
void GPIO_mode(GPIO_pin_type pin, GPIO_mode_type mode);

// return a value (0/1) for a given input pin
int GPIO_read(GPIO_pin_type pin);

// set or clear a given output pin
void GPIO_write(GPIO_pin_type pin, int value);

// set the PWM ration 0..1023 for hardware PWM pin (GPIO_P1_12)
void GPIO_pwm_write(GPIO_pin_type pin, uint32_t value);


#endif
