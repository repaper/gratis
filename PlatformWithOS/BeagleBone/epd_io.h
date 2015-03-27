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


#if !defined(EPD_IO_H)
#define EPD_IO_H 1

#define panel_on_pin  GPIO_P9_12
#define border_pin    GPIO_P9_15
#define discharge_pin GPIO_P9_23
#define pwm_pin       GPIO_P9_14
#define reset_pin     GPIO_P9_26
#define busy_pin      GPIO_P9_27

#define SPI_DEVICE    "/dev/spidev1.0"
//#define SPI_DEVICE    "/dev/spidev1.1"
#define SPI_BPS       8000000

#endif
