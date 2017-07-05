#!/bin/bash
set -eaux

rm -f *.nc

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
        double g(x);
        g:coordinates="x";

data:
        x = 2;
        g = 2;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
      x = 2 ;
 variables:
      double f(x) ;
              f:coordinates = "x" ;
      double g(x) ;
              g:coordinates = "x" ;
      double x(x) ;
 data:


  f = 1, 0;

  g = 0, 2;

  x = 1, 2;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
