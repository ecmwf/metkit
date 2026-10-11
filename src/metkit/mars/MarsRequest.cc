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
#include <utility>

#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/types/Types.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"

#include "eckit/message/Message.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/ParamID.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypeAny.h"

namespace metkit::mars {

namespace {

constexpr size_t DATA     = 0;
constexpr size_t POSTPROC = 1;
constexpr size_t OTHER    = 2;

/// The axes are all of category Data, and they are the most common keywords: they do not need a look at the type
bool isData(Keyword key) {
    return key > 0 && key < MarsLanguage::maxDataKeyword();
}

size_t containerOf(const Parameter& p) {
    if (p.typed()) {
        switch (p.type().category()) {
            case Category::Data:
                return DATA;
            case Category::PostProc:
                return POSTPROC;
            default:
                break;
        }
    }
    return OTHER;
}

}  // namespace

MarsDataCube::MarsDataCube(Verb verb) : verb_(verb) {}

Verb MarsDataCube::verbId() const {
    return verb_;
}

void MarsDataCube::verb(Verb id) { verb_ = id; }

const std::vector<std::string>& MarsDataCube::get(Keyword key) const {
    static const std::vector<std::string> empty;
    if (key && key < MarsLanguage::maxDataKeyword()) {
        for (const auto& p : data_) {
            if (p.id() == key) {
                return p.values();
            }
        }
    }
    return empty;
}

void MarsDataCube::set(Keyword key, const std::vector<std::string>& value) {
    if (key && key < MarsLanguage::maxDataKeyword()) {
        for (auto& p : data_) {
            if (p.id() == key) {
                p.values(value);
                return;
            }
        }
        if (!lang_ || lang_->verb() != verb_) {
            ASSERT(verb_);
            lang_ = &MarsLanguage::get(verb_);
        }
        data_.emplace_back(lang_->type(key), value);
    }
}
void MarsDataCube::set(Keyword key, std::vector<std::string>&& value) {
    if (key && key < MarsLanguage::maxDataKeyword()) {
        for (auto& p : data_) {
            if (p.id() == key) {
                p.values(std::move(value));
                return;
            }
        }
        if (!lang_ || lang_->verb() != verb_) {
            ASSERT(verb_);
            lang_ = &MarsLanguage::get(verb_);
        }
        data_.emplace_back(lang_->type(key), std::move(value));
    }
}

void MarsDataCube::unset(Keyword key) {
    if (key && key < MarsLanguage::maxDataKeyword()) {
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            if (it->id() == key) {
                data_.erase(it);
                break;
            }
        }
    }
}

bool MarsDataCube::has(Keyword key) const {
    if (key && key < MarsLanguage::maxDataKeyword()) {
        for (const auto& p : data_) {
            if (p.id() == key) {
                return true;
            }
        }
    }
    return false;
}

// void MarsDataCube::add(const MarsID& id) {
//     for (const auto& p : id.data()) {
//         set(p.id(), p.values());
//     }
// }

void MarsDataCube::merge(const MarsDataCube& other) {
    std::map<Keyword, const Parameter*> otherMap;
    for (const auto& p : other.data_) {
        otherMap[p.id()] = &p;
    }
    for (auto& p : data_) {
        auto it = otherMap.find(p.id());
        if (it != otherMap.end()) {
            p.merge(*it->second);
            otherMap.erase(it);
        }
    }
    // Add remaining items from otherMap that were not in data_
    for (const auto& [key, param] : otherMap) {
        data_.emplace_back(*param);
    }
}

