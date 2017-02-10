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
/// @author Manuel Fuentes
/// @author Tiago Quintino

/// @date Dec 2015

#ifndef grib_GribIndex_H
#define grib_GribIndex_H

#include "eckit/types/Types.h"
#include "eckit/io/Offset.h"
#include "eckit/serialisation/Stream.h"

namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

class GribMetaData;

struct GribIndex
{
    ~GribIndex();

    void readFrom( eckit::Stream& s );

    eckit::OffsetList         offset_;
    eckit::LengthList         length_;
    std::vector<GribMetaData*>  handle_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif
