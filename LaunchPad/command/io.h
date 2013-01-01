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


#if !defined(EPD_IO_H)
#define EPD_IO_H 1


#if !defined(CLK_MHz)
#define CLK_MHz 16
#endif


// IO layout
// Port 1
#define PORT1_RED_LED     BIT0
#define PORT1_RXD         BIT1
#define PORT1_TXD         BIT2
#define PORT1_SW2         BIT3
#define PORT1_TEMPERATURE BIT4
#define PORT1_SPI_CLK     BIT5
#define PORT1_SPI_MISO    BIT6
#define PORT1_SPI_MOSI    BIT7

// Port 2
#define PORT2_PWM         BIT0
#define PORT2_BUSY        BIT1
#define PORT2_RESET       BIT2
#define PORT2_PANEL_ON    BIT3
#define PORT2_DISCHARGE   BIT4
#define PORT2_BORDER      BIT5
#define PORT2_EPD_CS      BIT6
#define PORT2_FLASH_CS    BIT7

#define PORT1_DIR   (PORT1_RED_LED)
#define PORT2_DIR   (PORT2_RESET | PORT2_PANEL_ON | PORT2_DISCHARGE | PORT2_BORDER | PORT2_EPD_CS | PORT2_FLASH_CS)

#define PORT1_INIT  0
#define PORT2_INIT  (PORT2_FLASH_CS)

#endif
