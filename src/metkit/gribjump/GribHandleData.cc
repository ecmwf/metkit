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
#include "eccodes.h"

namespace metkit {
namespace gribjump {

GribHandleData::GribHandleData(const eckit::PathName& path):
    handle_(path.fileHandle()),
    ownsHandle_(true),
    opened_(false) {
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
    eckit::Offset pos = handle_->seek(offset);
    return (long long)pos;
}

long GribHandleData::read(void* buffer, long len) const {
    open();
    return handle_->read(buffer, len);
}

const pointdb::GribFieldInfo& GribHandleData::info() const {
    NOTIMP;
}

const GribInfo& GribHandleData::updateInfo(){
    if (!info_.ready()) {
        open();

        // count number of messages in file
        grib_context* c = nullptr;
        int n = 0;
        int err = codes_count_in_filename(c, gribpath_.asString().c_str(), &n);
        ASSERT(!err);

        // extract metadata from each message
        eckit::Offset offset = 0; // Note: we'll find the correct offset as we go
        for (size_t i = 0; i < n; i++) {
            grib::GribHandle h(*handle_, offset);
            info_.update(h);
            unsigned long fp = handle_->position();
            info_.set_msgStartOffset(fp - info_.get_totalLength());
            offset = handle_->position();
            std::cout << info_ << std::endl;

            // XXX for now, write each message data to its own json file.
            // Soon: stop using json, and write to binary file.
            eckit::PathName jsonName = gribpath_ + "_" + std::to_string(i) +  ".json";
            std::ofstream f(jsonName);
            eckit::JSON json(f, false);
            info_.toJSON(json);
            f.close();
        }
    }
    return info_;
}

void GribHandleData::print(std::ostream& s) const {
    s << "GribHandleData[" << *handle_ << "]" << std::endl;
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
