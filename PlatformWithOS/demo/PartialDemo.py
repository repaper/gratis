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
import random
from PIL import Image
from PIL import ImageDraw
from EPD import EPD

WHITE = 1
BLACK = 0

MAX_OBJECTS = 25
MAX_FRAMES = 1000

def main(argv):
    """main program - draw and display a test image"""

    try:
        objects = int(argv[0])
    except ValueError:
        sys.exit('object count is not an integer: {o:s}'.format(o=argv[0]))
    if objects < 1 or objects > MAX_OBJECTS:
        sys.exit('object count is out of range [1..{m:d}: {o:d}'.format(m=MAX_OBJECTS, o=objects))

    try:
        frames = int(argv[1])
    except ValueError:
        sys.exit('frame count is not an integer: {f:s}'.format(f=argv[1]))
    if frames < 1 or frames > MAX_FRAMES:
        sys.exit('frame count is out of range [1..{m:d}: {f:d}'.format(m=MAX_FRAMES, o=frames))


    epd = EPD()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s} COG={g:d} FILM={f:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog, f=epd.film))

    epd.clear()

    demo(epd, objects, frames)


def demo(epd, object_count, frame_count):
    """simple partial update demo - draw random shapes"""

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    width, height = image.size

    # repeat for number of frames
    for i in range(0, frame_count):
        for j in range(0, object_count):
            if random.randint(0, 1):
                fill = WHITE
                outline = BLACK
            else:
                fill = BLACK
                outline = WHITE
            w = random.randint(10, width / 2)
            h = random.randint(10, height / 2)
            x = random.randint(0, width - w)
            y = random.randint(0, height - h)
            box = (x, y, x + w, y + h)
            draw.rectangle(box, fill=fill, outline=outline)

        # display image on the panel
        epd.display(image)
        epd.partial_update()

# main
if "__main__" == __name__:
    if len(sys.argv) < 3:
        sys.exit('usage: {p:s} objects frames'.format(p=sys.argv[0]))
    main(sys.argv[1:])
