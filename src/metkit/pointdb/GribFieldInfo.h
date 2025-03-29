/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef FieldInfoData_H
#define FieldInfoData_H

#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"
#include "eckit/types/FixedString.h"


namespace eckit {
class PathName;
}
namespace metkit {
namespace grib {
class GribHandle;
}


namespace pointdb {


class GribDataSource;


class GribFieldInfo {
public:

    GribFieldInfo();

    std::string geographyHash() const { return geographyHash_; }

    bool ready() const { return numberOfValues_ > 0; }

    void update(const grib::GribHandle& h);

    double value(const GribDataSource&, size_t index) const;

    bool useInterpolation() const { return sphericalHarmonics_ != 0; }
    double interpolate(GribDataSource&, double& lat, double& lon) const;

private:

    double referenceValue_;
    long binaryScaleFactor_;
    long decimalScaleFactor_;
    unsigned long bitsPerValue_;
    unsigned long offsetBeforeData_;
    unsigned long offsetBeforeBitmap_;
    unsigned long numberOfValues_;
    unsigned long numberOfDataPoints_;
    long sphericalHarmonics_;

    eckit::FixedString<32> geographyHash_;

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const GribFieldInfo& f) {
        f.print(s);
        return s;
    }
};

}  // namespace pointdb
}  // namespace metkit


#endif
