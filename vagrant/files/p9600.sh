#!/bin/sh
serials='/dev/ttyACM0 /dev/ttyUSB0'

ERROR()
{
  echo error: $*
  exit 1
}

device=''
for s in ${serials}
do
  if [ -c "${s}" ]
  then
    device="${s}"
    break
  fi
done

[ -z "${device}" ] && ERROR unable to detect serial port

baud=9600
script="$(basename "$0")"

# for alternative names (using symlinks)
case "${script}" in
  (p1*)
    baud=115200
    ;;
  (*)
    baud=9600
    ;;
esac

exec picocom --baud "${baud}" --send-cmd cat --flow h --databits 8 "${device}"
