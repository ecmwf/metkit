/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   MarsParser.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Jun 2012

#include "metkit/MarsParser.h"
#include "eckit/utils/Translator.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

MarsParserCallback::~MarsParserCallback() {}

//----------------------------------------------------------------------------------------------------------------------

std::string MarsParser::parseString(char quote)
{
    consume(quote);
    std::string s;
    for (;;)
    {
        char c = next(true);
        if (c == '\\')
        {
            c = next(true);
            switch (c) {

            case '"':
                s += '"';
                break;

            case '\'':
                s += '\'';
                break;

            case '\\':
                s += '\\';
                break;

            case '/':
                s += '/';
                break;

            case 'b':
                s += '\b';
                break;

            case 'f':
                s += '\f';
                break;

            case 'n':
                s += '\n';
                break;

            case 'r':
                s += '\r';
                break;

            case 't':
                s += '\t';
                break;

            case 'u':
                throw StreamParser::Error(std::string("JSONTokenizer::parseString \\uXXXX format not supported"));
                break;
            default:
                throw StreamParser::Error(std::string("JSONTokenizer::parseString invalid \\ char '") + c + "'");
                break;
            }
        }
        else
        {
            if (c == quote)
            {
                return s; // void(s);
            }
            s += c;
        }

    }

}

static bool inindent(char c) {
    return isalnum(c) || c == '_' || c == ':' || c == '-' || c == '.';
}


void MarsParser::quoted(std::ostream& out, const std::string& value) {
    char quote = 0;
    for(std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        if(!inindent(*j)) {
            quote = '"';
            break;
        }
    }

    if(quote) {
        out << quote << value << quote;
    }
    else {
        out << value;
    }
}


std::string MarsParser::parseIndents() {
    std::ostringstream oss;
    oss << parseIndent();

    for (;;) {
        char c = peek(true);
        while (c == ' ') {
            next(true);
            c = peek(true);
        }

        if (!inindent(c)) {
            break;
        }

        oss << " " << parseIndent();
    }

    return oss.str();
}

std::string MarsParser::parseValue() {
    char c = peek();

    if (c ==  '\"' || c == '\'') {
        return parseString(c);
    }

    return parseIndents();

}

std::vector<std::string> MarsParser::parseValues() {
    std::vector<std::string> v(1, parseValue());
    char c = peek();
    while (c == '/') {
        consume('/');
        v.push_back(parseValue());
        c = peek();
    }
    return v;
}

std::string MarsParser::parseIndent()
{
    char c = peek();
    std::string s;
    while (inindent(c)) {
        s += next(true);
        c = peek(true);
    }
    return s;
}

std::string MarsParser::parseVerb() {
    char c = peek();
    if (!isalpha(c) && c != '_') {
        throw  StreamParser::Error(std::string("MarsParser::parseVerb invalid char '") + c + "'", line_ + 1);
    }
    return parseIndent();
}

MarsRequest MarsParser::parseRequest() {
    MarsRequest r(parseVerb());
    char c = peek();
    while (c == ',') {
        consume(',');
        std::string key = parseIndents();
        consume('=');
        r.values(key, parseValues());
        c = peek();
    }
    return r;
}

MarsParser::MarsParser(std::istream &in):
    StreamParser(in, true, "*#")
{
}

std::vector<MarsRequest> MarsParser::parse()
{
    std::vector<MarsRequest> result;

    char c;
    while ((c = peek()) != 0) {
        result.push_back(parseRequest());
    }

    return result;
}

void MarsParser::parse(MarsParserCallback& cb) {
    char c;
    while ((c = peek()) != 0) {
        cb(parseRequest());
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
