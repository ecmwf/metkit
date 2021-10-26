/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/ParamID.h"

#include <pthread.h>
#include <fstream>

#include "eckit/utils/Tokenizer.h"
#include "eckit/config/Resource.h"
#include "eckit/system/Library.h"
#include "eckit/parser/YAMLParser.h"

#include "metkit/config/LibMetkit.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

static std::vector<ParamID::WindFamily> windFamilies_;

static std::map<size_t, std::set<size_t>> paramTableExpansion_;

//----------------------------------------------------------------------------------------------------------------------

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable()
{
    eckit::PathName path = eckit::Resource<eckit::PathName>("marsWindPath;$MARS_WIND_PATH", "~metkit/share/metkit/wind");

    eckit::Log::debug<LibMetkit>() << "Parsing MARS wind " << path << std::endl;

    std::ifstream in(path.localPath());

    if (!in)
    {
        Log::error() << path << Log::syserr << std::endl;
        return;
    }

    char line[1024];
    while (in.getline(line, sizeof(line)))
    {
        eckit::Log::debug<LibMetkit>() << "MARS wind parsing [" << line << "]" << std::endl;

        eckit::Tokenizer parse(" ");
        std::vector<std::string> s;
        parse(line, s);

        eckit::Ordinal i = 0;
        while (i < s.size())
        {
            if (s[i].length() == 0)
                s.erase(s.begin() + i);
            else
                i++;
        }

        if (s.size() == 0 || s[0][0] == '#')
            continue;

        switch (s.size())
        {
        case 4:
            eckit::Log::debug<LibMetkit>() << "MARS wind = " << s[0] << ", " << s[1] << ", " << s[2] << ", " << s[3] << std::endl;
            windFamilies_.push_back(ParamID::WindFamily(s[0], s[1], s[2], s[3]));
            break;

        default:
            Log::warning() << "Invalid line ignored: " << line << std::endl;
            break;

        }
    }

    const eckit::Value pts = eckit::YAMLParser::decodeFile(LibMetkit::paramTableExpansionYamlFile());
    ASSERT(pts.isOrderedMap());

    const eckit::Value keys = pts.keys();
    ASSERT(keys.isList());

    for (size_t i = 0; i < keys.size(); ++i) {
        const eckit::Value& pt = pts.element(keys[i]);
        ASSERT(pt.isList());

        std::set<size_t> vals;
        for (size_t j=0; j<pt.size(); j++)
            vals.emplace((size_t) pt[j]);
        paramTableExpansion_[(size_t) keys[i]] = vals;
    }
}

const std::vector<ParamID::WindFamily>& ParamID::getWindFamilies() {
    pthread_once(&once, readTable);
    return windFamilies_;
}
const std::map<size_t, std::set<size_t>>& ParamID::getParamTableExpansion() {
    pthread_once(&once, readTable);
    return paramTableExpansion_;
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
