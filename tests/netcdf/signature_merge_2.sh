#!/bin/bash
set -eaux

rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1;
        b = 2;

variables:
        double x(x);
        double f(x);
        f:coordinates="x";
        f:standard_name="sn1";
        f:unit="kg";
        f:bounds="bnd";

        int bnd(x, b);

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
        double f(x);

        f:coordinates="x";
        f:standard_name="sn2";
        f:unit="kf";
        f:bounds="bnd";

        int bnd(x, b);

data:
        x = 2;
        f = 2;
        bnd = 30, 40;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
      b = 2 ;
      x = 2 ;
 variables:
      int bnd(x, b) ;
      double sn1(x) ;
              sn1:bounds = "bnd" ;
              sn1:coordinates = "x" ;
              sn1:standard_name = "sn1" ;
              sn1:unit = "kg" ;
      double sn2(x) ;
              sn2:bounds = "bnd" ;
              sn2:coordinates = "x" ;
              sn2:standard_name = "sn2" ;
              sn2:unit = "kf" ;
      double x(x) ;
 data:

  bnd =
   10, 20,
   30, 40 ;

  sn1 = 1, 0 ;

  sn2 = 0, 2 ;

  x = 1, 2 ;
}
EOF

../../bin/ncmerge one.nc two.nc
ncdump expect.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc
