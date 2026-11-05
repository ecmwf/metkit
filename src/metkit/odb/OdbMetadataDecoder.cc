// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/odb/OdbMetadataDecoder.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"
#include "metkit/odb/IdMapper.h"


namespace {

/// A set of conditions that must all hold for a column to be applied.
/// Each condition maps a MARS keyword to the set of accepted (lower-case) values.
struct ConditionalRule {
    std::map<std::string, std::set<std::string>> conditions;
};

class OdbColumnNameMapping {

public:

    static OdbColumnNameMapping& instance() {
        static OdbColumnNameMapping mapping;
        return mapping;
    }

    const std::vector<std::string>& columnNames() { return columnNames_; }
    const std::map<std::string, std::string>& table() { return mapping_; }

    bool conditionalRule(const std::string& odbColumnName, ConditionalRule& rule) const {
        auto it = conditional_.find(odbColumnName);
        if (it == conditional_.end()) {
            return false;
        }
        rule = it->second;
        return true;
    }

private:  // methods

    OdbColumnNameMapping() {
        static eckit::PathName configPath(
            eckit::Resource<eckit::PathName>("odbMarsRequestMapping", "~metkit/share/metkit/odb/marsrequest.yaml"));
        eckit::YAMLConfiguration config(configPath);

        for (const auto& rule : config.getSubConfigurations()) {
            const std::string name = rule.getString("name", "<unnamed>");

            if (!rule.has("request") || !rule.isSubConfiguration("request")) {
                std::ostringstream msg;
                msg << "OdbColumnNameMapping: rule '" << name << "' in " << configPath << " must have a 'request' map";
                throw eckit::BadValue(msg.str(), Here());
            }

            bool isDefault = false;
            ConditionalRule conditional;

            if (!rule.has("matcher")) {
                isDefault = true;
            }
            else if (rule.isString("matcher")) {
                const std::string m = eckit::StringTools::lower(rule.getString("matcher"));
                if (m != "default") {
                    std::ostringstream msg;
                    msg << "OdbColumnNameMapping: rule '" << name << "' has unknown matcher '" << m
                        << "' (expected 'default' or a map)";
                    throw eckit::BadValue(msg.str(), Here());
                }
                isDefault = true;
            }
            else if (rule.isSubConfiguration("matcher")) {
                const eckit::LocalConfiguration matcher = rule.getSubConfiguration("matcher");
                for (const auto& keyword : matcher.keys()) {
                    std::vector<std::string> values = matcher.isStringList(keyword)
                                                          ? matcher.getStringVector(keyword)
                                                          : std::vector<std::string>{matcher.getString(keyword)};
                    auto& accepted                  = conditional.conditions[eckit::StringTools::lower(keyword)];
                    for (const auto& v : values) {
                        accepted.insert(eckit::StringTools::lower(v));
                    }
                }
                isDefault = conditional.conditions.empty();
            }
            else {
                std::ostringstream msg;
                msg << "OdbColumnNameMapping: rule '" << name << "' has an invalid matcher";
                throw eckit::BadValue(msg.str(), Here());
            }

            const eckit::LocalConfiguration request = rule.getSubConfiguration("request");
            for (const auto& marsKey : request.keys()) {
                const std::string odbCol = request.getString(marsKey);
                mapping_[odbCol]         = eckit::StringTools::lower(marsKey);
                if (isDefault) {
                    conditional_.erase(odbCol);
                }
                else {
                    conditional_[odbCol] = conditional;
                }
            }
        }

        columnNames_.reserve(mapping_.size());
        for (const auto& kv : mapping_) {
            columnNames_.push_back(kv.first);
        }
    }

private:  // members

    std::map<std::string, std::string> mapping_;
    std::map<std::string, ConditionalRule> conditional_;
    std::vector<std::string> columnNames_;
};
}  // namespace

//----------------------------------------------------------------------------------------------------------------------

namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

const std::vector<std::string>& OdbMetadataDecoder::columnNames() {
    return OdbColumnNameMapping::instance().columnNames();
}

std::vector<std::string> OdbMetadataDecoder::columnNames(const odc::api::Frame& frame) {
    std::vector<std::string> cols;
    cols.reserve(OdbColumnNameMapping::instance().columnNames().size());

    for (const auto& c : OdbColumnNameMapping::instance().columnNames()) {
        if (frame.hasColumn(c)) {
            cols.push_back(c);
        }
    }

    return cols;
}

