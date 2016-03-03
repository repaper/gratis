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


import sys
import os
import subprocess
import Image
import ImageDraw
import ImageFont
from datetime import datetime
import time
from EPD import EPD
import smbus

bus = smbus.SMBus(1)
addr_temp = 0x49

WHITE = 1
BLACK = 0
FULL = False # Flag to indicate full, not partial, update at midnight rollover.

# fonts are in different places on Raspbian/Angstrom so search
possible_fonts = [
    '/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf',            # Debian B.B
#    '/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf',   # Debian B.B
    '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
    '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
#    '/usr/share/fonts/truetype/LiberationMono-Bold.ttf',              # B.B
#    '/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf'               # B.B
]

now = datetime.today()


FONT_FILE = ''
for f in possible_fonts:
    if os.path.exists(f):
        FONT_FILE = f
        break

if '' == FONT_FILE:
    raise 'no font file found'

CLOCK_FONT_SIZE = 80

# date

if (now.month in [2, 11, 12]): # February, November, December
	DATE_FONT_SIZE  = 25
	DATE_X = 10
elif (now.month in [9]): # September
        DATE_FONT_SIZE  = 24
        DATE_X = 10
elif (now.month in [1, 10]): # January, October
	DATE_FONT_SIZE  = 26
	DATE_X = 10
elif (now.month in [3, 4, 8]): # March, April, August
	DATE_FONT_SIZE  = 28
        DATE_X = 15
else: # May, June, July
	DATE_FONT_SIZE  = 32 
	DATE_X = 24

DATE_Y=50

# day of week

if (now.weekday() in [1]): # Tuesday
	WEEKDAY_FONT_SIZE  = 44
	WEEKDAY_X = 40

elif (now.weekday() in [2]): # Wednesday
	WEEKDAY_FONT_SIZE  = 44
	WEEKDAY_X = 13

elif (now.weekday() in [3, 5]): # Thursday, Saturday
	WEEKDAY_FONT_SIZE  = 44
	WEEKDAY_X = 30

else:
	WEEKDAY_FONT_SIZE  = 48 # Monday, Friday, Sunday
	WEEKDAY_X = 45

WEEKDAY_Y = 3

# temperature
TEMP_FONT_SIZE = 20
TEMP_X = 5
TEMP_Y = 80

# time
X_OFFSET = 18
Y_OFFSET = 95
COLON_SIZE = 5
COLON_GAP = 10


DAYS = [
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
]

MONTHS = [
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
]

def get_temp():
#	results = bus.read_i2c_block_data(addr_temp,0)
#	Temp = results[0] << 8 | results[1]
#	Temp = Temp >> 5
	output = subprocess.check_output(["cat", "/dev/epd/temperature"])
	Temp = int(output)*10
	Temp = float(Temp/10)
	return Temp


def main(argv):
    """main program - draw HH:MM clock on 2.70" size panel"""

    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s}  cog={g:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog))

    if 'EPD 2.7' != epd.panel:
        print('incorrect panel size')
        sys.exit(1)

    epd.clear()

    demo(epd)


def demo(epd):
    """simple partial update demo - draw a clock"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    width, height = image.size

    clock_font = ImageFont.truetype(FONT_FILE, CLOCK_FONT_SIZE)
    date_font = ImageFont.truetype(FONT_FILE, DATE_FONT_SIZE)
    weekday_font = ImageFont.truetype(FONT_FILE, WEEKDAY_FONT_SIZE)
    temp_font = ImageFont.truetype(FONT_FILE, TEMP_FONT_SIZE)

    # clear the display buffer
    draw.rectangle((0, 0, width, height), fill=WHITE, outline=WHITE)
    previous_day = 0

    first_start = True

    while True:
        while True:
            now = datetime.today()
            if now.second == 0 or first_start:
                first_start = False
                break
            time.sleep(0.2) # What is this for?

        if now.day != previous_day:
            FULL = True		
            draw.rectangle((1, 1, width - 1, height - 1), fill=WHITE, outline=BLACK)
            draw.rectangle((2, 2, width - 2, height - 2), fill=WHITE, outline=BLACK)
            draw.text((WEEKDAY_X, WEEKDAY_Y), '{w:s}'.format(w=DAYS[now.weekday()]), fill=BLACK, font=weekday_font)
            draw.text((DATE_X, DATE_Y), '{d:02d} {m:s} {y:04d}'.format(y=now.year, m=MONTHS[now.month-1], d=now.day), fill=BLACK, font=date_font)
            previous_day = now.day
        else:
            draw.rectangle((TEMP_X, TEMP_Y, width - 3, height - 3), fill=WHITE, outline=WHITE)
	   # print (width - X_OFFSET), "  ", (height - Y_OFFSET)	
	   # draw.rectangle((0,86,264,176), fill=WHITE, outline=WHITE)
#        draw.text((X_OFFSET, Y_OFFSET), '{h:02d}:{m:02d}'.format(h=now.hour, m=now.minute), fill=BLACK, font=clock_font)
	temp_str = "Temperature is " + str(get_temp()) + chr(176) + "C"
	draw.text((TEMP_X, TEMP_Y), temp_str, fill=BLACK, font=temp_font)
        draw.text((X_OFFSET, Y_OFFSET), '{h:02d}'.format(h=now.hour), fill=BLACK, font=clock_font)

        colon_x1 = width / 2 - COLON_SIZE
        colon_x2 = width / 2 + COLON_SIZE
        colon_y1 = CLOCK_FONT_SIZE / 2 + Y_OFFSET - COLON_SIZE
        colon_y2 = CLOCK_FONT_SIZE / 2 + Y_OFFSET + COLON_SIZE
        draw.rectangle((colon_x1, colon_y1 - COLON_GAP, colon_x2, colon_y2 - COLON_GAP), fill=BLACK, outline=BLACK)
        draw.rectangle((colon_x1, colon_y1 + COLON_GAP, colon_x2, colon_y2 + COLON_GAP), fill=BLACK, outline=BLACK)
        draw.text((X_OFFSET + width / 2, Y_OFFSET), '{m:02d}'.format(m=now.minute), fill=BLACK, font=clock_font)

        # display image on the panel
#	epd.clear()
        epd.display(image)
        if FULL == True:
		epd.update()
                FULL = False 
	else:
		epd.partial_update()

# main
if "__main__" == __name__:
    if len(sys.argv) < 1:
        sys.exit('usage: {p:s}'.format(p=sys.argv[0]))

    try:
        main(sys.argv[1:])
    except KeyboardInterrupt:
        sys.exit('interrupted')
        pass
