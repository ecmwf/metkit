/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/odb/IdMapper.h"

#include <vector>
#include <fstream>
#include <algorithm>

#include "eckit/config/Resource.h"
#include "eckit/utils/StringTools.h"
#include "eckit/log/Log.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/config/LibMetkit.h"

using namespace eckit;

namespace metkit {
namespace odb {

//----------------------------------------------------------------------------------------------------------------------

IdMapper::IdMapper() :
    maps_ {
        {"CLASS", IdMap{"class.table"}},
        {"TYPE", IdMap{"type.table"}},
        {"STREAM", IdMap{"stream.table"}},
        {"OBSGROUP", IdMap{"group.txt", ";", 0, 3}}} {}

IdMapper::~IdMapper() {}

IdMapper& IdMapper::instance() {
    static IdMapper themapper;
    return themapper;
}

//----------------------------------------------------------------------------------------------------------------------


bool IdMapper::alphanumeric(const std::string& keyword, long numeric, std::string& output)
{
    auto idmap = maps_.find(StringTools::upper(keyword));

    if (idmap == maps_.end()) {
        return false;
    }

    output = idmap->second.alphanumeric(numeric);
    return true;
}

bool IdMapper::alphanumeric(const std::string& keyword, const std::set<long>& numeric, std::set<std::string>& output)
{
    auto idmap = maps_.find(StringTools::upper(keyword));

    if (idmap == maps_.end()) {
        return false;
    }

    output.clear();
    std::transform(numeric.begin(), numeric.end(), std::inserter(output, output.begin()),
                   [&idmap](long l) { return idmap->second.alphanumeric(l); });

    return true;
}


static PathName& codes_path() {
    static PathName p = Resource<PathName>("$ODB_CODES", "~metkit/share/metkit/odb");
    return p;
}

IdMap::IdMap(const std::string& configFile,
             const std::string& fieldDelimiter,
             size_t numericIndex,
             size_t alphanumericIndex) {

    PathName configPath = codes_path() / configFile;
    LOG_DEBUG_LIB(LibMetkit) << "GribCodesBase::GribCodesBase: config file:" << configPath << std::endl;

	numeric2alpha_.clear();

    std::ifstream fin(configPath.asString());
    std::string line;

    while(std::getline(fin, line)) {

        std::vector<std::string> words = StringTools::split(fieldDelimiter, line);
		if (words.size() >= 2)
		{
            long num = eckit::Translator<std::string, long>()(StringTools::trim(words[numericIndex]));
            std::string alpha (StringTools::trim(words[alphanumericIndex]));
			numeric2alpha_[num] = StringTools::lower(alpha);
            LOG_DEBUG_LIB(LibMetkit) << "GribCodesBase::readConfig: num='" << num << "' alpha='" << alpha << "'" << std::endl;
		}
	}
}

IdMap::~IdMap() {}

std::string IdMap::alphanumeric(long numeric) {

    auto it = numeric2alpha_.find(numeric);

    if (it == numeric2alpha_.end()) {
        std::stringstream ss;
        ss << "Numeric code " << numeric << " not found";
        throw UserError(ss.str(), Here());
    }
    return it->second;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace odb
} // namespace metkit

