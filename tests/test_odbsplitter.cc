/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/io/BufferList.h"
#include "eckit/io/FileHandle.h"
#include "eckit/message/Reader.h"
#include "eckit/testing/Test.h"

#include <cstring>

using namespace eckit::testing;

namespace metkit {
namespace grib {
namespace test {

//----------------------------------------------------------------------------------------------------------------------

class NonSeekFileHandle : public eckit::FileHandle {

    using eckit::FileHandle::FileHandle;

    bool canSeek() const override { return false; }
    eckit::Offset seek(const eckit::Offset&) override { NOTIMP; }
};

//----------------------------------------------------------------------------------------------------------------------

CASE("read multiple matched odb frames") {

    // This ODB is formed of two frames with matched metadata. We should read both frames into
    // one message.

    eckit::Buffer msgdata;

    {
        NonSeekFileHandle fh("multiodb.odb");
        fh.openForRead();
        eckit::AutoClose closer(fh);
        eckit::message::Reader reader(fh);

        eckit::message::Message msg;

        msg = reader.next();
        ASSERT(msg);
        msgdata = eckit::Buffer{msg.data(), msg.length()};

        // There is only one Message in the file
        msg = reader.next();
        EXPECT(!msg);
    }

    // And read the data in in one blob

    eckit::Buffer comparedata;

    {
        NonSeekFileHandle fh("multiodb.odb");
        fh.openForRead();
        eckit::AutoClose closer(fh);

        size_t expected_size = fh.size();
        comparedata.resize(expected_size);
        EXPECT(expected_size > 0);
        EXPECT(fh.read(comparedata.data(), expected_size) == expected_size);
    }

    EXPECT(comparedata.size() == msgdata.size());
    EXPECT(::memcmp(comparedata.data(), msgdata.data(), comparedata.size()) == 0);

    eckit::Log::info() << "odb size: " << msgdata.size() << std::endl;
}

CASE("read multiple matched odb frames") {

    // This ODB file is formed of four frames. Two pairs of matched metadata. This should be deconstructed into
    // two messages.

    eckit::BufferList msgdata;

    {
        eckit::FileHandle fh("multiodb2.odb");
        fh.openForRead();
        eckit::AutoClose closer(fh);
        eckit::message::Reader reader(fh);

        eckit::message::Message msg;

        for (int i = 0; i < 2; ++i) {
            msg = reader.next();
            ASSERT(msg);
            msgdata.append(eckit::Buffer{msg.data(), msg.length()});
        }

        // There is only one Message in the file
        msg = reader.next();
        EXPECT(!msg);
    }

    // And read the data in in one blob

    eckit::Buffer comparedata;

    {
        eckit::FileHandle fh("multiodb2.odb");
        fh.openForRead();
        eckit::AutoClose closer(fh);

        size_t expected_size = fh.size();
        comparedata.resize(expected_size);
        EXPECT(expected_size > 0);
        EXPECT(fh.read(comparedata.data(), expected_size) == expected_size);
    }

    eckit::Buffer combineddata = msgdata.consolidate();

    EXPECT(comparedata.size() == combineddata.size());
    EXPECT(::memcmp(comparedata.data(), combineddata.data(), combineddata.size()) == 0);

    eckit::Log::info() << "odb size: " << combineddata.size() << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace test
}  // namespace grib
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
