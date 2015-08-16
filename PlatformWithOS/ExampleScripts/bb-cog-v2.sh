#!/bin/sh

# the list of commands to build the BeagleBone version
# with COG V2
# and use standard I/O layout

# ensure any files belonging to the old layout are removed
make bb-clean

# build with selected COG/layout the install
make COG_VERSION=V2 bb
sudo make COG_VERSION=V2 bb-install

# restart fuse service
sudo service epd-fuse restart
