# Copyright 2013-2015 Pervasive Displays, Inc.
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
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from datetime import datetime
import time
from EPD import EPD

WHITE = 1
BLACK = 0

# fonts are in different places on Raspbian/Angstrom so search
possible_fonts = [
    '/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf',            # Debian B.B
    '/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf',   # Debian B.B
    '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
    '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
    '/usr/share/fonts/truetype/LiberationMono-Bold.ttf',              # B.B
    '/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf',              # B.B
    '/usr/share/fonts/TTF/FreeMonoBold.ttf',                          # Arch
    '/usr/share/fonts/TTF/DejaVuSans-Bold.ttf'                        # Arch
]


FONT_FILE = ''
for f in possible_fonts:
    if os.path.exists(f):
        FONT_FILE = f
        break

if '' == FONT_FILE:
    raise 'no font file found'

class Settings27(object):
    # fonts
    CLOCK_FONT_SIZE = 100
    DATE_FONT_SIZE  = 42
    WEEKDAY_FONT_SIZE  = 42

    # time
    X_OFFSET = 5
    Y_OFFSET = 3
    COLON_SIZE = 5
    COLON_GAP = 10

    # date
    DATE_X = 10
    DATE_Y = 90

    WEEKDAY_X = 10
    WEEKDAY_Y = 130

class Settings20(object):
    # fonts
    CLOCK_FONT_SIZE = 46
    DATE_FONT_SIZE  = 30
    WEEKDAY_FONT_SIZE  = 30

    # time
    X_OFFSET = 25
    Y_OFFSET = 3
    COLON_SIZE = 3
    COLON_GAP = 5

    # date
    DATE_X = 10
    DATE_Y = 40

    WEEKDAY_X = 10
    WEEKDAY_Y = 65

DAYS = [
    "MONDAY",
    "TUESDAY",
    "WEDNESDAY",
    "THURSDAY",
    "FRIDAY",
    "SATURDAY",
    "SUNDAY"
]

def main(argv):
    """main program - draw HH:MM clock on 2.70" size panel"""

    global settings
    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s} COG={g:d} FILM={f:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog, f=epd.film))

    if 'EPD 2.7' == epd.panel:
        settings = Settings27()
    elif 'EPD 2.0' == epd.panel:
        settings = Settings20()
    else:
        print('incorrect panel size')
        sys.exit(1)

    epd.clear()

    demo(epd, settings)


def demo(epd, settings):
    """draw a clock"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    width, height = image.size

    clock_font = ImageFont.truetype(FONT_FILE, settings.CLOCK_FONT_SIZE)
    date_font = ImageFont.truetype(FONT_FILE, settings.DATE_FONT_SIZE)
    weekday_font = ImageFont.truetype(FONT_FILE, settings.WEEKDAY_FONT_SIZE)

    # initial time
    now = datetime.today()

    while True:
        # clear the display buffer
        draw.rectangle((0, 0, width, height), fill=WHITE, outline=WHITE)

        # border
        draw.rectangle((1, 1, width - 1, height - 1), fill=WHITE, outline=BLACK)
        draw.rectangle((2, 2, width - 2, height - 2), fill=WHITE, outline=BLACK)

        # date
        draw.text((settings.DATE_X, settings.DATE_Y), '{y:04d}-{m:02d}-{d:02d}'.format(y=now.year, m=now.month, d=now.day), fill=BLACK, font=date_font)
        # day
        draw.text((settings.WEEKDAY_X, settings.WEEKDAY_Y), '{w:s}'.format(w=DAYS[now.weekday()]), fill=BLACK, font=weekday_font)

        # hours
        draw.text((settings.X_OFFSET, settings.Y_OFFSET), '{h:02d}'.format(h=now.hour), fill=BLACK, font=clock_font)

        # colon
        colon_x1 = width / 2 - settings.COLON_SIZE
        colon_x2 = width / 2 + settings.COLON_SIZE
        colon_y1 = settings.CLOCK_FONT_SIZE / 2 + settings.Y_OFFSET - settings.COLON_SIZE
        colon_y2 = settings.CLOCK_FONT_SIZE / 2 + settings.Y_OFFSET + settings.COLON_SIZE
        draw.rectangle((colon_x1, colon_y1 - settings.COLON_GAP, colon_x2, colon_y2 - settings.COLON_GAP), fill=BLACK, outline=BLACK)
        draw.rectangle((colon_x1, colon_y1 + settings.COLON_GAP, colon_x2, colon_y2 + settings.COLON_GAP), fill=BLACK, outline=BLACK)

        # minutes
        draw.text((settings.X_OFFSET + width / 2, settings.Y_OFFSET), '{m:02d}'.format(m=now.minute), fill=BLACK, font=clock_font)

        # display image on the panel
        epd.display(image)
        epd.update()

        # wait for next minute
        while True:
            now = datetime.today()
            if now.second == 0:
                break
            time.sleep(0.5)


# main
if "__main__" == __name__:
    if len(sys.argv) < 1:
        sys.exit('usage: {p:s}'.format(p=sys.argv[0]))

    try:
        main(sys.argv[1:])
    except KeyboardInterrupt:
        sys.exit('interrupted')
        pass
