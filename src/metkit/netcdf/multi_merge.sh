#!/bin/bash
set -eaux

rm -f *.nc

count=100
first=1000
last=$((first+count))

for n in $(seq $first $last)
do

ncgen -b - << EOF
netcdf multi$n {
dimensions:
        x = 1;

variables:
        int x(x);
        float f(x);
        f:coordinates="x";

data:
        x = $n;
        f = $n;
}
EOF

done

list=$(seq $first $last)
list=$(echo $list | sed 's/ /, /g')

ncgen -b - << EOF
netcdf expect {
dimensions:
        x = $((count+1));

variables:
        int x(x);
        float f(x);
        f:coordinates="x";

data:
        x = $list;
        f = $list;
}
EOF

../../bin/ncmerge multi*.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
rm -f *.nc
