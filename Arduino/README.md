# Example Programs

## intro

A single file example that first clears the screen, then toggles
between two images.  This example now includes Energia support
so the this will run on both Arduino and LaunchPad.

## command

A command-line example that acceps single character command from the
serial port (9600 8N1).  Functions include XBM upload to FLASH,
display image from flash etc.

Currently this does not yet support Energia. (**TODO**)

## libraries

* **Images** - sample XBM files.
  The intro program includes two of these directly.
  The Command program can use these for its upload command
* **FLASH** - driver for the SPI FLASH chip on the EPD eval board
* **EPD** Panel driver

# Connection of EPD board to Arduino

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