size_t MarsDataCube::count() const {
    static const Keyword LEVELIST = MarsLanguage::keyword("levelist");
    static const Keyword PARAM = MarsLanguage::keyword("param");
    static const Keyword LEVTYPE = MarsLanguage::keyword("levtype");

    size_t result = 1;

    const std::set<std::string>& paramIdsSingleLevel = ParamID::getMlParamsSingleLevel();

    bool ml                  = false;
    size_t levels            = 1;
    size_t params            = 0;
    size_t paramsSingleLevel = 0;
    bool hasLevelOne         = false;

    for (const auto& p : data_) {
        Keyword key = p.id();
        ASSERT(key);
        const auto& values = p.values();
        if (values.size() == 1 && (values.at(0) == "all" || values.at(0) == "any")) {
            return 0;
        }
        if (key == LEVELIST) {
            levels = values.size();
            // For a parameter matching paramIdsSingleLevel, we can only include one value, and for level=1
            hasLevelOne = std::find(values.begin(), values.end(), "1") != values.end();
        }
        else {
            if (key == PARAM) {
                for (const auto& v : values) {
                    paramsSingleLevel += paramIdsSingleLevel.count(v);
                }
                params = values.size() - paramsSingleLevel;
            }
            else {
                if (key == LEVTYPE) {
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

//----------------------------------------------------------------------------------------------------------------------

MarsRequest::MarsRequest() : MarsRequest("retrieve") {}

MarsRequest::MarsRequest(Verb verb) : MarsDataCube(verb), verbName_(MarsLanguage::name(verb)) {}

MarsRequest::MarsRequest(const std::string& verb) : MarsDataCube(MarsLanguage::hasVerb(verb)), verbName_(verb) {}

MarsRequest::MarsRequest(const std::string& verb, const std::map<std::string, std::string>& vals) : MarsRequest(verb) {

    for (const auto& [k, v] : vals) {
        values(k, std::vector<std::string>(1, v));
    }
}

MarsRequest::MarsRequest(const std::string& s, const eckit::Value& vals) : MarsRequest(s) {
    eckit::ValueMap valueMap = vals;
    for (const auto& [param, value] : valueMap) {
        std::string name = param;
        if (value.isList()) {
            std::vector<std::string> vv;
            eckit::fromValue(vv, value);
            values(name, vv);
        }
        else {
            values(name, std::vector<std::string>(1, value));
        }
    }
}

MarsRequest::MarsRequest(eckit::Stream& s, bool validate, bool lowercase) : MarsDataCube(0) {
    int size;

    s >> verbName_;
    if (lowercase) {
        verbName_ = eckit::StringTools::lower(verbName_);
    }
    MarsDataCube::verb(MarsLanguage::hasVerb(verbName_));

    s >> size;
    ASSERT(size >= 0);
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
        lang_ = &MarsLanguage::get(verb_);
        std::vector<Parameter> typed;
        for (const auto& p : other_) {
            typed.emplace_back(lang_->type(MarsLanguage::keyword(p.name()))->getptr(), p.values());
        }
        other_.clear();
        for (auto& p : typed) {
            add(std::move(p));
        }
    }
}

Verb MarsRequest::verbId() const {
    if (verb_) {
        return verb_;
    }
    throw eckit::UserError("Unknown verb " + verbName_);
}

void MarsRequest::verb(Verb id) {
    MarsDataCube::verb(id);
    verbName_ = MarsLanguage::name(id);
}
void MarsRequest::verb(const std::string& name) {
    verbName_ = name;
    MarsDataCube::verb(MarsLanguage::hasVerb(name));
}

// ---------------------------------------------------------------------------------------------------------------------
// Parameters lookup

const std::vector<Parameter>& MarsRequest::container(size_t c) const {
    switch (c) {
        case DATA:
            return data_;
        case POSTPROC:
            return postproc_;
        default:
            ASSERT(c == OTHER);
            return other_;
    }
}

std::vector<Parameter>& MarsRequest::container(size_t c) {
    return const_cast<std::vector<Parameter>&>(std::as_const(*this).container(c));
}

void MarsRequest::add(Parameter&& p) {
    container(containerOf(p)).push_back(std::move(p));
}

MarsRequest::Position MarsRequest::locate(Keyword key) const {
    if (!key) {
        return {};
    }

    const std::string* name = nullptr;  // the name of the keyword, needed to compare with untyped parameters only
    auto search             = [&](size_t c) -> Position {
        const auto& params = container(c);
        for (size_t i = 0; i < params.size(); ++i) {
            const Parameter& p = params[i];
            if (p.typed()) {
                if (p.id() == key) {
                    return {c, i};
                }
            }
            else {
                if (!name) {
                    name = &MarsLanguage::name(key);
                }
                if (p.name() == *name) {
                    return {c, i};
                }
            }
        }
        return {};
    };

    if (isData(key)) {
        return search(DATA);
    }
    for (size_t c = 0; c <= OTHER; ++c) {
        if (Position pos = search(c)) {
            return pos;
        }
    }
    return {};
}

MarsRequest::Position MarsRequest::locate(const std::string& name) const {
    auto position = [this](const std::string& n) -> Position {
        for (size_t c = 0; c <= OTHER; ++c) {
            const auto& params = container(c);
            for (size_t i = 0; i < params.size(); ++i) {
                if (params[i].name() == n) {
                    return {c, i};
                }
            }
        }
        return {};
    };

    Position pos = position(name);
    if (!pos) {
        // the names of registered keywords are case insensitive, and typed parameters hold them in lowercase
        std::string lower = eckit::StringTools::lower(name);
        if (lower != name) {
            pos = position(lower);
        }
    }
    return pos;
}

const Parameter* MarsRequest::find(Keyword key) const {
    Position pos = locate(key);
    return pos ? &container(pos.container)[pos.index] : nullptr;
}

const Parameter* MarsRequest::find(const std::string& name) const {
    Position pos = locate(name);
    return pos ? &container(pos.container)[pos.index] : nullptr;
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
    return static_cast<bool>(locate(key));
}
bool MarsRequest::has(const std::string& name) const {
    return static_cast<bool>(locate(name));
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
    if (Position pos = locate(key)) {
        container(pos.container)[pos.index].values(vals);
    }
    else {
        add(Parameter(MarsLanguage::get(verb_).type(key)->getptr(), vals));
    }
}
void MarsRequest::values(const std::string& name, const std::vector<std::string>& vals) {
    if (Position pos = locate(name)) {
        container(pos.container)[pos.index].values(vals);
    }
    else {
        // the keyword tells whether it is an axis, which is held with the data parameters even if untyped
        Keyword key = MarsLanguage::hasKeyword(name);
        if (key) {
            add(Parameter(MarsLanguage::get(verb_).type(key)->getptr(), vals));
        }
        else {
            add(Parameter(name, vals));
        }
    }
}
void MarsRequest::values(Keyword key, std::vector<std::string>&& vals) {
    ASSERT(key);
    if (Position pos = locate(key)) {
        container(pos.container)[pos.index].values(std::move(vals));
    }
    else {
        add(Parameter(MarsLanguage::get(verb_).type(key), std::move(vals)));
    }
}
void MarsRequest::values(const std::string& name, std::vector<std::string>&& vals) {
    Keyword key = MarsLanguage::hasKeyword(name);
    if (key) {
        values(key, std::move(vals));
        return;
    }

    // no keyword exists for this name - use the untyped parameter handling
    if (Position pos = locate(name)) {
        container(pos.container)[pos.index].values(std::move(vals));
    }
    else {
        // the keyword tells whether it is an axis, which is held with the data parameters even if untyped
        add(Parameter(name, std::move(vals)));
    }
}

void MarsRequest::erase(Keyword key) {
    if (Position pos = locate(key)) {
        auto& params = container(pos.container);
        params.erase(params.begin() + pos.index);
    }
}
void MarsRequest::erase(const std::string& name) {
    if (Position pos = locate(name)) {
        auto& params = container(pos.container);
        params.erase(params.begin() + pos.index);
    }
}

void MarsRequest::setValuesTyped(std::shared_ptr<const Type> type, const std::vector<std::string>& values) {
    Position pos = locate(type->id());
    Parameter p(std::move(type), values);
    if (pos && pos.container == containerOf(p)) {
        container(pos.container)[pos.index] = std::move(p);
        return;
    }
    // the category changes when an untyped parameter becomes typed
    if (pos) {
        auto& params = container(pos.container);
        params.erase(params.begin() + pos.index);
    }
    add(std::move(p));
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
    for (const auto& p : *this) {
        out.push_back(p.name());
    }
    return out;
}

void MarsRequest::encode(eckit::Stream& s) const {
    s << verb();

    int numParams = size();
    s << numParams;

    for (const auto& p : *this) {
        s << p.name();

        const std::vector<std::string>& vv = p.values();
        int numValues                      = vv.size();  // For backward compatibility
        s << numValues;

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
    if (size()) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (const auto& p : *this) {
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
    for (const auto& p : *this) {
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

// recursively expand along keys in expvalues
void expand_along_keys(const MarsRequest& prototype,
                       const std::vector<std::pair<std::string, std::vector<std::string>>>& expvalues,
                       std::vector<MarsRequest>& requests, size_t i) {

    if (i == expvalues.size()) {
        requests.push_back(prototype);
        return;
    }

    const std::string& key                 = expvalues[i].first;
    const std::vector<std::string>& values = expvalues[i].second;

    MarsRequest req(prototype);
    for (auto& value : values) {
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
        if (v.empty())
            continue;
        n *= v.size();
        expvalues.emplace_back(std::make_pair(key, v));
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
    for (size_t c = 0; c <= OTHER; ++c) {
        for (const auto& p : container(c)) {
            if (keys.find(p.name()) != keys.end()) {
                req.container(c).push_back(p);
            }
        }
    }
    return req;
}

MarsRequest MarsRequest::extract(const std::string& category) const {
    Category cat = category == "data"       ? Category::Data
                   : category == "derived"  ? Category::Derived
                   : category == "postproc" ? Category::PostProc
                   : category == "sink"     ? Category::Sink
                                            : Category::None;

    MarsRequest req(verb_);
    for (size_t c = 0; c <= OTHER; ++c) {
        for (const auto& p : container(c)) {
            if (p.type().category() == cat) {
                req.container(c).push_back(p);
            }
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
    return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
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
    for (auto* params : {&data_, &postproc_, &other_}) {
        for (auto& p : *params) {
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
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest& matches) const {
    for (const auto& p : matches) {
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
    for (auto* params : {&data_, &postproc_, &other_}) {
        for (auto& param : *params) {
            LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << param.name() << std::endl;
            if (const Parameter* p = other.find(param.name())) {
                param.merge(*p);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
