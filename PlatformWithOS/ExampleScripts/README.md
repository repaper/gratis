# Some sample build scripts

`*.sh` are sample build scripts to illustrate the Makefile options

# Configure tmux

~~~
sudo apt-get install tmux
cp -p DOT.tmux.conf ~/.tmux.conf
tmux att
~~~

The above will install tmux and the configuration will open three tmux
windows on in home, one in `gratis/PlatformWithOS` for compiling and the
third in the demo directory.

Each window is protected from accidental logout using the command `set -o ignoreeof`.
Just use the tmux disconnect `Ctrl-B d` key sequence.

To make the system log out on disconect add the linex from `DOT.login`
to either `.zlogin` or `.bash_login` (depending on the default shell).

~~~
cat DOT.login >> ~/.zlogin
cat DOT.login >> ~/.bash_login
~~~
