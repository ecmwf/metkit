#!/bin/bash
set -eaux
rm -f *.nc


ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;
        y = 1;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;


data:
        x = 1;
        y = 1;
        f = 1;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;
        y = 1;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;

data:
        x = 1;
        y = 2;
        f = 2;
}
EOF

ncgen -b - <<EOF;
netcdf three {
dimensions:
        x = 1;
        y = 1;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;

data:
        x = 2;
        y = 1;
        f = 3;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;
        y = 2;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:_FillValue = 42;
data:
        x = 1, 2;
        y = 1, 2;
        f = 1, 2,
            3, _;
}
EOF

../../bin/ncmerge one.nc two.nc three.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
