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

#if !defined(EPD_PINOUT_ENERGIA_H)
#define EPD_PINOUT_ENERGIA_H 1


// For Energia, use pin numbers instead of pin names.
// * pin names are going to be deprecated
// * pin numbers ensure consistency across all LaunchPads

const int Pin_TEMPERATURE   = 6;  //  A10 = MSP432,   A6 = F5529, MSP430 = A4;

#if defined(__MSP432P401R__)
const int Pin_PANEL_ON      = 33; // P5_1 = MSP432, as P3_5, P3_6 and P3_7 are used by SPI: UCB2
#else
const int Pin_PANEL_ON      = 11; // P3_6 = MSP432, P8_1 = F5529, MSP430 = P2_3;
#endif

const int Pin_BORDER        = 13; // P5_0 = MSP432, P2_6 = F5529, MSP430 = P2_5;
const int Pin_DISCHARGE     = 12; // P5_2 = MSP432, P2_3 = F5529, MSP430 = P2_4;

const int Pin_RESET         = 10; // P6_4 = MSP432, P4_1 = F5529, MSP430 = P2_2;
const int Pin_BUSY          = 8;  // P4_6 = MSP432, P2_7 = F5529, MSP430 = P2_0;
const int Pin_EPD_CS        = 19; // P2_5 = MSP432, P2_0 = F5529, MSP430 = P2_6;
const int Pin_EPD_FLASH_CS  = 18; // P3_0 = MSP432, P2_2 = F5529, MSP430 = P2_7;

const int Pin_SW2           = PUSH2; // PUSH2 = MSP432, PUSH2 = F5529, MSP430 = PUSH2;
const int Pin_RED_LED       = RED_LED; // RED_LED = MSP432, RED_LED = F5529, MSP430 = RED_LED;

#if defined(__MSP430G2553__)
// MSP430G2553 LaunchPad IO layout

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#elif defined(__MSP430F5529__)

// __MSP430F5529__
// MSP430F5529 LaunchPad IO layout
// Pin_PWM requires a special macro P4_2_PWM
//
#if EPD_PWM_REQUIRED
// Simulated PWM on port P4_2 linked to port P7_4
// See http://forum.43oh.com/topic/4372-solved-help-software-pwm-for-the-msp430f5529/?p=39372
// Posted 17 September 2013 - 09:53 PM by energia
//
#define PM_P42_SET_MODE ((PM_TB0CCR2A << 8) | PORT_SELECTION0 | OUTPUT)
#define P4_2_PWM P7_4
const int Pin_PWM           = P4_2_PWM; // = F5529, MSP430 = P2_1;
#endif

#elif defined(__LM4F120H5QR__)

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#elif defined(__CC3200R1M1RGC__)

#if EPD_PWM_REQUIRED
const int Pin_PWM           = 9; // P6_5 = MSP432, P4_2_PWM = F5529, MSP430 = P2_1;
#endif

#elif defined(__MSP432P401R__)
// MSP432P401R LaunchPad IO layout
// Pin_PWM requires a dedicated timer task
//
#if EPD_PWM_REQUIRED
#error "No PWM on pin 9 for MSP432"
#endif

#else
#error "LaunchPad for Energia not supported"
#endif


#endif
