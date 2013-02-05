# EPD Extension Board

## Introduction

The EPD extension board supports driving
[Pervasive Displays Inc.](http://www.pervasivedisplays.com/products/panels)
PDi's 1.44, 2.0, and 2.7 inch E-Paper display (EPD) modules with your
development projects. This expansion board provides a 20 control pins
connection with your product or development kit to drive EPD via SPI
interface.


## Features

* Starter board to work with EPD
* Supports driving 1.44", 2.0" or 2.7" EPD Panels
* On board 8M bits serial flash memory and temperature sensor
* 20 input/output pins bridge to any development/evaluation kit or product
* Drect connect socket for Texas Instruments LaunchPad
* Documentation for Panels and driving waveform
* Source code for simple demonstration programs


## Board Picture

* Front View ![Front view](images/extension_board/front.jpg)
* Rear view  ![Rear view](images/extension_board/back.jpg)
* Board placement ![Board placement](images/extension_board/placement.png)
* 20 pin cable (for Arduino or bridging to other kit) ![Cable](images/extension_board/cable.jpg)


## Pin Assignment

Pin Number   Description
----------   -----------
1.           Vcc 3V
2.           *(LED1)*
3.           *(UART_RX)*
4.           *(UART_TX)*
5.           *(SW2)*
6.           Temperature
7.           SPI\_CLK
8.           BUSY
9.           PWM
10.          /RESET
11.          PANEL\_ON
12.          DISCHARGE
13.          BORDER_CONTROL
14.          SPI_MISO
15.          SPI_MOSI
16.          *(RST/SBWTDIO)*
17.          *(TEST/SBWTCK)*
18.          FLASH\_CS
19.          /EPD\_CS
20.          GND

### Notes

* The above values in parentheses like *(SW2)*, are not connected
  to the driver circuit and can be left open on non-LaunchPad
  projects.  These are only provided so that all the LaunchPad pins
  are brought through to the 20 pin connector.
* Active low signals are indicated with a leading solidus (slash) e.g. /RESET

