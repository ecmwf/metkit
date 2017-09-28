/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "grib_api.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/log/Bytes.h"
#include "eckit/parser/StringTools.h"

#include "metkit/grib/GribDataBlob.h"
#include "metkit/grib/GribHandle.h"



namespace metkit {
namespace grib {

// -------------------------------------------------------------------------------------------------

// Construct GribDataBlob builder object, to self-register this type with the
// DataBlobFactory
namespace {
    eckit::DataBlobBuilder<GribDataBlob> gribBlobBuilder("grib");
}

// -------------------------------------------------------------------------------------------------

GribDataBlob::GribDataBlob(const void* data, size_t length) :
    eckit::DataBlob(data, length),
    metadata_(data, length)
{
    size_t len = metadata_.length();
    ASSERT(len <= buffer_.size());
    actualLength_ = len;
}

GribDataBlob::GribDataBlob(eckit::DataHandle& dh, size_t length) :
    eckit::DataBlob(dh, length),
    metadata_(buffer(), length)
{
    size_t len = metadata_.length();
    ASSERT(len <= buffer_.size());
    actualLength_ = len;
}

GribDataBlob::~GribDataBlob() {
}

const eckit::Metadata &GribDataBlob::metadata() const {
    return metadata_;
}

void GribDataBlob::print(std::ostream& os) const {
    os << "GribDataBlob[size=" << eckit::Bytes(buffer_.size())
       << ",metadata=" << metadata_
       << "]";
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit
