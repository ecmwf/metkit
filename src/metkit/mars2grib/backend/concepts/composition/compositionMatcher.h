#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/composition/compositionEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t compositionMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribMatcherException;

    const auto param = get_or_throw<long>(mars, "param");

    // TODO: This is the range for CAMS, there are some unmapped parameters that may need to be supported for ERA6, etc.
    if (param < 400000 || param >= 500000) {
        return compile_time_registry_engine::MISSING;
    }

    const auto chem          = get_or_throw<long>(mars, "chem");
    const auto hasWavelength = has(mars, "wavelength");

    if (hasWavelength) {
        if (matchAny(param, 457000)) {
            if (matchAny(chem, range(900, 914), 918, 922, 923, range(933, 936))) {
                return static_cast<std::size_t>(CompositionType::AerosolOptical);
            }
        }
        else if (matchAny(param, 458000, 459000, 460000, 461000, 462000, 472000)) {
            if (matchAny(chem, 922)) {
                return static_cast<std::size_t>(CompositionType::AerosolOptical);
            }
        }
    }
    else {
        if (matchAny(param, 401000)) {
            if (matchAny(chem, range(900, 916))) {
                return static_cast<std::size_t>(CompositionType::Aerosol);
            }
            if (matchAny(chem, 2, 3, range(5, 24), range(26, 30), range(32, 50), 52, 53, range(55, 58), range(63, 80),
                         82, 83, 85, 86, range(99, 101), 107, 112, 159, 161, 169, range(173, 178), range(186, 204), 222,
                         range(224, 231), 233, 311, 359, 404, 917)) {
                return static_cast<std::size_t>(CompositionType::Chem);
            }
        }
        else if (matchAny(param, 402000)) {
            if (matchAny(chem, range(900, 917), 924)) {
                return static_cast<std::size_t>(CompositionType::Aerosol);
            }
            if (matchAny(chem, range(2, 24), range(26, 30), range(32, 50), 52, 53, range(55, 59), range(63, 80), 82, 83,
                         85, 86, range(99, 101), 107, 112, 118, 159, 161, 169, range(173, 178), range(186, 204), 222,
                         range(224, 230), 233, 236, 311)) {
                return static_cast<std::size_t>(CompositionType::Chem);
            }
        }
        else if (matchAny(param, 406000, 407000, 410000, 411000, 451000)) {
            if (matchAny(chem, range(901, 916))) {
                return static_cast<std::size_t>(CompositionType::Aerosol);
            }
        }
        else if (matchAny(param, 453000)) {
            if (matchAny(chem, range(901, 916), 922)) {
                return static_cast<std::size_t>(CompositionType::Aerosol);
            }
        }
        else if (matchAny(param, 400000)) {
            if (matchAny(chem, range(929, 931))) {
                return static_cast<std::size_t>(CompositionType::Aerosol);
            }
        }
        else if (matchAny(param, 444000)) {
            if (matchAny(chem, 6, 8, 13, 15, 17, 19, 26, 27, 33)) {
                return static_cast<std::size_t>(CompositionType::Chem);
            }
        }
        else if (matchAny(param, 445000)) {
            if (matchAny(chem, 6, 8, 13, 15, 17, 19, 27, 33, 236)) {
                return static_cast<std::size_t>(CompositionType::Chem);
            }
        }
        else if (matchAny(param, 479000)) {
            if (matchAny(chem, 404)) {
                return static_cast<std::size_t>(CompositionType::Chem);
            }
        }
        else if (matchAny(param, 469000)) {
            if (matchAny(chem, 2, 5, 9, 10, 12, 16, 18, 19, 42, range(45, 48), 52, 99, 100, 129, 224, 226, 233, 311,
                         933, 934)) {
                return static_cast<std::size_t>(CompositionType::ChemicalSource);
            }
        }
    }

    throw Mars2GribMatcherException(
        "compositionMatcher: matching logic is not implemented for param=" + std::to_string(param) +
            ", chem=" + std::to_string(chem) + ", hasWavelength=" + (hasWavelength ? "true" : "false"),
        Here());
}

}  // namespace metkit::mars2grib::backend::concepts_
