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
#include "eccodes.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "metkit/codes/GribHandle.h"
#include "metkit/gribjump/GribHandleData.h"
#include "metkit/gribjump/GribInfo.h"

namespace metkit {
namespace gribjump {

JumpHandle::JumpHandle(const eckit::PathName& path):
    handle_(path.fileHandle()),
    ownsHandle_(true),
    opened_(false),
    path_(path)
    {}

JumpHandle::JumpHandle(eckit::DataHandle *handle):
    handle_(handle),
    ownsHandle_(true),
    opened_(false) {}

JumpHandle::~JumpHandle() {
    if (opened_) handle_->close();
    if (ownsHandle_) delete handle_;
}

void JumpHandle::open() const {
    if (!opened_) {
        handle_->openForRead();
        opened_ = true;
    }
}

void JumpHandle::close() const {
    if (opened_) {
        handle_->close();
        opened_ = false;
    }
}

eckit::Offset JumpHandle::seek(const eckit::Offset& offset) const {
    open();
    eckit::Offset pos = handle_->seek(offset);
    return (long long)pos;
}

long JumpHandle::read(void* buffer, long len) const {
    open();
    return handle_->read(buffer, len);
}

eckit::Offset JumpHandle::position(){ 
    open();
    return handle_->position();
}

eckit::Length JumpHandle::size(){ 
    open();
    return handle_->size();
}

// todo: now we're supporting non file handles, further refactor here would be good.
const JumpInfo& JumpHandle::extractInfoFromFile(eckit::PathName& outName){

    ASSERT(path_.asString().size() > 0);

    // count number of messages in file
    grib_context* c = nullptr;
    int n = 0;
    int err = codes_count_in_filename(c, path_.asString().c_str(), &n);
    ASSERT(!err);

    // extract metadata from each message to a binary file
    eckit::Offset offset = 0;
    for (size_t i = 0; i < n; i++) {
        open();
        grib::GribHandle h(*handle_, offset);
        info_.update(h);
        unsigned long fp = handle_->position();
        info_.setStartOffset(fp - info_.length());
        offset = handle_->position();
        info_.toFile(outName, i!=0);

        // XXX: On linux, fp is wrong if handle is not closed and reopened.
        close();
    }
    return info_;
}

const JumpInfo& JumpHandle::extractInfo(){
    // Note: Requires handle at start of message, and will advance handle to end of message.
    open();

    // Explicitly check we are at beginning of GRIB message
    eckit::Offset initialPos = handle_->position();
    char buffer[4];
    ASSERT(read(buffer, 4) == 4);
    ASSERT(strncmp(buffer, "GRIB", 4) == 0);
    ASSERT(seek(initialPos) == initialPos);

    grib::GribHandle h(*handle_, initialPos);
    info_.update(h);
    eckit::Offset endOfField = initialPos + eckit::Offset(info_.length());
    ASSERT(seek(endOfField) == endOfField); // In anticipation of next call
    return info_;
}

void JumpHandle::print(std::ostream& s) const {
    s << "JumpHandle[" << *handle_ << "]" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
} // namespace gribjump

} // namespace metkit
