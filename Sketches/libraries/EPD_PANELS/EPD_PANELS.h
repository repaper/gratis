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

#if !defined(EPD_PANELS_H)
#define EPD_PANELS_H 1

// see the specific EPD_144 to set up this correctly

// When creating a new panel: (just copy existing EPD_144.h and update
// accordingly:
//
// need to: #define SCREEN_SIZE nnn
// then:    #include <EPD_PANELS.h>
// see below for supported nnn


// set up images from screen sizes
#if (SCREEN_SIZE == 144) && EPD_1_44_SUPPORT
#define EPD_SIZE EPD_1_44
#define EPD_PIXEL_WIDTH 128
#define EPD_PIXEL_HEIGHT 96
#define EPD_IMAGE_FILE_SUFFIX _1_44.xbm
#define EPD_IMAGE_NAME_SUFFIX _1_44_bits
#define EPD_FLASH_SECTORS_USED 1

#elif (SCREEN_SIZE == 190) && EPD_1_9_SUPPORT
#define EPD_SIZE EPD_1_9
#define EPD_PIXEL_WIDTH 144
#define EPD_PIXEL_HEIGHT 128
#define EPD_IMAGE_FILE_SUFFIX _1_9.xbm
#define EPD_IMAGE_NAME_SUFFIX _1_9_bits
#define EPD_FLASH_SECTORS_USED 1

#elif (SCREEN_SIZE == 200) && EPD_2_0_SUPPORT
#define EPD_SIZE EPD_2_0
#define EPD_PIXEL_WIDTH 200
#define EPD_PIXEL_HEIGHT 96
#define EPD_IMAGE_FILE_SUFFIX _2_0.xbm
#define EPD_IMAGE_NAME_SUFFIX _2_0_bits
#define EPD_FLASH_SECTORS_USED 1

#elif (SCREEN_SIZE == 260) && EPD_2_6_SUPPORT
#define EPD_SIZE EPD_2_6
#define EPD_PIXEL_WIDTH 232
#define EPD_PIXEL_HEIGHT 128
#define EPD_IMAGE_FILE_SUFFIX _2_6.xbm
#define EPD_IMAGE_NAME_SUFFIX _2_6_bits
#define EPD_FLASH_SECTORS_USED 2

#elif (SCREEN_SIZE == 270) && EPD_2_7_SUPPORT
#define EPD_SIZE EPD_2_7
#define EPD_PIXEL_WIDTH 264
#define EPD_PIXEL_HEIGHT 176
#define EPD_IMAGE_FILE_SUFFIX _2_7.xbm
#define EPD_IMAGE_NAME_SUFFIX _2_7_bits
#define EPD_FLASH_SECTORS_USED 2

#else
#error "Unknown EPD size: Change the #define SCREEN_SIZE to a supported value"
#endif

#endif
