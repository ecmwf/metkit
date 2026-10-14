// SPDX-FileCopyrightText: 1996-2013 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/pointdb/DataSource.h"
#include "metkit/pointdb/PointIndex.h"

namespace metkit::pointdb {

size_t DataSource::batch() const {
    return 0;
}

void PointResult::print(std::ostream& s) const {
    s << "PointResult[lat=" << lat_ << ",lon=" << lon_ << ",value=" << value_ << "]";
}

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::pointdb
