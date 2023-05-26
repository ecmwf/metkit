/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


// XXX (based on pointdb/GribFieldInfo)

#ifndef InfoData_H
#define InfoData_H

#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"
#include "eckit/types/FixedString.h"
#include <queue>


namespace eckit { class PathName; }
namespace metkit {
namespace grib {
class GribHandle;
}



namespace gribjump {

class GribHandleData;
void accumulateEdges(uint64_t &n, size_t &count, std::vector<size_t> &n_index, std::queue<size_t> &edges, bool&, size_t&);

class GribInfo {
public:

    GribInfo();

    bool ready() const { return numberOfValues_ > 0; }
    void update(const grib::GribHandle& h);
    double extractAtIndex(const GribHandleData&, size_t index) const;
    std::vector<double> extractAtIndexRangeOfRanges(const GribHandleData&, std::vector<std::tuple<size_t, size_t>> ranges) const;
    
    void print(std::ostream&) const;

    void toBinary(eckit::PathName, bool);
    void fromBinary(eckit::PathName, uint16_t msg_id=0);

    unsigned long get_numberOfDataPoints() const { return numberOfDataPoints_; }
    unsigned long get_totalLength() const { return totalLength_; }
    void set_msgStartOffset(eckit::Offset offset) { msgStartOffset_ = offset; }

private:
    double readDataValue(const GribHandleData&, size_t) const;

    static constexpr uint8_t currentVersion_ = 1; // later ASSERT version == currentVersion
    uint8_t version_;
    double        referenceValue_;
    long          binaryScaleFactor_;
    long          decimalScaleFactor_;
    unsigned long bitsPerValue_;
    unsigned long offsetBeforeData_;
    unsigned long offsetBeforeBitmap_;
    unsigned long numberOfValues_;
    unsigned long numberOfDataPoints_;
    unsigned long totalLength_;
    unsigned long msgStartOffset_;
    long          sphericalHarmonics_;

    double binaryMultiplier_; // = 2^binaryScaleFactor_
    double decimalMultiplier_; // = 10^-decimalScaleFactor_

    static constexpr size_t metadataSize = sizeof(version_) + \
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
                                           sizeof(decimalMultiplier_);

    friend std::ostream& operator<<(std::ostream& s, const GribInfo& f) {
        f.print(s);
        return s;
    }

};

} // namespace gribjump
} // namespace metkit


#endif
