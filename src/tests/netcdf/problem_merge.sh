#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;
        b = 2;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";
        f:standard_name="sn";
        f:unit="kg";
        f:bounds="bnd";

        double bnd(x, b);

data:
        x = 1;
        f = 1;
        bnd = 10, 20;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;
        b = 2;

variables:
        double x(x);
        double g(x);
        g:coordinates="x";
        g:standard_name="sn";
        g:unit="kg";
        g:bounds="bnd";

        double bnd(x, b);

data:
        x = 2;
        g = 2;
        bnd = 10, 20;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;
        b = 2;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";
        f:standard_name="sn";
        f:unit="kg";
        f:bounds="bnd";

        double bnd(x, b);

data:
        x = 1, 2;
        f = 1, 2;
        bnd = 10, 20;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
