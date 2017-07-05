#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        a = 1;
        b = 1;
variables:
        double a(a);
        double b(b);
        double f(a, b);

data:
        a = 1;
        b = 1;
        f = 1, 1;
}
EOF


ncgen -b - <<EOF;
netcdf one {
dimensions:
        a = 1;
        b = 1;
variables:
        double a(a);
        double b(b);
        double f(b, a);

data:
        a = 2;
        b = 2;
        f = 2, 2;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        a = 2;
        b = 2;

variables:
        double a(a);
        double b(b);
        double f(a, b);

data:
        x = 1, 2;
        f = 1, 1, 2, 2;
}
EOF

../../bin/ncmerge one.nc two.nc
python ../../bin/nccompare.py out.nc expect.nc
