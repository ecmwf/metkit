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
#include <fstream>

#include "metkit/gribjump/GribHandleData.h"
#include "metkit/gribjump/GribInfo.h"
#include "metkit/codes/GribHandle.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "eckit/log/JSON.h"

namespace metkit {
namespace gribjump {

GribHandleData::GribHandleData(const eckit::PathName& path,
        const eckit::Offset& offset):
    handle_(path.fileHandle()),
    ownsHandle_(true),
    opened_(false),
    offset_(offset) {
    gribpath_ = path;
}

GribHandleData::~GribHandleData() {
    if (opened_) {
        handle_->close();
    }
    if (ownsHandle_) {
        delete handle_;
    }
}

void GribHandleData::open() const {
    if (!opened_) {
        handle_->openForRead();
        opened_ = true;
    }
}

eckit::Offset GribHandleData::seek(const eckit::Offset& offset) const {
    open();
    eckit::Offset pos = handle_->seek(offset_ + offset);
    return (long long)pos - (long long)offset_;
}

long GribHandleData::read(void* buffer, long len) const {
    open();
    return handle_->read(buffer, len);
}

const pointdb::GribFieldInfo& GribHandleData::info() const {
    NOTIMP;
}

const GribInfo& GribHandleData::updateInfo(){
    // update info and write to json.
    if (!info_.ready()) {

        seek(offset_);
        grib::GribHandle h(*handle_);
        info_.update(h);

        // Write to json
        // XXX gribpath_ is set in constructor, but in future will probably always be reading from a file.
        eckit::PathName jsonName = gribpath_ + ".json";
        std::ofstream f(jsonName);
        eckit::JSON json(f, false);
        info_.toJSON(json);
        f.close();
    }
    return info_;
}

void GribHandleData::print(std::ostream& s) const {
    s << "GribHandleDataSource2[" << *handle_ << "]" << std::endl;
}

const std::map<std::string, eckit::Value>& GribHandleData::request() const {
    NOTIMP;
}

std::string GribHandleData::groupKey() const {
    NOTIMP;
}


std::string GribHandleData::sortKey() const {
    NOTIMP;
}


//----------------------------------------------------------------------------------------------------------------------
} // namespace gribjump

} // namespace metkit
