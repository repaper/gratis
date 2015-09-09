#!/usr/bin/python

import smbus
import time
import sys
import os
import subprocess
from PIL import Image
from PIL import ImageOps
import ImageDraw
import ImageFont
# import RPi.GPIO as GPIO


from EPD import EPD

bus = smbus.SMBus(1)
rtc = 0x68
hat_product = subprocess.check_output(["cat", "/proc/device-tree/hat/product"])
hat_product = hat_product[0:11]
hat_serial = subprocess.check_output(["cat", "/proc/device-tree/hat/uuid"])
serial_left = hat_serial[0:18]
serial_right = hat_serial[-18:]
serial_right = serial_right[0:17]
hat_vendor = subprocess.check_output(["cat", "/proc/device-tree/hat/vendor"])
vendor_left = hat_vendor[0:9]
vendor_middle = hat_vendor[10:21]
vendor_right = hat_vendor[-8:]
vendor_right = vendor_right[0:7]
print (hat_vendor)
print (hat_product)
print (hat_serial)

WHITE = 1
BLACK = 0

possible_fonts = [
    '/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf',            # R.Pi
    '/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf',   # R.Pi
    '/usr/share/fonts/truetype/freefont/FreeMono.ttf',                # R.Pi
]

FONT_FILE = ''

for f in possible_fonts:
    if os.path.exists(f):
        FONT_FILE = f
        break

if '' == FONT_FILE:
    raise 'no font file found'

TEXT_FONT_SIZE1 = 22
TEXT_FONT_SIZE2 = 32


def remove_rtc_module():
    exitCode = subprocess.call(["sudo", "rmmod", "rtc_ds3232"])
    if exitCode != 0:
        print ("Unable to remove RTC module, halting program");
        sys.exit()
    else:
        print ("RTC module removed");
        return;

def restore_rtc_module():
    exitCode = subprocess.call(["sudo", "modprobe", "rtc_ds3232"])
    if exitCode != 0:
        print ("Unable to restore RTC module, halting program")
        sys.exit()
    else:
        print ("RTC module restored");
        date = subprocess.check_output(["sudo", "hwclock", "-r"]);
        print (date);
        return;

def main(argv):
    """main program - display logo, report product details, test led, optional test buttons
    print 'Number of arguments:', len(sys.argv)
    print 'Argument List:', str(sys.argv)"""
    epd = EPD()
    epd.clear()

    print('panel = {p:s} {w:d} x {h:d}  version={v:s} COG={g:d} FILM={f:d}'.format(p=epd.panel, w=epd.width, h=epd.height, v=epd.version, g=epd.cog, f=epd.film))

    for file_name in argv:
        if not os.path.exists(file_name):
            sys.exit('error: image file{f:s} does not exist'.format(f=file_name))
        print('display: {f:s}'.format(f=file_name))
        display_file(epd, file_name)
        print ('Now some text')
        display_eedata(epd)
        print ('Next we test the LED')
        led_test()
        print ('Button presses left to right will flash red, green, blue and white, exits when all have been tested')
        button_test()
 	            
def display_eedata(epd):
    w = epd.width
    h = epd.height
    text_font1 = ImageFont.truetype(FONT_FILE, TEXT_FONT_SIZE1)
    text_font2 = ImageFont.truetype(FONT_FILE, TEXT_FONT_SIZE2)
    # initially set all white background
    image = Image.new('1', epd.size, WHITE)

    # prepare for drawing
    draw = ImageDraw.Draw(image)
    draw.rectangle((1, 1, w - 1, h - 1), fill=WHITE, outline=BLACK)
    draw.rectangle((2, 2, w - 2, h - 2), fill=WHITE, outline=BLACK)
    # text
    draw.text((42,3), vendor_left, fill=BLACK, font=text_font2)
    draw.text((25,30), vendor_middle, fill=BLACK, font=text_font2)
    draw.text((47,55), vendor_right, fill=BLACK, font=text_font2)
    draw.text((28,85), hat_product, fill=BLACK, font=text_font2)
    draw.text((15,125), serial_left, fill=BLACK, font=text_font1)
    draw.text((15,150), serial_right, fill=BLACK, font=text_font1)

    # display image on the panel
    epd.display(image)
    epd.update()


def display_file(epd, file_name):
    """display resized image"""

    image = Image.open(file_name)
    image = ImageOps.grayscale(image)

    rs = image.resize((epd.width, epd.height))
    bw = rs.convert("1", dither=Image.FLOYDSTEINBERG)

    epd.display(bw)
    epd.update()

    time.sleep(3) # delay in seconds

def led_test():    
    subprocess.call(["sudo", "./rgb_pwm_led_test2"])    
    return()

def button_test():
    subprocess.call(["sudo", "./buttonTest"])
    return()


# main
if "__main__" == __name__:
    if len(sys.argv) < 2:
        sys.exit('usage: {p:s} image-file'.format(p=sys.argv[0]))
    main(sys.argv[1:])
    

