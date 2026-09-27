/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsParsedRequest.h"

#include "eckit/log/Log.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/Parameter.h"


namespace metkit::mars {

MarsParsedRequest::MarsParsedRequest(const MarsRequest& request) : MarsRequest(), verb_(request.verb()) {
    
    const auto& parameters = request.parameters();
    params_.reserve(parameters.size());
    
    for (const auto& param : parameters) {
        const std::string& name = param->name();
        paramMap_[name] = params_.size();
        params_.push_back(new StringParameter(*param));
    }
}


MarsParsedRequest::MarsParsedRequest(const std::string& verb, size_t line) : verb_(verb), line_(line) {}

MarsParsedRequest::MarsParsedRequest(eckit::Stream& s, bool lowercase) {
    int size;

    s >> verb_;
    if (lowercase)
        verb_ = eckit::StringTools::lower(verb_);

    s >> size;
    params_.reserve(size);
    for (int i = 0; i < size; i++) {
        std::string param;
        int count;

        s >> param;
        if (lowercase)
            param = eckit::StringTools::lower(param);

        s >> count;
        std::vector<std::string> v;
        v.reserve(count);

        for (int k = 0; k < count; k++) {
            std::string value;
            s >> value;
            v.push_back(value);
        }
        
        paramMap_[param] = params_.size();
        params_.emplace_back(new StringParameter(param, std::move(v)));
    }
}

Verb MarsParsedRequest::verbId() const {
    // might throw if verb_ is not valid
    return MarsLanguage::verb(verb_);
}

void MarsParsedRequest::verb(Verb id) {
    // might throw if verb id is not valid
    verb_ = MarsLanguage::name(id);
}
void MarsParsedRequest::verb(const std::string& name) {
    verb_ = name;
}

size_t MarsParsedRequest::countValues(Keyword key) const {
    // might throw if key is not valid
    return countValues(MarsLanguage::name(key));
}
size_t MarsParsedRequest::countValues(const std::string& name) const {
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        return params_[it->second]->count();
    }
    return 0;
}
    
bool MarsParsedRequest::has(Keyword key) const {
    // might throw if key is not valid
    return has(MarsLanguage::name(key));
}
bool MarsParsedRequest::has(const std::string& name) const {
    return paramMap_.find(name) != paramMap_.end();
}

const std::vector<std::string>& MarsParsedRequest::values(Keyword key, bool emptyOk) const {
    // might throw if key is not valid
    return values(MarsLanguage::name(key), emptyOk);
}
const std::vector<std::string>& MarsParsedRequest::values(const std::string& name, bool emptyOk) const {
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        return params_[it->second]->values();
    }
    if (emptyOk) {
        static std::vector<std::string> empty;
        return empty;
    }

    std::ostringstream oss;
    oss << "No parameter called '" << name << "' in request " << *this;
    throw eckit::UserError(oss.str());
}

void MarsParsedRequest::values(Keyword key, const std::vector<std::string>& vals) {
    // might throw if key is not valid
    return values(MarsLanguage::name(key), vals);
}
void MarsParsedRequest::values(const std::string& name, const std::vector<std::string>& vals) {
    ASSERT(!name.empty());
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        params_[it->second]->values(vals);
    }
    else {
        paramMap_[name] = params_.size();
        params_.emplace_back(new StringParameter(name, vals));
    }
}

void MarsParsedRequest::erase(Keyword key) {
    // might throw if key is not valid
    erase(MarsLanguage::name(key));
}
void MarsParsedRequest::erase(const std::string& name) {
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        params_.erase(params_.begin() + it->second);
        paramMap_.erase(it);
    }
}

void MarsParsedRequest::setValuesTyped(const Type* type, const std::vector<std::string>& values) {
    const std::string& name = type->name();
    eckit::Log::warning() << "Type " << name << " will be omitted while setting values in MarsParsedRequest" << *this << std::endl;
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        params_[it->second] = new StringParameter(name, values);
        return;
    }
    paramMap_[name] = params_.size();
    params_.emplace_back(new StringParameter(name, values));
}

const Parameter* MarsParsedRequest::find(Keyword key) const {
    // might throw if key is not valid
    return find(MarsLanguage::name(key));
}
const Parameter* MarsParsedRequest::find(const std::string& name) const {
    auto it = paramMap_.find(name);
    if (it != paramMap_.end()) {
        return params_[it->second];
    }
    return nullptr;
}


// MarsParsedRequest::MarsParsedRequest(const std::string& verb, size_t line) : verb_(verb), line_(line) {}

// const std::string& MarsParsedRequest::verb() const {
//     return verb_;
// }

// void MarsParsedRequest::getParams(std::vector<std::string>& keys) const {
//     keys.clear();
//     for (const auto& p : params_) {
//         keys.push_back(p.name());
//     }
// }
// const std::vector<std::string>& MarsParsedRequest::values(const std::string& key, bool emptyOk) const {
//     auto p = find(key);
//     if (p) {
//         return p->get().values();
//     }
//     static const std::vector<std::string> empty;
//     return empty;
// }

// void MarsParsedRequest::verb(const std::string& v) {
//     verb_ = v;
// }

// void MarsParsedRequest::values(const std::string& key, const std::vector<std::string>& vals) {
//     auto p = find(key);
//     if (p) {
//         p->get().values(vals);
//     }
//     else {
//         params_.push_back(StringParameter{key, vals});
//     }
// }

// std::optional<std::reference_wrapper<const Parameter>> MarsParsedRequest::find(const std::string& name) const {
//     for (auto i = params_.begin(); i != params_.end(); ++i) {
//         if (i->name() == name) {
//             return std::cref(*i);
//         }
//     }
//     return std::nullopt;
// }
// std::optional<std::reference_wrapper<Parameter>> MarsParsedRequest::find(const std::string& name) {
//     for (auto i = params_.begin(); i != params_.end(); ++i) {
//         if (i->name() == name) {
//             return std::ref(*i);
//         }
//     }
//     return std::nullopt;
// }

// void MarsParsedRequest::unsetValues(const std::string& key) {
//     for (auto i = params_.begin(); i != params_.end(); ++i) {
//         if (i->name() == key) {
//             params_.erase(i);
//             return;
//         }
//     }
// }

// const std::list<StringParameter>& MarsParsedRequest::params() const {
//     return params_;
// }

// // void MarsParsedRequest::dump(std::ostream& s, const char* cr, const char* tab, bool verb) const {
// //     if (verb) {
// //         s << verb_ << ',';
// //     }
// //     std::string separator = "";
// //     if (!params_.empty()) {
// //         s << separator << cr << tab;
// //         separator = ",";

// //         int a = 0;
// //         for (const auto& p : params_) {
// //             if (a++) {
// //                 s << ',' << cr << tab;
// //             }

// //             int b = 0;
// //             s << p.name() << "=";

// //             for (const auto& k : p.values()) {
// //                 if (b++) {
// //                     s << '/';
// //                 }
// //                 MarsParser::quoted(s, k);
// //             }
// //         }
// //     }

// //     s << cr << cr;
// // }

void MarsParsedRequest::info(std::ostream& out) const {
    out << " Request starting line " << line_;
}

}  // namespace metkit::mars
