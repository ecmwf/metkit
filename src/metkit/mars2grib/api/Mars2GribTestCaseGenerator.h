// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <initializer_list>
#include <string>
#include <utility>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"

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
    testing_utils::RecordingDictionary generateOutput(const eckit::LocalConfiguration& mars,
                                                       const eckit::LocalConfiguration& misc);
    testing_utils::RecordingDictionary generateOutput(const eckit::LocalConfiguration& mars);

private:

    const eckit::Value language_;
    const Options opts_;
};

}  // namespace metkit::mars2grib
