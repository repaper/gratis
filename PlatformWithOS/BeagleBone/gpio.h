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

// assumes 32 bit GPIO ports
#define GPIO_PIN(bank, pin) ((bank) * 32 + ((pin) & 0x1f))

#define GPIO_PWM_PIN(n) (-2 - (n))

// pin types
typedef enum {
	// Connector P8
	GPIO_P8_01 = -1,               //  GND
	GPIO_P8_02 = -1,               //  GND
	GPIO_P8_03 = GPIO_PIN(1, 6),   //  GPIO1_6
	GPIO_P8_04 = GPIO_PIN(1, 7),   //  GPIO1_7
	GPIO_P8_05 = GPIO_PIN(1, 2),   //  GPIO1_2
	GPIO_P8_06 = GPIO_PIN(1, 3),   //  GPIO1_3
	GPIO_P8_07 = GPIO_PIN(2, 2),   //  GPIO2_2   TIMER4
	GPIO_P8_08 = GPIO_PIN(2, 3),   //  GPIO2_3   TIMER7
	GPIO_P8_09 = GPIO_PIN(2, 5),   //  GPIO2_5   TIMER5
	GPIO_P8_10 = GPIO_PIN(2, 4),   //  GPIO2_4   TIMER6
	GPIO_P8_11 = GPIO_PIN(1, 13),  //  GPIO1_13
	GPIO_P8_12 = GPIO_PIN(1, 12),  //  GPIO1_12
	GPIO_P8_13 = GPIO_PIN(0, 23),  //  GPIO0_23  EHRPWM2B
	GPIO_P8_14 = GPIO_PIN(0, 26),  //  GPIO0_26
	GPIO_P8_15 = GPIO_PIN(1, 15),  //  GPIO1_15
	GPIO_P8_16 = GPIO_PIN(1, 14),  //  GPIO1_14
	GPIO_P8_17 = GPIO_PIN(0, 27),  //  GPIO0_27
	GPIO_P8_18 = GPIO_PIN(2, 1),   //  GPIO2_1
	GPIO_P8_19 = GPIO_PIN(0, 22),  //  GPIO0_22  EHRPWM2A
	GPIO_P8_20 = GPIO_PIN(1, 31),  //  GPIO1_31
	GPIO_P8_21 = GPIO_PIN(1, 30),  //  GPIO1_30
	GPIO_P8_22 = GPIO_PIN(1, 5),   //  GPIO1_5
	GPIO_P8_23 = GPIO_PIN(1, 4),   //  GPIO1_4
	GPIO_P8_24 = GPIO_PIN(1, 1),   //  GPIO1_1
	GPIO_P8_25 = GPIO_PIN(1, 0),   //  GPIO1_0
	GPIO_P8_26 = GPIO_PIN(1, 29),  //  GPIO1_29
	GPIO_P8_27 = GPIO_PIN(2, 22),  //  GPIO2_22
	GPIO_P8_28 = GPIO_PIN(2, 24),  //  GPIO2_24
	GPIO_P8_29 = GPIO_PIN(2, 23),  //  GPIO2_23
	GPIO_P8_30 = GPIO_PIN(2, 25),  //  GPIO2_25
	GPIO_P8_31 = GPIO_PIN(0, 10),  //  GPIO0_10  UART5_CTSN
	GPIO_P8_32 = GPIO_PIN(0, 11),  //  GPIO0_11  UART5_RTSN
	GPIO_P8_33 = GPIO_PIN(0, 9),   //  GPIO0_9   UART4_RTSN
	GPIO_P8_34 = GPIO_PIN(2, 17),  //  GPIO2_17  UART3_RTSN
	GPIO_P8_35 = GPIO_PIN(0, 8),   //  GPIO0_8   UART4_CTSN
	GPIO_P8_36 = GPIO_PIN(2, 16),  //  GPIO2_16  UART3_CTSN
	GPIO_P8_37 = GPIO_PIN(2, 14),  //  GPIO2_14  UART5_TXD
	GPIO_P8_38 = GPIO_PIN(2, 15),  //  GPIO2_15  UART5_RXD
	GPIO_P8_39 = GPIO_PIN(2, 12),  //  GPIO2_12
	GPIO_P8_40 = GPIO_PIN(2, 13),  //  GPIO2_13
	GPIO_P8_41 = GPIO_PIN(2, 10),  //  GPIO2_10
	GPIO_P8_42 = GPIO_PIN(2, 11),  //  GPIO2_11
	GPIO_P8_43 = GPIO_PIN(2, 8),   //  GPIO2_8
	GPIO_P8_44 = GPIO_PIN(2, 9),   //  GPIO2_9
	GPIO_P8_45 = GPIO_PIN(2, 6),   //  GPIO2_6
	GPIO_P8_46 = GPIO_PIN(2, 7),   //  GPIO2_7

	// Connector P9
	GPIO_P9_01 = -1,               //  GND
	GPIO_P9_02 = -1,               //  GND
	GPIO_P9_03 = -1,               //  DC_3.3V
	GPIO_P9_04 = -1,               //  DC_3.3V
	GPIO_P9_05 = -1,               //  VDD_5V
	GPIO_P9_06 = -1,               //  VDD_5V
	GPIO_P9_07 = -1,               //  SYS_5V
	GPIO_P9_08 = -1,               //  SYS_5V
	GPIO_P9_09 = -1,               //  PWR_BUT
	GPIO_P9_10 = -1,               //  SYS_RESET
	GPIO_P9_11 = GPIO_PIN(0, 30),  //  GPIO0_30  UART4_RXD
	GPIO_P9_12 = GPIO_PIN(1, 28),  //  GPIO1_28
	GPIO_P9_13 = GPIO_PIN(0, 31),  //  GPIO0_31  UART4_TXD
	GPIO_P9_14 = GPIO_PIN(1, 18),  //  GPIO1_18  EHRPWM1A
	GPIO_P9_15 = GPIO_PIN(1, 16),  //  GPIO1_16
	GPIO_P9_16 = GPIO_PIN(1, 19),  //  GPIO1_19  EHRPWM1B
	GPIO_P9_17 = GPIO_PIN(0, 5),   //  GPIO0_5   I2C1_SCL
	GPIO_P9_18 = GPIO_PIN(0, 4),   //  GPIO0_4   I2C1_SDA
	GPIO_P9_19 = GPIO_PIN(0, 13),  //  GPIO0_13  I2C2_SCL
	GPIO_P9_20 = GPIO_PIN(0, 12),  //  GPIO0_12  I2C2_SDA
	GPIO_P9_21 = GPIO_PIN(0, 3),   //  GPIO0_3   UART2_TXD
	GPIO_P9_22 = GPIO_PIN(0, 2),   //  GPIO0_2   UART2_RXD
	GPIO_P9_23 = GPIO_PIN(1, 17),  //  GPIO1_17
	GPIO_P9_24 = GPIO_PIN(0, 15),  //  GPIO0_15  UART1_TXD
	GPIO_P9_25 = GPIO_PIN(3, 21),  //  GPIO3_21
	GPIO_P9_26 = GPIO_PIN(0, 14),  //  GPIO0_14  UART1_RXD
	GPIO_P9_27 = GPIO_PIN(3, 19),  //  GPIO3_19
	GPIO_P9_28 = GPIO_PIN(3, 17),  //  GPIO3_17  SPI1_CS0
	GPIO_P9_29 = GPIO_PIN(3, 15),  //  GPIO3_15  SPI1_D0
	GPIO_P9_30 = GPIO_PIN(3, 16),  //  GPIO3_16  SPI1_D1
	GPIO_P9_31 = GPIO_PIN(3, 14),  //  GPIO3_14  SPI1_SCLK
	GPIO_P9_32 = -1,               //  VADC
	GPIO_P9_33 = -1,               //  AIN4
	GPIO_P9_34 = -1,               //  AGND
	GPIO_P9_35 = -1,               //  AIN6
	GPIO_P9_36 = -1,               //  AIN5
	GPIO_P9_37 = -1,               //  AIN2
	GPIO_P9_38 = -1,               //  AIN3
	GPIO_P9_39 = -1,               //  AIN0
	GPIO_P9_40 = -1,               //  AIN1
	GPIO_P9_41 = -1,               //  CLKOUT2 / GPIO3_20
	GPIO_P9_42 = -1,               //  GPIO0_7 / GPIO3_18
	GPIO_P9_43 = -1,               //  GND
	GPIO_P9_44 = -1,               //  GND
	GPIO_P9_45 = -1,               //  GND
	GPIO_P9_46 = -1,               //  GND

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
int GPIO_read(int pin);

// set or clear a given output pin
void GPIO_write(GPIO_pin_type pin, int value);

// set the PWM ration 0..1023 for hardware PWM pin (GPIO_P1_12)
void GPIO_pwm_write(int pin, uint32_t value);


#endif
