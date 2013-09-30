# Raspberry Pi E-Ink Driver

# Driver Programs - Directory "driver"

* gpio_test - simple test for GPIO driver
* epd_test - test program for direct driving EPD panel
* epd_fuse - present EPD as a file for easy control


## Compiling

These test programs should compile with no additional libraries, but
the EPD driver needs the fuse development library installed.

~~~~~
sudo apt-get install libfuse-dev
~~~~~

### GPIO Test

Connect two LEDs each with a limiting resistor (1k..2k); first
resistor connects from `P1_10` to anode of LED1, second resistor
connects from `P1_12` to anode of LED2, and both LED cathode connect
to ground `P!_14`.  When the program is started LED1 will light and
LED2 will brighten and dim under PWM control.  Both LEDs will turn off
as program finishes (takes about a minute).  There are some other
commented out LED flashing loops in this test.

Build and run using:

~~~~~
make gpio_test
sudo ./gpio_test
~~~~~


### EPD Test

This will first clear the panel then display a series of images (all
2.0" images from Arduino example).  This need the Linux SPI module
installed.

Build and run using:

~~~~~
sudo modprobe spi-bcm2708
make epd_test
sudo ./epd_test
~~~~~

### EPD fuse

This allows the display to be represented as a virtual director of files, which are:

File         Read/Write   Description
--------     -----------  ---------------------------------
version      Read Only    The driver version number
panel        Read Only    String describing the panel and giving its pixel width and height
current      Read Only    Binary image that  matches the currently displayed image (big endian)
display      Read Write   Image being assembled for next display (big endian)
temperature  Read Write   Set this to the current temperature in Celsius
command      Write Only   Execute display operation
BE           Directory    Big endian version of current and display
LE           Directory    Little endian version of current and display

Command   Byte   Description
--------  -----  --------------------------------
'C'       0x43   Clear the EPD, set `current` to all zeros, `display` is not affected
'U'       0x5A   Erase `current` from EPD, output `display` to EPD, copy display to `current`

Notes:

* The default bit ordering for the display is big endian i.e. the top left pixel is
  the value 0x80 in the first byte.
* The `BE` directory is the same as the root `current` and `display`.
* The `LE` directory `current` and `display` reference the top left pixel as 0x01
  in the first byte.
* The `current_inverse` and `display_inverse` represent black as zero (0) and white as one (1)
  while those item without the suffix represent the display's natural coding (0=>white, 1=>black)
* The particular combination of `BE/display_inverse` is used in the Python EPD demo
  since it fits better with the Imaging library used.


Build and run using:

~~~~~
make epd_fuse
sudo modprobe spi-bcm2708
sudo mkdir /tmp/epd
sudo ./epd_fuse --panel=2.0 /tmp/epd
cat /tmp/epd/version
cat /tmp/epd/panel
echo C > /tmp/epd/command
./xbm2bin < cat_2_0.xbm > /tmp/epd/display
echo U > /tmp/epd/command
# try displaying other images
# to shut down:
sudo umount -f /tmp/fuse
rmdir /tmp/fuse
~~~~~


# Starting EPD FUSE at Boot

Need to install the launchd configuration in `/etc/init` and install
the EPD FUSE program in /usr/sbin, there is a make target that does
this.

~~~~~
sudo make install
sudo service epd-fuse start
ls -l /dev/epd
# to stop
sudo service epd-fuse stop
# to remove
sudo make remove
~~~~~


# Python Demo Programs - directory "demo"

These need the PIL library installed:

~~~~~
sudo apt-get install python-imaging
~~~~~


## Drawing Demo

Draw some lines, graphics and text

~~~~~
python DrawDemo.py
~~~~~


## Image demo

* Accepts a lists of image files on the command line.
* Converts them to grey scale to ensure formats like PNG, JPEG and GIF will work.
* Inverts the image since the E-Ink panel is reverse (i.e. black on white).
* Converts image to single bit (PIL "1" mode).
* Display the middle of the image (using crop function).
* Delay.
* Display the re-sized image.
* Delay before fetching next file.


Note if scratch is installed on the system, the following commands will
show some cartoon animals.  The images when re-sized will be distorted
if the aspect ration of the original image is not the same as the
display.

~~~~~
python ImageDemo.py /usr/share/scratch/Media/Costumes/Animals/cat*
python ImageDemo.py /usr/share/scratch/Media/Costumes/Animals/d*.png
~~~~~

## Twitter Demo

Setup easy_install and use it to get pip, then use pip to get tweepy.
Copy the sample configuration an edit to include your authentication
information.  Rather than using the basic authentication it is better
to set up a Twitter App and generate a token for this.  The token
generation is quick at
[https://dev.twitter.com/](https://dev.twitter.com/).  After creating
the App,just click the button to create an access token.

Use *Ctrl-C* to stop this program.

~~~~~
sudo apt-get install python-setuptools
sudo easy_install pip
sudo pip install tweepy
# setup the config
cp tweepy_auth.py-SAMPLE tweepy_auth.py
# *** edit the config
# run the demo (this watches for linux)
python TwitterDemo.py linux
~~~~~


## Partial Demo

Display random overlapping rectangles using partial update.  First
argument is number of rectangle to generate before updating the EPD,
second number is the number of frames to display before the program
exits.

~~~~~
python PartialDemo.py 3 20
~~~~~


## Counter Demo

Display a 4 digit hex counter uses partial update to only change the
updated digits.

Use *Ctrl-C* to stop this program.

~~~~~
python CounterDemo.py 3 20
~~~~~


# E-Ink Panel Board Connections

This is for connection to the Evaluation board.

Pin Number   Description       Colour   Raspberry Pi
----------   ---------------   ------   -------------
1.           Vcc 3V            Red      P1-01
2.           *(LED1)*          White    -
3.           *(UART_RX)*       Grey     -
4.           *(UART_TX)*       Purple   -
5.           *(SW2)*           Blue     -
6.           Temperature       Green    - (need external ADC if required)
7.           SPI\_CLK          Yellow   P1-23
8.           BUSY              Orange   P1-22
9.           PWM               Brown    P1-12
10.          /RESET            Black    P1-18
11.          PANEL\_ON         Red      P1-16
12.          DISCHARGE         White    P1-10
13.          BORDER_CONTROL    Grey     P1-08
14.          SPI_MISO          Purple   P1-21
15.          SPI_MOSI          Blue     P1-19
16.          *(RST/SBWTDIO)*   Green    -           -            -
17.          *(TEST/SBWTCK)*   Yellow   -           -            -
18.          /FLASH\_CS        Orange   P1-26
19.          /EPD\_CS          Brown    P1-24
20.          GND               Black    P1-25


# TO DO / BUGS

* Verify the compensation timer.

* Other demos, perhaps.
