/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/memory/NonCopyable.h"

struct grib_iterator;

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

class GribHandle;

class GribIterator : private eckit::NonCopyable {
public:

    GribIterator(const GribHandle&);
    ~GribIterator() noexcept(false);


    bool next(double& lat, double& lon, double& value);

private:  // members

    grib_iterator* iterator_;
};

//------------------------------------------------------------------------------------------------------

}  // namespace grib
}  // namespace metkit
