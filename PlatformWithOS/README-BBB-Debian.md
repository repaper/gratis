# Beagle Bone Black with Debian

Quick setup with bone-debian 2105-03-01.

## Extract image

~~~~~
curl -o bone-debian-7.8-lxde-4gb-armhf-2015-03-01-4gb.img.xz http://debian.beagleboard.org/images/bone-debian-7.8-lxde-4gb-armhf-2015-03-01-4gb.img.xz
xzcat bone-debian-7.8-lxde-4gb-armhf-2015-03-01-4gb.img.xz | sudo dd bs=64M of=/dev/sde
~~~~~

## System update

Login and update:

~~~~~
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo reboot
~~~~~


## Install optional

These are my personal prefered editor(mg) and shell(zsh),
skip this step it you want to.

~~~~~
sudo apt-get install mg lynx zsh
chsh -s /usr/bin/zsh
exit  # to logout
# I scp .mg and .zshrc from my host system
# then ssh back in again
~~~~~


# Install required tools and compile

Install tools and clone the repaper grats repository:
~~~~~
sudo apt-get install git build-essential libfuse-dev
git clone https://github.com/repaper/gratis.git
~~~~~

Install GPIO firmware - just add the P8 and P9 items so GPIO works:
~~~~~
git clone https://github.com/nomel/beaglebone.git
sudo cp beaglebone/gpio-header/generated/gpio-P[89].* /lib/firmware
~~~~~

Change to sources, compile and install:
**Be sure to choose the correct PANEL_VERSION**
~~~~~
cd gratis/PlatformWithOS
make PANEL_VERSION=V110_G1 bb
sudo make PANEL_VERSION=V110_G1 bb-install
~~~~~

Edit config file to match the display, just choose the editor you like:
~~~~~
sudo mg /etc/default/epd-fuse
sudo vi /etc/default/epd-fuse
sudo vim /etc/default/epd-fuse
sudo nano /etc/default/epd-fuse
~~~~~

Start the service and check panel size shows correctly:
~~~~~
sudo service epd-fuse start
cat /dev/epd/panel
~~~~~

Display an image:
**Use the correct image size to match panel**
~~~~~
echo C > /dev/epd/command
./driver-common/xbm2bin < ./driver-common/cat_2_7.xbm > /dev/epd/display
echo U > /dev/epd/command
~~~~~
