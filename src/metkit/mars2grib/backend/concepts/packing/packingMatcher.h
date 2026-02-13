#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/packing/packingEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t packingMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    const auto& packing = get_or_throw<std::string>(mars, "packing");
    if (packing == "simple") {
        return static_cast<std::size_t>(PackingType::Simple);
    }
    if (packing == "ccsds") {
        return static_cast<std::size_t>(PackingType::Ccsds);
    }
    if (packing == "complex") {
        return static_cast<std::size_t>(PackingType::SpectralComplex);
    }

    throw eckit::Exception{"Unknown value \"" + packing + "\" for mars keyword \"packing\"!", Here()};
}

}  // namespace metkit::mars2grib::backend::concepts_
