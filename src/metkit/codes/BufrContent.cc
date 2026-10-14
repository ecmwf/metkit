// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/codes/BufrContent.h"

#include "eccodes.h"

namespace metkit::codes {


//----------------------------------------------------------------------------------------------------------------------

BufrContent::BufrContent(std::unique_ptr<CodesHandle> handle) : CodesDataContent(std::move(handle)) {}

//----------------------------------------------------------------------------------------------------------------------

void BufrContent::transform(const eckit::OrderedStringDict& dict) {
    for (const auto& [key, value] : dict) {
        handle_->set(key, std::stol(value));
    }
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
