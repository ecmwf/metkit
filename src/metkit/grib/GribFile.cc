/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date Jan 2016

#include <string>

#include "grib_api.h"

#include "eckit/exception/Exceptions.h"

#include "metkit/grib/GribFile.h"
#include "metkit/grib/GribHandle.h"



namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

GribFile::GribFile(const eckit::PathName& path) :
    path_(path),
    file_(path_) {
}

GribFile::~GribFile() {
}

GribHandle* GribFile::next() {

    int err = 0;
    grib_handle* h = grib_handle_new_from_file(NULL, file_, &err);

    if(err == GRIB_SUCCESS)
        return new GribHandle(h);

    if(err == GRIB_END_OF_FILE)
        return NULL;

    std::ostringstream msg;
    msg << "Error reading GRIB file " << path_
        << " : " << grib_get_error_message(err);
    throw eckit::ReadError(msg.str(), Here());
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit


