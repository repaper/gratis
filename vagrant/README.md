# Vagrant + VirtualBox

Running Arduino IDE V1.6.3 in A Virtual Box VM on PC-BSD.


## Create VM and provision

~~~~~
vagrant up --provision
~~~~~


## To re-run provision

If you modified any of the YAML files and want to update the
configuration.

~~~~~
env UPDATE_PACKAGES=NO vagrant provision
~~~~~

Setting the environment variable to `NO` causes ansible to skip the
cache update step, as this is slow to download the package index
files.

Run this at as many times as necessary.


## Connect to the VM and run Arduino

Connect with X-forwarding using the local command:
but for now can just use the command:

~~~~~
vagrant ssh -- -Y
~~~~~

Should now be logged into the VM with above ssh command and able to
start the arduino program:

~~~~~
arduino
~~~~~

The Arduino UI should now be visible.
