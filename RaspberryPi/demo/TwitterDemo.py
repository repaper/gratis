# Copyright 2013 Pervasive Displays, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#   http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied.  See the License for the specific language
# governing permissions and limitations under the License.


import sys
import time
import Image
import ImageDraw
import ImageFont
from EPD import EPD
import tweepy
import textwrap

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

    #import socket
    #socket.setdefaulttimeout(timeout)

    # start up tweepy streaming

    if tweepy_auth.basic:
        auth = tweepy.BasicAuthHandler(tweepy_auth.USERNAME, tweepy_auth.PASSWORD)
    else:
        auth = tweepy.OAuthHandler(tweepy_auth.CONSUMER_KEY, tweepy_auth.CONSUMER_SECRET)
        auth.set_access_token(tweepy_auth.ACCESS_TOKEN, tweepy_auth.ACCESS_TOKEN_SECRET)

    listener = StreamMonitor(epd, image, draw)
    stream = tweepy.Stream(auth, listener)
    setTerms = argv
    # stream.sample()   # low bandwith public stream
    stream.filter(track=setTerms)



class StreamMonitor(tweepy.StreamListener):

    def __init__(self, epd, image, draw, *args, **kwargs):
        super(StreamMonitor, self).__init__(*args, **kwargs)
        self._epd = epd
        self._image = image
        self._draw = draw

    def on_status(self, status):
        screen_name = status.user.screen_name.encode('utf-8')
        text = status.text.encode('utf-8')
        print('@{u:s} Said:  {m:s}'.format(u=screen_name, m=text))

        font_name = ImageFont.truetype('/usr/share/fonts/truetype/freefont/FreeSans.ttf', 14)
        font = ImageFont.truetype('/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf', 12)

        w, h = self._image.size
        self._draw.rectangle((0, 0, w, h), fill=WHITE, outline=WHITE)
        self._draw.text((20, 0), '@' + status.user.screen_name, fill=BLACK, font=font_name)
        y = 18
        for line in textwrap.wrap(status.text, 24):   # tweet(140) / 24 => 6 lines
            self._draw.text((0, y), line, fill=BLACK, font=font)
            y = y + 12

        # display image on the panel
        self._epd.display(self._image)
        self._epd.update()


    def on_error(self, error):
        print("error = {e:d}".format(e=error))
        time.sleep(5)
        # contine reading stream even after error
        return True

    def on_timeout(self):
        print("timeout occurred")
        # contine reading stream even after timeout
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
