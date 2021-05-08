#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;

variables:
        double x(x);
        double f(x);

data:
        x = 1;
        f = 1;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        X = 1;

variables:
        double x(X);
        double f(X);

data:
        X = 2;
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

data:
        x = 1, 2;
        f = 1, 2;
}
EOF

../../bin/ncmerge one.nc two.nc
python ../../bin/nccompare.py out.nc expect.nc
