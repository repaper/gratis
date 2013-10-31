# Initially configuring Angstrom on the BeagleBone black

The BeagleBone Black is slightly more problematic to set up, so here
is a quick start guide:

Starting from a freshly created SD card image.  power on the system
and use ssh to log in.

~~~~~
ssh -l root beaglebone-ip-address
~~~~~


## Remove faulty package

First remove the bonescript package since this is broken by the update
and prevents opkg from configuring correctly after the upgrade.

~~~~~
opkg remove bonescript
~~~~~

The command will show error messages an indicate that no packages were
removed, not true as the package does appear to be gone. In any case
repeat the above command and this time it should only show the no
packages remove message.


## Configure DNS

The DHCP client does not appear to set the nameserver line in
/etc/resolv.conf correctly, edit this file to have your DNS IP,
instead of the default 127.0.0.1.


## Upgrade the system

~~~~~
opkg update
~~~~~

This takes a minute or so, then:

~~~~~
time opkg upgrade
~~~~~

Takes a long time (about 1 hour)and will probably fail with lots of
`wget: short write` Just use `CTRL-C` to abort this process then clean
temp and retry.

~~~~~
rm -rf /tmp/opkg-*
time opkg upgrade
~~~~~

It seems to be happening because the board has only limited RAM for
temporary file systems and `opkg` does not clean up downloads that
have already be installed.  Hopefully the second time will succeed and
takes another hour.  i.e. a little over two hours for the whole update
(depending on how early you catch the short write failure)

On a second try starting from a fresh SD Image, which was first
expanded to the full card the upgrade failed earlier and the
`resolv.conf` was back at 127.0.0.1, so it was necessary to reset it;
I used:

~~~~~
echo nameserver ${DNS_IP} > /etc/resolv.conf
~~~~~

(Note: This second try still suffered from the `wget: short write` and
needed the upgrade restarting - though it eventually finished, was
powered of, restarted and a final `opkg upgrade` showed no addition
upgrades and no configuration errors.  The remaining steps went as
detailed below and the epd_test program ran correctly.)


# Reboot the system

Ensure the latest kernel an libraries are loaded by rebooting.

~~~~~
reboot
~~~~~

Wait a while an re-login with ssh.


~~~~~
ssh -l root beaglebone-ip-address
~~~~~


## Enable SSL for git

Since Github is an SSL enabled site, it is necessary to enable the SSL
certificate chain for git otherwise the git clone command will fail
with an HTTP error.

~~~~~
opkg install ca-certificates
git config --global --replace-all http.sslVerify true
git config --global --replace-all http.sslCAInfo /etc/ssl/certs/ca-certificates.crt
~~~~~

Now clone the repaper repository:

~~~~~
git clone https://github.com/repaper/gratis.git
~~~~~

Also clone the nomel reposoitory from https://github.com/nomel/beaglebone
then install the necessary P9 firware files:

~~~~~
git clone https://github.com/nomel/beaglebone.git
cp -p beaglebone/gpio-header/generated/gpio-P9.* /lib/firmware
~~~~~

Optional step:

If you intend to make changes to use the P8 GPIO pin header then also
add the P8 firmware files as follows:

~~~~~
cp -p beaglebone/gpio-header/generated/gpio-P8.* /lib/firmware
~~~~~


## Compiling

Install the necessary libraries and compile

~~~~~
opkg install libfuse-dev
cd gratis/PlatformWithOS
make bb
~~~~~

Connect the EPD Eval Board, see the main README.md for cable details.


## Try the epd_test program

~~~~~
./driver-common/epd_test
~~~~~

The EPD should cycle through a series of images and finally return to
the command prompt.

Finally install and start the fuse driver

~~~~~
make bb-install
/etc/init.d/epd-fuse start
~~~~~

Test by clearing the screen.

~~~~~
echo C > /dev/epd/command
~~~~~

Try the python demo programs as described in the main README.md file

The fuse driver will load at boot, so also try rebooting, logging in
and running a python demo program to verify this.


## Run demo programs

As descriped in the main README.md
