/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeParam.h"

#include "eckit/codec/detail/Endian.h"
#include "eckit/config/Resource.h"
#include "eckit/io/FileLock.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/TypesFactory.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <set>
#include <unordered_map>
#include <unordered_set>

using eckit::Log;
using metkit::LibMetkit;

// paramid -> shortname aliases, as read from paramids.yaml
using ParamIdAliases = std::unordered_map<uint32_t, std::vector<std::string>>;

namespace {

const char* toLittleEndian(uint16_t* value) {
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        *value = (*value >> 8) | (*value << 8);
    }
    return reinterpret_cast<const char*>(value);
}

const char* toLittleEndian(uint32_t* value) {
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        *value = ((*value >> 24) & 0x000000FF) | ((*value >> 8) & 0x0000FF00) | ((*value << 8) & 0x00FF0000) |
                 ((*value << 24) & 0xFF000000);
    }
    return reinterpret_cast<const char*>(value);
}

uint16_t littleEndian2uint16(const char* value) {
    uint16_t v = *reinterpret_cast<const uint16_t*>(value);
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        v = (v >> 8) | (v << 8);
    }
    return v;
}
uint32_t littleEndian2uint32(const char* value) {
    uint32_t v = *reinterpret_cast<const uint32_t*>(value);
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        v = ((v >> 24) & 0x000000FF) | ((v >> 8) & 0x0000FF00) | ((v << 8) & 0x00FF0000) | ((v << 24) & 0xFF000000);
    }
    return v;
}

uint8_t read8(std::ifstream& file) {
    uint8_t size;
    if (file.read(reinterpret_cast<char*>(&size), sizeof(uint8_t))) {
        return size;
    }
    throw eckit::SeriousBug("Failed to read 8-bit value from file", Here());
}
uint16_t read16(std::ifstream& file) {
    uint16_t size;
    if (file.read(reinterpret_cast<char*>(&size), sizeof(uint16_t))) {
        return littleEndian2uint16(reinterpret_cast<const char*>(&size));
    }
    throw eckit::SeriousBug("Failed to read 16-bit value from file", Here());
}
uint32_t read32(std::ifstream& file) {
    uint32_t value;
    if (file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t))) {
        return littleEndian2uint32(reinterpret_cast<const char*>(&value));
    }
    throw eckit::SeriousBug("Failed to read 32-bit value from file", Here());
}
std::string readString(std::ifstream& file) {
    uint8_t size;
    if (file.read(reinterpret_cast<char*>(&size), sizeof(uint8_t))) {
        std::string str(size, '\0');
        if (file.read(str.data(), size)) {
            return str;
        }
    }
    throw eckit::SeriousBug("Failed to read string from file", Here());
}

