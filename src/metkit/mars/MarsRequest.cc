/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
#include "metkit/mars/TypeAny.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

MarsRequest::MarsRequest() : verb_(MarsLanguage::verb("retrieve")) {
    MarsLanguage::get(verb_);
}

MarsRequest::MarsRequest(Verb verb) : verb_(verb) {
    MarsLanguage::get(verb_);
}

MarsRequest::MarsRequest(const std::string& s) {
    ASSERT(s.find(',') == std::string::npos);
    verb_ = MarsLanguage::verb(s);
    MarsLanguage::get(verb_);
}

MarsRequest::MarsRequest(const std::string& s, const std::map<std::string, std::string>& values) {
    verb_ = MarsLanguage::verb(s);
    MarsLanguage::get(verb_);
    for (auto j = values.begin(); j != values.end(); ++j) {
        Keyword param = MarsLanguage::keyword((*j).first);
        const std::string& value = (*j).second;

        params_.push_back(TypeParameter(std::vector<std::string>(1, value), new TypeAny(param)));
    }
}


MarsRequest::MarsRequest(const std::string& s, const eckit::Value& values) {
    verb_ = MarsLanguage::verb(s);
    MarsLanguage::get(verb_);
    eckit::ValueMap m = values;
    for (auto j = m.begin(); j != m.end(); ++j) {
        Keyword param  = MarsLanguage::keyword((*j).first);
        const eckit::Value& value = (*j).second;

        if (value.isList()) {
            std::vector<std::string> vals;
            eckit::fromValue(vals, value);
            params_.push_back(TypeParameter(vals, new TypeAny(param)));
        }
        else {
            params_.push_back(TypeParameter(std::vector<std::string>(1, value), new TypeAny(param)));
        }
    }
}

MarsRequest::MarsRequest(const eckit::message::Message& message) {
    verb_ = MarsLanguage::verb("message");

    eckit::message::StringSetter<MarsRequest> setter(*this);
    message.getMetadata(setter);
}

MarsRequest::MarsRequest(eckit::Stream& s, bool lowercase) {
    int size;
    std::string verb;

    s >> verb;
    if (lowercase)
        verb = eckit::StringTools::lower(verb);
    verb_ = MarsLanguage::verb(verb);
    MarsLanguage::get(verb_);
    s >> size;

    for (int i = 0; i < size; i++) {
        std::string param;
        int count;

        s >> param;
        if (lowercase)
            param = eckit::StringTools::lower(param);
        Keyword key = MarsLanguage::keyword(param);
        s >> count;

        std::vector<std::string> v;
        v.reserve(count);

        for (int k = 0; k < count; k++) {
            std::string value;
            s >> value;
            v.push_back(value);
        }

        params_.push_back(TypeParameter(v, new TypeAny(key)));
    }
}

void MarsRequest::encode(eckit::Stream& s) const {
    s << MarsLanguage::name(verb_);
    int size = params_.size();
    s << size;


    for (std::list<TypeParameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        s << (*i).name();

        const std::vector<std::string>& v = (*i).values();

        int size = v.size();  // For backward compatibility
        s << size;

        for (std::vector<std::string>::const_iterator k = v.begin(); k != v.end(); ++k) {
            s << *k;
        }
    }
}

bool MarsRequest::empty() const {
    return params_.empty();
}


void MarsRequest::print(std::ostream& s) const {
    dump(s, "", "", true);
}

