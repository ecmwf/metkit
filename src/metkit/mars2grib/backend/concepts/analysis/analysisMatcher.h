#pragma once

// System include
#include <cstddef>
#include <iostream>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/analysis/analysisEnum.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t analysisMatcher( const MarsDict_t& mars, const OptDict_t& opt ){

    using metkit::mars2grib::backend::compile_time_registry_engine::NOT_APPLICABLE;

    std::cout << " - analysis matcher" << std::endl;
    // This doesn't compile because the compiler cannot find GeneralRegistry
    // If we include the header, we will have a circular dependency error
    // return GeneralRegistry::globalIndex(AnalysisType::Default);

    return NOT_APPLICABLE;
}

}
