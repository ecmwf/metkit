/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <iomanip>

#include "metkit/pointdb/GribHandleDataSource.h"
#include "metkit/pointdb/GribFieldInfo.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/PooledHandle.h"
#include "metkit/pointdb/PointIndex.h"
#include "eckit/utils/MD5.h"
#include "eckit/io/StdFile.h"
#include "metkit/grib/GribHandle.h"

namespace metkit {
namespace pointdb {

GribHandleDataSource::GribHandleDataSource(const eckit::PathName& path,
        const eckit::Offset& offset):
    handle_(path.fileHandle()),
    ownsHandle_(true),
    opened_(false),
    offset_(offset) {
}

GribHandleDataSource::GribHandleDataSource(eckit::DataHandle& handle,
        const eckit::Offset& offset):
    handle_(&handle),
    ownsHandle_(false),
    opened_(false),
    offset_(offset) {
}

GribHandleDataSource::GribHandleDataSource(eckit::DataHandle* handle,
        const eckit::Offset& offset):
    handle_(handle),
    ownsHandle_(true),
    opened_(false),
    offset_(offset) {
    ASSERT(handle_);
}


GribHandleDataSource::~GribHandleDataSource() {
    if (opened_) {
        handle_->close();
    }
    if (ownsHandle_) {
        delete handle_;
    }
}

void GribHandleDataSource::open() const {
    if (!opened_) {
        handle_->openForRead();
        opened_ = true;
    }
}

eckit::Offset GribHandleDataSource::seek(const eckit::Offset& offset) const {
    open();
    eckit::Offset pos = handle_->seek(offset_ + offset);
    return (long long)pos - (long long)offset_;
}

long GribHandleDataSource::read(void* buffer, long len) const {
    open();
    return handle_->read(buffer, len);
}

const GribFieldInfo& GribHandleDataSource::info() const {
    if (!info_.ready()) {

        eckit::MD5 md5;
        md5 << *handle_;
        md5 << static_cast<long long>(offset_);

        eckit::PathName cache = PointIndex::cachePath("grib-info", md5);
        if (cache.exists()) {
            eckit::StdFile f(cache);
            ASSERT(::fread(&info_, sizeof(info_), 1, f) == 1);
            f.close();
        }
        else {
            open();

            handle_->seek(offset_);

            grib::GribHandle h(*handle_);
            info_.update(h);

            cache.dirName().mkdir();

            eckit::StdFile f(cache, "w");
            ASSERT(::fwrite(&info_, sizeof(info_), 1, f) == 1);
            f.close();


            PointIndex::cache(h);
        }
    }
    return info_;
}

void GribHandleDataSource::print(std::ostream& s) const {
    s << "GribHandleDataSource[" << *handle_ << "]" << std::endl;
}

const std::map<std::string, eckit::Value>& GribHandleDataSource::request() const {
    NOTIMP; // Implement a grib2request like function
}

std::string GribHandleDataSource::groupKey() const {
    eckit::MD5 md5;
    md5 << *handle_;
    return md5;
}


std::string GribHandleDataSource::sortKey() const {
    std::ostringstream oss;
    oss << std::setfill ('0') << std::setw (20) << offset_;
    return oss.str();
}


//----------------------------------------------------------------------------------------------------------------------
} // namespace pointdb

} // namespace metkit
