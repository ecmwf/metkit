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


    return static_cast<size_t>(TablesType::Default);
}

}  // namespace metkit::mars2grib::backend::concepts_
