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
#include <numeric>

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
    #if __has_builtin(__builtin_popcount)
        return __builtin_popcountll(n);
    #endif

    // fallback: lookup table
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


std::vector<double> GribInfo::extractAtIndexRangeNaive(const GribHandleData& f, size_t i_start, size_t i_end) const {
    // simply a for loop around extractAtIndex, for testing purposes
    std::vector<double> values;
    values.reserve(i_end - i_start);
    for (size_t i = i_start; i < i_end; ++i) {
        values.push_back(extractAtIndex(f, i));
    }
    return values;
}

void accumulate_bits(size_t nread, uint64_t &n, size_t &count, std::vector<size_t> &n_index) {
    // count the next nread bits in n, storing the number of 1s in count
    // and push the index of each 1 to n_index
    constexpr uint64_t msb_64 = 0x8000000000000000; // 0b100....000

    if (nread == 0) return;
    ASSERT(nread <= 64);
    // first bit
    count += (n & msb_64) ? 1 : 0;
    n_index.push_back((n & msb_64) ? count : -1);

    // rest of the bits
    for (size_t i = 0; i < nread-1; ++i) {
        n <<= 1;
        count += (n & msb_64) ? 1 : 0;
        n_index.push_back((n & msb_64) ? count : -1);
    }
}

size_t GribInfo::readBitmapRange(const GribHandleData& f, unsigned long offset0, size_t i_start, size_t i_end, std::vector<size_t> &n_index) const{
    // will return count of set bits in range [i_start, i_end)
    // as well as push the index of each set bit to n_index
    // We will read in 8-byte chunks.
    uint64_t n;
    constexpr size_t n_bytes = sizeof(n);
    constexpr size_t n_bits = n_bytes * 8;
    constexpr uint64_t msb_64 = 0x8000000000000000; // 0b100....000
    size_t count = 0;
    size_t remaining_bits = i_end - i_start;


    // Jump to start of bitmap.
    Offset offset(offset0);
    ASSERT(f.seek(offset) == offset);

    // skip to byte containing i_start-th bit, counting set bits as we go.
    size_t skip = i_start / (n_bits);
    for (size_t i = 0; i < skip; ++i) {
        ASSERT(f.read(&n, n_bytes) == n_bytes);
        count += count_bits(n);
    }

    // -- Handle first word in range. --
    ASSERT(f.read(&n, n_bytes) == n_bytes);
    n = reverse_bytes(n);

    // move to first relevant bit.
    count += count_bits(n);
    n <<= (i_start%n_bits);
    count -= count_bits(n) + 1;

    size_t nread = n_bits - (i_start%n_bits); // number of bits left to read in this word

    // Case where i_start and i_end are both in the first word. i.e. we've already read all the bits we need.
    if (nread > remaining_bits) {
        Log::debug() << i_start << "," << i_end << "extractAtIndexRange: i_end and i_start in same word" << std::endl;
        nread = remaining_bits;
    }

    Log::debug() << "nread" << nread << std::endl;

    accumulate_bits(nread, n, count, n_index);
    remaining_bits -= nread;

    // -- Handle remaining whole words --
    size_t n_whole_words = remaining_bits/n_bits;
    for (size_t i = 0; i < n_whole_words; ++i) {
        Log::debug() << i_start << "," << i_end << "whole word" << std::endl;

        ASSERT(f.read(&n, n_bytes) == n_bytes);
        n = reverse_bytes(n);
        accumulate_bits(n_bits, n, count, n_index);
    }
    remaining_bits = remaining_bits % n_bits;
    
    // -- Handle final word, if incomplete word --
    if (remaining_bits) {
        Log::debug() << i_start << "," << i_end << "extractAtIndexRange: i_end in additional incomplete word" << std::endl;

        ASSERT(f.read(&n, n_bytes) == n_bytes);
        n = reverse_bytes(n);
        accumulate_bits(remaining_bits, n, count, n_index);
        remaining_bits = 0;
    }

    return count;
}

