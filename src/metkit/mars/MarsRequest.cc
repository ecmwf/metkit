/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/types/Types.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"
#include "eckit/value/Content.h"
#include "eckit/value/Value.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/ParamID.h"
#include "metkit/mars/Parameter.h"


namespace metkit::mars {

MarsRequest::MarsRequest(Verb verb) : verb_(MarsLanguage::name(verb)) {}

MarsRequest::MarsRequest(const std::string& verb) : verb_(verb) {}

MarsRequest::MarsRequest(const std::string& verb, const std::map<std::string, std::string>& vals) : verb_(verb) {
    for (const auto& [k, v] : vals) {
        values(k, std::vector<std::string>(1, v));
    }
}

MarsRequest::MarsRequest(const std::string& verb, const eckit::Value& vals) : MarsRequest(verb) {
    eckit::ValueMap vv = vals;
    for (const auto& [param, value] : vv) {
        std::string name = param;
        if (value.isList()) {
            std::vector<std::string> vals;
            eckit::fromValue(vals, value);
            values(name, vals);
        }
        else {
            values(name, std::vector<std::string>(1, value));
        }
    }
}

MarsRequest::MarsRequest(eckit::Stream& s, bool validate, bool lowercase) {
    int size;

    s >> verb_;
    if (lowercase) {
        verb_ = eckit::StringTools::lower(verb_);
    }

    s >> size;
    ASSERT(size >= 0);
    params_.reserve(size);
    for (int i = 0; i < size; i++) {
        std::string param;
        int count;

        s >> param;
        if (lowercase) {
            param = eckit::StringTools::lower(param);
        }
        s >> count;
        ASSERT(count >= 0);

        std::vector<std::string> v;
        v.reserve(count);

        for (int k = 0; k < count; k++) {
            std::string value;
            s >> value;
            v.push_back(std::move(value));
        }

        // a repeated parameter overrides the previous one
        values(param, v);
    }

    if (validate) {
        // every parameter must be a keyword of the language of the verb (UserError otherwise)
        const MarsLanguage& language = MarsLanguage::get(verb_);
        for (auto& p : params_) {
            p = Parameter(language.type(MarsLanguage::keyword(p.name()))->getptr(), p.values());
        }
    }
}

Verb MarsRequest::verbId() const {
    return MarsLanguage::verb(verb_);
}

void MarsRequest::verb(Verb id) {
    verb_ = MarsLanguage::name(id);
}

// ---------------------------------------------------------------------------------------------------------------------
// Parameters lookup

size_t MarsRequest::locate(Keyword key) const {
    if (key) {
        const std::string* name = nullptr;  // the name of the keyword, needed to compare with untyped parameters only
        for (size_t i = 0; i < params_.size(); ++i) {
            const Parameter& p = params_[i];
            if (p.typed()) {
                if (p.id() == key) {
                    return i;
                }
            }
            else {
                if (!name) {
                    name = &MarsLanguage::name(key);
                }
                if (p.name() == *name) {
                    return i;
                }
            }
        }
    }
    return params_.size();
}

size_t MarsRequest::locate(const std::string& name) const {
    auto position = [this](const std::string& n) {
        for (size_t i = 0; i < params_.size(); ++i) {
            if (params_[i].name() == n) {
                return i;
            }
        }
        return params_.size();
    };

    size_t i = position(name);
    if (i == params_.size()) {
        // the names of registered keywords are case insensitive, and typed parameters hold them in lowercase
        std::string lower = eckit::StringTools::lower(name);
        if (lower != name) {
            i = position(lower);
        }
    }
    return i;
}

const Parameter* MarsRequest::find(Keyword key) const {
    size_t i = locate(key);
    return i < params_.size() ? &params_[i] : nullptr;
}

const Parameter* MarsRequest::find(const std::string& name) const {
    size_t i = locate(name);
    return i < params_.size() ? &params_[i] : nullptr;
}

size_t MarsRequest::countValues(Keyword key) const {
    const Parameter* p = find(key);
    return p ? p->count() : 0;
}
size_t MarsRequest::countValues(const std::string& name) const {
    const Parameter* p = find(name);
    return p ? p->count() : 0;
}

bool MarsRequest::has(Keyword key) const {
    return locate(key) < params_.size();
}
bool MarsRequest::has(const std::string& name) const {
    return locate(name) < params_.size();
}

const std::vector<std::string>& MarsRequest::values(Keyword key, bool emptyOk) const {
    if (const Parameter* p = find(key)) {
        return p->values();
    }
    if (emptyOk) {
        static const std::vector<std::string> empty;
        return empty;
    }
    throw eckit::UserError("No parameter called '" + (key ? MarsLanguage::name(key) : std::string{}) + "'");
}
const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const {
    if (const Parameter* p = find(name)) {
        return p->values();
    }
    if (emptyOk) {
        static const std::vector<std::string> empty;
        return empty;
    }
    throw eckit::UserError("No parameter called '" + name + "'");
}

void MarsRequest::values(Keyword key, const std::vector<std::string>& vals) {
    ASSERT(key);
    size_t i = locate(key);
    if (i < params_.size()) {
        params_[i].values(vals);
    }
    else {
        params_.emplace_back(MarsLanguage::name(key), vals);
    }
}
void MarsRequest::values(const std::string& name, const std::vector<std::string>& vals) {
    size_t i = locate(name);
    if (i < params_.size()) {
        params_[i].values(vals);
    }
    else {
        params_.emplace_back(name, vals);
    }
}

void MarsRequest::erase(Keyword key) {
    size_t i = locate(key);
    if (i < params_.size()) {
        params_.erase(params_.begin() + i);
    }
}
void MarsRequest::erase(const std::string& name) {
    size_t i = locate(name);
    if (i < params_.size()) {
        params_.erase(params_.begin() + i);
    }
}

void MarsRequest::setValuesTyped(std::shared_ptr<const Type> type, const std::vector<std::string>& values) {
    size_t i = locate(type->id());
    if (i < params_.size()) {
        params_[i] = Parameter(std::move(type), values);
    }
    else {
        params_.emplace_back(std::move(type), values);
    }
}

const std::string& MarsRequest::operator[](const std::string& name) const {
    auto p = find(name);
    if (!p) {
        std::ostringstream oss;
        oss << "Parameter '" << name << "' is undefined";
        throw eckit::UserError(oss.str());
    }
    const std::vector<std::string>& c = p->values();
    if (c.size() > 1) {
        std::ostringstream oss;
        oss << "Parameter '" << name << "' has more than one value";
        throw eckit::UserError(oss.str());
    }

    return c[0];
}

std::optional<std::reference_wrapper<const std::vector<std::string>>> MarsRequest::get(
    const std::string& keyword) const {
    const Parameter* p = find(keyword);
    if (!p) {
        return std::nullopt;
    }
    return std::cref(p->values());
}

std::vector<std::string> MarsRequest::params() const {
    std::vector<std::string> out;
    for (const auto& p : parameters()) {
        out.push_back(p.name());
    }
    return out;
}

void MarsRequest::encode(eckit::Stream& s) const {
    s << verb();

    const auto& params = parameters();
    int size           = params.size();
    s << size;

    for (const auto& p : params) {
        s << p.name();

        const std::vector<std::string>& vv = p.values();
        int size                           = vv.size();  // For backward compatibility
        s << size;

        for (const auto& v : vv) {
            s << v;
        }
    }
}

void MarsRequest::print(std::ostream& s) const {
    dump(s, "", "", true);
}

void MarsRequest::dump(std::ostream& s, const char* cr, const char* tab, bool printVerb) const {
    if (printVerb) {
        s << verb() << ',';
    }
    std::string separator = "";
    if (!parameters().empty()) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (const auto& p : parameters()) {
            if (a++) {
                s << ',' << cr << tab;
            }

            int b = 0;
            s << p.name() << "=";

            for (const auto& v : p.values()) {
                if (b++) {
                    s << '/';
                }
                MarsParser::quoted(s, v);
            }
        }
    }

