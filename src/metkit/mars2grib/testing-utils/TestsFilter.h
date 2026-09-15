/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "eckit/filesystem/PathName.h"

namespace metkit::mars2grib::testing_utils {

class TestsFilter {
public:

    explicit TestsFilter(const eckit::LocalConfiguration& options);

    bool filter(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                const eckit::LocalConfiguration& opt) const;

private:

    bool filterPerturbedForecast_{true};
    bool filterModelLevel_{true};
    bool filterFrequencyDirection_{true};
};

void pruneTestsFile(const eckit::PathName& inputPath, const eckit::PathName& outputPath,
                    const eckit::LocalConfiguration& options);

}  // namespace metkit::mars2grib::testing_utils
