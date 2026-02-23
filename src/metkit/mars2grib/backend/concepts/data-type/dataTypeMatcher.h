#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/data-type/dataTypeEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t dataTypeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    return static_cast<std::size_t>(DataTypeType::Default);
}

}  // namespace metkit::mars2grib::backend::concepts_
