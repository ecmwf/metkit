/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/CodesSplitter.h"

#include "eccodes.h"

#include "eckit/config/Resource.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"

#include "metkit/codes/GribHandle.h"
#include "metkit/codes/MallocDataContent.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

CodesSplitter::CodesSplitter(eckit::PeekHandle& handle) :
    Splitter(handle) {}

CodesSplitter::~CodesSplitter() {}

static long readcb(void* data, void* buffer, long len) {
    eckit::DataHandle* handle = reinterpret_cast<eckit::DataHandle*>(data);
    try {
        long r = handle->read(buffer, len);
        // DataHandle is returning 0 on EOF. Codes expects -1 for EOF.
        return (r == 0) ? -1 : r;
    }
    catch (const std::exception& e) {
        eckit::Log::error() << "Exception thrown in CodesSplitter::readcb callback: " << e.what() << "." << std::endl
                            << " This may cause unexpected behaviour. Returning -2 instead." << std::endl;
        // Return negative other from -1 to signalize error.
        return -2;
    }
    catch (...) {
        eckit::Log::error() << "Unknown exception occured in CodesSplitter::readcb callback. This may cause unexpected behaviour. Returning -2 instead." << std::endl;
        // Return negative other from -1 to signalize error.
        return -2;
    }
}

eckit::message::Message CodesSplitter::next() {
    size_t size;
    int err    = 0;
    void* data = wmo_read_any_from_stream_malloc(&handle_, &readcb, &size, &err);

    if (err != 0 and err != GRIB_END_OF_FILE) {
        if (data) {
            ::free(data);
        }
        if (err == GRIB_WRONG_LENGTH && handle_.canSeek()) {
            eckit::Offset off = handle_.position() - eckit::Length(size);
            handle_.seek((off < eckit::Offset(0) ? eckit::Offset(0) : off) + eckit::Offset(4));
        }
        CODES_CALL(err);
    }

    if (!data) {
        return eckit::message::Message();
    }

    return eckit::message::Message(new MallocDataContent(data, size, 0));
}

void CodesSplitter::print(std::ostream& s) const {
    s << "CodesSplitter[]";
}

}  // namespace codes
}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

namespace eckit {
namespace message {

template <>
bool SplitterBuilder<metkit::codes::CodesSplitter>::match(eckit::PeekHandle& handle) const {

    unsigned char c0 = handle.peek(0);
    unsigned char c1 = handle.peek(1);
    unsigned char c2 = handle.peek(2);
    unsigned char c3 = handle.peek(3);

    if (c0 == 'G' and c1 == 'R' and c2 == 'I' and c3 == 'B') {
        return true;
    }

    if (c0 == 'B' and c1 == 'U' and c2 == 'F' and c3 == 'R') {
        return true;
    }

    if (c0 == 'B' and c1 == 'U' and c2 == 'D' and c3 == 'G') {
        return true;
    }

    if (c0 == 'T' and c1 == 'I' and c2 == 'D' and c3 == 'E') {
        return true;
    }

    return false;
}

}  // namespace message
}  // namespace eckit

static eckit::message::SplitterBuilder<metkit::codes::CodesSplitter> splitter;

