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
import Image
import ImageDraw
import ImageFont
from EPD import EPD

WHITE = 1
BLACK = 0


# fonts are in different places on Raspbian/Angstrom so search
possible_fonts = [
    '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
    '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
    '/usr/share/fonts/truetype/LiberationMono-Bold.ttf',              # B.B
    '/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf'               # B.B
]


FONT_FILE = ''
for f in possible_fonts:
    if os.path.exists(f):
        FONT_FILE = f
        break

if '' == FONT_FILE:
    raise 'no font file found'

TITLE_FONT_SIZE = 15


def main(argv):
    """main program - draw and display a test image"""

    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version))

    epd.clear()

    demo(epd)


def demo(epd):
    """simple barcode demo - black drawing on white background"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    width, height = image.size

    # fonts
    title_font = ImageFont.truetype(FONT_FILE, TITLE_FONT_SIZE)

    # border
    draw.rectangle((2, 2, width - 2, height - 2), fill=WHITE, outline=BLACK)

    # title
    draw.text((30, 3), 'EAN Bar Codes', font=title_font, fill=BLACK)

    # plot a bar code
    from EanBarCode import EanBarCode

    bar = EanBarCode()
    bar.drawBarCode('123456765209', draw, 20, 25, height=30)

    bar.drawBarCode('325416720596', draw, 50, 60, height=30)

    # display image on the panel
    epd.display(image)
    epd.update()


# main
if "__main__" == __name__:
    if len(sys.argv) < 1:
        sys.exit('usage: {p:s}'.format(p=sys.argv[0]))
    main(sys.argv[1:])
