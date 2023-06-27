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
#include <bitset>
#include <numeric>
#include <queue>
#include "eckit/io/DataHandle.h"
#include "eckit/utils/MD5.h"
using namespace eckit;
using namespace metkit::grib;


extern "C" {
    unsigned long grib_decode_unsigned_long(const unsigned char* p, long* offset, int bits);
    double grib_power(long s, long n);
}

namespace metkit {
namespace gribjump {

static GribAccessor<long>          editionNumber("editionNumber");
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
static GribAccessor<unsigned long> totalLength("totalLength");
static GribAccessor<unsigned long> offsetBSection6("offsetBSection6");
static GribAccessor<std::string> md5GridSection("md5GridSection");

static Mutex mutex;

#define MISSING 9999

static int bits[65536] = {
#include "metkit/pointdb/bits.h"
};

// clang, gcc 10+
#if defined(__has_builtin)
    #if __has_builtin(__builtin_popcountll)
        #define POPCOUNT_AVAILABLE 1
    #else
        #define POPCOUNT_AVAILABLE 0
    #endif
// gcc 3.4+
#elif defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
    #define POPCOUNT_AVAILABLE 1
#else
    #define POPCOUNT_AVAILABLE 0
#endif

static inline int count_bits(unsigned long long n) {
    if (POPCOUNT_AVAILABLE) {
        return __builtin_popcountll(n);
    }
    // TODO: see also _mm_popcnt_u64 in <immintrin.h>, but not suitable for ARM
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
    version_(0),
    editionNumber_(0),
    referenceValue_(0),
    binaryScaleFactor_(0),
    decimalScaleFactor_(0),
    bitsPerValue_(0),
    offsetBeforeData_(0),
    bitmapPresent_(0),
    offsetBeforeBitmap_(0),
    numberOfValues_(0),
    numberOfDataPoints_(0),
    totalLength_(0),
    msgStartOffset_(0),
    sphericalHarmonics_(0),
    md5GridSection_(),
    binaryMultiplier_(0),
    decimalMultiplier_(0)
    {
}

void GribInfo::update(const GribHandle& h) {
    editionNumber_      = editionNumber(h);
    ASSERT(editionNumber_ == 1 || editionNumber_ == 2);
    binaryScaleFactor_  = binaryScaleFactor(h);
    decimalScaleFactor_ = decimalScaleFactor(h);
    bitsPerValue_       = bitsPerValue(h);
    referenceValue_     = referenceValue(h);
    offsetBeforeData_   = offsetBeforeData(h);
    numberOfDataPoints_ = numberOfDataPoints(h);
    numberOfValues_     = numberOfValues(h);
    sphericalHarmonics_ = sphericalHarmonics(h); // todo: make quiet
    totalLength_        = totalLength(h);
    md5GridSection_     = md5GridSection(h);
    bitmapPresent_ = bitmapPresent(h);
    if (bitmapPresent_)
        offsetBeforeBitmap_ = editionNumber_ == 1? offsetBeforeBitmap(h) : offsetBSection6(h);
    else
        offsetBeforeBitmap_ = 0;

    binaryMultiplier_ = grib_power(binaryScaleFactor_, 2);
    decimalMultiplier_ = grib_power(-decimalScaleFactor_, 10);
}

void GribInfo::print(std::ostream& s) const {
    s << "GribInfo[";
    s << "version=" << +version_ << ",";
    s << "editionNumber=" << editionNumber_ << ",";
    s << "binaryScaleFactor=" << binaryScaleFactor_ << ",";
    s << "decimalScaleFactor=" << decimalScaleFactor_ << ",";
    s << "bitsPerValue=" << bitsPerValue_ << ",";
    s << "referenceValue=" << referenceValue_ << ",";
    s << "offsetBeforeData=" << offsetBeforeData_ << ",";
    s << "numberOfDataPoints=" << numberOfDataPoints_ << ",";
    s << "numberOfValues=" << numberOfValues_ << ",";
    s << "offsetBeforeBitmap=" << offsetBeforeBitmap_ << ",";
    s << "sphericalHarmonics=" << sphericalHarmonics_ << ",";
    s << "binaryMultiplier=" << binaryMultiplier_ << ",";
    s << "decimalMultiplier=" << decimalMultiplier_ << ",";
    s << "totalLength=" << totalLength_ << ",";
    s << "msgStartOffset=" << msgStartOffset_ << ",";
    s << "md5GridSection=" << md5GridSection_ << "]";
    s << std::endl;
}

void GribInfo::toBinary(eckit::PathName pathname, bool append){
    std::unique_ptr<DataHandle> dh(pathname.fileHandle());
    version_ = currentVersion_;

    append ? dh->openForAppend(0) : dh->openForWrite(0);

    dh->write(&version_, sizeof(version_));
    dh->write(&editionNumber_, sizeof(editionNumber_));
    dh->write(&binaryScaleFactor_, sizeof(binaryScaleFactor_));
    dh->write(&decimalScaleFactor_, sizeof(decimalScaleFactor_));
    dh->write(&bitsPerValue_, sizeof(bitsPerValue_));
    dh->write(&referenceValue_, sizeof(referenceValue_));
    dh->write(&offsetBeforeData_, sizeof(offsetBeforeData_));
    dh->write(&numberOfDataPoints_, sizeof(numberOfDataPoints_));
    dh->write(&numberOfValues_, sizeof(numberOfValues_));
    dh->write(&offsetBeforeBitmap_, sizeof(offsetBeforeBitmap_));
    dh->write(&sphericalHarmonics_, sizeof(sphericalHarmonics_));
    dh->write(&binaryMultiplier_, sizeof(binaryMultiplier_));
    dh->write(&decimalMultiplier_, sizeof(decimalMultiplier_));
    dh->write(&totalLength_, sizeof(totalLength_));
    dh->write(&msgStartOffset_, sizeof(msgStartOffset_));
    dh->write(md5GridSection_.data(), md5GridSection_.size());
    dh->close();
}
void GribInfo::fromBinary(eckit::PathName pathname, uint16_t msg_id){
    std::unique_ptr<DataHandle> dh(pathname.fileHandle());

    dh->openForRead();
    dh->seek(msg_id*metadataSize);
    dh->read(&version_, sizeof(version_));
    dh->read(&editionNumber_, sizeof(editionNumber_));
    dh->read(&binaryScaleFactor_, sizeof(binaryScaleFactor_));
    dh->read(&decimalScaleFactor_, sizeof(decimalScaleFactor_));
    dh->read(&bitsPerValue_, sizeof(bitsPerValue_));
    dh->read(&referenceValue_, sizeof(referenceValue_));
    dh->read(&offsetBeforeData_, sizeof(offsetBeforeData_));
    dh->read(&numberOfDataPoints_, sizeof(numberOfDataPoints_));
    dh->read(&numberOfValues_, sizeof(numberOfValues_));
    dh->read(&offsetBeforeBitmap_, sizeof(offsetBeforeBitmap_));
    dh->read(&sphericalHarmonics_, sizeof(sphericalHarmonics_));
    dh->read(&binaryMultiplier_, sizeof(binaryMultiplier_));
    dh->read(&decimalMultiplier_, sizeof(decimalMultiplier_));
    dh->read(&totalLength_, sizeof(totalLength_));
    dh->read(&msgStartOffset_, sizeof(msgStartOffset_));
    dh->read(md5GridSection_.data(), md5GridSection_.size());
    dh->close();

    ASSERT (version_ == currentVersion_);
}

void accumulate_bits(size_t nread, uint64_t &n, size_t &count, std::vector<size_t> &newIndex) {
    // count the next nread bits in n, storing the number of 1s in count
    // and push the index of each 1 to newIndex
    constexpr uint64_t msb64 = 0x8000000000000000; // 0b100....000

    if (nread == 0) return;
    ASSERT(nread <= 64);

    // first bit
    count += (n & msb64) ? 1 : 0;
    newIndex.push_back((n & msb64) ? count : -1);

    // rest of the bits
    for (size_t i = 0; i < nread-1; ++i) {
        n <<= 1;
        count += (n & msb64) ? 1 : 0;
        newIndex.push_back((n & msb64) ? count : -1);
    }
}

void accumulateEdges(uint64_t &n, size_t &count, std::vector<size_t> &newIndex, std::queue<size_t> &edges, bool &rangeToggle, size_t &bp) {
    // count the set bits in n
    // and push the new index of each set bit to newIndex, if rangeToggle = true. At each edge, toggle rangeToggle.

    ASSERT(!edges.empty());
    ASSERT(bp%64 == 0); // bp must be a multiple of 64 (i.e. we must be at the start of a new uint64_t)

    constexpr uint64_t msb64 = 0x8000000000000000; // 0b100....000
    size_t endbit = bp + 64;
    while (bp < endbit) {
        if (bp == edges.front()) {
            rangeToggle = !rangeToggle;
            edges.pop();
            if (edges.empty()) break;
        }
        bool set = n & msb64;
        count += set ? 1 : 0;
        if (rangeToggle) newIndex.push_back(set ? count : 0);
        n <<= 1;
        ++bp;
    }
}

double GribInfo::extractAtIndex(const GribHandleData& f, size_t index) const {

    if (bitsPerValue_ == 0)
        return referenceValue_;

    ASSERT(!sphericalHarmonics_);

    if (offsetBeforeBitmap_) {
        ASSERT(index < numberOfDataPoints_);

        // Jump to start of bitmap.
        Offset offset = off_t(msgStartOffset_) + off_t(offsetBeforeBitmap_);
        ASSERT(f.seek(offset) == offset);

        // We will read in 8-byte chunks.
        uint64_t n;
        const size_t nBytes = sizeof(n);

        // skip to the byte containing the bit we want, counting set bits as we go.
        size_t count = 0;
        size_t skip = index / (8*nBytes);
        for (size_t i = 0; i < skip; ++i) {
            ASSERT(f.read(&n, nBytes) == nBytes);
            count += count_bits(n);
        }
        ASSERT(f.read(&n, nBytes) == nBytes);

        // XXX We need to reverse the byte order of n (but not the bits in each byte).
        n = reverse_bytes(n);

        // check if the bit is set, if not then the value is marked missing
        // bit we want is, from the left, index%(nBytes*8)
        n = (n >> (nBytes*8 -index%(nBytes*8) -1));
        count += count_bits(n);

        if (!(n & 1)) {
            return MISSING;
        }

        // update index to be the index of the value in the (not-missing) data section
        index = count -1;
    }
    ASSERT(index < numberOfValues_);

    return readDataValue(f, index);
}

std::vector<double> GribInfo::extractAtIndexRangeOfRanges(const GribHandleData& f, std::vector<std::tuple<size_t, size_t>> ranges) const {
    
    ASSERT(!sphericalHarmonics_);

    // sort ranges by start index
    // TODO: unsort the ranges later, so that the output is in the same order as the input
    std::sort(ranges.begin(), ranges.end(), [](const std::tuple<size_t, size_t>& a, const std::tuple<size_t, size_t>& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    size_t sizeAllRanges = 0;

    // sanity check ranges, and count total number of values
    for (size_t i = 0; i < ranges.size(); ++i) {
        size_t istart = std::get<0>(ranges[i]);
        size_t iend = std::get<1>(ranges[i]);
        ASSERT(istart < iend);
        // assert no overlap with other ranges
        for (size_t j = i+1; j < ranges.size(); ++j) {
            size_t jstart = std::get<0>(ranges[j]);
            ASSERT(iend <= jstart);
        }
        sizeAllRanges += iend - istart;
        ASSERT(iend <= numberOfDataPoints_);
    }

    std::vector<double> values;

    if (bitsPerValue_ == 0) {
        values = std::vector<double>(sizeAllRanges, referenceValue_);
        return values;
    }

    values.reserve(sizeAllRanges);

    // set bufferSize equal to minimum bytes that will hold the largest range.
    // TODO: There should be an upper limit. If range is too large, read it in chunks.
    size_t bufferSize = 0;
    for (auto r : ranges) {
        size_t i0 = std::get<0>(r);
        size_t i1 = std::get<1>(r);
        bufferSize = std::max(bufferSize, 1 + ((i1 - i0)*bitsPerValue_ + 7 )/8);
    }

    // find also the largest seperation between ranges, so we can allocate a buffer for the bitmap
    size_t maxSeparation = std::get<0>(ranges[0]); // need to parse up to start of first range at least.
    for (size_t i = 0; i < ranges.size()-1; ++i) {
        size_t i1 = std::get<1>(ranges[i]);
        size_t j0 = std::get<0>(ranges[i+1]);
        maxSeparation = std::max(maxSeparation, (j0 - i1));
    }

    if (!offsetBeforeBitmap_) {
        // no bitmap, just read the values
        std::unique_ptr<unsigned char[]> buf(new unsigned char[bufferSize]);

        for (auto r : ranges) {
            size_t istart = std::get<0>(r);
            size_t iend = std::get<1>(r);

            Offset offset = off_t(msgStartOffset_) + off_t(offsetBeforeData_)  + off_t(istart * bitsPerValue_ / 8);
            ASSERT(f.seek(offset) == offset);

            long len = 1 + ((iend - istart)*(bitsPerValue_) + 7) / 8;
            ASSERT (len <= bufferSize);

            ASSERT(f.read(buf.get(), len) == len);
            long bitp = ((istart) * bitsPerValue_) % 8;

            for (size_t i = istart; i < iend; ++i) {
                unsigned long p = grib_decode_unsigned_long(buf.get(), &bitp, bitsPerValue_); // TODO: does eccodes have an array version of grib_decode_unsigned_long?
                double v = (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_);
                values.push_back(v);
            }
        }
        return values;
    }
    // else we have a bitmap
    std::vector<size_t> newIndex;
    newIndex.reserve(sizeAllRanges); // new index after skipping missing values. Use 0 to denote missing values.

    // Form a queue of `range edges`, denoting when we enter and leave a ranged region.
    // e.g. range(1, 5), range(7, 10) range(10, 20), range(20, 30) -> edges(1, 5, 7, 30)
    // This will make it easier to toggle counting mode on and off, as we enter and leave ranges.
    std::queue<size_t> edges;
    edges.push(std::get<0>(ranges[0]));
    size_t prevEnd = std::get<1>(ranges[0]);
    for (size_t i = 1; i < ranges.size(); ++i) {
        size_t istart = std::get<0>(ranges[i]);
        if (istart != prevEnd) {
            edges.push(prevEnd);
            edges.push(istart);
        }
        size_t iend = std::get<1>(ranges[i]);
        prevEnd = iend;
    }
    edges.push(prevEnd);

    // Jump to start of bitmap.
    Offset offset = off_t(msgStartOffset_) + off_t(offsetBeforeBitmap_);
    ASSERT(f.seek(offset) == offset);

    uint64_t n;
    constexpr size_t nBytes = sizeof(n);
    constexpr size_t nBits = nBytes * 8;
    size_t bp = 0;
    size_t count = 0;
    bool rangeToggle = false; // whether we are within a range we care about or not.

    std::vector<uint64_t> bufskip(maxSeparation/nBits + 1);

    while (!edges.empty()) {
        if (!rangeToggle){
            size_t nWordsToSkip = (edges.front() - bp)/nBits;
            size_t nBytesToSkip = nWordsToSkip * nBytes;
            ASSERT(f.read(bufskip.data(), nBytesToSkip) == nBytesToSkip);
            for (size_t i = 0; i < nWordsToSkip; ++i) {
                count += count_bits(bufskip[i]);
            }
            bp += nWordsToSkip * nBits;
        }
        ASSERT(f.read(&n, nBytes) == nBytes);
        n = reverse_bytes(n);
        accumulateEdges(n, count, newIndex, edges, rangeToggle, bp);
    }

    // read the values
    std::unique_ptr<unsigned char[]> buf(new unsigned char[bufferSize]);
    count = 0;
    for (size_t ri = 0; ri < ranges.size(); ++ri) {
        auto r = ranges[ri];
        size_t i0 = std::get<0>(r);
        size_t i1 = std::get<1>(r);
        size_t index0;
        size_t index1;
        // find index of first and last non-missing values in this range
        for (size_t i = count; i < count + (i1 - i0); ++i) {
            index0 = newIndex[i];
            if (index0!=0) break;
        }
        if (index0 == 0){
            // all values in this range are missing
            for (size_t i = 0; i < i1 - i0; ++i) {
                values.push_back(MISSING);
            }
            count += i1 - i0;
            continue;
        }
        for (size_t i = count + (i1 - i0) - 1; i >= count; --i) {
            index1 = newIndex[i];
            if (index1!=0) break;
        }

        long bitp = -1;
        index1 += 1; // we read up to but not including index1
        for (size_t i = count; i < count + (i1 - i0); ++i) {
            size_t index = newIndex[i];
            if (index == 0){
                values.push_back(MISSING);
                continue;
            } else if (bitp == -1){
                // At first non-missing value for this range, so read into buffer and set bitp
                Offset offset = off_t(msgStartOffset_) + off_t(offsetBeforeData_)  + off_t((index0-1) * bitsPerValue_ / 8);
                ASSERT(f.seek(offset) == offset);
                long len = 1 + ((index1 - index0)*(bitsPerValue_) + 7) / 8;
                ASSERT (len <= bufferSize);
                ASSERT(f.read(buf.get(), len) == len);
                bitp = ((index0-1) * bitsPerValue_) % 8;
            }
            unsigned long p = grib_decode_unsigned_long(buf.get(), &bitp, bitsPerValue_); // TODO: does eccodes have an array version of grib_decode_unsigned_long?
            double v = (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_);
            values.push_back(v);
        }
        count += i1 - i0;
    }
    return values;
}

double GribInfo::readDataValue(const GribHandleData& f, size_t index) const {
    // Read the data value at index.
    // We will do no error checking here, so make sure index is valid
    
    // seek to start of byte containing value
    Offset offset = off_t(msgStartOffset_) + off_t(offsetBeforeData_)  + off_t(index * bitsPerValue_ / 8);
    ASSERT(f.seek(offset) == offset);

    // read `len` whole bytes into buffer
    long len = 1 + (bitsPerValue_ + 7) / 8;
    unsigned char buf[8];
    ASSERT(f.read(buf, len) == len);

    // interpret as double
    long bitp = (index * bitsPerValue_) % 8; // bit position in first byte
    unsigned long p = grib_decode_unsigned_long(buf, &bitp, bitsPerValue_);
    return (double) (((p * binaryMultiplier_) + referenceValue_) * decimalMultiplier_);
}

} // namespace gribjump
} // namespace metkit
