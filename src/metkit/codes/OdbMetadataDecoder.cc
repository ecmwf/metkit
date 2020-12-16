/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <sstream>
#include <iomanip>

#include "eccodes.h"

#include "metkit/codes/OdbMetadataDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Type.h"
#include "metkit/odb/IdMapper.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

class Mapping {

public:
    static Mapping& instance() {
        static Mapping mapping;
        return mapping;
    }

    std::vector<std::string>& columnNames() { return columnNames_; }

    std::map<std::string, std::string>& table() { return mapping_; }

private: // methods

    Mapping() {
        static eckit::PathName configPath(eckit::Resource<eckit::PathName>(
                                              "odbMarsRequestMapping",
                                              "~metkit/share/metkit/odb/marsrequest.yaml"));
        eckit::YAMLConfiguration config(configPath);
        for (const auto& key : config.keys()) {
            mapping_[config.getString(key)] = eckit::StringTools::lower(key);
        }

        columnNames_.reserve(mapping_.size());
        for (const auto& kv : mapping_) {
            columnNames_.push_back(kv.first);
        }
    }

private: // members

    std::map<std::string, std::string> mapping_;
    std::vector<std::string> columnNames_;

};

//----------------------------------------------------------------------------------------------------------------------

std::vector<std::string>& OdbMetadataDecoder::columnNames() {
    return Mapping::instance().columnNames();
}

template <typename T>
void OdbMetadataDecoder::visit(const std::string& columnName, const std::set<T>& vals, const metkit::mars::MarsLanguage& language) {

    auto mapitr = Mapping::instance().table().find(columnName);
    ASSERT(mapitr != Mapping::instance().table().end());
    std::string keyword = eckit::StringTools::lower(mapitr->second);
    metkit::mars::Type* t = language.type(keyword);

    for(auto val: vals) {
        std::string stringVal = eckit::Translator<T, std::string>()(val);
        std::string tidyVal = t->tidy(stringVal);
        if (stringVal == tidyVal) // if t->tidy had no effect, set the original value
            gather_.setValue(keyword, val);
        else
            gather_.setValue(keyword, tidyVal);
    }
}


OdbMetadataDecoder::OdbMetadataDecoder(eckit::message::MetadataGatherer& gather, const std::string& verb) :
    language_(verb), gather_(gather) {}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<long>& vals) {
    LOG_DEBUG_LIB(LibMetkit) << "OdbMetadataDecoder::operator() columnName: " << columnName
                             << " vals: " << vals << std::endl;

    auto mapitr = Mapping::instance().table().find(columnName);
    ASSERT(mapitr != Mapping::instance().table().end());

    std::set<std::string> mapped;
    if (metkit::odb::IdMapper::instance().alphanumeric(mapitr->second, vals, mapped)) {
        visit(columnName, mapped, language_);
    }
    else {
        visit(columnName, vals, language_);
    }
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<double>& vals) {
    visit(columnName, vals, language_);
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<std::string>& vals) {
    visit(columnName, vals, language_);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
