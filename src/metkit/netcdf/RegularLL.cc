/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "metkit/netcdf/RegularLL.h"
#include "metkit/netcdf/Variable.h"

#include <iostream>

namespace metkit {
namespace netcdf {

RegularLL::RegularLL(const Variable &variable,
                     double north,
                     double south,
                     double north_south_increment,
                     double west,
                     double east,
                     double west_east_increment):
    GridSpec(variable),
    north_(north),
    south_(south),
    north_south_increments_(north_south_increment),
    west_(west),
    east_(east),
    west_east_increment_(west_east_increment)
{
}

RegularLL::~RegularLL()
{
}


void RegularLL::print(std::ostream& s) const
{
    s << "RegularLL[bbox="
      << north_
      << ","
      << west_
      << ","
      << south_
      << ","
      << east_
      << ",grid="
      << west_east_increment_
      << ","
      << north_south_increments_
      << "]";
}

static bool check_axis(const Variable & axis,
                       double& first,
                       double& last,
                       double& increment) {

    if (axis.numberOfDimensions() != 1) {
        return false;
    }

    std::vector<double> v;
    axis.values(v);

    if (v.size() < 2) {
        return false;
    }


    double d = v[1] - v[0];

    for (size_t i = 1; i < v.size(); ++i) {
        if (( v[i] - v[i - 1]) != d) {
            return false;
        }
    }

    first = v[0];
    last = v[v.size() - 1];
    increment = d;

    return true;
}

GridSpec* RegularLL::guess(const Variable &variable,
                           const Variable &latitudes,
                           const Variable &longitudes) {

    double north, south, north_south_increment;
    if (!check_axis(latitudes, north, south, north_south_increment)) {
        return 0;
    }

    double west, east, west_east_increment;
    if (!check_axis(longitudes, west, east, west_east_increment)) {
        return 0;
    }

    return new RegularLL(variable,
                         north,
                         south,
                         north_south_increment,
                         west,
                         east,
                         west_east_increment);

}


static GridSpecGuesserBuilder<RegularLL> builder(0); // First choice
}
}
