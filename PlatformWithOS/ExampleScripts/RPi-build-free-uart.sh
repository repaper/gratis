#!/bin/sh

# the list of commands to build the RaspberryPi version
# with COG V1 ( the default)
# and use an alternative I/O layout that leaves the UART free for other applications

# ensure any files belonging to the old layout are removed
make rpi-clean

# build with selected layout the install
make EPD_IO=epd_io_free_uart.h rpi
sudo make EPD_IO=epd_io_free_uart.h rpi-install

# restart fuse service
sudo service epd-fuse restart
