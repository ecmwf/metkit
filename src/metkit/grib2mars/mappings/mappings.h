#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/mappings/Grib2MarsReturnValue.h"
#include "metkit/grib2mars/mappings/MarsTopology.h"
#include "metkit/grib2mars/mappings/rules/extract.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules {

template <class OutDict_t>
Grib2MarsResult<OutDict_t> convertAll(const metkit::codes::CodesHandle& grib) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {

        std::unique_ptr<OutDict_t> mars                 = std::make_unique<OutDict_t>();
        std::unique_ptr<eckit::LocalConfiguration> misc = std::make_unique<eckit::LocalConfiguration>();

        //
        // Topology is the subset of MARS keywords that are relevant to describe the input grib message.
        const metkit::grib2mars::rules::impl::MarsTopology topology(grib);

        //
        // This is used to inject the values
        for (const std::string& keyword : topology) {
            impl::extract(keyword, grib, *mars, *misc);
        }

        return Grib2MarsResult<OutDict_t>{std::move(*mars), std::move(*misc)};
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to convertAll grib message", Here()));
    }
}

}  // namespace metkit::grib2mars::rules