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

// const std::string& MarsID::getValue(Keyword id) const {
//     auto it = values_.find(id);
//     if (it != values_.end()) {
//         return it->second;
//     }
//     static const std::string empty;
//     return empty;
// }
// void MarsID::setValue(Keyword id, const std::string& value) {
//     values_[id] = value;
// }
// void MarsID::unsetValue(Keyword id) {
//     values_.erase(id);
// }
// bool MarsID::hasValue(Keyword id) const {
//     return values_.find(id) != values_.end();
// }
// const std::unordered_map<Keyword, std::string>& values() const {
//     return values_;
// }

//----------------------------------------------------------------------------------------------------------------------

// MarsRequest::MarsRequest(const MarsRequest& other) {
//     // do not copy parameters from the other request, since we want each MarsRequest to manage its own parameters subtype
// }

// MarsRequest::MarsRequest() : verb_(MarsLanguage::verb("retrieve")) {
//     MarsLanguage::get(verb_);
// }

// MarsRequest::MarsRequest(Verb verb) : verb_(verb) {
//     MarsLanguage::get(verb_);
// }

// MarsRequest::MarsRequest(const std::string& s) {
//     ASSERT(s.find(',') == std::string::npos);
//     verb_ = MarsLanguage::verb(s);
//     MarsLanguage::get(verb_);
// }

// MarsRequest::MarsRequest(const std::string& s, const std::map<std::string, std::string>& values) {

//     verb_ = MarsLanguage::verb(s);
//     MarsLanguage::get(verb_);

//     params_.reserve(values.size());
//     for (const auto& [name, value] : values) {
//         Keyword param = MarsLanguage::keyword(name);

//         paramMap_[param] = params_.size();
//         params_.emplace_back(std::vector<std::string>(1, value), new TypeAny(param));
//     }
// }


// MarsRequest::MarsRequest(const std::string& s, const eckit::Value& values) {

//     verb_ = MarsLanguage::verb(s);
//     MarsLanguage::get(verb_);

//     eckit::ValueMap m = values;
//     params_.reserve(m.size());
//     for (const auto& [name, value] : m) {
//         Keyword param = MarsLanguage::keyword(name);

//         paramMap_[param] = params_.size();
//         if (value.isList()) {
//             std::vector<std::string> vals;
//             eckit::fromValue(vals, value);
//             params_.emplace_back(vals, new TypeAny(param));
//         }
//         else {
//             params_.emplace_back(std::vector<std::string>(1, value), new TypeAny(param));
//         }
//     }
// }

// MarsRequest::MarsRequest(const eckit::message::Message& message) {
//     verb_ = MarsLanguage::verb("message");

//     eckit::message::StringSetter<MarsRequest> setter(*this);
//     message.getMetadata(setter);
// }

void MarsRequest::encode(eckit::Stream& s) const {
    s << verb();

    int size = params_.size();
    s << size;

    for (const auto& p : params_) {
        s << p->name();

        const std::vector<std::string>& vv = p->values();
        int size = vv.size();  // For backward compatibility
        s << size;

        for (const auto& v : vv) {
            s << v;
        }
    }
}

bool MarsRequest::empty() const {
    return params_.empty();
}

void MarsRequest::print(std::ostream& s) const {
    dump(s, "", "", true);
}

