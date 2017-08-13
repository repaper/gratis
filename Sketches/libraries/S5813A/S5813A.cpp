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

// Updated 2015-08-01 by Rei Vilo
// . Added #include Energia
// . Changed uV to mV to avoid overflows

// Updated 18-04-2016 by Chiara Ruggeri (chiara@arduino.org)
// . Added Arduino Due / Arduino M0 / Arduino Tian compatibility

// Updated 2016-2017 by Wolfgang Astleitner (mrwastl@users.sourceforge.net)
// . Added Teensy 3.1/3.2 and ESP32  compatibility

#if defined(ENERGIA)
#include <Energia.h>
#else
#include <Arduino.h>
#endif

#include "S5813A.h"


//#if defined(__MSP430_CPU__)

// TI LaunchPad defaults
// ---------------------
#if defined(__MSP430G2553__)
#warning __MSP430G2553__ for ADC_COUNTS

// LaunchPad / TI MSP430G2553 runs at 3.3V .. 3.5V
#define PIN_TEMPERATURE  A4

// but use the 2.5V internal reference - seems to be better
#define ANALOG_REFERENCE INTERNAL2V5

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   2500000L
#define ADC_MAXIMUM_mV   2500L
#define ADC_COUNTS       1024L

#elif defined(__MSP432P401R__)
#warning __MSP432P401R__ for ADC_COUNTS

#define PIN_TEMPERATURE  6

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   3300000L
#define ADC_MAXIMUM_mV   3300L
#define ADC_COUNTS       1024L

#elif defined(__MSP430F5529__)
#warning __MSP430F5529__ for ADC_COUNTS

#define PIN_TEMPERATURE  6

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   3300000L
#define ADC_MAXIMUM_mV   3300L
#define ADC_COUNTS       4096L

#elif defined(__LM4F120H5QR__)
#warning __LM4F120H5QR__ for ADC_COUNTS

#define PIN_TEMPERATURE  6

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   3300000L
#define ADC_MAXIMUM_mV   3300L
#define ADC_COUNTS       4096L

#elif defined(__CC3200R1M1RGC__)
#warning __CC3200R1M1RGC__ for ADC_COUNTS

#define PIN_TEMPERATURE  6

// ADC maximum voltage at counts
// See SWAS032E –JULY 2013–REVISED JUNE 2014
// § 3.2 Drive Strength and Reset States for Analog-Digital Multiplexed Pins
#define ADC_MAXIMUM_uV   1460000L
#define ADC_MAXIMUM_mV   1460L
#define ADC_COUNTS       4096L


// Arduino defaults
// ----------------

#elif defined(ARDUINO_ARCH_SAMD)
//  Arduino M0 / Arduino M0 Pro / Arduino Tian runs at 3.3V
#define PIN_TEMPERATURE  A0

#define ANALOG_REFERENCE DEFAULT

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   3300000L
#define ADC_MAXIMUM_mV   3300L
#define ADC_COUNTS       1024L

#elif defined(ARDUINO_ARCH_SAM) || defined(CORE_TEENSY)
// Arduino Due and Teensy run at 3.3V
#define PIN_TEMPERATURE  A0

#define ANALOG_REFERENCE DEFAULT

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   3300000L
#define ADC_MAXIMUM_mV   3300L
#define ADC_COUNTS       1024L

#elif defined(ESP32)
// ESP32 run at 3.3V, but measures from 0 to 3.6/3.9V (when using ADC_11db)
#define PIN_TEMPERATURE  A6

#define ANALOG_REFERENCE DEFAULT

// ADC maximum voltage at counts
// NOTA BENE: 4.1V-4.2V is to be determined empirically!
// it is unclear, why this doesn't match the voltage range that would be expected when using ADC_11db)
// tested with an ESP32 DevKitC : some value around 4.1
// and a DOIT Devkit ESP32 V.1: some value around 4.2
//#define ADC_MAXIMUM_uV   4100000L
//#define ADC_MAXIMUM_mV   4100L
#define ADC_MAXIMUM_uV   4200000L
#define ADC_MAXIMUM_mV   4200L
#define ADC_COUNTS       4096L

