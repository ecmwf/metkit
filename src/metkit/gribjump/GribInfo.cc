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

static uint64_t masks[64] = {
#include "metkit/pointdb/masks.h"
};

static inline int count_bits(unsigned long long n) {
    return bits[n         & 0xffffu]
           +  bits[(n >> 16) & 0xffffu]
           +  bits[(n >> 32) & 0xffffu]
           +  bits[(n >> 48) & 0xffffu];
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
    // set precision to 15 digits
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
    unsigned char buf[8]; // 8 byte buffer

    if (bitsPerValue_ == 0)
        return referenceValue_; // all values are the same?

    ASSERT(!sphericalHarmonics_);

    if (offsetBeforeBitmap_) {
        // TODO: Still need to go through this section...
        // and also find a grib that will use it.
        ASSERT(index < numberOfDataPoints_); // difference between this and numberOfValues_ is ..? 
        // COPILOT: [numberOfValues_ is the number of values in the data section, numberOfDataPoints_ is the number of points in the bitmap]

        Log::info() << "offsetBeforeBitmap_ " << offsetBeforeBitmap_
                    << ",index " << index
                    << std::endl;

        Offset offset(offsetBeforeBitmap_); 

        f.seek(offset); // Jump to the bitmap?
        ASSERT(f.seek(offset) == offset); // This seeks twice? Why?
        size_t count = 0;
        uint64_t n;
        ASSERT(sizeof(n) == 8); // paranoia..., should be 64 bits

        size_t skip = index / (sizeof(n) * 8); // ie number of 64 bit words to skip

        Log::info() << "Read " << skip  << " words" << std::endl;
        for (size_t i = 0; i < skip; ++i) {
            ASSERT(f.read(&n, sizeof(n)) == sizeof(n));
            count += count_bits(n);
        }
        Log::info() << "Count " << count << std::endl;

        size_t pos = index % (sizeof(n) * 8);

        Log::info() << "Read last bits, " << pos << std::endl;
        ASSERT(f.read(&n, sizeof(n)) == sizeof(n));

        Log::info() << "Read last bits, " << pos << ", before mask " << std::bitset<64>(n) << std::endl;

        n &= masks[pos];

        Log::info() << "Read last bits, " << pos << ", after mask " << std::bitset<64>(n) << ", bits="
                    << count_bits(n) <<
                    std::endl;

        count += count_bits(n);
        Log::info() << "Count " << count << std::endl;

        n = (n >> (64 - pos)) & 1; 

        if (!n) {
            return MISSING;
        }

        ASSERT(count);
        index = count - 1;
    }

    Log::info() << "index " << index << ", numberOfValues " << numberOfValues_ << std::endl;
    ASSERT(index < numberOfValues_);

    {
        Offset offset = off_t(offsetBeforeData_)  + off_t(index * bitsPerValue_ / 8); // offset to start of byte containing value
        ASSERT(f.seek(offset) == offset); // seek to byte containing value

        long len = (bitsPerValue_ + 7) / 8; // number of bytes per value, rounded up
        ASSERT(f.read(buf, len) == len); // read [len] whole bytes containing value into buf
    }

    long bitp = (index * bitsPerValue_) % 8; // bit offset within byte to start of value
    unsigned long p = grib_decode_unsigned_long(buf, &bitp, bitsPerValue_); // decode value from buf as unsigned long
    double v = (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_); // translate value into double

    return v;
}

} // namespace gribjump
} // namespace metkit