void write16(std::ofstream& file, uint16_t size) {
    file.write(toLittleEndian(&size), sizeof(uint16_t));
}
void write32(std::ofstream& file, uint32_t size) {
    file.write(toLittleEndian(&size), sizeof(uint32_t));
}
void write8(std::ofstream& file, size_t size) {
    if (size > static_cast<size_t>(std::numeric_limits<uint8_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - count of " << size << " exceeds the maximum of "
            << static_cast<size_t>(std::numeric_limits<uint8_t>::max()) << " supported by the uint8_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint8_t size8 = static_cast<uint8_t>(size);
    file.write(reinterpret_cast<const char*>(&size8), sizeof(uint8_t));
}
void write16(std::ofstream& file, size_t size) {
    if (size > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - count of " << size << " exceeds the maximum of "
            << static_cast<size_t>(std::numeric_limits<uint16_t>::max()) << " supported by the uint16_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint16_t size16 = static_cast<uint16_t>(size);
    file.write(toLittleEndian(&size16), sizeof(uint16_t));
}
void write32(std::ofstream& file, size_t size) {
    ASSERT(size <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    uint32_t size32 = static_cast<uint32_t>(size);
    file.write(toLittleEndian(&size32), sizeof(uint32_t));
}
void writeString(std::ofstream& file, const std::string& str) {
    if (str.size() > static_cast<size_t>(std::numeric_limits<uint8_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - string '" << str << "' has length " << str.size()
            << " which exceeds the maximum of " << static_cast<size_t>(std::numeric_limits<uint8_t>::max())
            << " supported by the uint8_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint8_t size = static_cast<uint8_t>(str.size());
    file.write(reinterpret_cast<const char*>(&size), sizeof(uint8_t));
    file.write(str.data(), size);
}

static eckit::Mutex* local_mutex = 0;
static pthread_once_t once       = PTHREAD_ONCE_INIT;


class Matcher {

    std::string name_;
    std::vector<std::string> values_;

    friend class Rule;

public:

    Matcher(const std::string& name, std::vector<std::string>&& values);
    Matcher(std::ifstream& file);

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;

    void write(std::ofstream& out) const;
    void print(std::ostream& out) const;

    friend std::ostream& operator<<(std::ostream& out, const Matcher& matcher) {
        matcher.print(out);
        return out;
    }
};

Matcher::Matcher(const std::string& name, std::vector<std::string>&& values) :
    name_(name), values_(std::move(values)) {}

Matcher::Matcher(std::ifstream& file) {
    name_             = readString(file);
    uint8_t numValues = read8(file);
    values_.reserve(numValues);
    for (uint32_t i = 0; i < numValues; i++) {
        values_.push_back(readString(file));
    }
}

bool Matcher::match(const metkit::mars::MarsRequest& request, bool partial) const {

    std::vector<std::string> vals = request.values(name_, true);
    if (vals.size() == 0) {
        return partial;
    }

    for (const auto& v : values_) {
        if (v == vals[0]) {
            return true;
        }
    }

    return false;
}

void Matcher::write(std::ofstream& out) const {
    writeString(out, name_);
    write8(out, values_.size());
    for (const auto& v : values_) {
        writeString(out, v);
    }
}

void Matcher::print(std::ostream& out) const {
    out << name_ << "=[";
    std::string separator{};
    for (const auto& v : values_) {
        out << separator << v;
        separator = ",";
    }
    out << "]";
}

//----------------------------------------------------------------------------------------------------------------------

class Rule {

    std::vector<Matcher> matchers_;

    std::unordered_set<uint32_t> values_;
    mutable std::map<std::string, std::string> mapping_;

    // bare param number (no table) -> paramId of the only table holding that number in this context
    std::map<uint32_t, uint32_t> bareIds_;

    static std::unordered_set<uint32_t> defaultValues_;
    static std::map<std::string, std::string> defaultMapping_;

public:

    static void init();

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;
    std::string lookup(const std::string& s) const;

    void setBareIds(const eckit::Value& contextIds);
    bool hasBareIds() const { return !bareIds_.empty(); }
    bool resolveBareId(std::string& s) const;

    Rule(const eckit::Value& matchers, const eckit::Value& setters, const ParamIdAliases& ids);
    Rule(std::ifstream& file);

    static void setDefault(const eckit::Value& setters, const ParamIdAliases& ids);

    void write(std::ofstream& out) const;
    void print(std::ostream& out) const;

    friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
        rule.print(out);
        return out;
    }
};

static void initRules() {
    Rule::init();
}

std::unordered_set<uint32_t> Rule::defaultValues_;
std::map<std::string, std::string> Rule::defaultMapping_;

void Rule::setDefault(const eckit::Value& values, const ParamIdAliases& ids) {

    std::map<std::string, size_t> precedence;

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        uint32_t paramid  = static_cast<uint32_t>(std::stoul(first));
        defaultValues_.insert(paramid);

        auto it = ids.find(paramid);
        if (it == ids.end()) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << std::endl;
            continue;
        }


        for (size_t j = 0; j < it->second.size(); ++j) {
            std::string v = it->second.at(j);

            if (defaultMapping_.find(v) != defaultMapping_.end()) {

                if (precedence[v] <= j) {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition ignored: param " << v << "='" << first << "', keeping previous value of '"
                        << defaultMapping_[v] << "' " << std::endl;
                    continue;
                }
                else {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition of param " << v << "='" << first << "', overriding previous value of '"
                        << defaultMapping_[v] << "' " << std::endl;

                    precedence[v] = j;
                }
            }
            else {
                precedence[v] = j;
            }

            defaultMapping_[v] = first;
        }
    }
}

Rule::Rule(const eckit::Value& matchers, const eckit::Value& values, const ParamIdAliases& ids) {

    std::map<std::string, size_t> precedence;

    const eckit::Value& keys = matchers.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string name = keys[i];

        auto rawValues = matchers[name];
        std::vector<std::string> mvalues;
        if (!rawValues.isList()) {
            mvalues.push_back(rawValues);
        }
        else {
            for (size_t j = 0; j < rawValues.size(); j++) {
                std::string v = rawValues[j];
                mvalues.push_back(v);
            }
        }

        matchers_.emplace_back(name, std::move(mvalues));
    }

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        uint32_t paramid  = static_cast<uint32_t>(std::stoul(first));
        values_.insert(paramid);

        auto it = ids.find(paramid);
        if (it == ids.end() || it->second.empty()) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << " " << *this << std::endl;
            continue;
        }
        const auto& aliases = it->second;


        for (size_t j = 0; j < aliases.size(); ++j) {
            const std::string& v = aliases[j];

            if (mapping_.find(v) != mapping_.end()) {

                if (precedence[v] <= j) {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition ignored: param " << v << "='" << first << "', keeping previous value of '"
                        << mapping_[v] << "' " << *this << std::endl;
                    continue;
                }
                else {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition of param " << v << "='" << first << "', overriding previous value of '"
                        << mapping_[v] << "' " << *this << std::endl;

                    precedence[v] = j;
                }
            }
            else {
                precedence[v] = j;
            }

            mapping_[v] = first;
        }
    }
}

