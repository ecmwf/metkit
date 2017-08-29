#!/bin/bash
set -eaux

ncgen -b - <<EOF;
netcdf one {
dimensions:
	x = 1 ;
    y = 1 ;
	z = 1 ;
variables:
	double y(y);
	float f(x, y, z) ;
		f:_FillValue = 42 ;
		f:coordinates = "x y z" ;
	int x(x);
	double z(z) ;

data:

 y = 1;
 f = 1;
 x = 1;
 z = 1;

}

EOF

ncgen -b - <<EOF;
netcdf two {
dimensions:
	x = 1 ;
	z = 1 ;
    y = 1 ;

variables:
	double y(y);

	float f(x, y, z) ;
		f:_FillValue = 42 ;
		f:coordinates = "x y z" ;

	int x(x) ;
	double z(z) ;


data:

 y = 2;
 f = 2;
 x = 2;
 z = 2;

}
EOF


ncgen -b - <<EOF;
netcdf expect {
dimensions:
      x = 2 ;
      y = 2 ;
      z = 2 ;
 variables:
      float f(x, y, z) ;
              f:_FillValue = 42.f ;
              f:coordinates = "x y z" ;
      int x(x) ;
      double y(y) ;
      double z(z) ;
 data:

  f = 1, _, _, _, _, _, _, 2 ;
  x = 1, 2 ;
  y = 1, 2 ;
  z = 1, 2 ;

}
EOF

../../bin/ncmerge one.nc two.nc
ncdump out.nc
python ../../bin/nccompare.py out.nc expect.nc


# ncgen -b - <<EOF;
# netcdf one {
# dimensions:
#     x = 1 ;
#     z = 1 ;
# variables:
#     double y;
#     float f(x, z) ;
#         f:_FillValue = 42 ;
#         f:coordinates = "x y z" ;
#     int x(x);
#     double z(z) ;

# data:

#  y = 1;
#  f = 0;
#  x = 1;
#  z = 1;

# }

# EOF

# ncgen -b - <<EOF;
# netcdf two {
# dimensions:
#     x = 1 ;
#     z = 1 ;

# variables:
#     double y;

#     float f(x, z) ;
#         f:_FillValue = 42 ;
#         f:coordinates = "x y z" ;

#     int x(x) ;
#     double z(z) ;


# data:

#  y = 2;
#  f = 0;
#  x = 2;
#  z = 2;

# }
# EOF


# ncgen -b - <<EOF;
# netcdf expect {
# dimensions:
#         x = 1;
#         y = 3;

# variables:
#         double x(x);
#         double y(y);
#         y:units="seconds since 2009-01-20 00:00:00";
#         y:calendar = "gregorian" ;
#         double f(x, y);
#         f:coordinates="x y";

# data:
#         x = 1;
#         y = 172800, 86400, 0;
#         f = 1, 2, 3;
# }
# EOF

# ../../bin/ncmerge one.nc two.nc
# ncdump out.nc
# python ../../bin/nccompare.py out.nc expect.nc
