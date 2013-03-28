# Example Programs

The example programs support both the Energia (TI LaunchPad with MSP430G2553)
and the Arduino Leonardo (Atmel AVR).

The files include some conditional code to switch between the two platforms.
This code does the following:

1. Convert PROGMEM types to normal types for the TI MCU since it has a unified
   address space.
2. Adjust the I/O pins definitions and ADC reference for differences between
   the two platforms.

## demo

This example first clears the screen, then toggles between two images.
Needs the serial port (9600 8N1) connected and displays the version,
temperature and compensation values on each cycle.

This is built upon the EPD API in the libraries folder ans shows how
to use the API to display images from the MCU FLASH.  Only a few images
are possible to be stored since the on-chip FLASH is limited.

## command

A command-line example that acceps single character command from the
serial port (9600 8N1).  Functions include XBM upload to to the SPI
FLASH chip on the EPD evaluation board, display image from this FLASH
and several other functions.

Use the `h` command on the serial port (9600 8N1) to obtain a list
of commands.

When using the serial monitor on Arduino/Energia IDE any command that
take a hex number as parameter need a `<space>` character after it, as
the **Send** button will not automatically add a CR/LF.  For single
letter commands like the `t` temperature sensor read just type the
charater and click **Send**.

The 4 stage display cycle is split into two separate commands. The `r`
command removes an image and the `i` command displays an image.
e.g. if the current image was from sector 30 and you wanted to change
to sector 43 then type `r30<space>i43<space>` into the serial monitor
and click **Send**.

## thermo (Arduino Mega only)

A frame buffer graphic demo uses 4800 bytes of SRAM for 2.0" display
so is restricted to Arduino Mega or ATmega1280/ATmega2560 chip
designs.  It draws a temperature display in digits and a a simple
scale plus a few graphic elements.  Delays for amnute the refreshed
the display.

## amslide (AlaMode)

A demonstartion slide show for the 2.0" display connected to a
[Wyolum](http://www.wyolum.com)
[AlaMode](http://wyolum.com/projects/alamode/).  This is basically an
Arduino Uno (ATMega328) type of device that can be connected directly
to a Rasberry-Pi and programmed from the Arduino IDE running on a
Rasberry-Pi (see the Wyolum web sit to [get started](http://wyolum.com/projects/alamode/alamode-getting-started/).

This demo simply diplays images from a card in the AlaMode microSD
slot; a file `index.txt` lists the files to be displayed and how many
seconds to display the image.  The format is that each line has the
number of seconds as a decimal integer, a space and the image file
path.

The images are binary files that match the display, there is a zip
file containing some demo images and a sample `index.txt`.  Just unzip
this file to the root of a micro SD card, download the amdemo.ino and
the images will be displayed continuously.  I the serial monitor is
running the the names of the files will be displayed.

The images in the sample are derived from the XBM library images using
the Command: `tail -n +4 "${xbm}" | xxd -r -p > "${bin}"` Where the
variables `xbm` represents the XBM source file ane and `bin`
represents the binary output file name.  This results in the bytes in
the binary file having the same values as the hex numbers in the XBM
file.  Note that the Arduino SD code uses the 8.3 filename format so
choose a compatible name e.g. `cat.20`

Portability - This should work on other Arduinos that have an SD or
micro SD interface either directy attached or on a plug-in shield; but
may need some changes to I/O pin order if the SD CS is not routed to
Pin 10 as in AlaMode case.

## libraries

* **Images** - Sample XBM files.  The intro program includes two of
  these directly.  The Command program can use these for its upload
  command.
* **FLASH** - Driver for the SPI FLASH chip on the EPD eval board.
* **EPD** - E-Ink Panel driver.
* **EPD_GFX** - This subclasses the
  [Adafruit_GFX library](https://github.com/adafruit/Adafruit-GFX-Library)
  which needs to be downloaded an installed in to this folder.
  **NOTE** - This needs at least 8 kBytes of SRAM on the AVR MCU to
  function.  So can only this can be used with Arduino Mega or
  ATmega1280/ATmega2560 chip designs.
* **S5813A** - Temperature sensor driver.


# Connection of EPD board to LaunchPad

The board simply plugs onto the LaunchPad.

# Connection of EPD board to Arduino

The board needs a cable to connect to the Leonardo.  V1 boards are
3.3V and need level conversion. V2 and later boards are dual voltage
including a 3.3V regulator for the EPD panel and level converters and
so can directly connect to the Leonardo.

## Level conversion for V1 Boards

**Only applies to V1 boards**

* The EPD board is 3.3V.
* The Arduino is 5V.
* All digital outputs from Arduino to need 5V to 3.3V divider.  A 4k7
  from Arduion to EPD with a 10k pulldown on the EPD side is
  sufficient to drop the 5V to 3.3V, Becareful of the SPI clock
  signal, not to degrade the edges as the FLASH read is quite
  sensitive to this.
* All digital inputs to the Arduino need 3.3V to 5V step up.  an
  N-Channel MOSFET such a 2N7000 (DIP), 2N7002 (SMT), BSS138 (SMT) or
  equivalent. Source connects to EPD output with a 10k pullup to 3.3V;
  Gate connect to 3.3V and the Drain connect to the Arduino input with
  a 10k pullup to 5V.  Two converters are required for BUSY and MISO lines.
* Temperature line is analog so can connect directly to ADC input.

A prebuilt converter can be obtained from:

* [Sparkfun Logic Level Converter](https://www.sparkfun.com/products/8745).
* [Hobbytronics](http://www.hobbytronics.co.uk) also have the Sparkfun converter and have a short
  [tutorial with circuits](http://www.hobbytronics.co.uk/mosfet-voltage-level-converter).
* [Adafruit 74LVC245](http://www.adafruit.com/products/735) can be used as a level shifter.
