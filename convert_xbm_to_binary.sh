#!/bin/sh
# generate images

suffix=_2_0.xbm
prefix=Sketches/libraries/Images/
out=.20

for f in "${prefix}"*"${suffix}"
do
  i="${f#${prefix}}"
  i="${i%${suffix}}${out}"
  echo $f '->' $i
  tail -n +4 "${f}" | xxd -r -p > "${i}"
done
