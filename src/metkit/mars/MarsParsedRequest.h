/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#ifndef metkit_MarsParsedRequest_H
#define metkit_MarsParsedRequest_H

#include "metkit/mars/MarsParserContext.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsParsedRequest : public MarsRequest, public MarsParserContext {
public:

    MarsParsedRequest(const std::string& verb, size_t line, size_t column);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
