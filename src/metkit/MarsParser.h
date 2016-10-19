/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Jun 2012

#ifndef eckit_JSONParser_h
#define eckit_JSONParser_h

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"
#include "metkit/MarsRequest.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class MarsParser : public eckit::StreamParser {

public: // methods

    MarsParser(std::istream& in);

    std::vector<MarsRequest> parse();

private: // methods

    MarsRequest parseRequest();
    std::string parseVerb();
    std::string parseKeyword();
    std::vector<std::string> parseValues();
    std::string parseValue();
    std::string parseIndent();
    std::string parseString(char c);

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
