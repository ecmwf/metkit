#!/bin/bash
set -eaux

rm -f *.nc

for v in f g
do
for x in $(seq 1 1)
do
for y in $(seq 1 2)
do
for z in $(seq 1 2)
do
ncgen -b - <<EOF;
netcdf ${v}data$x$y$z {
dimensions:
        x = 1;
        y = 1;
        z = 1;

variables:
        double y(y);
        double x(x);
        double z(z);
        double $v(x, y, z);
        $v:coordinates="x y z";
        $v:_FillValue = 42;

data:
        x = $x;
        z = $z;
        y = $y;

        $v = 1$x$y$z;
}
EOF
done
done
done
done


../../bin/ncmerge fdata*.nc
mv out.nc fout.nc
../../bin/ncmerge gdata*.nc
mv out.nc gout.nc

../../bin/ncmerge fout.nc gout.nc
mv out.nc expect.nc

../../bin/ncmerge ?data*.nc


ncdump fout.nc
ncdump gout.nc

ncdump out.nc
ncdump expect.nc

python ../../bin/nccompare.py out.nc expect.nc
