/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/TestsFilter.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>
#include <fstream>
#include <string>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/CoreOperations.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/frontend/resolution/resolveActiveConcepts.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::testing_utils {
namespace {

template <class Cntx_t>
bool option(const eckit::LocalConfiguration& options, const std::string& name, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = options.has(name) ? options.getBool(name) : true;
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
eckit::LocalConfiguration requiredObject(const eckit::LocalConfiguration& root, const std::string& key,
                                          std::size_t lineNumber, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::Exception(
            "Test-case record at line " + std::to_string(lineNumber) + " requires object `" + key + "`", Here());
    }
    eckit::LocalConfiguration result = root.getSubConfiguration(key);
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
bool filterImpl(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                const eckit::LocalConfiguration& opt, bool filterPerturbedForecast, bool filterModelLevel,
                bool filterFrequencyDirection, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const auto activeConcepts = frontend::resolution::resolve_ActiveConcepts_or_throw(
        mars, opt, utils::profiling::callSite(cntx, Here()));
    const backend::models::product_time_spec::ProductTimeSpec productTimeSpec{
        detail::innerStatisticalProcessing(activeConcepts, utils::profiling::callSite(cntx, Here())), mars, misc, opt,
        utils::profiling::callSite(cntx, Here())};
    (void)productTimeSpec;

    const std::array conditions{
        !filterPerturbedForecast || !mars.has("type") || mars.getString("type") != "pf" ||
            (mars.has("number") && mars.getLong("number") == 1),
        !filterModelLevel || !mars.has("levtype") || mars.getString("levtype") != "ml" || !mars.has("levelist") ||
            mars.getLong("levelist") == 1,
        !filterFrequencyDirection || !mars.has("frequency") || !mars.has("direction") ||
            (mars.getLong("frequency") == 1 && mars.getLong("direction") == 1),
    };

    const bool result = std::all_of(conditions.begin(), conditions.end(), [](bool condition) { return condition; });
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
void pruneTestsFileImpl(const eckit::PathName& inputPath, const eckit::PathName& outputPath,
                        const eckit::LocalConfiguration& options, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    std::ifstream input{inputPath.asString()};
    if (!input) {
        throw eckit::Exception("Unable to open input test-case file `" + inputPath.asString() + "`", Here());
    }

    std::ofstream output{outputPath.asString(), std::ios::out | std::ios::trunc};
    if (!output) {
        throw eckit::Exception("Unable to open output test-case file `" + outputPath.asString() + "`", Here());
    }

    const bool filterPerturbedForecast =
        option(options, "filter-perturbed-forecast", utils::profiling::callSite(cntx, Here()));
    const bool filterModelLevel = option(options, "filter-model-level", utils::profiling::callSite(cntx, Here()));
    const bool filterFrequencyDirection =
        option(options, "filter-frequency-direction", utils::profiling::callSite(cntx, Here()));
    std::string record;
    std::size_t lineNumber = 0;
    while (std::getline(input, record)) {
        ++lineNumber;
        if (!record.empty() && record.back() == '\r') {
            record.pop_back();
        }
        if (record.empty()) {
            throw eckit::Exception("Empty test-case record at line " + std::to_string(lineNumber), Here());
        }

        try {
            const eckit::LocalConfiguration root{eckit::YAMLConfiguration{record}};
            const auto mars = requiredObject(root, "mars", lineNumber, utils::profiling::callSite(cntx, Here()));
            const auto misc = requiredObject(root, "misc", lineNumber, utils::profiling::callSite(cntx, Here()));
            const auto opt  = requiredObject(root, "opt", lineNumber, utils::profiling::callSite(cntx, Here()));
            (void)requiredObject(root, "out", lineNumber, utils::profiling::callSite(cntx, Here()));

            if (filterImpl(mars, misc, opt, filterPerturbedForecast, filterModelLevel, filterFrequencyDirection,
                            utils::profiling::callSite(cntx, Here()))) {
                output << record << '\n';
                if (!output) {
                    throw eckit::Exception("Unable to write output test-case file `" + outputPath.asString() + "`",
                                           Here());
                }
            }
        }
        catch (const eckit::Exception& exception) {
            throw eckit::Exception(
                "Unable to process test-case record at line " + std::to_string(lineNumber) + ": " + exception.what(),
                Here());
        }
        catch (const std::exception& exception) {
            throw eckit::Exception(
                "Unable to process test-case record at line " + std::to_string(lineNumber) + ": " + exception.what(),
                Here());
        }
        catch (...) {
            throw eckit::Exception(
                "Unable to process test-case record at line " + std::to_string(lineNumber) + ": unknown exception",
                Here());
        }
    }

    if (input.bad()) {
        throw eckit::Exception("Error while reading input test-case file `" + inputPath.asString() + "`", Here());
    }

    output.close();
    if (!output) {
        throw eckit::Exception("Unable to complete output test-case file `" + outputPath.asString() + "`", Here());
    }
    utils::profiling::profileExitFunction(cntx, Here());
}

}  // namespace

TestsFilter::TestsFilter(const eckit::LocalConfiguration& options) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    filterPerturbedForecast_ =
        option(options, "filter-perturbed-forecast", utils::profiling::callSite(cntx, Here()));
    filterModelLevel_ = option(options, "filter-model-level", utils::profiling::callSite(cntx, Here()));
    filterFrequencyDirection_ =
        option(options, "filter-frequency-direction", utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
}

bool TestsFilter::filter(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                         const eckit::LocalConfiguration& opt) const {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = filterImpl(mars, misc, opt, filterPerturbedForecast_, filterModelLevel_,
                                   filterFrequencyDirection_, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

void pruneTestsFile(const eckit::PathName& inputPath, const eckit::PathName& outputPath,
                    const eckit::LocalConfiguration& options) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    pruneTestsFileImpl(inputPath, outputPath, options, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
}

}  // namespace metkit::mars2grib::testing_utils
