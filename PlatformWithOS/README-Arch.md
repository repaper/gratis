# Using Arch Linux

This was only briefly tested on the Beaglebone Black.  This also
works with Arch on Raspberry Pi.

**Problems**

1. **This need to be re-checked on Beaglebone Black**


# Initial SD Card creation

The Arch Linux has documented way to set this up:

For Beaglebone Black:

http://archlinuxarm.org/platforms/armv7/ti/beaglebone-black

For Raspberry Pi:

http://archlinuxarm.org/platforms/armv6/raspberry-pi


# Installing packages

Basic use of pacman can be found at:

https://wiki.archlinux.org/index.php/pacman

start by upgrading the existing packages and then reboot to ensure
system is consistent and latest kernel is running.

~~~~~
pacman -Syu
sync
reboot
~~~~~

After start-up login as root again Install whatever utilites, editor,
shells you would normally use; ntp for time correction etc.

~~~~~
pacman -S mg zsh curl rsync ntp
~~~~~

Install git, development and fuse libraries:

~~~~~
# when propted for package selection press Enter for "all"
pacman -S git base-devel fuse
~~~~~

Install python2 imaging libraries (PIL) and some fonts

~~~~~
pacman -S python2-pillow ttf-dejavu
# Clock27.py uses:
pacman -S ttf-freefont
~~~~~

All above/below on a single install: (for convenience)

~~~~~
pacman -S mg zsh curl rsync ntp git base-devel fuse python2-pillow ttf-dejavu ttf-freefont tmux
~~~~~


# Create a user to use for EPD development

Create a user for development, I like to set its shell to zsh.

~~~~~
# EITHER use the default shell (likely to be bash)
useradd -c 'E-Ink devlopment' -G root,wheel,adm -m -U repaper
# OR to use zsh as the user shell
useradd -c 'E-Ink Development' -G root,wheel,adm -m -U -s /usr/bin/zsh repaper
~~~~~

You may wish to change `root` and `repaper` passwords at this point.

~~~~~
passwd root
passwd repaper
~~~~~

## Enable sudo access

Enable sudo for the repaper user; the initial user creation added
repaper to the wheel group, but this need to be enabled in
/etc/sudoers.

~~~~~
visudo -f /etc/sudoers
~~~~~

Find the wheel line by typing: `/%wheel`
Press *Enter*
The line initialilly looks like:

~~~~~
# %wheel ALL=(ALL) ALL
~~~~~

Type a *zero* followed by two **x**s  i.e.: `0xx`
(this is: home cursor and two delete chars)

The line will now look like:

~~~~~
%wheel ALL=(ALL) ALL
~~~~~

Exit and save the file *excape* *colon* **x** *Enter*

## Perhaps even set up ssh authorisation:

~~~~~
su -l repaper    # login as repaper
mkdir -m 0700 .ssh
cat > .ssh/authorized_keys
~~~~~

Now paste the correct public key into the terminal, press `Enter` and
then press `Ctrl-D`.  Press `Ctrl-D` to logout from `repaper` and try
ssh as repaper in another console on the host machine.

**Optional step:**

Now use `scp` to copy any shell or editor configuration files from the host
(I copy my zsh and mg settings)

~~~~~
#* this is on the host machine and I have the DCHP server provide DNS for Beaglebone Black
scp -p ~/.zshrc ~/.mg repaper@beaglebone:
#* it maybe necessary to use IP address (replace with your correct IP address)
scp -p ~/.zshrc ~/.mg repaper@192.168.1.123:
~~~~~

See the ExampleScripts/README.md for a sample tmux configuration

~~~~~
# to install tmux
pacman -S tmux
~~~~~

# Creating the EPD driver

Login as the newly created `repaper` user and clone the repaper repository:

~~~~~
git clone https://github.com/repaper/gratis.git
~~~~~

## Note for RaspberryPI

Note: the SPI may be initially disabled; check that the file:
`/boot/config.txt` contains the line:

~~~~~
device_tree_param=spi=on
~~~~~

If this line is commented with the `#` character, uncomment, save file
and reboot.


## Special GPIO section for Beaglebone Black

This is to install the cape manager firmware files that allow the
Beaglebone Black version of Arch Linux to have access to the GPIO.

Clone the nomel repository from https://github.com/nomel/beaglebone
then install the necessary P9 firmware files:

~~~~~
git clone https://github.com/nomel/beaglebone.git
sudo cp beaglebone/gpio-header/generated/gpio-P9.* /lib/firmware
~~~~~

Optional step:

If you intend to make changes to use the P8 GPIO pin header then also
add the P8 firmware files as follows:

~~~~~
sudo cp beaglebone/gpio-header/generated/gpio-P8.* /lib/firmware
~~~~~


## Compiling and running

Install the necessary libraries and compile

~~~~~
cd gratis/PlatformWithOS
make help   # follow the various make options
#
# e.g. for Beaglebone Black
make COG_VERSION=V2 bb-clean bb
#
# Alternative for Raspberry Pi
make COG_VERSION=V2 rpi-clean rpi
#
# run the test program (in this case a 2.7" panel was connected)
sudo ./driver-common/epd_test 2.7
# the panel should cycle through several images
~~~~~

## Setup systemd configuration

**Note:**
* **the make install taget currently only works on debian**
* **use the notes below to install manually**

Arch is using systemd and the this part is sysv init typs of startup script

setup the panel confguration in /etc/default

~~~~~
sudo cp ./driver-common/epd-fuse.default /etc/default/epd-fuse
#
# Ensure correct panel size in config: (use your editor: vi vim mg nano
sudo $EDITOR /etc/default/epd-fuse
~~~~~

Set up the systemd service file

~~~~~
sudo cp ./driver-common/epd-fuse.service /usr/lib/systemd/system/
#
# ensure systemd picks up changes in: /usr/lib/systemd/system/
sudo systemctl daemon-reload
#
sudo systemctl list-unit-files |grep -i epd
# response:
#   epd-fuse.service                       disabled
#
# enable and start the driver
sudo systemctl enable epd-fuse
sudo systemctl start epd-fuse
#
cat /dev/epd/panel
# this should display panel size and COG version
#
# try clearing the panel (all white)
echo 'C' > /dev/epd/command
~~~~~

## Try the DrawDemo

~~~~~
python2 demo/DrawDemo.py
~~~~~

Note that once the demo has run successfully, please reboot, login and
check the fuse driver was loaded:

~~~~~
cat /dev/epd/panel
echo 'C' > /dev/epd/command
cd gratis/PlatformWithOS
python2 demo/DrawDemo.py
~~~~~

Any of the other demos should now be functional.


# Note starting NTP and timezone

Below are brief notes to set up NTP and the timezone; useful to try
the clock demos - e.g. to have EPD clock in some other zone.  As
configured the system will be UTC.

For detailed instructions consult the excellent Arch Linux Wiki, the
time page has an entry for timezone:

https://wiki.archlinux.org/index.php/time#Time_zone


## NTP

If not already installed install the NTP daemon:

~~~~~
sudo pacman -S ntp
~~~~~

Enable and start the NTP system:

~~~~~
sudo systemctl enable ntpd
sudo systemctl enable ntpdate
sudo systemctl start ntpdate
sudo systemctl start ntpd
~~~~~

Check if operating - may need to repeat to see how synchronisation is
progressing:

~~~~~
sudo ntpq -nc peers 127.0.0.1
~~~~~


## Timezone

View current timezone status

~~~~~
sudo timedatectl status
~~~~~

List available current timezone, then scroll through the list to
find a suitable one.  (Type 'Q' to exit)

~~~~~
sudo timedatectl list-timezones
~~~~~

Set the desired timezone and check the date to see if it is correct.

~~~~~
sudo timedatectl set-timezone Asia/Taipei
date
sudo timedatectl status
~~~~~


## Running the Clock27.py demo as a service

~~~~~
sudo cp demo/Clock27.service /usr/lib/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable Clock27.service
sudo systemctl start Clock27.service
~~~~~
