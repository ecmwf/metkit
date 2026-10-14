// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date Jun 2012

#pragma once

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"
#include "metkit/mars/MarsParsedRequest.h"

namespace metkit::mars {

class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------

class MarsParserCallback {
public:

    virtual void operator()(const MarsRequest&) = 0;

protected:

    virtual ~MarsParserCallback() = default;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsParser : public eckit::StreamParser {

public:  // methods

    MarsParser(std::istream& in);

    std::vector<MarsParsedRequest> parse();

    void parse(MarsParserCallback& cb);

    static void quoted(std::ostream& out, const std::string& value);

private:  // methods

    MarsParsedRequest parseRequest();
    std::string parseVerb();
    std::string parseKeyword();
    std::vector<std::string> parseValues();
    std::string parseValue();
    std::string parseIndent();
    std::string parseIndents();

    std::string parseString(char c);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
