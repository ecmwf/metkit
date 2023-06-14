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


void GribHandleData::close() const {
    if (opened_) {
        handle_->close();
        opened_ = false;
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

const GribInfo& GribHandleData::extractMetadata(eckit::PathName& binName){
    if (!info_.ready()) {
        // count number of messages in file
        grib_context* c = nullptr;
        int n = 0;
        int err = codes_count_in_filename(c, gribpath_.asString().c_str(), &n);
        ASSERT(!err);

        // extract metadata from each message to a binary file
        eckit::Offset offset = 0; // Note: we'll find the correct offset as we go
        for (size_t i = 0; i < n; i++) {
            open();
            grib::GribHandle h(*handle_, offset);
            info_.update(h);
            unsigned long fp = handle_->position();
            info_.set_msgStartOffset(fp - info_.get_totalLength());
            offset = handle_->position();
            info_.toBinary(binName, i!=0);

            // XXX: On linux, fp is wrong if handle is not closed and reopened.
            // XXX: Possibly due to interaction in GribHandle constructor between openf and eccodes.
            // XXX: This needs to be investigated further.
            close();
        }
    }
    return info_;
}

void GribHandleData::print(std::ostream& s) const {
    s << "GribHandleData[" << *handle_ << "]" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
} // namespace gribjump

} // namespace metkit
