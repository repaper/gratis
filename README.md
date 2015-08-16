# gratis
Added by Neil Matthews for Percheron Electronics Ltd 2015-08-16

For the E-paper Hat with the Raspberry Pi, everything you need is in
PlatformWithOS and its subdirectories. The Makefile in the PlatformWithOS 
folder has already been edited to use the correct panel type and epd_io configuration.
The epd-fuse.default file in driver common has also been suitably edited.
In driver-common there are test programs in C which you can compile from the 
PlatformWithOS directory by typing make rpi. You can then run sudo driver-common/epd_test 2.7 
which should scroll through a sequence of seven images at ~2 second intervals.
Then type sudo make rpi-install to install and configure the fuse driver.


Updated 2015-08-01 by Rei Vilo

* Added `#include Energia.h`
* For Energia, changed pin names to pin numbers (pin names are deprecated)
* Works on MSP430F5529, LM4F120, TM4C123
* Fails on MSP432 and CC3200
* [Recognize the FPL material type by the rear labels](http://www.pervasivedisplays.com/products/label_info)
* Use branch [rei-vilo / gratis / CC3200 and MSP432](https://github.com/rei-vilo/gratis/tree/CC3200-and-MSP432)


## Sketches

These are example programs that will compile and run on the following platforms

1. TI LaunchPad with M430G2553 using the [Energia](https://github.com/energia) IDE
2. Arduino Leonardo using the [Arduino](http://arduino.cc) IDE

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
