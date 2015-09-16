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

#if !defined(EPD_S5813A_H)
#define EPD_S5813A_H 1

#if defined(ENERGIA)
#include <Energia.h>
#else
#include <Arduino.h>
#endif


class S5813A_Class {
private:
	uint8_t temperature_pin;

	S5813A_Class(const S5813A_Class &f);  // prevent copy

public:
	int read(void);
	long readVoltage(void);  // returns micro volts

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	// allow external program to configure actual analog pin to use
	void begin(uint8_t input_pin);
	void end(void);

	S5813A_Class(uint8_t input_pin);

};

extern S5813A_Class S5813A;

#endif
