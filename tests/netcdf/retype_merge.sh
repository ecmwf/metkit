#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;
        y = 1;

variables:
        double x(x);
        int y(y);
        float f(x, y);
        f:coordinates="x y";

        int g(y);
        g:coordinates="y";

data:
        x = 1;
        y = 1;
        f = 1;
        g = 1;
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

        float g(y);
        g:coordinates="y";

data:
        x = 2;
        y = 1;
        f = 2;
        g = 1;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;
        y = 1;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

        float g(y);
        g:coordinates="y";

data:
        x = 1, 2;
        y = 1;
        f = 1, 2;

        g = 1;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
