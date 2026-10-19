// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/mars/MarsKeyword.h"

#include "eckit/serialisation/Stream.h"

#include "metkit/mars/MarsLanguage.h"

//----------------------------------------------------------------------------------------------------------------------

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

MarsKeyword::MarsKeyword(const std::string& s) : value_(MarsLanguage::keyword(s)) {}

std::ostream& operator<<(std::ostream& s, const MarsKeyword& x) {
    return s << x.value_;
}

eckit::Stream& operator<<(eckit::Stream& s, const MarsKeyword& x) {
    return s << x.value_;
}

eckit::Stream& operator>>(eckit::Stream& s, MarsKeyword& x) {
    return s >> x.value_;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
