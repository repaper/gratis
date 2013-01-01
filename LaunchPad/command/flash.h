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

#if !defined(EPD_FLASH_H)
#define EPD_FLASH_H 1

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>


bool FLASH_initialise(void);
void FLASH_info(uint8_t *maufacturer, uint16_t *device);
void FLASH_read(void *buffer, uint32_t address, uint16_t length);
bool FLASH_is_busy(void);
void FLASH_write_enable(void);
void FLASH_write_disable(void);
void FLASH_write(uint32_t address, void *buffer, uint16_t length);
void FLASH_sector_erase(uint32_t address);

#endif


