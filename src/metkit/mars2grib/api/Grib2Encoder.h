/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <vector>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/codes/api/CodesAPI.h"

namespace metkit::mars2grib {

class Grib2Encoder {
public:

    Grib2Encoder();
    Grib2Encoder(const eckit::LocalConfiguration& opts);

    Grib2Encoder(const Grib2Encoder&)           = delete;
    Grib2Encoder(Grib2Encoder&&)                = delete;
    Grib2Encoder operator=(const Grib2Encoder&) = delete;
    Grib2Encoder operator=(Grib2Encoder&&)      = delete;

    ~Grib2Encoder() = default;

    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const eckit::LocalConfiguration& geom,
                                                       const std::vector<double>& values);
    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const eckit::LocalConfiguration& geom,
                                                       const std::vector<float>& values);

    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const eckit::LocalConfiguration& geom, const double* values,
                                                       size_t length);
    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const eckit::LocalConfiguration& geom, const float* values,
                                                       size_t length);

    // TODO : variants of encode without geom?
    // TODO : variants of encode without values?

private:

    eckit::LocalConfiguration opts_;
};

}  // namespace metkit::mars2grib
