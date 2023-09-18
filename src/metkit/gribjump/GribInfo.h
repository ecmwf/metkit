/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_JumpInfo_H
#define metkit_JumpInfo_H

#include <queue>
#include "eckit/filesystem/PathName.h"
#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"
#include "eckit/types/FixedString.h"


// namespace eckit { class PathName; }
namespace metkit {
namespace grib { class GribHandle; }

namespace gribjump {

class JumpHandle;
void accumulateIndexes(uint64_t &n, size_t &count, std::vector<size_t> &n_index, std::queue<size_t> &edges, bool&, size_t&);

class JumpInfo {
public:

    JumpInfo();
    JumpInfo(const grib::GribHandle& h);

    bool ready() const { return numberOfValues_ > 0; }
    void update(const grib::GribHandle& h);
    double extractValue(const JumpHandle&, size_t index) const;
    std::vector<double> extractRanges(const JumpHandle&, std::vector<std::tuple<size_t, size_t>> ranges) const;
    
    void print(std::ostream&) const;

    void toFile(eckit::PathName, bool);
    void fromFile(eckit::PathName, uint16_t msg_id=0);

    unsigned long getNumberOfDataPoints() const { return numberOfDataPoints_; }
    unsigned long length() const { return totalLength_; }
    void setStartOffset(eckit::Offset offset) { msgStartOffset_ = offset; }

private:
    double readDataValue(const JumpHandle&, size_t) const;

    static constexpr uint8_t currentVersion_ = 2;
    uint8_t version_;
    double        referenceValue_;
    long          binaryScaleFactor_;
    long          decimalScaleFactor_;
    unsigned long editionNumber_;
    unsigned long bitsPerValue_;
    unsigned long offsetBeforeData_;
    unsigned long bitmapPresent_;
    unsigned long offsetBeforeBitmap_;
    unsigned long numberOfValues_;
    unsigned long numberOfDataPoints_;
    unsigned long totalLength_;
    unsigned long msgStartOffset_;
    long          sphericalHarmonics_;
    eckit::FixedString<32> md5GridSection_;
    eckit::FixedString<64> packingType_;

    double binaryMultiplier_; // = 2^binaryScaleFactor_
    double decimalMultiplier_; // = 10^-decimalScaleFactor_

    static constexpr size_t metadataSize = sizeof(version_) + \
                                           sizeof(editionNumber_) + \
                                           sizeof(referenceValue_) + \
                                           sizeof(binaryScaleFactor_) + \
                                           sizeof(decimalScaleFactor_) + \
                                           sizeof(bitsPerValue_) + \
                                           sizeof(offsetBeforeData_) + \
                                           sizeof(offsetBeforeBitmap_) + \
                                           sizeof(numberOfValues_) + \
                                           sizeof(numberOfDataPoints_) + \
                                           sizeof(totalLength_) + \
                                           sizeof(msgStartOffset_) + \
                                           sizeof(sphericalHarmonics_) + \
                                           sizeof(binaryMultiplier_) + \
                                           sizeof(decimalMultiplier_) + \
                                           sizeof(md5GridSection_) + \
                                           sizeof(packingType_);

    friend std::ostream& operator<<(std::ostream& s, const JumpInfo& f) {
        f.print(s);
        return s;
    }

};

} // namespace gribjump
} // namespace metkit


#endif
