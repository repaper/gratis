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

#if !defined(EPD_EPD_H)
#define EPD_EPD_H 1

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>


typedef enum {
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;


typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;


typedef struct {
	EPD_size size;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;
	const uint8_t *gate_source;
	uint16_t gate_source_length;

	bool filler;
} EPD_type;

typedef void EPD_reader(void *buffer, uint32_t address, uint16_t length);

void EPD_initialise(EPD_type *cog, EPD_size size);
void EPD_frame_fixed(EPD_type *cog, uint8_t fixed_value);
void EPD_frame_data(EPD_type *cog, const uint8_t *image, EPD_stage stage);
void EPD_frame_cb(EPD_type *cog, uint32_t address, EPD_reader *reader, EPD_stage stage);
void EPD_line(EPD_type *cog, uint16_t line, const uint8_t *data, uint8_t fixed_value, EPD_stage stage);
void EPD_finalise(EPD_type *cog);

#endif


