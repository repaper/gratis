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
from EPD import EPD

WHITE = 1
BLACK = 0

# fonts are in different places on Raspbian/Angstrom so search
possible_fonts = [
    '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
    '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
    '/usr/share/fonts/truetype/LiberationMono-Bold.ttf',              # B.B
    '/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf'               # B.B
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

FONT_SIZE = 40

MAX_START = 0xffff

def main(argv):
    """main program - draw and display a test image"""

    try:
        start = int(argv[0])
    except ValueError:
        sys.exit('start is not an integer: {s:s}'.format(s=argv[0]))
    if start < 0 or start > MAX_START:
        sys.exit('object count is out of range [0..0x{m:04x}: 0x{s:04x}'.format(m=MAX_START, s=start))


    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s} COG={g:d} FILM={f:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog, f=epd.film))

    epd.clear()

    demo(epd, start)


def demo(epd, start):
    """simple partial update demo - draw random shapes"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    width, height = image.size

    font = ImageFont.truetype(FONT_FILE, FONT_SIZE)

    counter = start & 0xffff

    while True:
        draw.rectangle((0, 0, width, height), fill=WHITE, outline=WHITE)
        draw.text((0, 0), '{c:04X}'.format(c=counter), fill=BLACK, font=font)
        counter = (counter + 1) & 0xffff

        # display image on the panel
        epd.display(image)
        epd.partial_update()

# main
if "__main__" == __name__:
    if len(sys.argv) < 2:
        sys.exit('usage: {p:s} start'.format(p=sys.argv[0]))

    try:
        main(sys.argv[1:])
    except KeyboardInterrupt:
        sys.exit('interrupted')
        pass
