#!/bin/bash
set -eaux

rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;
        y = 1;
        b = 2;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:bounds="bnd";
        f:_FillValue = 42;

        int bnd(x, b);

data:
        x = 1;
        y = 1;
        f = 1;
        bnd = 10, 20;
}
EOF


ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1;
        y = 1;
        b = 2;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";
        f:bounds="bnd";
        f:_FillValue = 42;

        int bnd(x, b);

data:
        x = 2;
        y = 1;
        f = 2;
        bnd = 30, 40;
}
EOF


ncgen -b - <<EOF;
netcdf expect {
 dimensions:
      b = 2 ;
      x = 2 ;
      y = 1 ;
 variables:
      int bnd(x, b) ;
      double f(x, y) ;
              f:_FillValue = 42. ;
              f:bounds = "bnd" ;
              f:coordinates = "x y" ;
      double x(x) ;
      double y(y) ;
 data:

  bnd = 10, 20, 30, 40 ;
  f = 1, 2 ;
  x = 1, 2 ;
  y = 1 ;
}

EOF

../../bin/ncmerge one.nc two.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