void MarsRequest::dump(std::ostream& s, const char* cr, const char* tab, bool printVerb) const {
    if (printVerb) {
        s << verb() << ',';
    }
    std::string separator = "";
    if (params_.size()) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (const auto& p : params_) {
            if (a++) {
                s << ',' << cr << tab;
            }

            int b = 0;
            s << p->name() << "=";

            for (const auto& v : p->values()) {
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
        s << p->name();

        const std::vector<std::string>& vv = p->values();

        bool list = vv.size() != 1 || (array && p->multiple());
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

// void MarsRequest::md5(eckit::MD5& md5) const {
//     std::ostringstream oss;
//     oss << *this;
//     md5.add(oss.str());
// }

// void MarsRequest::unsetValues(Keyword id) {
//     erase(id);
// }

void MarsRequest::unsetValues(const std::string& name) {
    erase(name);
}

bool MarsRequest::filter(const MarsRequest& filter) {

    Keyword date = MarsLanguage::keyword("date");
    Keyword day  = MarsLanguage::keyword("day");

    for (const auto& p : params_) {
        if (p->id() == date) {
            auto fp = filter.find("day");
            if (fp) {
                if (!p->filter(day, fp->values())) {
                    return false;
                }
            }
        }

        auto fp = filter.find(p->id());
        if (!fp) {
            continue;
        }

        if (!p->filter(fp->values())) {
            return false;
        }
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest& matches) const {
    for (const auto& p : matches.params_) {
        const auto& mp = find(p->id());
        if (!mp) {
            return false;
        }

        if (!mp->matches(p->values())) {
            return false;
        }
    }

    return true;
}

// void MarsRequest::values(Keyword id, const std::vector<std::string>& v) {
//     ASSERT(id);
//     auto it = paramMap_.find(id);
//     if (it != paramMap_.end()) {
//         params_[it->second].values(v);
//     }
//     else {
//         paramMap_[id] = params_.size();
//         params_.emplace_back(TypeParameter(v, new TypeAny(id)));
//     }
// }

// void MarsRequest::values(const std::string& name, const std::vector<std::string>& v) {
//     Keyword key = MarsLanguage::keyword(eckit::StringTools::lower(name));
//     values(key, v);
// }


// size_t MarsRequest::countValues(Keyword id) const {
//     auto p = find(id);
//     if (p) {
//         return p->get().values().size();
//     }
//     return 0;
// }

// size_t MarsRequest::countValues(const std::string& name) const {
//     return countValues(MarsLanguage::keyword(eckit::StringTools::lower(name)));
// }

// bool MarsRequest::has(Keyword id) const {
//     auto p = find(id);
//     return p.has_value();
// }

// bool MarsRequest::has(const std::string& name) const {
//     Keyword id = MarsLanguage::hasKeyword(eckit::StringTools::lower(name));

//     return id ? has(id) : false;
// }


// bool MarsRequest::is(Keyword id, const std::string& value) const {
//     auto p = find(id);
//     if (p) {
//         const std::vector<std::string>& v = p->get().values();
//         return v.size() == 1 && v[0] == value;
//     }
//     return false;
// }

// bool MarsRequest::is(const std::string& name, const std::string& value) const {
//     return is(MarsLanguage::keyword(eckit::StringTools::lower(name)), value);
// }

// const std::vector<std::string>& MarsRequest::values(Keyword id, bool emptyOk) const {
//     auto p = find(id);
//     if (!p) {
//         if (emptyOk) {
//             static std::vector<std::string> empty;
//             return empty;
//         }

//         std::ostringstream oss;
//         oss << "No parameter called '" << MarsLanguage::name(id) << "' in request " << *this;
//         throw eckit::UserError(oss.str());
//     }
//     return p->get().values();
// }

// const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const {
//     return values(MarsLanguage::keyword(eckit::StringTools::lower(name)), emptyOk);
// }

// std::optional<std::reference_wrapper<const std::vector<std::string>>> MarsRequest::get(Keyword id) const {
//     auto p = find(id);
//     if (!p) {
//         return std::nullopt;
//     }
//     return std::cref(p->get().values());
// }

// std::optional<std::reference_wrapper<const std::vector<std::string>>> MarsRequest::get(
//     const std::string& keyword) const {
//     return get(MarsLanguage::keyword(eckit::StringTools::lower(keyword)));
// }

// const std::string& MarsRequest::operator[](Keyword id) const {
//     auto p = find(id);
//     if (!p) {
//         std::ostringstream oss;
//         oss << "Parameter '" << MarsLanguage::name(id) << "' is undefined";
//         throw eckit::UserError(oss.str());
//     }
//     const std::vector<std::string>& c = p->get().values();
//     if (c.size() > 1) {
//         std::ostringstream oss;
//         oss << "Parameter '" << MarsLanguage::name(id) << "' has more than one value";
//         throw eckit::UserError(oss.str());
//     }

//     return c[0];
// }

// const std::string& MarsRequest::operator[](const std::string& name) const {
//     return operator[](MarsLanguage::keyword(eckit::StringTools::lower(name)));
// }


// void MarsRequest::getParams(std::vector<std::string>& p) const {
//     p.clear();
//     for (const auto& [k, param] : params_) {
//         p.push_back(param.name());
//     }
// }

size_t MarsRequest::count() const {
    size_t result = 1;

    const std::set<std::string>& paramIdsSingleLevel = ParamID::getMlParamsSingleLevel();

    bool ml                  = false;
    size_t levels            = 1;
    size_t params            = 0;
    size_t paramsSingleLevel = 0;
    bool hasLevelOne         = false;

    for (const auto& p : params_) {
        const auto& name = p->name();
        const auto& values = p->values();
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
                result *= p->count();
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

// std::vector<std::string> MarsRequest::params() const {
//     std::vector<std::string> p;
//     getParams(p);
//     return p;
// }

// MarsRequest::operator eckit::Value() const {
//     NOTIMP;
// }

// // recursively expand along keys in expvalues
// void expand_along_keys(const MarsRequest& prototype,
//                        const std::vector<std::pair<std::string, std::vector<std::string>>>& expvalues,
//                        std::vector<MarsRequest>& requests, size_t i) {

//     if (i == expvalues.size()) {
//         requests.push_back(prototype);
//         return;
//     }

//     const std::string& key                 = expvalues[i].first;
//     const std::vector<std::string>& values = expvalues[i].second;

//     MarsRequest req(prototype);
//     for (auto& value : values) {
//         req.setValue(key, value);
//         expand_along_keys(req, expvalues, requests, i + 1);
//     }
// }

// std::vector<MarsRequest> MarsRequest::split(const std::vector<std::string>& keys) const {

//     size_t n = 1;

//     LOG_DEBUG_LIB(LibMetkit) << "Splitting request with keys" << keys << std::endl;

//     std::vector<std::pair<std::string, std::vector<std::string>>> expvalues;
//     for (auto& key : keys) {
//         std::vector<std::string> v = values(key, true);  // ok to be empty
//         LOG_DEBUG_LIB(LibMetkit) << "splitting along key " << key << " n values " << v.size() << " values " << v
//                                  << std::endl;
//         if (v.empty())
//             continue;
//         n *= v.size();
//         expvalues.emplace_back(std::make_pair(key, v));
//     }

//     std::vector<MarsRequest> requests;
//     requests.reserve(n);

//     if (n == 1) {
//         requests.push_back(*this);
//         return requests;
//     }

//     expand_along_keys(*this, expvalues, requests, 0);

//     return requests;
// }

// std::vector<MarsRequest> MarsRequest::split(const std::string& key) const {
//     std::vector<std::string> keys = {key};
//     return split(keys);
// }

void MarsRequest::merge(const MarsRequest& other) {
    for (auto& param : params_) {
        LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << param << std::endl;
        const Parameter* p = other.find(param->name());
        if (p) {
            param->merge(*p);
        }
    }
}

// MarsRequest MarsRequest::subset(const std::set<std::string>& keys) const {
//     MarsRequest req(verb_);
//     for (const auto& [k,p] : params_) {
//         if (keys.find(p.name()) != keys.end()) {
//             req.params_.emplace(k, p);
//         }
//     }
//     return req;
// }

// void MarsRequest::verb(const std::string& verb) {
//     verb_ = MarsLanguage::verb(verb);
//     MarsLanguage::get(verb_);
// }

// bool MarsRequest::operator<(const MarsRequest& other) const {
//     if (verb_ != other.verb_) {
//         return verb_ < other.verb_;
//     }
//     return params_ < other.params_;
// }

// const std::string& MarsRequest::verb() const {
//     return MarsLanguage::name(verb_);
// }

// std::optional<std::reference_wrapper<const Parameter>> MarsRequest::find(Keyword key) const {
//     auto it = params_.find(key);
//     if (it != params_.end()) {
//         return std::cref(it->second);
//     }
//     return std::nullopt;
// }

// std::optional<std::reference_wrapper<Parameter>> MarsRequest::find(Keyword key) {
//     auto it = params_.find(key);
//     if (it != params_.end()) {
//         return std::ref(it->second);
//     }
//     return std::nullopt;
// }

// std::optional<std::reference_wrapper<const Parameter>> MarsRequest::find(const std::string& name) const {
//     return find(MarsLanguage::keyword(name));
// }

// std::optional<std::reference_wrapper<Parameter>> MarsRequest::find(const std::string& name) {
//     return find(MarsLanguage::keyword(name));
// }

// void MarsRequest::erase(Keyword id) {
//     auto it = params_.find(id);
//     if (it != params_.end()) {
//         params_.erase(it);
//     }
// }
// void MarsRequest::erase(const std::string& name) {
//     erase(MarsLanguage::keyword(name));
// }

std::string MarsRequest::asString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}
//----------------------------------------------------------------------------------------------------------------------

std::vector<MarsValidatedRequest> parse(std::istream& in, bool strict) {
    MarsParser parser(in);
    MarsExpansion expand(true, strict);
    return expand.expand(parser.parse());
}

MarsValidatedRequest parse(const std::string& s, bool strict) {
    std::istringstream in(s);
    std::vector<MarsValidatedRequest> v = parse(in, strict);
    ASSERT(v.size() == 1);
    return v[0];
}

//----------------------------------------------------------------------------------------------------------------------

MarsValidatedRequest::MarsValidatedRequest(Verb verb) : MarsRequest(), verb_(verb) {}

MarsValidatedRequest::MarsValidatedRequest(const MarsRequest& request) : MarsRequest(), verb_(request.verbId()) {
    for (const auto& param : request.parameters()) {
        Keyword key = param->id();
        paramMap_[key] = params_.size();
        params_.push_back(std::make_unique<TypeParameter>(param->values(), new TypeAny(key)));
    }
}


MarsValidatedRequest::MarsValidatedRequest(eckit::Stream& s, bool lowercase) {
    int size;
    std::string verb;

    s >> verb;
    if (lowercase)
        verb = eckit::StringTools::lower(verb);
    verb_ = MarsLanguage::verb(verb);
    MarsLanguage::get(verb_);

    s >> size;
    params_.reserve(size);
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
        
        paramMap_[key] = params_.size();
        params_.emplace_back(std::make_unique<TypeParameter>(v, new TypeAny(key)));
    }
}

const std::string& MarsValidatedRequest::verb() const {
    return MarsLanguage::name(verb_);
}

size_t MarsValidatedRequest::countValues(Keyword key) const {
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
        return params_[it->second]->count();
    }
    return 0;
}
size_t MarsValidatedRequest::countValues(const std::string& name) const {
    // might throw if keyword name is not valid
    return countValues(MarsLanguage::keyword(name));
}

bool MarsValidatedRequest::has(Keyword key) const {
    return paramMap_.find(key) != paramMap_.end();
}   
bool MarsValidatedRequest::has(const std::string& name) const {
    // might throw if keyword name is not valid
    return has(MarsLanguage::keyword(name));
}

const std::vector<std::string>& MarsValidatedRequest::values(Keyword key, bool emptyOk) const {
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
        return params_[it->second]->values();
    }
    if (emptyOk) {
        static std::vector<std::string> empty;
        return empty;
    }

    std::ostringstream oss;
    oss << "No parameter called '" << MarsLanguage::name(key) << "' in request " << *this;
    throw eckit::UserError(oss.str());
}
const std::vector<std::string>& MarsValidatedRequest::values(const std::string& name, bool emptyOk) const {
    // might throw if keyword name is not valid
    return values(MarsLanguage::keyword(name), emptyOk);
}

void MarsValidatedRequest::values(Keyword key, const std::vector<std::string>& vals) {
    ASSERT(key);
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
        params_[it->second]->values(vals);
    }
    else {
        paramMap_[key] = params_.size();
        params_.emplace_back(std::make_unique<TypeParameter>(vals, new TypeAny(key)));
    }
}
void MarsValidatedRequest::values(const std::string& name, const std::vector<std::string>& vals) {
    // might throw if keyword name is not valid
    values(MarsLanguage::keyword(name), vals);
}

void MarsValidatedRequest::erase(Keyword key) {
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
        params_.erase(params_.begin() + it->second);
        paramMap_.erase(it);
    }
}
void MarsValidatedRequest::erase(const std::string& name) {
    // might throw if keyword name is not valid
    erase(MarsLanguage::keyword(name));
}

void MarsValidatedRequest::merge(const MarsRequest& other) {
    for (auto& param : params_) {
        LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << *param << std::endl;
        const Parameter* p = other.find(param->id());
        if (p) {
            param->merge(*p);
        }
    }
}

void MarsValidatedRequest::setValuesTyped(const Type* type, const std::vector<std::string>& values) {
    auto it = paramMap_.find(type->id());
    if (it != paramMap_.end()) {
        params_[it->second] = std::make_unique<TypeParameter>(values, type);
        return;
    }
    paramMap_[type->id()] = params_.size();
    params_.emplace_back(std::make_unique<TypeParameter>(values, type));
}

const Parameter* MarsValidatedRequest::find(Keyword key) const {
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
        return params_[it->second].get();
    }
    return nullptr;
}
const Parameter* MarsValidatedRequest::find(const std::string& name) const {
    // might throw if keyword name is not valid
    return find(MarsLanguage::keyword(name));
}

//----------------------------------------------------------------------------------------------------------------------


}  // namespace metkit::mars
