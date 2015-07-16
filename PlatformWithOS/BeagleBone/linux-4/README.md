# Experimental GPIO driver for Linux 4.x

This is a test gpio.c replacement for systems using Linux 4.x.

Currently this uses a modified version of the cape-universal device tree.

on Arch Linux as root to compile/install the device tree:

~~~~~
pacman -S dtc-overlay
dtc -@ -I dts -O dtb -o /lib/firmware/cape-universal-00A0.dtbo cape-universal-00A0.dts
~~~~~

Current state:
* GPIO and PWM P9-14 operational.  gpio_test works.
* SPI does not appear to be enabled.
