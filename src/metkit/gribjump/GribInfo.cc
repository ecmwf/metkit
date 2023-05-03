/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/gribjump/GribInfo.h"
#include "metkit/codes/GribAccessor.h"
#include "metkit/gribjump/GribHandleData.h"
#include "eckit/parser/JSONParser.h"
#include <bitset>

using namespace eckit;
using namespace metkit::grib;


extern "C" {
    unsigned long grib_decode_unsigned_long(const unsigned char* p, long* offset, int bits);
    double grib_power(long s, long n);
}

namespace metkit {
namespace gribjump {

static GribAccessor<long>          bitmapPresent("bitmapPresent");
static GribAccessor<long>          binaryScaleFactor("binaryScaleFactor");
static GribAccessor<long>          decimalScaleFactor("decimalScaleFactor");
static GribAccessor<unsigned long> bitsPerValue("bitsPerValue");
static GribAccessor<double>        referenceValue("referenceValue");
static GribAccessor<unsigned long> offsetBeforeData("offsetBeforeData");
static GribAccessor<unsigned long> offsetBeforeBitmap("offsetBeforeBitmap");
static GribAccessor<unsigned long> numberOfValues("numberOfValues");
static GribAccessor<unsigned long> numberOfDataPoints("numberOfDataPoints");
static GribAccessor<long>          sphericalHarmonics("sphericalHarmonics");

static Mutex mutex;

#define MISSING 9999

static int bits[65536] = {
#include "metkit/pointdb/bits.h"
};


static inline int count_bits(unsigned long long n) {
    return bits[n         & 0xffffu]
           +  bits[(n >> 16) & 0xffffu]
           +  bits[(n >> 32) & 0xffffu]
           +  bits[(n >> 48) & 0xffffu];
}

static inline uint64_t reverse_bytes(uint64_t n) {
    return ((n & 0x00000000000000FF) << 56) |
           ((n & 0x000000000000FF00) << 40) |
           ((n & 0x0000000000FF0000) << 24) |
           ((n & 0x00000000FF000000) << 8) |
           ((n & 0x000000FF00000000) >> 8) |
           ((n & 0x0000FF0000000000) >> 24) |
           ((n & 0x00FF000000000000) >> 40) |
           ((n & 0xFF00000000000000) >> 56);
}

GribInfo::GribInfo():
    referenceValue_(0),
    binaryScaleFactor_(0),
    decimalScaleFactor_(0),
    bitsPerValue_(0),
    offsetBeforeData_(0),
    offsetBeforeBitmap_(0),
    numberOfValues_(0),
    numberOfDataPoints_(0),
    sphericalHarmonics_(0) {
    binaryMultiplier_ = 1;
    decimalMultiplier_ = 1;
}

void GribInfo::update(const GribHandle& h) {
    binaryScaleFactor_  = binaryScaleFactor(h);
    decimalScaleFactor_ = decimalScaleFactor(h);
    bitsPerValue_       = bitsPerValue(h);
    referenceValue_     = referenceValue(h);
    offsetBeforeData_   = offsetBeforeData(h);
    numberOfDataPoints_ = numberOfDataPoints(h);
    numberOfValues_     = numberOfValues(h);
    sphericalHarmonics_ = sphericalHarmonics(h);

    if (bitmapPresent(h))
        offsetBeforeBitmap_ = offsetBeforeBitmap(h);
    else
        offsetBeforeBitmap_ = 0;

    binaryMultiplier_ = grib_power(binaryScaleFactor_, 2);
    decimalMultiplier_ = grib_power(-decimalScaleFactor_, 10);
}

void GribInfo::print(std::ostream& s) const {
    s << "GribInfo[" << std::endl;
    s << "    binaryScaleFactor=" << binaryScaleFactor_ << std::endl;
    s << "    decimalScaleFactor=" << decimalScaleFactor_ << std::endl;
    s << "    bitsPerValue=" << bitsPerValue_ << std::endl;
    s << "    referenceValue=" << referenceValue_ << std::endl;
    s << "    offsetBeforeData=" << offsetBeforeData_ << std::endl;
    s << "    numberOfDataPoints=" << numberOfDataPoints_ << std::endl;
    s << "    numberOfValues=" << numberOfValues_ << std::endl;
    s << "    offsetBeforeBitmap=" << offsetBeforeBitmap_ << std::endl;
    s << "    sphericalHarmonics=" << sphericalHarmonics_ << std::endl;
    s << "]";
    s << std::endl;
}

void GribInfo::toJSON(eckit::JSON& json) const {
    json.precision(15);
    json.startObject();
    json << "binaryScaleFactor" << binaryScaleFactor_;
    json << "decimalScaleFactor" << decimalScaleFactor_;
    json << "bitsPerValue" << bitsPerValue_;
    json << "referenceValue" << referenceValue_;
    json << "offsetBeforeData" << offsetBeforeData_;
    json << "numberOfDataPoints" << numberOfDataPoints_;
    json << "numberOfValues" << numberOfValues_;
    json << "offsetBeforeBitmap" << offsetBeforeBitmap_;
    json << "sphericalHarmonics" << sphericalHarmonics_;
    json << "binaryMultiplier" << binaryMultiplier_;
    json << "decimalMultiplier" << decimalMultiplier_;
    json.endObject();
}

void GribInfo::fromJSONFile(eckit::PathName jsonFileName) {
    std::cout << "GribInfo::fromJSONFile " << jsonFileName << std::endl;
    eckit::Value v = eckit::JSONParser::decodeFile(jsonFileName);
    binaryScaleFactor_ = v["binaryScaleFactor"];
    decimalScaleFactor_ = v["decimalScaleFactor"];
    bitsPerValue_ = v["bitsPerValue"];
    referenceValue_ = v["referenceValue"];
    offsetBeforeData_ = v["offsetBeforeData"];
    numberOfDataPoints_ = v["numberOfDataPoints"];
    numberOfValues_ = v["numberOfValues"];
    offsetBeforeBitmap_ = v["offsetBeforeBitmap"];
    sphericalHarmonics_ = v["sphericalHarmonics"];
    binaryMultiplier_ = v["binaryMultiplier"];
    decimalMultiplier_ = v["decimalMultiplier"];
}


double GribInfo::extractAtIndex(const GribHandleData& f, size_t index) const {

    if (bitsPerValue_ == 0)
        return referenceValue_; // XXX: single valued?

    ASSERT(!sphericalHarmonics_);

    if (offsetBeforeBitmap_) {
        ASSERT(index < numberOfDataPoints_);

        // Jump to start of bitmap.
        Offset offset(offsetBeforeBitmap_); 
        ASSERT(f.seek(offset) == offset);

        // We will read in 8-byte chunks.
        uint64_t n;
        const size_t n_bytes = sizeof(n);

        // skip to the byte containing the bit we want, counting set bits as we go.
        size_t count = 0;
        size_t skip = index / (8*n_bytes);
        for (size_t i = 0; i < skip; ++i) {
            ASSERT(f.read(&n, n_bytes) == n_bytes);
            count += count_bits(n);
        }
        ASSERT(f.read(&n, n_bytes) == n_bytes);

        // XXX We need to reverse the byte order of n (but not the bits in each byte).
        n = reverse_bytes(n);

        // check if the bit is set, if not then the value is marked missing
        // bit we want is, from the left, index%(n_bytes*8)
        n = (n >> (n_bytes*8 -index%(n_bytes*8) -1));
        count += count_bits(n);

        if (!(n & 1)) {
            return MISSING;
        }

        // update index to be the index of the value in the (not-missing) data section
        index = count -1;
    }

    ASSERT(index < numberOfValues_);

    // seek to start of first byte containing value
    Offset offset = off_t(offsetBeforeData_)  + off_t(index * bitsPerValue_ / 8);
    ASSERT(f.seek(offset) == offset);

    // read `len` whole bytes into buffer
    long len = (bitsPerValue_ + 7) / 8;
    unsigned char buf[8];
    ASSERT(f.read(buf, len) == len);

    // interpret as double
    long bitp = (index * bitsPerValue_) % 8; // bit position in first byte
    unsigned long p = grib_decode_unsigned_long(buf, &bitp, bitsPerValue_);
    double v = (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_);

    return v;
}

} // namespace gribjump
} // namespace metkit
