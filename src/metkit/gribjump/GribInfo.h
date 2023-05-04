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
#include "eckit/log/JSON.h"


namespace eckit { class PathName; }
namespace metkit {
namespace grib {
class GribHandle;
}



namespace gribjump {


class GribHandleData;

class GribInfo {
public:

    GribInfo();

    bool ready() const { return numberOfValues_ > 0; }
    void update(const grib::GribHandle& h);
    double extractAtIndex(const GribHandleData&, size_t index) const;
    
    void print(std::ostream&) const;
    void toJSON(eckit::JSON&) const;
    void fromJSONFile(eckit::PathName);

private:
    static constexpr unsigned int currentVersion_ = 1; // later ASSERT version == currentVersion
    unsigned int version_;
    double        referenceValue_;
    long          binaryScaleFactor_;
    long          decimalScaleFactor_;
    unsigned long bitsPerValue_;
    unsigned long offsetBeforeData_;
    unsigned long offsetBeforeBitmap_;
    unsigned long numberOfValues_;
    unsigned long numberOfDataPoints_;
    long          sphericalHarmonics_;

    double binaryMultiplier_; // = 2^binaryScaleFactor_
    double decimalMultiplier_; // = 10^-decimalScaleFactor_

    friend std::ostream& operator<<(std::ostream& s, const GribInfo& f) {
        f.print(s);
        return s;
    }

};

} // namespace gribjump
} // namespace metkit


#endif