    s << cr << cr;
}

void MarsRequest::setValuesTyped(const Type* type, const std::vector<std::string>& values) {
    // `type` may already be owned by a shared_ptr (e.g. Type::finalise() passing `this` for a
    // Type registered in MarsLanguage's type registry), or may be a freshly-constructed object
    // with no owner yet (e.g. `request.setValuesTyped(new TypeAny(name), values)`, a supported
    // external API used by fdb5). Reuse existing ownership when present - getptr()/
    // shared_from_this() would throw std::bad_weak_ptr otherwise - and adopt it as a new
    // shared_ptr only when it truly has no owner yet.
    std::shared_ptr<const Type> owned = type->weak_from_this().lock();
    if (!owned) {
        owned = std::shared_ptr<const Type>(type);
    }
    setValuesTyped(std::move(owned), values);
}

void MarsRequest::json(eckit::JSON& s, bool array) const {
    s.startObject();
    for (const auto& p : parameters()) {
        s << p.name();

        const std::vector<std::string>& vv = p.values();

        bool list = vv.size() != 1 || (array && p.multiple());
        if (list) {
            s.startList();
        }
        for (const auto& v : vv) {
            s << v;
        }
        if (list) {
            s.endList();
        }
    }
    s.endObject();
}

size_t MarsRequest::count() const {
    size_t result = 1;

    const std::set<std::string>& paramIdsSingleLevel = ParamID::getMlParamsSingleLevel();

    bool ml                  = false;
    size_t levels            = 1;
    size_t params            = 0;
    size_t paramsSingleLevel = 0;
    bool hasLevelOne         = false;

    for (const auto& p : params_) {
        const auto& name   = p.name();
        const auto& values = p.values();
        if (values.size() == 1 && (values.at(0) == "all" || values.at(0) == "any")) {
            return 0;
        }
        if (name == "levelist") {
            levels = values.size();
            // For a parameter matching paramIdsSingleLevel, we can only include one value, and for level=1
            hasLevelOne = std::find(values.begin(), values.end(), "1") != values.end();
        }
        else {
            if (name == "param") {
                for (const auto& v : values) {
                    paramsSingleLevel += paramIdsSingleLevel.count(v);
                }
                params = values.size() - paramsSingleLevel;
            }
            else {
                if (name == "levtype") {
                    ml = std::find(values.begin(), values.end(), "ml") != values.end();
                }
                result *= p.count();
            }
        }
    }
    if ((params + paramsSingleLevel) > 0) {
        if (ml) {
            result *= (levels * params + (hasLevelOne ? paramsSingleLevel : 0));
        }
        else {
            result *= (levels * (params + paramsSingleLevel));
        }
    }
    return result;
}

