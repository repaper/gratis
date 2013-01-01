# Copyright 2013 Pervasive Displays, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied.  See the License for the specific language
# governing permissions and limitations under the License.


# set the display size on the command line:
#   make EPD=1.44
#   make EPD=2.0
#   make EPD=2.7
# Note: "make clean" when changing display size

EPD ?= 2.7

PROG = command
SOURCES = $(wildcard *.c)
MCU = msp430g2553

PORT = /dev/ttyACM0
BPS = 9600


# detect EPD size
ifeq (1.44,$(strip ${EPD}))
DISPLAY=1_44
endif
ifeq (2.0,$(strip ${EPD}))
DISPLAY=2_0
endif
ifeq (2.7,$(strip ${EPD}))
DISPLAY=2_7
endif

# validate
ifeq (,$(strip ${EPD}))
$(error set display size on make commandline e.g. make EPD=2.7)
endif

OBJECTS = ${SOURCES:.c=.o}

CFLAGS = -Wall -Werror
CFLAGS += -DUART_BPS='${BPS}'
CFLAGS += -Os
CFLAGS = -D EPD_SIZE="EPD_${DISPLAY}"


.PHONY: all
all: prog

.PHONY: prog
prog: ${PROG}.elf
	mspdebug rf2500 'prog ${PROG}.elf'


.PHONY: com
com:
	ls -l '${PORT}'
	picocom --baud='${BPS}' --send-cmd='cat' '${PORT}'

${PROG}.elf: ${OBJECTS}
	msp430-gcc -std=gnu99 -mmcu='${MCU}' ${CFLAGS} -o '$@' ${OBJECTS}

.PHONY: clean
clean:
	rm -f *.o *.elf *~

%.o: %.c
	msp430-gcc -std=gnu99 -mmcu='${MCU}' ${CFLAGS} -o '$@' -c '$<'
