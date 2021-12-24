# gratis

## Notes for mrwastl/gratis-branch

This branch adds basic support for Teensy 3.1/3.2 and ESP32.

### Teensy 3.1/3.2
Default IO layout is defined in EPD_PINOUT/EPD_PINOUT_Teensy.h.

Pin numbers can be assigned individually when calling the constructor.

### ESP32
Default IO layout is defined in EPD_PINOUT/EPD_PINOUT_ESP32.h.

Pin numbers and PWM channel ID (EPD_V110_G1 only) can be assigned individually when calling the constructor.

SPI channel ID and SPI pin numbers can only be changed in EPD_PINOUT_ESP32.h, a more flexible handling would require
some API changes.


### Tests

As I just own an EPD_V110_G1 module (2.7") I can only test this one. All other combinations are tested if compilation is successful.





## Original README of repaper/gratis-branch below

## Fast Update notes, July 2017

The update rate for Partial Update is determined by the temperature dependent stage time.
For 25 degrees C a typical max update rate for the 2.7" display is about 1.5 Hz.

The stage time decreases as the measured temperature increases.
For some applications a faster update rate can be useful.
This can be achieved by setting the stage time directly bypassing the normal temperature
dependent setting of the stage time.
This is the same mechanism as used in the Pervasive Displays Windows application for the
TI Launchpad and the EXT2 development board (http://www.pervasivedisplays.com/kits/ext2_kit).
  
Decreasing the stage time increases the max update rate. If the stage time gets too small
some ghosting (left overs from the previous image) will occur.
Depending on the application some ghosting may be acceptable.

This implementation uses the 'F' command to do a Fast Update using the stage time as set in
/dev/epd/f_stage_time. 

A video demonstrating the normal Partial Update and Fast Update for various stage times is
available on YouTube.
[![Fast Update Demo](https://img.youtube.com/vi/neGKnfJFHx4/0.jpg)](https://www.youtube.com/watch?v=neGKnfJFHx4)

## Partial Update notes, February 2016

This partial update implementation follows the algorithm outlined
in the discussion of issue #19 on the repaper/gratis github repository.

The original code in epd_fuse.c and V231G2/epd.c under PlatformWithOS did a full 4 cycle display update
including COG power on and COG power down sequence.
This resulted in the update taking more than 3 seconds with the display first showing the updated image,
then the original image followed finally by the updated image.
This made things like a clock, counters and 'game of life' look very bad.

Following changes as outlined in issue #13:
* in epd_fuse.c run_command() do not call EPD_end() at the end of the 'P' command (only if partial update is supported by the display)
* in epd.c introduce COG_on status variable in EPD_struct.
* in epd.c EPD_begin() return immediately when COG_on is true
* in epd.c EPD_begin() set COG_on to true at the end of the function.
* in epd.c EPD_end() set COG_on to false at the end of the function.
* in epd.c EPD_partial_image() only call the last stage (EPD_normal).

With these changes you can get a proper partial display update frequency < 1 Hz.

For a YouTube video showing the difference between before and after the fix see https://youtu.be/dciaFRKtetU

<a href="http://www.youtube.com/watch?feature=player_embedded&v=dciaFRKtetU" target="_blank"><img src="http://img.youtube.com/vi/dciaFRKtetU/0.jpg" alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10" /></a>

## Notes for Energia support, August 2015

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

### Notes for Arduinos with the embedded USB controller

* Arduinos with the embedded USB controller ('Un' suffix) do not have
  a real serial port, it is emulated using firmware and uses the USB
  interface.

* Arduino Loenardo uses one of these chips.

* The Command program needs the serial to accept commands so it will
  wait indefinitely for the serial monitor to be opened before doing
  any display. (the additional delay code is commented out)

* The Demo, Flash Loader and Thermo programs do no need serial port to
  accept commands, it is display only.  The additional delay is used
  and the indefinite wait is commented out.  Therefore on a Leonardo
  to see the initial messages open the serial monitor *before*
  uploading.  If the serial monitor is not open these programs will
  now automatically start.


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
