/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/grib/GribIterator.h"
#include "metkit/grib/GribHandle.h"
#include "eckit/exception/Exceptions.h"

#include "grib_api.h"


using namespace std;


namespace metkit {
namespace grib {

GribIterator::GribIterator(const GribHandle& handle):
    iterator_(nullptr) {

    int ret = 0;
    iterator_ = grib_iterator_new(handle.raw(), 0, &ret);
    ASSERT(iterator_);
    GRIB_CALL(ret);

}

GribIterator::~GribIterator() noexcept(false) {
    if (iterator_) {
        GRIB_CALL(grib_iterator_delete(iterator_));
        iterator_ = 0;
    }
}

bool GribIterator::next(double& lat, double& lon, double& value) {
    return grib_iterator_next(iterator_, &lat, &lon, &value);
}


}  // namespace grib
}  // namespace metkit
