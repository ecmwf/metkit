#include <cstdio>
#include <cstring>

#include "netcdf.h"

/* This is the name of the data file we will create. */
#define FILE_NAME "sfc_pres_temp.nc"
/* We are writing 2D data, a 6 x 12 lat-lon grid. We will need two
 * netCDF dimensions. */
#define NDIMS 2
#define NLAT 6
#define NLON 12
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
/* Names of things. */
#define PRES_NAME "pressure"
#define TEMP_NAME "temperature"
#define UNITS "units"
#define DEGREES_EAST "degrees_east"
#define DEGREES_NORTH "degrees_north"
/* These are used to construct some example data. */
#define SAMPLE_PRESSURE 900
#define SAMPLE_TEMP 9.0
#define START_LAT 25.0
#define START_LON -125.0
/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERR(e)                                 \
    {                                          \
        printf("Error: %s\n", nc_strerror(e)); \
        return 2;                              \
    }
int main() {
    int ncid, lon_dimid, lat_dimid, pres_varid, temp_varid;
    /* In addition to the latitude and longitude dimensions, we will also
       create latitude and longitude netCDF variables which will hold the
       actual latitudes and longitudes. Since they hold data about the
       coordinate system, the netCDF term for these is: "coordinate
       variables." */
    int lat_varid, lon_varid;
    int dimids[NDIMS];
    /* We will write surface temperature and pressure fields. */
    float pres_out[NLAT][NLON];
    float temp_out[NLAT][NLON];
    float lats[NLAT], lons[NLON];

    /* It's good practice for each netCDF variable to carry a "units" attribute. */
    char pres_units[] = "hPa";
    char temp_units[] = "celsius";

    /* Error handling. */
    int retval;

    for (int lat = 0; lat < NLAT; lat++)
        lats[lat] = START_LAT + 5. * lat;
    for (int lon = 0; lon < NLON; lon++)
        lons[lon] = START_LON + 5. * lon;

    for (int lat = 0; lat < NLAT; lat++) {
        for (int lon = 0; lon < NLON; lon++) {
            pres_out[lat][lon] = SAMPLE_PRESSURE + (lon * NLAT + lat);
            temp_out[lat][lon] = SAMPLE_TEMP + .25 * (lon * NLAT + lat);
        }
    }


    /* Create the file. */
    if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
        ERR(retval);


    /* Define the dimensions. */
    if ((retval = nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)))
        ERR(retval);
    if ((retval = nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)))
        ERR(retval);


    // define latitudes
    if ((retval = nc_def_var(ncid, "lat", NC_FLOAT, 1, &lat_dimid, &lat_varid)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, lat_varid, "standard_name", strlen(LAT_NAME), LAT_NAME)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, lat_varid, UNITS, strlen(DEGREES_NORTH), DEGREES_NORTH)))
        ERR(retval);

    // define longitudes
    if ((retval = nc_def_var(ncid, "lon", NC_FLOAT, 1, &lon_dimid, &lon_varid)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, lon_varid, "standard_name", strlen(LON_NAME), LON_NAME)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, lon_varid, UNITS, strlen(DEGREES_EAST), DEGREES_EAST)))
        ERR(retval);


    /* Define the netCDF variables. The dimids array is used to pass the dimids of the dimensions of the variables.*/

    dimids[0] = lat_dimid;
    dimids[1] = lon_dimid;


    /// pressure
    if ((retval = nc_def_var(ncid, "press", NC_FLOAT, NDIMS, dimids, &pres_varid)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, pres_varid, "standard_name", strlen(PRES_NAME), PRES_NAME)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, pres_varid, UNITS, strlen(pres_units), pres_units)))
        ERR(retval);

    /// temperature
    if ((retval = nc_def_var(ncid, "temp", NC_FLOAT, NDIMS, dimids, &temp_varid)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, temp_varid, "standard_name", strlen(TEMP_NAME), TEMP_NAME)))
        ERR(retval);
    if ((retval = nc_put_att_text(ncid, temp_varid, UNITS, strlen(temp_units), temp_units)))
        ERR(retval);


    /* End define mode. */
    if ((retval = nc_enddef(ncid)))
        ERR(retval);


    // write data

    if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval);

    if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval);

    if ((retval = nc_put_var_float(ncid, pres_varid, &pres_out[0][0])))
        ERR(retval);

    if ((retval = nc_put_var_float(ncid, temp_varid, &temp_out[0][0])))
        ERR(retval);


    // Close the file
    if ((retval = nc_close(ncid)))
        ERR(retval);


    return 0;
}
