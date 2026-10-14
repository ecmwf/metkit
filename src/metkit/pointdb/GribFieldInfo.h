// SPDX-FileCopyrightText: 1996-2013 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once


#include "metkit/codes/api/CodesAPI.h"

#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"
#include "eckit/types/FixedString.h"


namespace eckit {
class PathName;
}
namespace metkit::pointdb {


class GribDataSource;


class GribFieldInfo {
public:

    GribFieldInfo();

    std::string geographyHash() const { return geographyHash_; }

    bool ready() const { return numberOfValues_ > 0; }

    void update(const codes::CodesHandle& h);

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

}  // namespace metkit::pointdb
