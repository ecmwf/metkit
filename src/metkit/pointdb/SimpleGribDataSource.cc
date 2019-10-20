/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/pointdb/SimpleGribDataSource.h"
#include "metkit/pointdb/GribFieldInfo.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "metkit/grib/MetFile.h"
#include "eckit/io/Buffer.h"
#include "metkit/grib/GribHandle.h"
#include "metkit/pointdb/PointIndex.h"


namespace metkit {
namespace pointdb {

SimpleGribDataSource::SimpleGribDataSource(const eckit::PathName& path,
        const eckit::Offset& offset):
    handle_(path.fileHandle()),
    ownsHandle_(true),
    opened_(false),
    offset_(offset) {
}

SimpleGribDataSource::SimpleGribDataSource(eckit::DataHandle& handle,
        const eckit::Offset& offset):
    handle_(&handle),
    ownsHandle_(false),
    opened_(false),
    offset_(offset) {
}

SimpleGribDataSource::SimpleGribDataSource(eckit::DataHandle* handle,
        const eckit::Offset& offset):
    handle_(handle),
    ownsHandle_(true),
    opened_(false),
    offset_(offset) {
    ASSERT(handle_);
}


SimpleGribDataSource::~SimpleGribDataSource() {
    if (opened_) {
        handle_->close();
    }
    if (ownsHandle_) {
        delete handle_;
    }
}

void SimpleGribDataSource::open() const {
    if (!opened_) {
        handle_->openForRead();
    }
}

eckit::Offset SimpleGribDataSource::seek(const eckit::Offset& offset) const {
    open();
    eckit::Offset pos = handle_->seek(offset_ + offset);
    return (long long)pos - (long long)offset_;
}

long SimpleGribDataSource::read(void* buffer, long len) const {
    open();
    return handle_->read(buffer, len);
}

const GribFieldInfo& SimpleGribDataSource::info() const {
    if (!info_.ready()) {


        grib::MetFile in(*handle_, opened_);
        in.seek(offset_);

        eckit::Buffer buffer(64 * 1024 * 1024); // TODO: Parametrise
        in.readSome(buffer);

        grib::GribHandle h(buffer, false);
        info_.update(h);

        PointIndex::cache(h);
    }
    return info_;
}

void SimpleGribDataSource::print(std::ostream& s) const {
    s << info() << std::endl;
}


//----------------------------------------------------------------------------------------------------------------------
} // namespace pointdb

} // namespace metkit
