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
        f:_FillValue = 42;

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
        f:_FillValue = 42;

data:
        x = 2;
        f = 2;
}
EOF

ncgen -b - <<EOF;
netcdf three {
dimensions:
        x = 1;

variables:
        double x(x);
        double g(x);
        g:coordinates="x";
        g:_FillValue = 42;

data:
        x = 1;
        g = 3;
}
EOF

ncgen -b - <<EOF;
netcdf four {
dimensions:
        x = 1;

variables:
        double x(x);
        double g(x);
        g:coordinates="x";
        g:_FillValue = 42;

data:
        x = 2;
        g = 4;
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
        f:_FillValue = 42;
        double g(x);
        g:coordinates="x";
        g:_FillValue = 42;

data:
        x = 1, 2;
        f = 1, 2;
        g = 3, 4;
}
EOF

../../bin/ncmerge one.nc two.nc three.nc four.nc
ncdump out.nc
ncdump expect.nc

python ../../bin/nccompare.py out.nc expect.nc
