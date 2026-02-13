#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/tables/tablesEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t tablesMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    const auto param = get_or_throw<long>(mars, "param");
    // Chemical products
    if (matchAny(param, range(228080, 228082), range(233032, 233035), range(235062, 235064))) {
        return static_cast<std::size_t>(TablesType::Custom);
    }

    return static_cast<size_t>(TablesType::Default);
}

}  // namespace metkit::mars2grib::backend::concepts_
