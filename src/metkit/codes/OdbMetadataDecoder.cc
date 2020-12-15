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
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/message/Message.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/odb/IdMapper.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

std::map<std::string, std::string> OdbMetadataDecoder::mapping_;
std::vector<std::string> OdbMetadataDecoder::columnNames_;

pthread_once_t once = PTHREAD_ONCE_INIT;

void OdbMetadataDecoder::init() {
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

std::vector<std::string>& OdbMetadataDecoder::columnNames() {
    pthread_once(&once, OdbMetadataDecoder::init);

    return columnNames_;
}

OdbMetadataDecoder::OdbMetadataDecoder(eckit::message::MetadataGatherer& gather, const std::string& verb) :
    language_(verb), gather_(gather) {
    pthread_once(&once, OdbMetadataDecoder::init);
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<long>& vals) {
    LOG_DEBUG_LIB(LibMetkit) << "MarsRequestSetter::operator() columnName: " << columnName
                             << " vals: " << vals << std::endl;

    auto mapitr = mapping_.find(columnName);
    ASSERT(mapitr != mapping_.end());

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
