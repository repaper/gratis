#!/bin/sh -e
### BEGIN INIT INFO
# Provides:          epd-fuse
# Required-Start:    udev
# Required-Stop:     udev
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start epd_fuse to control E-Ink panel
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin

# Angstom is missing this:
if [ -e /lib/lsb/init-functions ]
then
  . /lib/lsb/init-functions
else
  # simplistic substitutes
  log_daemon_msg()
  {
    logger -t daemon "$1"
  }

  log_end_msg()
  {
    logger -t daemon "${DAEMON} returned: $1"
  }

  status_of_proc()
  {
    local pid
    pid=$(pgrep -f "${DAEMON}" || true)
    if [ -z "${pid}" ]
    then
      echo Daemon: $2 is NOT RUNNING
    else
      echo Daemon: $2 is rinning, PID = ${pid}
    fi
  }
fi

DAEMON=/usr/sbin/epd_fuse
#PIDFILE=/var/run/epd_fuse.pid

test -x "${DAEMON}" || exit 5

# default configuration (change /etc/default/epd-fuse, not these)
EPD_MOUNTPOINT=/dev/epd
EPD_SIZE=2.0
EPD_OPTS='-o allow_other -o default_permissions'

if [ -r /etc/default/epd-fuse ]
then
  . /etc/default/epd-fuse
fi

case $1 in
  (start)
    log_daemon_msg "Starting EPD" "epd_fuse"
    mkdir -p "${EPD_MOUNTPOINT}"
    "${DAEMON}" --panel="${EPD_SIZE}" ${EPD_OPTS} "${EPD_MOUNTPOINT}"
    log_end_msg "$?"
    ;;
  (stop)
    log_daemon_msg "Stopping EPD" "epd_fuse"
    umount -f "${EPD_MOUNTPOINT}"
    log_end_msg "$?"
    rm -f "${PIDFILE}"
    ;;
  (restart|force-reload)
    "$0" stop && sleep 2 && "$0" start
    ;;
  (try-restart)
    if "$0" status > /dev/null
    then
      "$0" restart
    else
      exit 0
    fi
    ;;
  (reload)
    exit 3
    ;;
  (status)
    status_of_proc "${DAEMON}" "EPD FUSE"
    ;;
  (*)
    echo "Usage: $0 {start|stop|restart|try-restart|force-reload|status}"
    exit 2
    ;;
esac
