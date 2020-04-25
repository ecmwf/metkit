/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/pointdb/DataSource.h"
#include "metkit/pointdb/PointIndex.h"


namespace metkit {
namespace pointdb {

DataSource::~DataSource() {

}


PointResult DataSource::extract(double lat,
                                double lon) const {





    PointResult result;

    // ASSERT(!source.needInterpolation());


    PointIndex& pi = PointIndex::lookUp(geographyHash());
    PointIndex::NodeInfo n = pi.nearestNeighbour(lat, lon);

    result.lat_      = n.point().lat();
    result.lon_      = n.point().lon();
    result.value_    = value(n.point().payload_);
    result.source_   = this;

    return result;

}

void PointResult::print(std::ostream& s) const {
    s << "PointResult[lat=" << lat_ << ",lon=" << lon_ << ",value=" << value_ << "]";
}


//----------------------------------------------------------------------------------------------------------------------
} // namespace pointdb

} // namespace metkit
