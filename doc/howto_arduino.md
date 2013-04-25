
% RePaper Arduino Howto

# Overview

This tutorial is for our 1.8" diagonal TFT display & microSD breakout board. This breakout is the best way to add a small, colorful and bright display to any project. Since the display uses 4-wire SPI to communicate and has its own pixel-addressable frame buffer, it can be used with every kind of microcontroller. Even a very small one with low memory and few pins available!

# Connecting the display to the Expansion board

There are two ways to wire up these displays - one is a more flexible method (you can use any pins on the Arduino) and the other is much faster (4-8x faster, but you are required to use the hardware SPI pins) We will begin by showing how to use the more flexible method.

# Connecting the Expansion board to Arduino

The board needs a cable to connect to the Arduino.  The EPD boards
are dual voltage and include a 3.3V regulator for the EPD panel and
level converters and so it can directly connect to the 5 Volt
Arduinos.  Note that the board uses the SPI interface, which is on
different pins depending on the particular Arduino version.  The
[Extension Board](http://repaper.org/doc/extension_board.html) has a
table of [Pin Assignments](http://repaper.org/doc/extension_board.html#pin-assignment)
for some Arduinos; the main difference is the three SPI pin (SI, SO,
CLK) location which vary between the various Arduinos an can be on
dedicated pins, overlapped with Digital I/O or shared with the ICSP
header.

# Testing the display

Once you have the display wired up, its time to run some demo sketches to verify everything is setup correctly.

1. Download our Arduino library (see bottom of page) from github by clicking on Download in the top right corner.
2. Uncompress the folder and rename it Adafruit_ST7735 - inside the folder you should see the Adafruit_ST7735.cpp andAdafruit_ST7735.h files.
3. Install the Adafruit_ST7735 library foler by placing it in your
arduinosketchfolder/libraries folder. You may have to create the libraries subfolder if this is your first library. You can read more about installing libraries in our
tutorial (http://adafru.it/aHr).
4. Restart the Arduino IDE. You should now be able to select File > Examples > Adafruit_ST7735 > graphicstest sketch. Upload the sketch to your Arduino wired as above.

Once uploaded, the Arduino should perform all the test display procedures! If you're not seeing anything - first check if you have the backlight on, if the backlight is not lit something is wrong with the power/backlight wiring. If the backlight is lit but you see nothing on the display make sure you're using our suggested wiring.

1. download + unzip file
2. drop content of libraries into libraries folder
3. open example code
4. make these connections
5. instructions on how to use the flex connector
6. include a video at the bottom of the demo (or other code) working properly so that novice users know what to expect (and to let them know when their circuit isn't working properly).

# Graphics Library

We've written a full graphics library specifically for this display which will get you up and running quickly. The code is written in C/C++ for Arduino but is easy to port to any microcontroller by rewritting the low level pin access functions.
The TFT LCD library is based off of the Adafruit GFX graphics core library. GFX has many ready to go functions that should help you start out with your project. Its not exhaustive and we'll try to update it if we find a really useful function. Right now it supports pixels, lines, rectangles, circles, round-rects, triangles and printing text as well as rotation.
Two libraries need to be downloaded and installed: first is the ST7735
library (http://adafru.it/aHm) (this contains the low-level code specific to this device), and second is the Adafruit GFX Library (http://adafru.it/aJa) (which handles graphics operations common to many displays we carry). Download both ZIP files, uncompress and rename the folders to 'Adafruit_ST7735' and 'Adafruit_GFX' respectively, place them inside your Arduino libraries folder and restart the Arduino IDE. If this is all unfamiliar, we have atutorial introducing Arduino library concepts and installation (http://adafru.it/aHr).

<PICTURE>

Check out the GFX tutorial for detailed information about what is supported and how to use it! (http://adafru.it/aJH)

# Displaying Bitmaps

# Downloads