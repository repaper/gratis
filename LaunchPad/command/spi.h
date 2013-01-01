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

#if !defined(EPD_SPI_H)
#define EPD_SPI_H 1

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>


void SPI_initialise(void);
void SPI_put(uint8_t c);
uint8_t SPI_get(void);
uint8_t SPI_transfer(uint8_t c);
void SPI_put_wait(uint8_t c);
void SPI_send(uint8_t port2_cs_pin, const uint8_t *buffer, uint16_t length);

#endif
