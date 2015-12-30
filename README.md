# gratis

Updated 2015-08-01 by Rei Vilo

* Added `#include Energia.h`
* For Energia, changed pin names to pin numbers (pin names are deprecated)
* Works on MSP430F5529, LM4F120, TM4C123
* Fails on MSP432 and CC3200
* [Recognize the FPL material type by the rear labels](http://www.pervasivedisplays.com/products/label_info)
* Use branch [rei-vilo / gratis / CC3200 and MSP432](https://github.com/rei-vilo/gratis/tree/CC3200-and-MSP432)
* The master supports MSP432 but not with direct connection as Energia startup code allocates
  one pin as not GPIO

## Sketches

These are example programs that will compile and run on the following platforms

1. TI LaunchPad with M430G2553 using the [Energia](https://github.com/energia) IDE
2. Arduino Leonardo using the [Arduino](http://arduino.cc) IDE

Note:
* Arduino with the embedded USB controller ('Un' suffix) will
  wait for the serial monitor to be opened before doing any display.
  This is because the serial port is emulated in the USB controller and there
  is a wait loop before the first serial print.


## PlatformWithOS

Contains an example driver and Python demo programs that can be
compiled and run on:

1. [Raspberry Pi](http://www.raspberrypi.org/).
2. [BeagleBone Black](http://www.beagleboard.org/)

## Other software links

These are other projects on GitHub or other sites, please check them out.
(If you have a project, let us know an we will add a link here)

1. https://github.com/aerialist/repaper_companion - Companion software for rePaper. Resize image and convert to XBM format.


======

Gratis is a Repaper.org repository, initiated by E Ink and PDI for the purpose of making sure ePaper can go everywhere.
