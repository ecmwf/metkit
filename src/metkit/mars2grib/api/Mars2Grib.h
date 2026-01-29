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
#include "metkit/mars2grib/api/Options.h"

namespace metkit::mars2grib {

class Mars2Grib {
public:

    Mars2Grib();
    Mars2Grib(const Options& opts);
    Mars2Grib(const eckit::LocalConfiguration& opts);

    Mars2Grib(const Mars2Grib&)           = delete;
    Mars2Grib(Mars2Grib&&)                = delete;
    Mars2Grib operator=(const Mars2Grib&) = delete;
    Mars2Grib operator=(Mars2Grib&&)      = delete;

    ~Mars2Grib() = default;

    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const std::vector<double>& values);
    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc,
                                                       const std::vector<float>& values);

    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc, const double* values,
                                                       size_t length);
    std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc, const float* values,
                                                       size_t length);

private:

    const Options opts_;
};

}  // namespace metkit::mars2grib
