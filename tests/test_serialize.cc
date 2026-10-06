/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_serialize.cc
/// @date   October 2026

#include <unistd.h>

#include <cstdint>
#include <fstream>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/testing/Test.h"

#include "metkit/mars/Serialize.h"

namespace metkit::mars::test {

namespace {

/// Temporary file removed on destruction
class TempFile {
public:

    explicit TempFile(const std::string& name) { path_ = eckit::PathName::unique("metkit_test_serialize_" + name); }

    ~TempFile() { path_.unlink(); }

    TempFile(const TempFile&)            = delete;
    TempFile& operator=(const TempFile&) = delete;

    std::string path() const { return path_.asString(); }

    std::ofstream open() const { return std::ofstream(path_, std::ios::binary | std::ios::trunc); }

private:

    eckit::PathName path_;
};

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

CASE("Integers round trip") {
    TempFile tmp("integers");
    {
        auto out = tmp.open();
        write8(out, uint8_t{0});
        write8(out, uint8_t{255});
        write16(out, uint16_t{0});
        write16(out, uint16_t{0x1234});
        write16(out, std::numeric_limits<uint16_t>::max());
        write32(out, uint32_t{0});
        write32(out, uint32_t{0x12345678});
        write32(out, std::numeric_limits<uint32_t>::max());
        writeBool(out, true);
        writeBool(out, false);
        out.close();
        EXPECT(out);
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read8(), 0);
    EXPECT_EQUAL(in.read8(), 255);
    EXPECT_EQUAL(in.read16(), 0);
    EXPECT_EQUAL(in.read16(), 0x1234);
    EXPECT_EQUAL(in.read16(), std::numeric_limits<uint16_t>::max());
    EXPECT_EQUAL(in.read32(), 0u);
    EXPECT_EQUAL(in.read32(), 0x12345678u);
    EXPECT_EQUAL(in.read32(), std::numeric_limits<uint32_t>::max());
    EXPECT_EQUAL(in.read8(), 1);  // true
    EXPECT_EQUAL(in.read8(), 0);  // false
    EXPECT(in.atEnd());
}

CASE("Integers are written little endian") {
    TempFile tmp("endian");
    {
        auto out = tmp.open();
        write16(out, uint16_t{0x0102});
        write32(out, uint32_t{0x03040506});
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read8(), 0x02);
    EXPECT_EQUAL(in.read8(), 0x01);
    EXPECT_EQUAL(in.read8(), 0x06);
    EXPECT_EQUAL(in.read8(), 0x05);
    EXPECT_EQUAL(in.read8(), 0x04);
    EXPECT_EQUAL(in.read8(), 0x03);
    EXPECT(in.atEnd());
}

CASE("size_t overloads round trip") {
    TempFile tmp("size_t");
    {
        auto out = tmp.open();
        write8(out, size_t{200});
        write16(out, size_t{60000});
        write32(out, size_t{4000000000});
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read8(), 200);
    EXPECT_EQUAL(in.read16(), 60000);
    EXPECT_EQUAL(in.read32(), 4000000000u);
    EXPECT(in.atEnd());
}

CASE("size_t overloads reject out of range values") {
    TempFile tmp("size_t_overflow");
    auto out = tmp.open();

    EXPECT_THROWS_AS(write8(out, size_t{256}), eckit::SeriousBug);
    EXPECT_THROWS_AS(write16(out, size_t{65536}), eckit::SeriousBug);

    // the boundaries are accepted
    write8(out, size_t{255});
    write16(out, size_t{65535});
}

CASE("Strings round trip") {
    TempFile tmp("strings");

    const std::string embedded("a\0b", 3);
    const std::string longest(std::numeric_limits<uint16_t>::max(), 'x');
    const std::string utf8 = "caf\xc3\xa9";
    {
        auto out = tmp.open();
        writeString(out, "");
        writeString(out, "a");
        writeString(out, "hello world");
        writeString(out, embedded);
        writeString(out, utf8);
        writeString(out, longest);
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.readString(), "");
    EXPECT_EQUAL(in.readString(), "a");
    EXPECT_EQUAL(in.readString(), "hello world");
    EXPECT_EQUAL(in.readString(), std::string_view(embedded));
    EXPECT_EQUAL(in.readString(), utf8);
    EXPECT_EQUAL(in.readString(), longest);
    EXPECT(in.atEnd());
}

CASE("String longer than 65535 characters is rejected") {
    TempFile tmp("string_too_long");
    auto out = tmp.open();
    EXPECT_THROWS_AS(writeString(out, std::string(std::numeric_limits<uint16_t>::max() + 1u, 'x')), eckit::SeriousBug);
}

CASE("String with explicit length") {
    TempFile tmp("string_length");
    {
        auto out = tmp.open();
        // raw characters without length prefix
        const std::string raw = "abcdefgh";
        out.write(raw.data(), static_cast<std::streamsize>(raw.size()));
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.readString(3), "abc");
    EXPECT_EQUAL(in.readString(0), "");
    EXPECT_EQUAL(in.readString(5), "defgh");
    EXPECT(in.atEnd());
    EXPECT_THROWS_AS(in.readString(1), eckit::SeriousBug);
}

CASE("String set round trip") {
    TempFile tmp("string_set");

    const std::set<std::string> empty;
    const std::set<std::string> values{"alpha", "beta", "gamma", "", "alpha2"};
    {
        auto out = tmp.open();
        writeStringSet(out, empty);
        writeStringSet(out, values);
    }

    MemFile in(tmp.path());
    EXPECT(in.readStringSet() == empty);
    EXPECT(in.readStringSet() == values);
    EXPECT(in.atEnd());
}

CASE("String vector round trip") {
    TempFile tmp("string_vector");

    const std::vector<std::string> empty;
    const std::vector<std::string> values{"one", "two", "two", "", "three"};  // order and duplicates are kept
    {
        auto out = tmp.open();
        writeStringVector(out, empty);
        writeStringVector(out, values);
    }

    MemFile in(tmp.path());
    EXPECT(in.readStringVector() == empty);
    EXPECT(in.readStringVector() == values);
    EXPECT(in.atEnd());
}

CASE("Collections with more than 255 elements are rejected") {
    TempFile tmp("collection_too_big");
    auto out = tmp.open();

    std::vector<std::string> v(256, "x");
    EXPECT_THROWS_AS(writeStringVector(out, v), eckit::SeriousBug);

    std::set<std::string> s;
    for (int i = 0; i < 256; ++i) {
        s.insert(std::to_string(i));
    }
    EXPECT_THROWS_AS(writeStringSet(out, s), eckit::SeriousBug);

    v.resize(255);
    writeStringVector(out, v);
}

CASE("Mixed content round trip") {
    TempFile tmp("mixed");
    {
        auto out = tmp.open();
        writeBool(out, true);
        write8(out, uint8_t{42});
        write16(out, uint16_t{1000});
        write32(out, uint32_t{123456789});
        writeString(out, "param");
        writeStringSet(out, {"a", "b", "c"});
        writeStringVector(out, {"z", "y", "x"});
        write32(out, uint32_t{7});
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read8(), 1);
    EXPECT_EQUAL(in.read8(), 42);
    EXPECT_EQUAL(in.read16(), 1000);
    EXPECT_EQUAL(in.read32(), 123456789u);
    EXPECT_EQUAL(in.readString(), "param");
    EXPECT((in.readStringSet() == std::set<std::string>{"a", "b", "c"}));
    EXPECT((in.readStringVector() == std::vector<std::string>{"z", "y", "x"}));
    EXPECT_EQUAL(in.read32(), 7u);
    EXPECT(in.atEnd());
}

CASE("Seek") {
    TempFile tmp("seek");
    {
        auto out = tmp.open();
        write32(out, uint32_t{111});  // offset 0
        writeString(out, "abc");      // offset 4
        write16(out, uint16_t{222});  // offset 9
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read32(), 111u);
    EXPECT_EQUAL(in.readString(), "abc");
    EXPECT_EQUAL(in.read16(), 222);
    EXPECT(in.atEnd());

    in.seek(9);
    EXPECT(!in.atEnd());
    EXPECT_EQUAL(in.read16(), 222);

    in.seek(4);
    EXPECT_EQUAL(in.readString(), "abc");

    in.seek(0);
    EXPECT_EQUAL(in.read32(), 111u);

    in.seek(11);  // seeking to the end is allowed
    EXPECT(in.atEnd());

    EXPECT_THROWS_AS(in.seek(12), eckit::SeriousBug);
    EXPECT_THROWS_AS(in.seek(-1), eckit::SeriousBug);
}

CASE("Reading past the end fails") {
    TempFile tmp("past_end");
    {
        auto out = tmp.open();
        write8(out, uint8_t{1});
        write16(out, uint16_t{2});
        write16(out, uint16_t{10});  // string length larger than the remaining data
        out.write("ab", 2);
    }

    MemFile in(tmp.path());
    EXPECT_EQUAL(in.read8(), 1);
    EXPECT_EQUAL(in.read16(), 2);
    EXPECT_THROWS_AS(in.readString(), eckit::SeriousBug);

    in.seek(3);
    in.read16();
    in.read16();
    EXPECT(in.atEnd());
    EXPECT_THROWS_AS(in.read8(), eckit::SeriousBug);
    EXPECT_THROWS_AS(in.read16(), eckit::SeriousBug);
    EXPECT_THROWS_AS(in.read32(), eckit::SeriousBug);
    EXPECT_THROWS_AS(in.readString(), eckit::SeriousBug);

    // not enough bytes left for the wider type
    in.seek(5);  // 2 bytes left
    EXPECT_THROWS_AS(in.read32(), eckit::SeriousBug);
}

CASE("Truncated collections fail") {
    TempFile tmp("truncated");
    {
        auto out = tmp.open();
        write8(out, uint8_t{3});  // announces 3 strings, only 1 is written
        writeString(out, "only");
    }

    {
        MemFile in(tmp.path());
        EXPECT_THROWS_AS(in.readStringVector(), eckit::SeriousBug);
    }
    {
        MemFile in(tmp.path());
        EXPECT_THROWS_AS(in.readStringSet(), eckit::SeriousBug);
    }
}

CASE("Opening missing or empty files fails") {
    TempFile missing("missing");
    EXPECT_THROWS_AS(MemFile(missing.path()), eckit::SeriousBug);

    TempFile empty("empty");
    { auto out = empty.open(); }
    EXPECT_THROWS_AS(MemFile(empty.path()), eckit::SeriousBug);
}

CASE("Move semantics") {
    TempFile tmp("move");
    {
        auto out = tmp.open();
        write32(out, uint32_t{1});
        write32(out, uint32_t{2});
        writeString(out, "moved");
    }

    MemFile a(tmp.path());
    EXPECT_EQUAL(a.read32(), 1u);

    // position is carried over by the move constructor
    MemFile b(std::move(a));
    EXPECT_EQUAL(b.read32(), 2u);

    // move assignment replaces a default constructed (empty) file
    MemFile c;
    c = std::move(b);
    EXPECT_EQUAL(c.readString(), "moved");
    EXPECT(c.atEnd());

    // move assignment onto a mapped file releases the previous mapping
    MemFile d(tmp.path());
    c = std::move(d);
    EXPECT_EQUAL(c.read32(), 1u);
}

CASE("String views stay valid while the MemFile is alive") {
    TempFile tmp("views");
    {
        auto out = tmp.open();
        writeString(out, "first");
        writeString(out, "second");
    }

    MemFile in(tmp.path());
    std::string_view first  = in.readString();
    std::string_view second = in.readString();
    EXPECT_EQUAL(first, "first");
    EXPECT_EQUAL(second, "second");
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