void MarsRequest::dump(std::ostream& s, const char* cr, const char* tab, bool verb) const {
    auto begin = params_.begin();
    auto end   = params_.end();

    if (verb) {
        s << MarsLanguage::name(verb_) << ',';
    }
    std::string separator = "";
    if (begin != end) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (const auto& p : params_) {
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

void MarsRequest::json(eckit::JSON& s, bool array) const {
    s.startObject();
    for (const auto& p : params_) {
        s << p.name();
        const std::vector<std::string>& vv = p.values();

        bool list = vv.size() != 1 || (array && p.type().multiple());
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

void MarsRequest::md5(eckit::MD5& md5) const {
    std::ostringstream oss;
    oss << *this;
    md5.add(oss.str());
}

void MarsRequest::unsetValues(Keyword id) {
    erase(id);
}

void MarsRequest::unsetValues(const std::string& name) {
    unsetValues(MarsLanguage::keyword(name));
}

void MarsRequest::setValuesTyped(const Type* type, const std::vector<std::string>& values) {
    auto p = find(type->id());
    if (p) {
        p->get() = TypeParameter(values, type);
    }
    else {
        params_.push_back(TypeParameter(values, type));
    }
}

bool MarsRequest::filter(const MarsRequest& filter) {

    Keyword date = MarsLanguage::keyword("date");
    Keyword day = MarsLanguage::keyword("day");

    for (auto& p : params_) {
        if (p.id() == date) {
            auto fp = filter.find("day");
            if (fp) {
                if (!p.filter(day, fp->get().values())) {
                    return false;
                }
            }
        }

        auto fp = filter.find(p.id());
        if (!fp) {
            continue;
        }
        
        if (!p.filter(fp->get().values())) {
            return false;
        }
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest& matches) const {
    std::vector<std::string> params = matches.params();
    for (auto p : matches.params_) {
        auto mp = find(p.id());
        if (!mp) {
            return false;
        }

        if (!mp->get().matches(p.values())) {
            return false;
        }
    }

    return true;
}

void MarsRequest::values(Keyword id, const std::vector<std::string>& v) {
    auto p = find(id);
    if (p) {
        p->get().values(v);
    }
    else {
        params_.push_back(TypeParameter(v, new TypeAny(id)));
    }
}

void MarsRequest::values(const std::string& name, const std::vector<std::string>& v) {
    Keyword key = MarsLanguage::keyword(eckit::StringTools::lower(name));
    values(key, v);
}


size_t MarsRequest::countValues(Keyword id) const {
    auto p = find(id);
    if (p) {
        return p->get().values().size();
    }
    return 0;
}

size_t MarsRequest::countValues(const std::string& name) const {
    return countValues(MarsLanguage::keyword(eckit::StringTools::lower(name)));
}

bool MarsRequest::has(Keyword id) const {
    auto p = find(id);
    return p.has_value();
}

bool MarsRequest::has(const std::string& name) const {
    Keyword id = MarsLanguage::hasKeyword(eckit::StringTools::lower(name));

    return id ? has(id) : false;
}


bool MarsRequest::is(Keyword id, const std::string& value) const {
    auto p = find(id);
    if (p) {
        const std::vector<std::string>& v = p->get().values();
        return v.size() == 1 && v[0] == value;
    }
    return false;
}

bool MarsRequest::is(const std::string& name, const std::string& value) const {
    return is(MarsLanguage::keyword(eckit::StringTools::lower(name)), value);
}

const std::vector<std::string>& MarsRequest::values(Keyword id, bool emptyOk) const {
    auto p = find(id);
    if (!p) {
        if (emptyOk) {
            static std::vector<std::string> empty;
            return empty;
        }

        std::ostringstream oss;
        oss << "No parameter called '" << MarsLanguage::name(id) << "' in request " << *this;
        throw eckit::UserError(oss.str());
    }
    return p->get().values();
}

const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const {
    return values(MarsLanguage::keyword(eckit::StringTools::lower(name)), emptyOk);
}

std::optional<std::reference_wrapper<const std::vector<std::string>>> MarsRequest::get(Keyword id) const {
    auto p = find(id);
    if (!p) {
        return std::nullopt;
    }
    return std::cref(p->get().values());
}

std::optional<std::reference_wrapper<const std::vector<std::string>>> MarsRequest::get(
    const std::string& keyword) const {
    return get(MarsLanguage::keyword(eckit::StringTools::lower(keyword)));
}

const std::string& MarsRequest::operator[](Keyword id) const {
    auto p = find(id);
    if (!p) {
        std::ostringstream oss;
        oss << "Parameter '" << MarsLanguage::name(id) << "' is undefined";
        throw eckit::UserError(oss.str());
    }
    const std::vector<std::string>& c = p->get().values();
    if (c.size() > 1) {
        std::ostringstream oss;
        oss << "Parameter '" << MarsLanguage::name(id) << "' has more than one value";
        throw eckit::UserError(oss.str());
    }

    return c[0];
}

const std::string& MarsRequest::operator[](const std::string& name) const {
    return operator[](MarsLanguage::keyword(eckit::StringTools::lower(name)));
}


void MarsRequest::getParams(std::vector<std::string>& p) const {
    p.clear();
    for (std::list<TypeParameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        p.push_back((*i).name());
    }
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
        if (p.values().size() == 1 && (p.values().at(0) == "all" || p.values().at(0) == "any")) {
            return 0;
        }
        if (p.name() == "levelist") {
            levels = p.values().size();
            // For a parameter matching paramIdsSingleLevel, we can only include one value, and for level=1
            hasLevelOne = std::find(p.values().begin(), p.values().end(), "1") != p.values().end();
        }
        else {
            if (p.name() == "param") {
                for (const auto& v : p.values()) {
                    paramsSingleLevel += paramIdsSingleLevel.count(v);
                }
                params = p.values().size() - paramsSingleLevel;
            }
            else {
                if (p.name() == "levtype") {
                    ml = std::find(p.values().begin(), p.values().end(), "ml") != p.values().end();
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

std::vector<std::string> MarsRequest::params() const {
    std::vector<std::string> p;
    getParams(p);
    return p;
}

MarsRequest::operator eckit::Value() const {
    NOTIMP;
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

std::vector<MarsRequest> MarsRequest::split(const std::string& key) const {
    std::vector<std::string> keys = {key};
    return split(keys);
}

void MarsRequest::merge(const MarsRequest& other) {
    for (auto& param : params_) {
        LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << param << std::endl;
        auto p = other.find(param.id());
        if (p) {
            param.merge(p->get());
        }
    }
}

MarsRequest MarsRequest::subset(const std::set<std::string>& keys) const {
    MarsRequest req(verb_);
    for (std::list<TypeParameter>::const_iterator it = params_.begin(); it != params_.end(); ++it) {
        if (keys.find(it->name()) != keys.end()) {
            req.params_.push_back(*it);
        }
    }
    return req;
}

void MarsRequest::verb(const std::string& verb) {
    verb_ = MarsLanguage::verb(verb);
    MarsLanguage::get(verb_);
}

bool MarsRequest::operator<(const MarsRequest& other) const {
    if (verb_ != other.verb_) {
        return verb_ < other.verb_;
    }
    return params_ < other.params_;
}

const std::string& MarsRequest::verb() const {
    return MarsLanguage::name(verb_);
}

std::optional<std::reference_wrapper<const Parameter>> MarsRequest::find(Keyword key) const {
    for (std::list<TypeParameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).id() == key) {
            return std::cref(*i);
        }
    }
    return std::nullopt;
}

std::optional<std::reference_wrapper<Parameter>> MarsRequest::find(Keyword key) {
    for (std::list<TypeParameter>::iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).id() == key) {
            return std::ref(*i);
        }
    }
    return std::nullopt;
}

std::optional<std::reference_wrapper<const Parameter>> MarsRequest::find(const std::string& name) const {
    return find(MarsLanguage::keyword(name));
}

std::optional<std::reference_wrapper<Parameter>> MarsRequest::find(const std::string& name) {
    return find(MarsLanguage::keyword(name));
}

void MarsRequest::erase(Keyword id) {
    for (std::list<TypeParameter>::iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).id() == id) {
            params_.erase(i);
            return;
        }
    }
}
void MarsRequest::erase(const std::string& name) {
    erase(MarsLanguage::keyword(name));
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
    std::vector<MarsRequest> v = parse(in, strict);
    ASSERT(v.size() == 1);
    return v[0];
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
