// SPDX-FileCopyrightText: 1996-2013 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/pointdb/GribDataSource.h"
#include "metkit/pointdb/GribFieldInfo.h"


namespace metkit {
namespace pointdb {


PointResult GribDataSource::extract(double lat, double lon) const {


    PointResult result;

    // ASSERT(!source.needInterpolation());


    PointIndex& pi         = PointIndex::lookUp(geographyHash());
    PointIndex::NodeInfo n = pi.nearestNeighbour(lat, lon);

    result.lat_    = n.point().lat();
    result.lon_    = n.point().lon();
    result.value_  = value(n.point().payload_);
    result.source_ = this;

    return result;
}

double GribDataSource::value(size_t index) const {
    return info().value(*this, index);
}

std::string GribDataSource::geographyHash() const {
    return info().geographyHash();
}


//----------------------------------------------------------------------------------------------------------------------
}  // namespace pointdb

}  // namespace metkit
