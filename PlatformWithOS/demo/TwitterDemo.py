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
import time
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from EPD import EPD
import tweepy
import textwrap
import socket

# create this from tweepy_auth.py-sample
import tweepy_auth


# colours
BLACK = 0
WHITE = 1

def main(argv):
    """main program - display list of images"""

    epd = EPD()

    epd.clear()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version))

    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)

    # set a longer timeout on socket operations
    socket.setdefaulttimeout(60)

    # find some fonts
    # fonts are in different places on Raspbian/Angstrom so search
    possible_name_fonts = [
#        '/usr/share/fonts/truetype/freefont/FreeSans.ttf',                # R.Pi
        '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
        '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
        '/usr/share/fonts/truetype/LiberationMono-Bold.ttf',              # B.B
        '/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf'               # B.B
        '/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf'                    # Arch
    ]

    possible_message_fonts = [
        '/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf',      # R.Pi
        '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf',            # R.Pi
        '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
        '/usr/share/fonts/truetype/LiberationSans-Regular.ttf',           # B.B
        '/usr/share/fonts/truetype/DejaVuSans.ttf'                        # B.B
        '/usr/share/fonts/TTF/DejaVuSans.ttf'                             # Arch
    ]


    name_font_name = find_font(possible_name_fonts)
    if '' == name_font_name:
        raise 'no name font file found'

    message_font_name = find_font(possible_message_fonts)
    if '' == message_font_name:
        raise 'no message font file found'

    name_font = ImageFont.truetype(name_font_name, 14)
    message_font = ImageFont.truetype(message_font_name, 12)

    # start up tweepy streaming

    if tweepy_auth.basic:
        auth = tweepy.BasicAuthHandler(tweepy_auth.USERNAME, tweepy_auth.PASSWORD)
    else:
        auth = tweepy.OAuthHandler(tweepy_auth.CONSUMER_KEY, tweepy_auth.CONSUMER_SECRET)
        auth.set_access_token(tweepy_auth.ACCESS_TOKEN, tweepy_auth.ACCESS_TOKEN_SECRET)

    listener = StreamMonitor(epd, image, draw, name_font, message_font)
    stream = tweepy.Stream(auth, listener)
    setTerms = argv
    # stream.sample()   # low bandwidth public stream
    stream.filter(track=setTerms)


def find_font(font_list):
    """find a font file from a list of possible paths"""
    for f in font_list:
        if os.path.exists(f):
            return f
    return ''


class StreamMonitor(tweepy.StreamListener):
    """class to receive twitter message"""

    def __init__(self, epd, image, draw, name_font, message_font, *args, **kwargs):
        super(StreamMonitor, self).__init__(*args, **kwargs)
        self._epd = epd
        self._image = image
        self._draw = draw
        self._name_font = name_font
        self._message_font = message_font

    def on_status(self, status):
        screen_name = status.user.screen_name.encode('utf-8')
        text = status.text.encode('utf-8')
        print('@{u:s} Said:  {m:s}'.format(u=screen_name, m=text))

        w, h = self._image.size
        self._draw.rectangle((0, 0, w, h), fill=WHITE, outline=WHITE)
        self._draw.text((20, 0), '@' + status.user.screen_name, fill=BLACK, font=self._name_font)
        y = 18
        for line in textwrap.wrap(status.text, 24):   # tweet(140) / 24 => 6 lines
            self._draw.text((0, y), line, fill=BLACK, font=self._message_font)
            y = y + 12

        # display image on the panel
        self._epd.display(self._image)
        self._epd.update()


    def on_error(self, error):
        print("error = {e:d}".format(e=error))
        time.sleep(5)
        # continue reading stream even after error
        return True

    def on_timeout(self):
        print("timeout occurred")
        # continue reading stream even after timeout
        return True


# main
if "__main__" == __name__:
    if len(sys.argv) < 2:
        sys.exit('usage: {p:s} image-file'.format(p=sys.argv[0]))

    try:
        main(sys.argv[1:])
    except KeyboardInterrupt:
        sys.exit('interrupted')
        pass
