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

#if !defined(EPD_SERIAL_H)
#define EPD_SERIAL_H 1

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>


void Serial_initialise(void);
uint8_t Serial_getc(void);
uint16_t Serial_gethex(bool echo);
void Serial_putc(uint8_t c);
void Serial_puts(const char *s);
void Serial_puthex_double(uint32_t n);
void Serial_puthex_word(uint16_t n);
void Serial_puthex_byte(uint8_t n);
void Serial_puthex_dump(uint32_t address, const void *buffer, uint16_t length);

#endif


