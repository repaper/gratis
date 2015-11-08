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
from PIL import Image
from PIL import ImageDraw
from EPD import EPD

WHITE = 1
BLACK = 0

def main(argv):
    """main program - draw and display a test image"""

    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s} COG={g:d} FILM={f:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog, f=epd.film))

    epd.clear()

    demo(epd)


def demo(epd):
    """simple drawing demo - black drawing on white background"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)

    # three pixels in top left corner
    draw.point((0, 0), fill=BLACK)
    draw.point((1, 0), fill=BLACK)
    draw.point((0, 1), fill=BLACK)

    # lines
    draw.line([(10,20),(100,20)], fill=BLACK)
    draw.line([(10,90),(100,60)], fill=BLACK)

    # filled circle, elipse
    draw.ellipse((120, 10, 150, 40), fill=BLACK, outline=BLACK)
    draw.ellipse((120, 60, 170, 90), fill=WHITE, outline=BLACK)

    # text
    draw.text((30, 30), 'hello world', fill=BLACK)

    # display image on the panel
    epd.display(image)
    epd.update()


# main
if "__main__" == __name__:
    if len(sys.argv) < 1:
        sys.exit('usage: {p:s}'.format(p=sys.argv[0]))
    main(sys.argv[1:])