// recursively expand along keys in expvalues
static void expand_along_keys(const MarsRequest& prototype,
                              const std::vector<std::pair<std::string, std::vector<std::string>>>& expvalues,
                              std::vector<MarsRequest>& requests, size_t i) {

    if (i == expvalues.size()) {
        requests.push_back(prototype);
        return;
    }

    const auto& [key, values] = expvalues[i];  // [] -> .at() ?

    MarsRequest req(prototype);
    for (const auto& value : values) {
        req.setValue(key, value);
        expand_along_keys(req, expvalues, requests, i + 1);
    }
}

std::vector<MarsRequest> MarsRequest::split(const std::vector<std::string>& keys) const {

    size_t n = 1;

    LOG_DEBUG_LIB(LibMetkit) << "Splitting request with keys" << keys << std::endl;

    std::vector<std::pair<std::string, std::vector<std::string>>> expvalues;
    for (auto& key : keys) {
        std::vector<std::string> v = values(key, true);  // ok to be empty
        LOG_DEBUG_LIB(LibMetkit) << "splitting along key " << key << " n values " << v.size() << " values " << v
                                 << std::endl;
        if (v.empty()) {
            continue;
        }
        n *= v.size();
        expvalues.emplace_back(key, v);
    }

    std::vector<MarsRequest> requests;
    requests.reserve(n);

    if (n == 1) {
        requests.push_back(*this);
        return requests;
    }

    expand_along_keys(*this, expvalues, requests, 0);

    return requests;
}

MarsRequest MarsRequest::subset(const std::set<std::string>& keys) const {
    MarsRequest req(verb());
    for (const auto& p : parameters()) {
        if (keys.find(p.name()) != keys.end()) {
            req.params_.push_back(p);
        }
    }
    return req;
}

void MarsRequest::md5(eckit::MD5& md5) const {
    std::ostringstream oss;
    oss << *this;
    md5.add(oss.str());
}

bool MarsRequest::operator<(const MarsRequest& other) const {
    if (verb() != other.verb()) {
        return verb() < other.verb();
    }
    return parameters() < other.parameters();
}

std::string MarsRequest::asString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}
//----------------------------------------------------------------------------------------------------------------------

std::vector<MarsRequest> MarsRequest::parse(std::istream& in, bool strict) {
    MarsParser parser(in);
    MarsExpansion expand(true, strict);
    return expand.expand(parser.parse());
}

MarsRequest MarsRequest::parse(const std::string& s, bool strict) {
    std::istringstream in(s);
    auto v = parse(in, strict);
    ASSERT(v.size() == 1);
    return v[0];
}

//----------------------------------------------------------------------------------------------------------------------

bool MarsRequest::filter(const MarsRequest& filter) {
    for (auto& p : params_) {
        if (p.name() == "date") {
            if (const Parameter* fp = filter.find("day")) {
                if (!p.filter("day", fp->values())) {
                    return false;
                }
            }
        }

        const Parameter* fp = filter.find(p.name());
        if (!fp) {
            continue;
        }

        if (!p.filter(fp->values())) {
            return false;
        }
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest& other) const {
    for (const auto& p : other.parameters()) {
        const Parameter* mp = find(p.name());
        if (!mp) {
            return false;
        }

        if (!mp->matches(p.values())) {
            return false;
        }
    }
    return true;
}

void MarsRequest::merge(const MarsRequest& other) {
    for (auto& param : params_) {
        LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << param.name() << std::endl;
        if (const Parameter* p = other.find(param.name())) {
            param.merge(*p);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
