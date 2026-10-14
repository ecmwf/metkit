// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/mars/MarsParsedRequest.h"

namespace metkit::mars {

MarsParsedRequest::MarsParsedRequest(const std::string& verb, size_t line) : MarsRequest(verb), line_(line) {}

void MarsParsedRequest::info(std::ostream& out) const {
    out << " Request starting line " << line_;
}

}  // namespace metkit::mars
