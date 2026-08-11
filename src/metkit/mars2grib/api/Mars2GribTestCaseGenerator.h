/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <initializer_list>
#include <string>
#include <utility>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"

#include "metkit/mars2grib/api/Options.h"

namespace metkit::mars2grib {

class Mars2GribTestCaseGenerator {
public:

    using OptionEntry = std::pair<std::string, eckit::Value>;
    using OptionList  = std::initializer_list<OptionEntry>;

    Mars2GribTestCaseGenerator();
    explicit Mars2GribTestCaseGenerator(const Options& opts);
    explicit Mars2GribTestCaseGenerator(const eckit::LocalConfiguration& opts);
    explicit Mars2GribTestCaseGenerator(OptionList opts);

    Mars2GribTestCaseGenerator(const Mars2GribTestCaseGenerator&)            = delete;
    Mars2GribTestCaseGenerator& operator=(const Mars2GribTestCaseGenerator&) = delete;
    Mars2GribTestCaseGenerator(Mars2GribTestCaseGenerator&&)                 = delete;
    Mars2GribTestCaseGenerator& operator=(Mars2GribTestCaseGenerator&&)      = delete;

    ~Mars2GribTestCaseGenerator() = default;

    std::string generate(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc);
    std::string generate(const eckit::LocalConfiguration& mars);

private:

    const eckit::Value language_;
    const Options opts_;
};

}  // namespace metkit::mars2grib