std::vector<double> GribInfo::extractAtIndexRange(const GribHandleData& f, size_t i_start, size_t i_end) const {

    ASSERT(i_start < i_end);
    ASSERT(i_end <= numberOfDataPoints_);
    ASSERT(!sphericalHarmonics_);

    Log::debug() << "GribInfo::extractAtIndexRange " << i_start << ", " << i_end << std::endl;

    std::vector<double> values;
    std::vector<size_t> n_index;
    size_t remaining_bits = i_end - i_start;

    if (bitsPerValue_ == 0) {
        values = std::vector<double>(remaining_bits, referenceValue_);
        return values;
    }

    values.reserve(remaining_bits);

    if (!offsetBeforeBitmap_){
        // no bitmap, just read the values
        for (size_t i = i_start; i < i_end; ++i) {
            double v = readDataValue(f, i);
            values.push_back(v);
        }
        return values;
    }
    // else, we have a bitmap, so we need to read it first and find the new indices
    n_index.reserve(remaining_bits); 
    readBitmapRange(f, offsetBeforeBitmap_, i_start, i_end, n_index);

    for (auto index : n_index) {
        double v = index == -1 ? MISSING : readDataValue(f, index);
        values.push_back(v);
    }
    return values;
}

double GribInfo::extractAtIndex(const GribHandleData& f, size_t index) const {

    if (bitsPerValue_ == 0)
        return referenceValue_;

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

std::vector<double> GribInfo::extractAtIndexRangeOfRangesNaive(const GribHandleData& f, std::vector<std::tuple<size_t, size_t>> ranges) const {
    // just a for loop over the ranges, calling extractAtIndexRange for each
    std::vector<double> values;
    for (auto r : ranges) {
        auto v = extractAtIndexRange(f, std::get<0>(r), std::get<1>(r));
        values.insert(values.end(), v.begin(), v.end());
    }
    return values;
}

std::vector<double> GribInfo::extractAtIndexRangeOfRanges(const GribHandleData& f, std::vector<std::tuple<size_t, size_t>> ranges) const {
    
    // sort ranges by start index
    std::sort(ranges.begin(), ranges.end(), [](const std::tuple<size_t, size_t>& a, const std::tuple<size_t, size_t>& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // print out ranges
    for (auto r : ranges) {
        std::cout << std::get<0>(r) << "," << std::get<1>(r) << std::endl;
    }

    size_t remaining_bits = 0;

    // sanity check ranges, and count total number of values
    for (size_t i = 0; i < ranges.size(); ++i) {
        size_t i_start = std::get<0>(ranges[i]);
        size_t i_end = std::get<1>(ranges[i]);
        ASSERT(i_start < i_end);
        // assert no overlap with other ranges
        for (size_t j = i+1; j < ranges.size(); ++j) {
            size_t j_start = std::get<0>(ranges[j]);
            ASSERT(i_end <= j_start);
        }
        remaining_bits += i_end - i_start;
    }

    size_t i_start = std::get<0>(ranges.front());
    size_t i_end = std::get<1>(ranges.back());    

    ASSERT(i_start < i_end);
    ASSERT(i_end < numberOfValues_);
    ASSERT(!sphericalHarmonics_);

    std::vector<double> values;
    std::vector<size_t> n_index;

    if (bitsPerValue_ == 0) {
        values = std::vector<double>(remaining_bits, referenceValue_);
        return values;
    }

    values.reserve(remaining_bits);

    if (!offsetBeforeBitmap_) {
        // no bitmap, just read the values
        for (auto r : ranges) {
            size_t i_start = std::get<0>(r);
            size_t i_end = std::get<1>(r);
            for (size_t i = i_start; i < i_end; ++i) {
                double v = readDataValue(f, i);
                values.push_back(v);
            }
        }
        return values;
    }
    // else we have a bitmap, so we need to read the bitmap and skip missing values
    n_index.reserve(remaining_bits); // new index after skipping missing values

    NOTIMP; // not implemented yet
}

double GribInfo::readDataValue(const GribHandleData& f, size_t index) const {
    // Read the data value at index.
    // We will do no error checking here, so make sure index is valid
    
    // seek to start of byte containing value
    Offset offset = off_t(offsetBeforeData_)  + off_t(index * bitsPerValue_ / 8);
    ASSERT(f.seek(offset) == offset);

    // read `len` whole bytes into buffer
    long len = (bitsPerValue_ + 7) / 8;
    unsigned char buf[8];
    ASSERT(f.read(buf, len) == len);

    // interpret as double
    long bitp = (index * bitsPerValue_) % 8; // bit position in first byte
    unsigned long p = grib_decode_unsigned_long(buf, &bitp, bitsPerValue_);
    return (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_);
}

} // namespace gribjump
} // namespace metkit
