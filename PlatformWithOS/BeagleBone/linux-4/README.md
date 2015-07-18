# Experimental GPIO driver for Linux 4.x

This is a test gpio.c replacement for systems using Linux 4.x.

Currently this uses a modified version of the cape-universal device tree.

on Arch Linux as root to compile/install the device tree:

~~~~~
pacman -S dtc-overlay
dtc -@ -I dts -O dtb -o /lib/firmware/cape-universal-00A0.dtbo cape-universal-00A0.dts
~~~~~

## Differences in this cape-universal-00A0.dts

The cape-universal-00A0.dts in this directory is almost the same as
the file from universal-io, the following will show the differences.

~~~~~
cd
git clone https://github.com/cdsteinkuehler/beaglebone-universal-io.git
cd ~/gratis/PlatformWithOS
diff ~/beaglebone-universal-io/cape-universal-00A0.dts BeagleBone/linux-4/cape-universal-00A0.dts
~~~~~

The changes are to change gpio1 .. gpio4 in original to gpio0
... gpio3 on Arch kernel the original will not load and `dmesg`
contains error about gpio4 after the attempt.  The modified version
loads correctly, but does seem to need a reboot if the original was
tried and failed.

# How to use

~~~~~
cd ~/gratis/PlatformWithOS
cp BeagleBone/linux-4/gpio.c BeagleBone/gpio.c
~~~~~

Now make as normal.
