/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsParsedRequest.h"

namespace metkit {
namespace mars {

MarsParsedRequest::MarsParsedRequest(const std::string& verb, size_t line) :
    MarsRequest(verb), MarsParserContext(line) {}

}  // namespace mars
}  // namespace metkit