#else
// Arduino Leonardo / Atmel MEGA32U4 runs at 5V
#define PIN_TEMPERATURE  A0

// use the default 5V reference
#define ANALOG_REFERENCE DEFAULT

// ADC maximum voltage at counts
#define ADC_MAXIMUM_uV   5000000L
#define ADC_MAXIMUM_mV   5000L
#define ADC_COUNTS       1024L

#endif


// temperature chip parameters
// these may need adjustment for a particular chip
// (typical values taken from data sheet)
#define Vstart_uV 1145000L
#define Tstart_C  100
#define Vslope_uV -11040L

#define Vstart_mV 1145L
#define Vslope_mV -11


// there is a potential divider on the input, so as scale to the
// correct voltage as would be seen on the temperature output pin
// Divider:
//   Rhigh = 26.7k   Rlow = 17.8k
// (be careful to avoid overflow if values too large for "long")
#define Rdiv_high 267L
#define Rdiv_low 178L
#define REV_PD(v) ((Rdiv_high + Rdiv_low) * (v) / Rdiv_low)


// the default Temperature device
S5813A_Class S5813A(PIN_TEMPERATURE);


S5813A_Class::S5813A_Class(uint8_t input_pin) : temperature_pin(input_pin) {
}


// initialise the analog system
void S5813A_Class::begin(uint8_t input_pin)
{
	pinMode(input_pin, INPUT);
#if defined(__MSP430G2553__)
	analogReference(ANALOG_REFERENCE);
#endif
#if defined(ESP32)
	analogSetPinAttenuation(input_pin, ADC_11db);  /* 0 - 3.9V */
	analogReadResolution(12); // matches ADC_COUNTS
#endif
	this->temperature_pin = input_pin;
}


void S5813A_Class::end(void) {
}


// return sensor output voltage in uV
// not the ADC value, but the value that should be measured on the
// sensor output pin
long S5813A_Class::readVoltage(void) {
#ifndef ESP32
	long vADC = analogRead(this->temperature_pin);
#else
  // ESP32: analog reads are not very reliable: take 3 samples, determine 2 with shortest distance and average these
  long vADC = 0.0;
  long vADCser[3];
  long vdiff01, vdiff02, vdiff12;
  vADCser[0] = analogRead(this->temperature_pin);
  vADCser[1] = analogRead(this->temperature_pin);
  vADCser[2] = analogRead(this->temperature_pin);
  // calculate absolute distances between all samples
  vdiff01 = vADCser[0] - vADCser[1];  if (vdiff01 < 0.0) vdiff01 *= -1.0;
  vdiff02 = vADCser[0] - vADCser[2];  if (vdiff02 < 0.0) vdiff02 *= -1.0;
  vdiff12 = vADCser[1] - vADCser[2];  if (vdiff12 < 0.0) vdiff12 *= -1.0;
  // find two samples with shortest distance and average these
  if (vdiff01 < vdiff02) { // 0-1 < 0-2
    vADC = (vdiff12 < vdiff01)  // 1-2 < 0-1 ?
           ? (vADCser[1] + vADCser[2])  // shortest: 1-2
           : (vADCser[0] + vADCser[1]); // shortest: 0-1
  } else { // 0-2 < 0-1
    vADC = (vdiff12 < vdiff02) // 1-2 < 0-2 ?
           ? (vADCser[1] + vADCser[2])  // shortest: 1-2
           : (vADCser[0] + vADCser[2]); // shortest: 0-2
  }
  vADC /= 2.0; // two values added together: calculate average
#endif
/*
    Serial.print("analogRead=");
    Serial.print(vADC);
    Serial.print("\treadVoltage=");
    Serial.println(REV_PD((vADC * ADC_MAXIMUM_mV) / ADC_COUNTS));
 */
	return REV_PD((vADC * ADC_MAXIMUM_mV) / ADC_COUNTS);
}


// return temperature as integer in Celsius
int S5813A_Class::read(void) {
    long vADC = this->readVoltage();
    int result = Tstart_C + (1.0 * vADC - Vstart_mV) / Vslope_mV;
/*
    Serial.print("vADC=");
    Serial.print(vADC, DEC);
    Serial.print("\tresult=");
    Serial.println(result, DEC);
*/
    return result;
}
