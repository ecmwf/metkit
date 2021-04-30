#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
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


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 2;

variables:
        double x(x);
        double f(x);

data:
        x = 2, 3;
        f = 2, 3;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 3;

variables:
        double x(x);
        double f(x);

data:
        x = 1, 2, 3;
        f = 1, 2, 3;
}
EOF

../../bin/ncmerge one.nc two.nc
python ../../bin/nccompare.py out.nc expect.nc
