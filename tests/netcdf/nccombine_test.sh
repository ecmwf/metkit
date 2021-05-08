#!/bin/bash
set -eaux
rm -f *.nc

result=$1
ref="$(basename $result .cdl).nc"
ncgen -b $result
shift

args=""
for n in "$@"
do
    m=$(basename $n .cdl)
    ncgen -b $n
    args="$args $m.nc"
done

../../bin/ncmerge one.nc two.nc
python ../../bin/nccompare.py tmp.nc result.nc
