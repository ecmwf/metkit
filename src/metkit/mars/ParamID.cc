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

static std::vector<size_t> dropTables_;

//----------------------------------------------------------------------------------------------------------------------

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable()
{
    eckit::PathName paramMatchingPath = eckit::Resource<eckit::PathName>("paramMatchingPath;$PARAM_MATCHING_PATH", LibMetkit::paramMatchingYamlFile());

    const eckit::Value paramMatching = eckit::YAMLParser::decodeFile(paramMatchingPath);
    const eckit::Value wind = paramMatching["wind"];
    ASSERT(wind.isList());
    for (size_t i = 0; i < wind.size(); ++i) {
        const eckit::Value s = wind[i];
        ASSERT(s.isList());
        ASSERT(s.size() == 4);
        windFamilies_.push_back(ParamID::WindFamily(s[0], s[1], s[2], s[3]));
    }

    const eckit::Value dropTables = paramMatching["drop-tables"];
    ASSERT(dropTables.isList());
    for (size_t i = 0; i < dropTables.size(); ++i) {
        dropTables_.push_back(dropTables[i]);
    }

    std::string pathName = eckit::Resource<std::string>("marsWindPath;$MARS_WIND_PATH", "");
    if (!pathName.empty()) {
        eckit::PathName path(pathName);

        eckit::Log::debug<LibMetkit>() << "Parsing MARS wind " << path << std::endl;

        std::ifstream in(path.localPath());

        if (!in)
        {
            Log::error() << path << Log::syserr << std::endl;
            return;
        }

        windFamilies_.clear(); // replacing existing windFamilies

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
    }
}

const std::vector<ParamID::WindFamily>& ParamID::getWindFamilies() {
    pthread_once(&once, readTable);
    return windFamilies_;
}
const std::vector<size_t>& ParamID::getDropTables() {
    pthread_once(&once, readTable);
    return dropTables_;
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
