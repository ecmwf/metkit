#!/bin/bash
set -eaux

rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
	x = 1 ;
	y = 1 ;
	z = 1 ;
variables:
	double y(y) ;
	double x(x) ;
	double z(z) ;
	double f(x, z) ;
		f:coordinates = "x y z" ;
		f:_FillValue = 42. ;
data:

 y = 1 ;

 x = 1 ;

 z = 2 ;

 f =
  1112 ;
}
EOF

ncgen -b - <<EOF;
netcdf two {
dimensions:
	x = 1 ;
	y = 1 ;
	z = 1 ;
variables:
	double y(y) ;
	double x(x) ;
	double z(z) ;
	double f(x, z) ;
		f:coordinates = "x y z" ;
		f:_FillValue = 42. ;
data:

 y = 2 ;

 x = 1 ;

 z = 1 ;

 f =
  1121 ;
}
EOF
ncgen -b - <<EOF;
netcdf three {
dimensions:
	x = 1 ;
	y = 1 ;
	z = 1 ;
variables:
	double y(y) ;
	double x(x) ;
	double z(z) ;
	double f(x, z) ;
		f:coordinates = "x y z" ;
		f:_FillValue = 42. ;
data:

 y = 2 ;

 x = 1 ;

 z = 2 ;

 f =
  1122 ;
}
EOF

ncgen -b - <<EOF;
netcdf expect {
dimensions:
        x = 2;
        y = 6;

variables:
        double x(x);
        double y(y);
        double f(x, y);
        f:coordinates="x y";

data:
        x = 1, 2, 3, 4;
        y = 1, 2, 3, 4, 5, 6;
        f = 1, 2, 3, 7, 8, 9,
            4, 5, 6, 10, 11, 12;
}
EOF

../../bin/ncmerge one.nc two.nc three.nc
ncdump out.nc

python ../../bin/nccompare.py out.nc expect.nc