template <typename T>
void OdbMetadataDecoder::visit(const std::string& columnName, const std::set<T>& vals,
                               const metkit::mars::MarsLanguage& language) {

    auto mapitr = OdbColumnNameMapping::instance().table().find(columnName);
    ASSERT(mapitr != OdbColumnNameMapping::instance().table().end());
    std::string keyword         = eckit::StringTools::lower(mapitr->second);
    const metkit::mars::Type* t = language.type(keyword);

    ASSERT(options_.valueRepresentation == eckit::message::ValueRepresentation::String);

    for (auto val : vals) {
        std::string stringVal = eckit::Translator<T, std::string>()(val);
        std::string tidyVal   = t->tidy(stringVal);
        if (stringVal == tidyVal)  // if t->tidy had no effect, set the original value
            gather_.setValue(keyword, val);
        else
            gather_.setValue(keyword, tidyVal);
    }
}


OdbMetadataDecoder::OdbMetadataDecoder(eckit::message::MetadataGatherer& gather,
                                       const eckit::message::GetMetadataOptions& options,
                                       const metkit::mars::MarsLanguage& language) :
    language_(language), gather_(gather), options_(options) {}

template <typename T>
bool OdbMetadataDecoder::deferIfConditional(const std::string& columnName, const std::set<T>& vals) {
    ConditionalRule rule;
    if (!OdbColumnNameMapping::instance().conditionalRule(columnName, rule)) {
        return false;
    }

    // Conditional columns are not inspected at all (no type lookup, no value mapping, no tidy)
    // until all the conditions of their rule have been verified in finalize().
    deferred_[columnName] = [this, columnName, vals, rule]() {
        for (const auto& [keyword, accepted] : rule.conditions) {
            auto it = keywordValues_.find(keyword);
            if (it == keywordValues_.end()) {
                return;
            }
            const auto& seen = it->second;
            if (std::none_of(seen.begin(), seen.end(), [&](const std::string& v) { return accepted.count(v); })) {
                return;
            }
        }
        process(columnName, vals);
    };
    return true;
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<long>& vals) {
    LOG_DEBUG_LIB(LibMetkit) << "OdbMetadataDecoder::operator() columnName: " << columnName << " vals: " << vals
                             << std::endl;
    if (!deferIfConditional(columnName, vals)) {
        process(columnName, vals);
    }
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<double>& vals) {
    if (!deferIfConditional(columnName, vals)) {
        process(columnName, vals);
    }
}

void OdbMetadataDecoder::operator()(const std::string& columnName, const std::set<std::string>& vals) {
    if (!deferIfConditional(columnName, vals)) {
        process(columnName, vals);
    }
}

void OdbMetadataDecoder::process(const std::string& columnName, const std::set<long>& vals) {
    auto mapitr = OdbColumnNameMapping::instance().table().find(columnName);
    ASSERT(mapitr != OdbColumnNameMapping::instance().table().end());
    const std::string keyword   = mapitr->second;
    const metkit::mars::Type* t = language_.type(keyword);

    std::set<std::string> mapped;
    if (metkit::odb::IdMapper::instance().alphanumeric(keyword, vals, mapped)) {
        for (const auto& v : mapped) {
            keywordValues_[keyword].insert(eckit::StringTools::lower(t->tidy(v)));
        }
        visit(columnName, mapped, language_);
    }
    else {
        for (const auto& v : vals) {
            std::string stringVal = eckit::Translator<long, std::string>()(v);
            keywordValues_[keyword].insert(eckit::StringTools::lower(t->tidy(stringVal)));
        }
        visit(columnName, vals, language_);
    }
}

void OdbMetadataDecoder::process(const std::string& columnName, const std::set<double>& vals) {
    auto mapitr = OdbColumnNameMapping::instance().table().find(columnName);
    ASSERT(mapitr != OdbColumnNameMapping::instance().table().end());
    const std::string keyword   = mapitr->second;
    const metkit::mars::Type* t = language_.type(keyword);
    for (const auto& v : vals) {
        std::string stringVal = eckit::Translator<double, std::string>()(v);
        keywordValues_[keyword].insert(eckit::StringTools::lower(t->tidy(stringVal)));
    }
    visit(columnName, vals, language_);
}

void OdbMetadataDecoder::process(const std::string& columnName, const std::set<std::string>& vals) {
    auto mapitr = OdbColumnNameMapping::instance().table().find(columnName);
    ASSERT(mapitr != OdbColumnNameMapping::instance().table().end());
    const std::string keyword   = mapitr->second;
    const metkit::mars::Type* t = language_.type(keyword);
    for (const auto& v : vals) {
        keywordValues_[keyword].insert(eckit::StringTools::lower(t->tidy(v)));
    }
    visit(columnName, vals, language_);
}

void OdbMetadataDecoder::finalize() {
    for (auto& kv : deferred_) {
        kv.second();
    }
    deferred_.clear();
    keywordValues_.clear();
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
