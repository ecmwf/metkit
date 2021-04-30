#!/bin/bash
set -eaux

rm -f *.nc
ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 2;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2;
        y = 1, 2, 3;
        f = 1, 2, 3,
            4, 5, 6;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 2;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2;
        y = 4, 5, 6;
        f = 7, 8, 9,
            10, 11, 12;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;
        y = 6;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2, 3, 4;
        y = 1, 2, 3, 4, 5, 6;
        f = 1, 2, 3, 7, 8, 9,
            4, 5, 6, 10, 11, 12;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc

python <<EOF;
from netCDF4 import Dataset
nc = Dataset("out.nc", "r")
f  = nc.variables['f']
print f[:]
EOF

python ../../bin/nccompare.py out.nc expect.nc
rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 2;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2;
        y = 1, 2, 3;
        f = 1, 2, 3,
            4, 5, 6;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 2;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 3, 4;
        y = 1, 2, 3;
        f = 7, 8, 9,
            10, 11, 12;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 4;
        y = 3;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2, 3, 4;
        y = 1, 2, 3;
        f = 1, 2, 3,
            4, 5, 6,
            7, 8, 9,
            10, 11, 12;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc

python ../../bin/nccompare.py out.nc expect.nc
