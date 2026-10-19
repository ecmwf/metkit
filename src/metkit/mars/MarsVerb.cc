// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/mars/MarsVerb.h"

#include "eckit/serialisation/Stream.h"

#include "metkit/mars/MarsLanguage.h"

//----------------------------------------------------------------------------------------------------------------------

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

MarsVerb::MarsVerb(const std::string& s) : value_(MarsLanguage::verb(s)) {}

std::ostream& operator<<(std::ostream& s, const MarsVerb& x) {
    return s << x.value_;
}

eckit::Stream& operator<<(eckit::Stream& s, const MarsVerb& x) {
    return s << x.value_;
}

eckit::Stream& operator>>(eckit::Stream& s, MarsVerb& x) {
    return s >> x.value_;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
