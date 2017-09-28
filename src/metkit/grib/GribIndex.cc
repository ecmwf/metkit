/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/grib/GribMetaData.h"
#include "metkit/grib/GribIndex.h"



namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

GribIndex::~GribIndex()
{
    for( size_t i = 0; i < handle_.size(); ++i ) {
		GribMetaData* h = handle_[i];
		delete h;
	}
}

void GribIndex::readFrom( eckit::Stream& s )
{
    ASSERT( length_.size() == 0);
    ASSERT( offset_.size() == 0);
    ASSERT( handle_.size() == 0);

    unsigned long count;

    s >> count;

    length_.resize(count);
    offset_.resize(count);
    handle_.resize(count);

    for( size_t i = 0; i < count; ++i )
    {
        unsigned long long x;
        s >> x; offset_[i] = x;
        s >> x; length_[i] = x;
        handle_[i] = new GribMetaData(s);
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit
