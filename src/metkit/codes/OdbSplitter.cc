/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */




#include "metkit/codes/OdbSplitter.h"
#include "metkit/codes/OdbContent.h"

#include "eckit/config/Resource.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/io/SeekableHandle.h"
#include "eckit/io/Buffer.h"

#include "odc/core/Header.h"
#include "odc/core/MetaData.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
OdbSplitter::OdbSplitter(eckit::PeekHandle& handle):
    Splitter(handle),
    first_(true) {
}

OdbSplitter::~OdbSplitter() {
}

const int32_t BYTE_ORDER_INDICATOR = 1;

template<typename T>
static void swap(T& o, int32_t byteOrder) {
    if (byteOrder != BYTE_ORDER_INDICATOR) {
        std::reverse(reinterpret_cast<char*>(&o),
                     reinterpret_cast<char*>(&o) + sizeof(T));
    }
}

static int64_t frame_size(eckit::PeekHandle& handle) {
    eckit::SeekableHandle f(handle);

    if (false) {
        odc::core::MetaData md;
        odc::core::Properties props;
        odc::core::Header header(md, props);
        if (odc::core::Header::readMagic(f)) {
            header.loadAfterMagic(f);
            eckit::Log::info() << "nextFrameOffset " << header.dataSize()  << std::endl;
            eckit::Log::info() << "numberOfRows " << header.rowsNumber()  << std::endl;

        }
        return handle.size();

    }
    else {

        auto here = f.position();

        uint16_t magic;
        if(f.read(&magic, sizeof(magic)) == 0) {
            eckit::Log::info() << "EOF" << std::endl;
            return 0;
        }
        ASSERT(magic == 0xffff);

        eckit::Log::info() << "magic " << magic  << std::endl;

        unsigned char c;
        f.read(&c, sizeof(c)); ASSERT(c == 'O');
        f.read(&c, sizeof(c)); ASSERT(c == 'D');
        f.read(&c, sizeof(c)); ASSERT(c == 'A');

        int32_t byteOrder;
        f.read(&byteOrder, sizeof(byteOrder));
        ASSERT(byteOrder == BYTE_ORDER_INDICATOR);
        eckit::Log::info() << "byteOrder " << byteOrder  << std::endl;

        int32_t formatVersionMajor;
        f.read(&formatVersionMajor, sizeof(formatVersionMajor));
        swap(formatVersionMajor, byteOrder);
        eckit::Log::info() << "formatVersionMajor " << formatVersionMajor  << std::endl;

        int32_t formatVersionMinor;
        f.read(&formatVersionMinor, sizeof(formatVersionMinor));
        swap(formatVersionMinor, byteOrder);
        eckit::Log::info() << "formatVersionMinor " << formatVersionMinor  << std::endl;

        int32_t len;
        f.read(&len, sizeof(len));
        swap(len, byteOrder);
        eckit::Buffer buf1(len);
        f.read(buf1, len);

        eckit::Log::info() << "headerDigest " << len << " " << std::string((char*)buf1, ((char*)buf1) + len)  << std::endl;

        // Buffer?
        int32_t headerSize;
        f.read(&headerSize, sizeof(headerSize));
        swap(headerSize, byteOrder);
        eckit::Log::info() << "headerSize " << headerSize  << std::endl;

        size_t fixedHeaderSize = f.position() - here;

        int64_t nextFrameOffset;
        f.read(&nextFrameOffset, sizeof(nextFrameOffset));
        swap(nextFrameOffset, byteOrder);
        eckit::Log::info() << "nextFrameOffset " << nextFrameOffset  << std::endl;

        // int64_t prevFrameOffset;
        // f.read(&prevFrameOffset, sizeof(prevFrameOffset));
        // swap(prevFrameOffset, byteOrder);
        // eckit::Log::info() << "prevFrameOffset " << prevFrameOffset  << std::endl;

        // int64_t numberOfRows;
        // f.read(&numberOfRows, sizeof(numberOfRows));
        // swap(numberOfRows, byteOrder);
        // eckit::Log::info() << "numberOfRows " << numberOfRows  << std::endl;


        eckit::Log::info() << "fixedHeaderSize " << fixedHeaderSize  << std::endl;

        eckit::Log::info() << "size " << (nextFrameOffset + headerSize + fixedHeaderSize)  << std::endl;

        return nextFrameOffset + headerSize + fixedHeaderSize;
    }

}

Message OdbSplitter::next() {

    return Message{new OdbContent(handle_, frame_size(handle_))};

    // if (first_) {
    //     first_ = false;
    //     // return Message{new OdbContent(handle_, frame_size(handle_))};
    //     return Message{new OdbContent(handle_, handle_.size())};
    // }
    // return Message();
}

void OdbSplitter::print(std::ostream& s) const {
    s << "OdbSplitter[]";
}

template<>
bool SplitterBuilder<OdbSplitter>::match(eckit::PeekHandle& handle) const {

    unsigned char c0 = handle.peek(0);
    unsigned char c1 = handle.peek(1);
    unsigned char c2 = handle.peek(2);
    unsigned char c3 = handle.peek(3);
    unsigned char c4 = handle.peek(4);

    if (c0 == 0xff and c1 == 0xff and c2 == 'O' and c3 == 'D' and c4 == 'A') {
        return true;
    }

    return false;
}

static SplitterBuilder<OdbSplitter> splitter;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
