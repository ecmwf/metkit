#!/bin/bash
set -eaux
rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 3;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;


data:
        x = 1, 2, 3;
        y = 1, 2, 3;
        f = 1, 2, 3,
            4, _, 6,
            7, 8, 9;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;
        y = 1;
        b = 2;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;

data:
        x = 2;
        y = 2;
        f = 5;
}
EOF


ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 3;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;


data:
        x = 1, 2, 3;
        y = 1, 2, 3;
        f = 1, 2, 3,
            4, 5, 6,
            7, 8, 9;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc

python ../../bin/nccompare.py out.nc expect.nc
