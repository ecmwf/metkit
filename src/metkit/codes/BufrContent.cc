/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/BufrContent.h"

#include "eccodes.h"

namespace metkit {
namespace codes {


//----------------------------------------------------------------------------------------------------------------------

BufrContent::BufrContent(std::unique_ptr<CodesHandle> handle) : CodesDataContent(std::move(handle)) {}

//----------------------------------------------------------------------------------------------------------------------

void BufrContent::transform(const eckit::OrderedStringDict& dict) {
    for (const auto& [key, value] : dict) {
        handle_->set(key, std::stol(value));
    }
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