Rule::Rule(std::ifstream& file) {
    uint8_t numMatchers = read8(file);
    matchers_.reserve(numMatchers);
    for (uint8_t i = 0; i < numMatchers; ++i) {
        matchers_.emplace_back(file);
    }
    uint16_t numValues = read16(file);
    for (uint16_t i = 0; i < numValues; ++i) {
        values_.insert(read32(file));
    }
    uint16_t numMappings = read16(file);
    for (uint16_t i = 0; i < numMappings; ++i) {
        auto key = readString(file);
        mapping_.emplace(std::move(key), readString(file));
    }
    uint16_t numBareIds = read16(file);
    for (uint16_t i = 0; i < numBareIds; ++i) {
        const uint32_t number = read16(file);
        bareIds_.emplace(number, read32(file));
    }
}

/// Like MARS, a param number given without a table (e.g. 246) that no table-128 param of this context has is
/// read from the only other table of the context holding that number (e.g. 246 at levtype=sfc is 228246).
/// When several other tables hold the number, table 228 takes precedence (e.g. 227 at levtype=sfc is 228227,
/// not 260227); any other ambiguity is left to the default lookup.
void Rule::setBareIds(const eckit::Value& contextIds) {
    std::unordered_set<uint32_t> ids;
    for (size_t i = 0; i < contextIds.size(); ++i) {
        ids.insert(static_cast<uint32_t>(std::stoul(std::string(contextIds[i]))));
    }

    std::map<uint32_t, std::vector<uint32_t>> candidates;
    for (const auto id : ids) {
        const uint32_t table  = id / 1000;
        const uint32_t number = id % 1000;
        if (table != 0 && table != 128 && number != 0) {
            candidates[number].push_back(id);
        }
    }

    for (const auto& [number, tableIds] : candidates) {
        if (ids.find(number) != ids.end()) {
            continue;
        }
        if (tableIds.size() == 1) {
            bareIds_.emplace(number, tableIds.front());
            continue;
        }
        const auto table228 = std::find(tableIds.begin(), tableIds.end(), 228000 + number);
        if (table228 != tableIds.end()) {
            bareIds_.emplace(number, *table228);
        }
    }
}

