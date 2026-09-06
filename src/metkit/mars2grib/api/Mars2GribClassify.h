/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <initializer_list>
#include <string>
#include <utility>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/value/Value.h"

#include "metkit/mars2grib/api/Options.h"

namespace metkit::mars2grib {

struct ProductTimeSpecResult {
    std::string json;
    std::string classification;
};

class Mars2GribClassify {
public:
    using OptionEntry = std::pair<std::string, eckit::Value>;
    using OptionList  = std::initializer_list<OptionEntry>;

    Mars2GribClassify();
    explicit Mars2GribClassify(const Options& opts);
    explicit Mars2GribClassify(const eckit::LocalConfiguration& opts);
    explicit Mars2GribClassify(OptionList opts);

    Mars2GribClassify(const Mars2GribClassify&)            = delete;
    Mars2GribClassify& operator=(const Mars2GribClassify&) = delete;
    Mars2GribClassify(Mars2GribClassify&&)                 = delete;
    Mars2GribClassify& operator=(Mars2GribClassify&&)      = delete;

    ~Mars2GribClassify() = default;

    std::string computeActiveConcepts(const eckit::LocalConfiguration& mars,
                                      const eckit::LocalConfiguration& misc);
    std::string computeActiveConcepts(const eckit::LocalConfiguration& mars);

    ProductTimeSpecResult computeProductTimeSpec(const eckit::LocalConfiguration& mars,
                                                 const eckit::LocalConfiguration& misc);
    ProductTimeSpecResult computeProductTimeSpec(const eckit::LocalConfiguration& mars);

private:
    const eckit::Value language_;
    const Options opts_;
};

}  // namespace metkit::mars2grib
