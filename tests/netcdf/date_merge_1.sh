#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;

variables:
        double x(x);
        double y;
        y:units="seconds since 2009-01-20 00:00:00";
        double f(x);
        f:coordinates="x y";

data:
        x = 1;
        y = 0;
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
        y:units="seconds since 2009-01-21 00:00:00";
        double f(x);
        f:coordinates="x y";

data:
        x = 1;
        y = 0;
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
        y:units="seconds since 2009-01-22 00:00:00";
        double f(x);
        f:coordinates="x y";

data:
        x = 1;
        y = 0;
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
        y:units="seconds since 2009-01-20 00:00:00";
        y:calendar = "gregorian";
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1;
        y = 0, 86400, 172800;
        f = 1, 2, 3;
}
EOF

../../bin/ncmerge one.nc two.nc three.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