bool Rule::resolveBareId(std::string& s) const {
    if (s.empty() || s.size() > 3 ||
        !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); })) {
        return false;
    }

    const auto it = bareIds_.find(static_cast<uint32_t>(std::stoul(s)));
    if (it == bareIds_.end()) {
        return false;
    }

    s = std::to_string(it->second);
    return true;
}


bool Rule::match(const metkit::mars::MarsRequest& request, bool partial) const {
    for (std::vector<Matcher>::const_iterator j = matchers_.begin(); j != matchers_.end(); ++j) {
        if (!(*j).match(request, partial)) {
            return false;
        }
    }
    return true;
}

std::string Rule::lookup(const std::string& s) const {

    size_t table = 0;
    size_t param = 0;
    size_t* n    = &param;
    bool numeric = true;

    for (std::string::const_iterator k = s.begin(); k != s.end(); ++k) {
        switch (*k) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                (*n) *= 10;
                (*n) += (*k) - '0';
                break;

            case '.':
                if (n == &param) {
                    n = &table;
                }
                else {
                    numeric = false;
                }
                break;

            default:
                numeric = false;
                break;
        }
    }

    if (numeric && param > 0) {
        if (table == 128) {
            table = 0;
        }

        if (table > 0 && param >= 1000) {
            throw eckit::UserError("Unrecognised format for parameter " + s, Here());
        }

        constexpr size_t maxParamId = std::numeric_limits<uint32_t>::max();
        if (param > maxParamId || table > (maxParamId - param) / 1000) {
            throw eckit::UserError("Cannot match parameter " + s, Here());
        }
        uint32_t pp = static_cast<uint32_t>(table * 1000 + param);

        auto it = values_.find(pp);
        if (it == values_.end()) {
            it = defaultValues_.find(pp);
            if (it == defaultValues_.end()) {
                std::ostringstream ss;
                ss << "Cannot match parameter " << pp;
                throw eckit::UserError(ss.str(), Here());
            }
        }

        std::ostringstream ss;
        ss << pp;
        return ss.str();
    }

    std::string pp = eckit::StringTools::lower(s);

    // not numeric: check the aliases (shortnames) - we do not accept fuzzy matching in the list of params
    auto it = mapping_.find(pp);
    if (it == mapping_.end()) {
        it = defaultMapping_.find(pp);
        if (it == defaultMapping_.end()) {
            throw eckit::UserError("Cannot match parameter " + s, Here());
        }
    }
    return it->second;
}

void Rule::write(std::ofstream& out) const {

    write8(out, matchers_.size());
    for (const auto& m : matchers_) {
        m.write(out);
    }
    write16(out, values_.size());
    for (const auto& v : values_) {
        write32(out, v);
    }
    write16(out, mapping_.size());
    for (const auto& [name, id] : mapping_) {
        writeString(out, name);
        writeString(out, id);
    }
    write16(out, bareIds_.size());
    for (const auto& [number, id] : bareIds_) {
        write16(out, static_cast<uint16_t>(number));
        write32(out, id);
    }
}

void Rule::print(std::ostream& out) const {
    out << "matchers=[";
    std::string sep = "";
    for (const auto& m : matchers_) {
        out << sep << m;
        sep = ",";
    }
    out << "],values=[";
    sep = "";
    for (const auto& v : values_) {
        out << sep << v;
        sep = ",";
    }
    out << "],aliases=[";
    sep = "";
    for (const auto& [s, k] : mapping_) {
        out << sep << s << "->" << k;
        sep = ",";
    }
    out << "]";
    if (!bareIds_.empty()) {
        out << ",bareIds=[";
        sep = "";
        for (const auto& [number, id] : bareIds_) {
            out << sep << number << "->" << id;
            sep = ",";
        }
        out << "]";
    }
}

static std::vector<Rule>* rules = nullptr;

// Rules resolving bare param numbers, one per fully specified context (class, levtype, stream, type), so that at
// most one matches a request. Kept apart from `rules`, whose first match selects the shortname context.
static std::vector<Rule>* bareIdRules = nullptr;

bool isFullContext(const eckit::Value& matchers) {
    static const std::set<std::string> fullContext{"class", "levtype", "stream", "type"};
    const eckit::Value keys = matchers.keys();
    std::set<std::string> names;
    for (size_t i = 0; i < keys.size(); ++i) {
        names.insert(keys[i]);
    }
    return names == fullContext;
}

