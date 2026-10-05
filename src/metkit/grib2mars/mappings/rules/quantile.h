#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

// MARS `quantile` is "<quantileValue>:<totalNumberOfQuantiles>" (e.g. `10:100`), the same form as the
// ecCodes `mars.quantile` alias. It is built from the section-4 keys so it does not depend on the ecCodes
// MARS definitions of the input stream/type.
template <class MarsDict, class MiscDict, class OptDict_t>
void extractQuantile(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
                     const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        (void)misc;

        for (const char* key : {"quantileValue", "totalNumberOfQuantiles"}) {
            if (!grib.has(key)) {
                throw Grib2MarsGenericException(
                    std::string("Missing GRIB key `") + key + "` required to extract MARS keyword `" + keyword + "`",
                    Here());
            }
        }

        const long value = grib.getLong("quantileValue");
        const long total = grib.getLong("totalNumberOfQuantiles");

        set_or_throw<std::string>(mars, keyword, std::to_string(value) + ":" + std::to_string(total));
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
