/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/pointdb/GribFieldInfo.h"
#include "metkit/grib/GribAccessor.h"
#include "metkit/pointdb/GribDataSource.h"
#include <bitset>

using namespace eckit;
using namespace metkit::grib;


extern "C" {
    unsigned long grib_decode_unsigned_long(const unsigned char* p, long* offset, int bits);
    double grib_power(long s, long n);
}

namespace metkit {
namespace pointdb {

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

GribFieldInfo::GribFieldInfo():
    referenceValue_(0),
    binaryScaleFactor_(0),
    decimalScaleFactor_(0),
    bitsPerValue_(0),
    offsetBeforeData_(0),
    offsetBeforeBitmap_(0),
    numberOfValues_(0),
    numberOfDataPoints_(0),
    sphericalHarmonics_(0) {
}

void GribFieldInfo::update(const GribHandle& h) {
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

    if (!sphericalHarmonics_)
        geographyHash_ = h.geographyHash();
}

void GribFieldInfo::print(std::ostream& s) const {
    s << "GribFieldInfo[";

    s << "binaryScaleFactor=" << binaryScaleFactor_;
    s << ",decimalScaleFactor=" << decimalScaleFactor_;
    s << ",bitsPerValue=" << bitsPerValue_;
    s << ",referenceValue=" << referenceValue_;
    s << ",offsetBeforeData=" << offsetBeforeData_;
    s << ",numberOfDataPoints=" << numberOfDataPoints_;
    s << ",numberOfValues=" << numberOfValues_;
    s << ",offsetBeforeBitmap=" << offsetBeforeBitmap_;
    s << ",sphericalHarmonics=" << sphericalHarmonics_;
    s << ",geographyHash=" << geographyHash_;

    s << "]";
}


double GribFieldInfo::interpolate(GribDataSource &f, double& lat, double& lon) const {
    NOTIMP;
}

double GribFieldInfo::value(const GribDataSource &f, size_t index) const {
    unsigned char buf[8];

    if (bitsPerValue_ == 0)
        return referenceValue_;

    ASSERT(!sphericalHarmonics_);

    if (offsetBeforeBitmap_) {
        ASSERT(index < numberOfDataPoints_);

        Log::info() << "offsetBeforeBitmap_ " << offsetBeforeBitmap_
                    << ",index " << index
                    << std::endl;


        Offset offset(offsetBeforeBitmap_);
        ASSERT(f.seek(offset) == offset);
        size_t count = 0;

        uint64_t n;
        ASSERT(sizeof(n) == 8);

        size_t skip = index / 8;

        Log::info() << "Read " << (skip * 8) << " bits" << std::endl;
        for (size_t i = 0; i < skip; ++i) {
            ASSERT(f.read(&n, sizeof(n)) == sizeof(n));
            count += count_bits(n);
        }
        Log::info() << "Count " << count << std::endl;

        size_t pos = index % 8;

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

    cout << "index " << index << ", numberOfValues " << numberOfValues_ << endl;
    ASSERT(index < numberOfValues_);

    {
        Offset offset = off_t(offsetBeforeData_)  + off_t(index * bitsPerValue_ / 8);
        // ASSERT(offset + eckit::Length(4) < length_); // 4 is for 7777
        SYSCALL(f.seek(offset) == offset);


        long len = (bitsPerValue_ + 7) / 8;
        ASSERT(f.read(buf, len) == len);
    }

    long bitp = (index * bitsPerValue_) % 8;
    unsigned long p = grib_decode_unsigned_long(buf, &bitp, bitsPerValue_);

    // TODO: store the precomputed values
    double s = grib_power(binaryScaleFactor_, 2);
    double d = grib_power(-decimalScaleFactor_, 10) ;

    double v = (double) (((p * s) + referenceValue_) * d);

    return v;
}

} // namespace pointdb
} // namespace metkit
