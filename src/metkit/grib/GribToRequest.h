/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Manuel Fuentes
/// @author Tiago Quintino

/// @date Dec 2015

#ifndef metkit_GribToRequest_H
#define metkit_GribToRequest_H

struct grib_handle;

#include "metkit/grib/GribHandle.h"

namespace metkit {

namespace mars {
class MarsRequest;
}

namespace grib {

//----------------------------------------------------------------------------------------------------------------------

/// Utility class to build MarsRequest from GribHandle

/// Part of this code is taken from mars-metkit grib.c

class GribToRequest {
public:  // methods
    static void handleToRequest(grib_handle* const grib, mars::MarsRequest& req);

    static void handleToRequest(const grib::GribHandle& grib, mars::MarsRequest& req);

    static void gribToRequest(const void* buffer, size_t length, mars::MarsRequest& req);

    static void messageToRequest(const void* buffer, size_t length, mars::MarsRequest& req);

private:
    GribToRequest() = delete;
    ~GribToRequest() = delete;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace grib
}  // namespace metkit

#endif
