#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;

variables:
        char x(x);
        double f(x);
        f:coordinates="x";

data:
        x = "a";
        f = 1;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;

variables:
        char x(x);
        double f(x);
        f:coordinates="x";

data:
        x = "b";
        f = 2;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;

variables:
        char x(x);
        double f(x);
        f:coordinates="x";

data:
        x = "a", "b";
        f = 1, 2;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
