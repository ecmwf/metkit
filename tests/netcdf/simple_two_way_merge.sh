#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";

data:
        x = 1;
        f = 1;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";

data:
        x = 2;
        f = 2;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";

data:
        x = 1, 2;
        f = 1, 2;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