void addBareIdRules(const eckit::ValueMap& contexts) {
    for (const auto& [matchers, ids] : contexts) {
        if (!isFullContext(matchers)) {
            continue;
        }
        Rule rule{matchers, eckit::Value::makeList(), ParamIdAliases{}};
        rule.setBareIds(ids);
        if (rule.hasBareIds()) {
            bareIdRules->push_back(std::move(rule));
        }
    }
}

}  // namespace

void Rule::init() {

    static bool metkitForceBinfileCreation = eckit::Resource<bool>("$METKIT_FORCE_BINFILE_CREATION", false);
    static bool metkitLegacyParamCheck =
        eckit::Resource<bool>("metkitLegacyParamCheck;$METKIT_LEGACY_PARAM_CHECK", false);
    static bool metkitRawParam = eckit::Resource<bool>("metkitRawParam;$METKIT_RAW_PARAM", false);

    local_mutex = new eckit::Mutex();
    rules       = new std::vector<Rule>();
    bareIdRules = new std::vector<Rule>();

    if (!metkitForceBinfileCreation && !metkitLegacyParamCheck && !metkitRawParam) {
        eckit::PathName paramBinFile = LibMetkit::paramsBinaryFile();
        if (paramBinFile.exists()) {
            std::ifstream file(paramBinFile.localPath(), std::ios::binary);

            try {
                file.exceptions(std::fstream::failbit | std::fstream::badbit | std::fstream::eofbit);
                ASSERT(file.good());  // ensure the file stream is good before reading the header

                std::string header(4, '\0');
                file.read(header.data(), 4);
                uint16_t version = read16(file);

                LOG_DEBUG_LIB(LibMetkit) << "Reading parameter binary file header: " << header
                                         << " version: " << version << std::endl;

                if ("PARA" == header && version == LibMetkit::binaryFilesVersion()) {
                    // read defaultValues_
                    uint32_t numDefaultValues = read32(file);
                    for (uint32_t i = 0; i < numDefaultValues; i++) {
                        defaultValues_.insert(read32(file));
                    }
                    // read defaultMapping_
                    uint32_t numDefaultMappings = read32(file);
                    for (uint32_t i = 0; i < numDefaultMappings; i++) {
                        auto key = readString(file);
                        defaultMapping_.emplace(std::move(key), readString(file));
                    }
                    // read rules
                    uint32_t numRules = read32(file);
                    rules->reserve(numRules);
                    for (uint32_t ruleIdx = 0; ruleIdx < numRules; ruleIdx++) {
                        rules->emplace_back(file);
                    }
                    // read bareIdRules
                    uint32_t numBareIdRules = read32(file);
                    bareIdRules->reserve(numBareIdRules);
                    for (uint32_t ruleIdx = 0; ruleIdx < numBareIdRules; ruleIdx++) {
                        bareIdRules->emplace_back(file);
                    }
                    size_t filesize = file.tellg();     // current position is supposed to be the end of the file
                    file.seekg(0, std::ios_base::end);  // go to end of the file
                    size_t endpos = file.tellg();

                    if (!(filesize == endpos)) {
                        std::ostringstream ss;
                        ss << "Error reading parameter binary file: " << paramBinFile << " - File not fully read";
                        throw eckit::SeriousBug(ss.str(), Here());
                    }
                    file.close();

                    return;
                }

                eckit::Log::error() << "Incompatible version of parameter binary file '" << paramBinFile.asString()
                                    << "' - version expected: " << LibMetkit::binaryFilesVersion()
                                    << " found: " << version << " - using slow config file parsing" << std::endl;
            }
            catch (const std::exception& e) {
                defaultMapping_.clear();
                defaultMapping_.clear();
                rules->clear();
                bareIdRules->clear();
                eckit::Log::error() << "Error reading parameter binary file '" << paramBinFile.asString()
                                    << "': " << e.what() << " - using slow config file parsing" << std::endl;
            }
        }
        else {
            eckit::Log::error() << "Parameter binary file '" << paramBinFile.asString()
                                << "' missing - using slow config file parsing" << std::endl;
        }
    }

    const eckit::Value rawIds = eckit::YAMLParser::decodeFile(LibMetkit::paramIDYamlFile());
    ASSERT(rawIds.isOrderedMap());
    auto keys = rawIds.keys();
    ParamIdAliases ids;
    ASSERT(keys.isList());
    for (size_t i = 0; i < keys.size(); ++i) {
        uint32_t id     = keys[i];
        auto rawAliases = rawIds[keys[i]];
        std::vector<std::string> aliases;
        for (size_t j = 0; j < rawAliases.size(); j++) {
            std::string alias = rawAliases[j];
            // add only shortnames - ignore 2nd alias (descriptions) and strings too long or containing blanks
            if (j != 1 && alias.size() < 20 && alias.find(" ") == std::string::npos) {
                aliases.push_back(alias);
            }
        }
        ids.emplace(id, aliases);
    }

    eckit::ValueMap merge;

    if (metkitLegacyParamCheck || (!metkitRawParam)) {
        eckit::Value r = eckit::YAMLParser::decodeFile(LibMetkit::paramYamlFile());
        ASSERT(r.isList());

        const eckit::Value rs = eckit::YAMLParser::decodeFile(LibMetkit::paramStaticYamlFile());
        ASSERT(rs.isList());

        // merge r and rs
        for (size_t i = 0; i < r.size(); ++i) {
            const eckit::Value& rule = r[i];

            if (!rule.isList()) {
                rule.dump(Log::error()) << std::endl;
            }
            ASSERT(rule.isList());
            ASSERT(rule.size() == 2);

            merge.emplace(rule[0], rule[1]);
        }

        for (size_t i = 0; i < rs.size(); ++i) {
            const eckit::Value& rule = rs[i];

            if (!rule.isList()) {
                rule.dump(Log::error()) << std::endl;
            }
            ASSERT(rule.isList());
            ASSERT(rule.size() == 2);

            auto it = merge.find(rule[0]);
            if (it == merge.end()) {
                merge.emplace(rule[0], rule[1]);
            }
            else {
                it->second += rule[1];
            }
        }
    }

    if (metkitLegacyParamCheck) {
        for (auto it = merge.begin(); it != merge.end(); it++) {
            (*rules).push_back(Rule(it->first, it->second, ids));
        }
        addBareIdRules(merge);
        return;
    }

    Rule::setDefault(keys, ids);

    if (metkitRawParam) {
        // empty rule, to enable default
        (*rules).push_back(Rule(eckit::Value::makeMap(), eckit::Value::makeList(), ParamIdAliases{}));
        return;
    }

    std::set<std::string> shortnames;
    std::set<std::string> associatedIDs;

    const eckit::Value pc = eckit::YAMLParser::decodeFile(LibMetkit::shortnameContextYamlFile());
    ASSERT(pc.isList());

    for (size_t i = 0; i < pc.size(); i++) {
        shortnames.emplace(pc[i]);
    }

    for (size_t i = 0; i < keys.size(); i++) {
        auto el = rawIds.element(keys[i]);
        for (size_t j = 0; j < el.size(); j++) {
            if (shortnames.find(el[j]) != shortnames.end()) {
                associatedIDs.emplace(keys[i]);
            }
        }
    }

    for (auto it = merge.begin(); it != merge.end(); it++) {
        auto listIDs = eckit::Value::makeList();

        for (size_t j = 0; j < it->second.size(); j++) {
            if (associatedIDs.find(it->second[j]) != associatedIDs.end()) {
                listIDs.append(it->second[j]);
            }
        }
        if (listIDs.size() > 0) {
            (*rules).push_back(Rule{it->first, listIDs, ids});
        }
    }

    (*rules).push_back(Rule{eckit::Value::makeMap(), eckit::Value::makeList(), ParamIdAliases{}});

    addBareIdRules(merge);

    if (metkitForceBinfileCreation && !metkitLegacyParamCheck && !metkitRawParam) {  // creating the binary file
        eckit::PathName paramBinFile = LibMetkit::paramsBinaryFile();
        if (!paramBinFile.exists()) {

            // serialise across processes: only one writer generates params.bin at a time
            eckit::FileLock lock(paramBinFile.asString() + ".lock");
            eckit::AutoLock<eckit::FileLock> locker(lock);

            if (!paramBinFile.exists()) {  // another process may have written it while we were waiting for the lock

                // write to a sibling temp file and rename, so readers never see a partial file
                auto tmpFile = eckit::PathName::unique(paramBinFile.asString());
                {
                    std::ofstream file{tmpFile.localPath(), std::ios::binary};

                    try {
                        file.exceptions(std::fstream::failbit | std::fstream::badbit | std::fstream::eofbit);

                        const char* header = "PARA";
                        file.write(header, 4);
                        write16(file, LibMetkit::binaryFilesVersion());

                        write32(file, Rule::defaultValues_.size());
                        for (auto v : defaultValues_) {
                            write32(file, v);
                        }
                        write32(file, Rule::defaultMapping_.size());
                        for (const auto& [name, id] : defaultMapping_) {
                            writeString(file, name);
                            writeString(file, id);
                        }
                        write32(file, rules->size());
                        for (const auto& r : *rules) {
                            r.write(file);
                        }
                        write32(file, bareIdRules->size());
                        for (const auto& r : *bareIdRules) {
                            r.write(file);
                        }
                        file.flush();
                        file.close();
                        eckit::PathName::rename(tmpFile, paramBinFile);
                    }
                    catch (const std::exception& e) {
                        std::ostringstream ss;
                        ss << "Error writing " << paramBinFile << ": " << e.what() << std::endl;
                        throw eckit::SeriousBug(ss.str(), Here());
                    }
                }
            }
        }
    }
}

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeParam::TypeParam(const std::string& name, const eckit::Value& settings) : Type(name, settings), firstRule_(false) {

    if (settings.contains("first_rule")) {
        firstRule_ = settings["first_rule"];
    }
}

void TypeParam::print(std::ostream& out) const {
    out << "TypeParam[name=" << name_ << "]";
}

void TypeParam::pass2(MarsRequest& request) const {

    pthread_once(&once, initRules);

    const Rule* rule                = 0;
    std::vector<std::string> values = request.values(name_, true);

    if (values.size() == 1 && values[0] == "all") {
        return;
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    for (const auto& r : *rules) {
        if (r.match(request)) {
            rule = &r;
            break;
        }
    }

    if (!rule) {
        Log::warning() << "TypeParam: cannot find a context to expand 'param' in " << request << std::endl;

        if (firstRule_) {
            bool found = false;
            for (const auto& r : *rules) {
                if (r.match(request, true)) {
                    for (std::vector<std::string>::iterator j = values.begin(); j != values.end() && !rule; ++j) {
                        std::string& s = (*j);
                        try {
                            s    = r.lookup(s);
                            rule = &r;
                            Log::warning() << "TypeParam: using 'first matching rule' option " << r << std::endl;
                        }
                        catch (...) {
                        }
                    }
                }
            }
        }
        if (!rule) {
            std::ostringstream oss;
            oss << "TypeParam: cannot find a context to expand 'param' in " << request;
            throw eckit::SeriousBug(oss.str());
        }
    }


    const Rule* bareIdRule = nullptr;
    for (const auto& r : *bareIdRules) {
        if (r.match(request)) {
            bareIdRule = &r;
            break;
        }
    }

    for (std::vector<std::string>::iterator j = values.begin(); j != values.end(); ++j) {
        std::string& s = (*j);
        if (bareIdRule && bareIdRule->resolveBareId(s)) {
            continue;
        }
        try {
            s = rule->lookup(s);
        }
        catch (...) {
            Log::error() << *rule << std::endl;
            throw;
        }
    }

    request.setValuesTyped(this, values);
}

bool TypeParam::expand(std::string&, const MarsRequest&) const {
    // Work done on pass2()
    return true;
}

static TypeBuilder<TypeParam> type("param");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
