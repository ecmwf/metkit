/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars2grib/CodesKeySetter.h"


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

CodesKeySetter::CodesKeySetter(grib::GribHandle& h) :
    handle_{h} {}


void CodesKeySetter::setValue(const std::string& key, const std::string& value) {
    handle_.setValue(key, value);
}
void CodesKeySetter::setValue(const std::string& key, long value) {
    handle_.setValue(key, value);
}
void CodesKeySetter::setValue(const std::string& key, double value) {
    handle_.setValue(key, value);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
