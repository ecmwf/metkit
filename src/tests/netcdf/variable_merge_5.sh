#!/bin/bash
set -eaux

rm -f *.nc

ncgen -b - <<EOF;
netcdf one {
dimensions:
        x = 1 ;
variables:
        double f(x) ;
                f:coordinates = "x" ;
        double x(x) ;
data:

 f = 1 ;
 x = 1 ;


}
EOF

ncgen -b - <<EOF;
netcdf two {
dimensions:
        x = 1 ;

variables:
        double g(x) ;
        g:coordinates = "x" ;
        double x(x) ;
data:

 g = 2;
 x = 1 ;


}

EOF



ncgen -b - <<EOF;
netcdf expect {
dimensions:
     x = 1 ;
variables:
     double f(x) ;
             f:coordinates = "x" ;
     double g(x) ;
             g:coordinates = "x" ;
     double x(x) ;
data:

 f = 1 ;

 g = 2 ;

 x = 1 ;
}

EOF


../../bin/ncmerge one.nc two.nc


ncdump out.nc
ncdump expect.nc

python ../../bin/nccompare.py out.nc expect.nc
