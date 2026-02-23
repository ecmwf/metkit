#pragma once

// System include
#include <cstddef>

// Utils
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/backend/concepts/wave/waveEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t waveMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, range(140114, 140120))) {
        return static_cast<std::size_t>(WaveType::Period);
    }

    if (matchAny(param, 140251)) {
        ASSERT(has(mars, "frequency"));
        ASSERT(has(mars, "direction"));
        return static_cast<std::size_t>(WaveType::Spectra);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_
