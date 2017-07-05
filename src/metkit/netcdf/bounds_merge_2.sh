#!/bin/bash
set -eaux

rm -f *.nc

ncgen -b - << EOF;
netcdf one {
dimensions:

  time = 1 ;
  tbnds = 2 ;

variables:

  double atime ;
    atime:units = "seconds since 2009-01-20 00:00:00" ;


  float field(time) ;
    field:coordinates = "atime time" ;
    field:_FillValue = 42;

  double time(time) ;
    time:units = "seconds since 2009-01-20 00:00:00" ;
    time:bounds = "time_bnds" ;

  double time_bnds(time, tbnds) ;

data:

    atime = 0;
    field = 1 ;
    time = 43200 ;
    time_bnds = 0, 86400 ;
}
EOF

ncgen -b - << EOF;
netcdf two {
dimensions:

  time = 1 ;
  tbnds = 2 ;

variables:

  float atime;
    atime:units = "seconds since 2009-01-20 00:00:00" ;

  float field(time) ;
    field:coordinates = "atime time" ;
    field:_FillValue = 42;

  double time(time) ;
    time:units = "seconds since 2009-01-20 00:00:00" ;
    time:bounds = "time_bnds" ;

  double time_bnds(time, tbnds) ;

data:

    atime = 0;
    field = 2 ;
    time = 129600 ;
    time_bnds = 86400, 172800 ;

}
EOF

ncgen -b - << EOF;
netcdf three {
dimensions:

  time = 1 ;
  tbnds = 2 ;

variables:

  double atime ;
    atime:units = "seconds since 2009-01-21 00:00:00" ;

  float field(time) ;
    field:coordinates = "atime time" ;
    field:_FillValue = 42;

  double time(time) ;
    time:units = "seconds since 2009-01-21 00:00:00" ;
    time:bounds = "time_bnds" ;

  double time_bnds(time, tbnds) ;

data:

    atime = 0;
    field = 3 ;
    time = 43200 ;
    time_bnds = 0, 86400 ;
}
EOF


ncgen -b - <<EOF;
netcdf expect {
dimensions:

  atime = 2 ;
  tbnds = 2 ;
  time = 2 ;

 variables:

    double atime(atime) ;
      atime:calendar = "gregorian" ;
      atime:units = "seconds since 2009-01-20 00:00:00" ;

    float field(atime, time) ;
      field:coordinates = "atime time" ;
        field:_FillValue = 42;

    double time(atime, time) ;
      time:bounds = "time_bnds" ;
      time:calendar = "gregorian" ;
      time:units = "seconds since 2009-01-20 00:00:00" ;

    double time_bnds(atime, time, tbnds) ;

 data:

    atime = 0, 86400 ;
    field = 1, 2, _, 3 ;
    time = _, 43200, 129600, _ ;
    time_bnds = 0, 86400, 86400, 172800, 0, 0, 0, 86400 ;
 }

EOF
../../bin/ncmerge one.nc two.nc three.nc #four.nc
ncdump out.nc
ncdump expect.nc

python ../../bin/nccompare.py out.nc expect.nc
