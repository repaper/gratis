% RePaper Code

# Code


## Source Code Repository

The source code to the Repaper software is hosted by
[GitHub](https://github.com/repaper/gratis). The [example programs](#example-programs) are in
[Sketches](https://github.com/repaper/gratis/tree/master/Sketches) directory.


## Development Tools

This project officially supports both the Arduino-based and Ti LaunchPad platforms.

### Arduino

The [Arduino web site](http://www.arduino.cc) has download links for
Windows, Mac OS/X and other operating systems.

Note: [Java](http://java.com) is necessary to run the GUI, but it is
possible to install a command line only version.

### TI LaunchPad

The [Energia](http://energia.nu/) IDE can be downloaded from their
[Home Page](http://energia.nu/), Windows, Mac OS/X and Linux are
supported and this needs [Java](http://java.com) to be installed.


# Example Programs

**IMPORTANT NOTES for COG G1/G2 and different Panel Versions**

**CURRENTLY In Progress: New way to access demos - Read CAREFULLY!**

1. There are now three panel versions and three corresponding drivers:
  a) Panel: **V110 G1**  Driver: `EPD_V110_G1`.
  b) Panel: **V230 G2**  Driver: `EPD_V230_G2`. (does not allow partial update)
  c) Panel: **V231 G2**  Driver: `EPD_V231_G2`. (supports new 1.9" and 2.6" size panels)

2. Each on the demos is being canged to make example loading easier.
   This uses a Ruby script (`make-examples`) that converts the
   following demos into examples thata are placed into the `examples`
   folder for each driver.  A Separate example is created for each
   panel size so not editing is needed to try any panel.  (Note: The
   base demo files contain items like `{% SECTION:item %}`that are
   changes by the script)

3. Load example by **File** → **Examples** → **EPD_V**vvv**_G**g → **demo_**sss
   (where vvv=Film[110,230,231], g=COG[1,2] and sss=Size[144,190,200,260,270])

4. Demos updated: `demo`, `command`, `flash_loader` and `thermo`

5. Graphics `EPD_GFX` made into a single header file so tha the
   theremo examples will appear along with other demos.  No need for
   multiple EPD_GFX version.

6. Panels using COG G2 can be used with current extension board

7. Panels using COG G2 does not use PWM - this can be disconnected from
   the LaunchPad/Arduino.

---

The example programs support both the Arduino (Atmel AVR) and the
Energia (TI LaunchPad with MSP430G2553).

The files include some conditional code to switch between the two platforms.
This code does the following:

1. Convert PROGMEM types to normal types for the TI MCU since it has a unified
   address space.

2. Adjust the I/O pins definitions and ADC reference for differences between
   the two platforms.

## Demo Sketch

> Link to the [demo source](https://github.com/repaper/gratis/tree/master/Sketches/demo).

This example first clears the screen, then toggles between two images.
Needs the serial port (9600 8N1) connected and displays the version,
temperature and compensation values on each cycle.

This is built upon the EPD API in the libraries folder and shows how
to use the API to display images from the MCU FLASH.  Only a few images
are possible to be stored since the on-chip FLASH is limited.

Not: This will not run on the TI LaunchPad with a 2.7" display as the
resulting code exceeds the 16kB memory size, only 1.44" and 2.0" will
fit on this platform.

## Command Sketch

> Link to the [command source](https://github.com/repaper/gratis/tree/master/Sketches/command).

A command-line example that accepts single character command from the
serial port (9600 8N1).  Functions include XBM upload to the SPI FLASH
chip on the EPD evaluation board, display image from this FLASH and
several other functions.

Use the `h` command on the serial port (9600 8N1) to obtain a list of
commands.  Some of the commands are shown like `e<ss>` this *<ss>*
represents a two digit FLASH sector number in the range *00..ff* (a
total of 256 sectors).  The 1.44" and 2.0" display images take one sector
but the 2.7" displays take two adjacent sectors.

When using the serial monitor on Arduino/Energia IDE any command that
take a hex number as parameter needs a `<space>` character after it, as
the **Send** button will not automatically add a CR/LF.  For single
letter commands like the `t` temperature sensor read just type the
character and click **Send**.

The 4 stage display cycle is split into two separate commands. The `r`
command removes an image and the `i` command displays an image.
e.g. if the current image was from sector 30 and you wanted to change
to sector 43 then type `r30<space>i43<space>` into the serial monitor
and click **Send**.

The upload command `u` need a terminal emulator with ASCII upload
capability or the ability to respond to a paste of the entire contents
of an XBM file.  Also note that the `u` command does not erase the
sector before uploading.  To use the upload to upload an XBM into
sector 3b for example type `u3b<space>` then start the ASCII upload or
paste the contents of the XBM file into the terminal window on upload
completion an image size message is displayed.

The image stored is compatible with the flash_loader sketch as
described below and that program can be used to cycle through a set of
images uploaded by this program.


## Flash Loader Sketch

> Link to the [flash loader source](https://github.com/repaper/gratis/tree/master/Sketches/flash_loader).

this program has two modes of operation:

1. Copy a #included image to the FLASH chip on the eval board.  define
   the image name and the destination sector.  After programming the
   image will be displayed

2. Display a sequence of images from the FLASH chip on the eval board.
   A list of sector numbers an millisecod delay times defined by the
   `DISPLAY_LIST` macro to enable this mode.  In this mode the flash
   programming does not occur.  The images are stored in the same
   format as the command program above, so any images uploaded by it
   can be displayed by this program


## Thermo Sketch (Arduino Mega only)

> Link to the [thermo source](https://github.com/repaper/gratis/tree/master/Sketches/thermo).

A frame buffer graphic demo uses 4800 bytes of SRAM for 2.0" display
so is restricted to Arduino Mega or ATmega1280/ATmega2560 chip
designs.  It draws a temperature display in digits and a simple
scale plus a few graphic elements.  Delays for a minute then refreshes
the display.


## Amslide Sketch (AlaMode)

> Link to the [amslide source](https://github.com/repaper/gratis/tree/master/Sketches/amslide).

A demonstration slide show for the 2.0" display connected to a
[Wyolum](http://www.wyolum.com)
[AlaMode](http://wyolum.com/projects/alamode/).  This is basically an
Arduino Uno (ATMega328) type of device that can be connected directly
to a Raspberry-Pi and programmed from the Arduino IDE running on a
Raspberry-Pi (see the Wyolum web sit to [get started](http://wyolum.com/projects/alamode/alamode-getting-started/).

This demo simply displays images from a card in the AlaMode microSD
slot; a file `index.txt` lists the files to be displayed and how many
seconds to display the image.  The format is that each line has the
number of seconds as a decimal integer, a space and the image file
path.

The images are binary files that match the display, there is a zip
file containing some demo images and a sample `index.txt`.  Just unzip
this file to the root of a micro SD card, download the amdemo.ino and
the images will be displayed continuously. When the serial monitor is
running the names of the files will be displayed.

The images in the sample are derived from the XBM library images using
the Command: `tail -n +4 "${xbm}" | xxd -r -p > "${bin}"` Where the
variables `xbm` represents the XBM source file and `bin`
represents the binary output file name.  This results in the bytes in
the binary file having the same values as the hex numbers in the XBM
file.  Note that the Arduino SD code uses the 8.3 file name format so
choose a compatible name e.g. `cat.20`

Portability - This should work on other Arduinos that have an SD or
micro SD interface either directly attached or on a plug-in shield; but
may need some changes to I/O pin order if the SD CS is not routed to
Pin 10 as in AlaMode case.

## Libraries

> Link to the [libraries source](https://github.com/repaper/gratis/tree/master/Sketches/libraries).
(copy all of these to you local libraries folder)

* **Images** - Sample XBM files.  The demo program includes two of
  these directly.  The Command program can use these files for its
  upload command.
* **FLASH** - Driver for the SPI FLASH chip on the EPD eval board.
* **EPD_V**vvv**_G**g - E-Ink Panel driver (Panel Vvvv COG Gg).
* **EPD_GFX** - This sub-classes the
  [Adafruit_GFX library](https://github.com/adafruit/Adafruit-GFX-Library)
  which needs to be downloaded an installed in to the libraries folder
  an named **Adafruit_GFX**.  **IMPORTANT** - This library module needs at
  least 8 kBytes of SRAM on the AVR MCU to function.  So can only this
  can be used with Arduino Mega or ATmega1280/ATmega2560 chip designs.
* **S5813A** - Temperature sensor driver.


# Connection of EPD board to Arduino

> See: Using the Extension board with Ardunio ([HTML](http://learn.adafruit.com/repaper-eink-development-board), [PDF](http://learn.adafruit.com/downloads/pdf/repaper-eink-development-board.pdf)) by [Adafruit](http://www.adafruit.com)

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

![EPD with Arduino Mega Photo by <a href="http://learn.adafruit.com/assets/8320">Bill Earl</a>](/images/expansion_board/epd-mega.jpg)

# Connection of EPD board to LaunchPad

The board simply plugs onto the LaunchPad, just be sure to orient the
board correctly; when correctly oriented the LaunchPad S1 and S2
buttons will clear of the board:

![EDP with LaunchPad](/images/expansion_board/epd-boosterpack.jpg)
