/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Jun 2012

#ifndef metkit_MARSParser_h
#define metkit_MARSParser_h

#include "eckit/parser/StreamParser.h"
#include "eckit/types/Types.h"
#include "metkit/MarsRequest.h"

namespace metkit {


//----------------------------------------------------------------------------------------------------------------------


class MarsParserCallback {
public:
    virtual void operator()(const MarsRequest&) = 0;
protected:
    ~MarsParserCallback();
};


//----------------------------------------------------------------------------------------------------------------------


class MarsParser : public eckit::StreamParser {

public: // methods

    MarsParser(std::istream& in);

    std::vector<MarsRequest> parse();

    void parse(MarsParserCallback& cb);

    static void quoted(std::ostream& out, const std::string& value);

private: // methods

    MarsRequest parseRequest();
    std::string parseVerb();
    std::string parseKeyword();
    std::vector<std::string> parseValues();
    std::string parseValue();
    std::string parseIndent();
    std::string parseIndents();

    std::string parseString(char c);

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
