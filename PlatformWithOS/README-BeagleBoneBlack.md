# Initially configuring Angstrom on the Beagle Bone black

The Beagle Bone Black is slightly more problematic to set up, so here
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
temporary file systems and opkg does not clean up downloads that have
already be installed.  Hopefully the second time will succeed and
takes another hour.  i.e. a little ove two hours for the whole update
(depending on how early you catch the short write failure)


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

Now clone the repository:

~~~~~
git clone https://github.com/repaper/gratis.git
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
