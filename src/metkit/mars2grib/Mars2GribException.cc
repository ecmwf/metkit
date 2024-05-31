/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars2grib/Mars2GribException.h"


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

Mars2GribException::Mars2GribException(const std::string& reason, const eckit::CodeLocation& l) :
    eckit::Exception(std::string("Mars2GribException: ") + reason, l){};
    

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
