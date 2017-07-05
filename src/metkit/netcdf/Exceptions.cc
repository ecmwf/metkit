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

#include "metkit/netcdf/Exceptions.h"

#include <netcdf.h>

namespace metkit {
namespace netcdf {

NCError::NCError(int e, const std::string& call, const std::string &path):
    eckit::Exception(std::string("NCError: ") + nc_strerror(e) +  + " " + call +  " (" + path + ")") {}


}
}
