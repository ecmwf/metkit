#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;

variables:
        double x(x);
        double y;
        double z;
        double f(x);
        f:coordinates="x y z";

data:
        x = 1;
        y = 1;
        z = 1;
        f = 1;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;

variables:
        double x(x);
        double y;
        double z;
        double f(x);
        f:coordinates="x y z";

data:
        x = 1;
        y = 2;
        z = 1;
        f = 2;
}
EOF

ncgen -b - <<EOF;
netcdf three {
dimensions:
        x = 1;

variables:
        double x(x);
        double y;
        double z;
        double f(x);
        f:coordinates="x y z";

data:
        x = 1;
        y = 3;
        z = 1;
        f = 3;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 1;
        y = 3;

variables:
        double x(x);
        double y(y);
        double z;
        double f(x, y);
        f:coordinates="x y z";

data:
        x = 1;
        y = 1, 2, 3;
        z = 1;
        f = 1, 2, 3;
}
EOF

../../bin/ncmerge one.nc two.nc three.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc

