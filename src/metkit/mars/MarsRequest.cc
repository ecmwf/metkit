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
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeAny.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

MarsRequest::MarsRequest() {}

MarsRequest::MarsRequest(const std::string& s) : verb_(s) {}

MarsRequest::MarsRequest(const std::string& verb, const std::vector<std::pair<std::string, std::string>>& values)
    : verb_(verb) {
    for (const auto& [param, value] : values) {
        params_.push_back(Parameter(std::vector<std::string>(1, value), new TypeAny(param)));
    }
}

MarsRequest::MarsRequest(const std::string& s, const std::map<std::string, std::string>& values) :
    verb_(s) {
    for (auto j = values.begin(); j != values.end(); ++j) {
        const std::string& param = (*j).first;
        const std::string& value = (*j).second;


        params_.push_back(Parameter(std::vector<std::string>(1, value), new TypeAny(param)));
    }
}


MarsRequest::MarsRequest(const std::string& s, const eckit::Value& values) : verb_(s) {
    eckit::ValueMap m = values;
    for (auto j = m.begin(); j != m.end(); ++j) {
        const std::string& param  = (*j).first;
        const eckit::Value& value = (*j).second;

        if (value.isList()) {
            std::vector<std::string> vals;
            eckit::fromValue(vals, value);
            params_.push_back(Parameter(vals, new TypeAny(param)));
        }
        else {
            params_.push_back(Parameter(std::vector<std::string>(1, value), new TypeAny(param)));
        }
    }
}

MarsRequest::MarsRequest(const eckit::message::Message& message) : verb_("message") {
    eckit::message::StringSetter<MarsRequest> setter(*this);
    message.getMetadata(setter);
}

MarsRequest::MarsRequest(eckit::Stream& s, bool lowercase) {
    int size;

    s >> verb_;
    if (lowercase)
        verb_ = eckit::StringTools::lower(verb_);
    s >> size;

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

        params_.push_back(Parameter(v, new TypeAny(param)));
    }
}

void MarsRequest::encode(eckit::Stream& s) const {
    s << verb_;
    int size = params_.size();
    s << size;


    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
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
    std::list<Parameter>::const_iterator begin = params_.begin();
    std::list<Parameter>::const_iterator end   = params_.end();

    if (verb) {
        s << verb_ << ',';
    }
    std::string separator = "";
    if (begin != end) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (std::list<Parameter>::const_iterator i = begin; i != end; ++i) {
            if (a++) {
                s << ',';
                s << cr << tab;
            }

            int b = 0;
            s << (*i).name()
              //               << "." << (*i).second.type()
              << "=";
            const std::vector<std::string>& v = (*i).values();

            for (std::vector<std::string>::const_iterator k = v.begin(); k != v.end(); ++k) {
                if (b++) {
                    s << '/';
                }
                MarsParser::quoted(s, *k);
            }

            // s << " {" << (*i).type().category() << "}";
        }
    }

    s << cr << cr;
}

void MarsRequest::json(eckit::JSON& s, bool array) const {
    s.startObject();
    // s << "_verb" << verb_;
    std::list<Parameter>::const_iterator begin = params_.begin();
    std::list<Parameter>::const_iterator end   = params_.end();

    for (std::list<Parameter>::const_iterator i = begin; i != end; ++i) {
        s << (*i).name();
        const std::vector<std::string>& v = (*i).values();

        bool list = v.size() != 1 || (array && (*i).type().multiple());
        if (list) {
            s.startList();
        }

        for (std::vector<std::string>::const_iterator k = v.begin(); k != v.end(); ++k) {
            s << (*k);
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

void MarsRequest::unsetValues(const std::string& name) {
    std::list<Parameter>::iterator i = find(name);
    if (i != params_.end()) {
        params_.erase(i);
    }
}

void MarsRequest::setValuesTyped(Type* type, const std::vector<std::string>& values) {
    std::list<Parameter>::iterator i = find(type->name());
    if (i != params_.end()) {
        (*i) = Parameter(values, type);
    }
    else {
        params_.push_back(Parameter(values, type));
    }
}

bool MarsRequest::filter(const MarsRequest& filter) {
    for (std::list<Parameter>::iterator i = params_.begin(); i != params_.end(); ++i) {
        std::list<Parameter>::const_iterator j = filter.find((*i).name());
        if (j == filter.params_.end()) {
            continue;
        }

        if (!(*i).filter((*j).values())) {
            return false;
        }
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest& matches) const {
    std::vector<std::string> params = matches.params();
    for (std::vector<std::string>::const_iterator j = params.begin(); j != params.end(); ++j) {
        std::list<Parameter>::const_iterator k = find(*j);
        if (k == params_.end()) {
            return false;
        }

        if (!(*k).matches(matches.values(*j))) {
            return false;
        }
    }

    return true;
}

void MarsRequest::values(const std::string& name, const std::vector<std::string>& v) {
    std::list<Parameter>::iterator i = find(name);
    if (i != params_.end()) {
        (*i).values(v);
    }
    else {
        params_.push_back(Parameter(v, new TypeAny(name)));
    }
}


size_t MarsRequest::countValues(const std::string& name) const {
    std::list<Parameter>::const_iterator i = find(name);
    if (i != params_.end()) {
        return (*i).values().size();
    }
    return 0;
}

bool MarsRequest::has(const std::string& name) const {
    return find(name) != params_.end();
}


bool MarsRequest::is(const std::string& name, const std::string& value) const {
    std::list<Parameter>::const_iterator i = find(name);
    if (i != params_.end()) {
        const std::vector<std::string>& v = (*i).values();
        return v.size() == 1 && v[0] == value;
    }
    return false;
}

const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const {
    std::list<Parameter>::const_iterator i = find(name);
    if (i == params_.end()) {
        if (emptyOk) {
            static std::vector<std::string> empty;
            return empty;
        }

        std::ostringstream oss;
        oss << "No parameter called '" << name << "' in request " << *this;
        throw eckit::UserError(oss.str());
    }
    return (*i).values();
}

const std::string& MarsRequest::operator[](const std::string& name) const {
    std::list<Parameter>::const_iterator i = find(name);
    if (i == params_.end()) {
        std::ostringstream oss;
        oss << "Parameter '" << name << "' is undefined";
        throw eckit::UserError(oss.str());
    }
    const std::vector<std::string>& c = (*i).values();
    if (c.size() > 1) {
        std::ostringstream oss;
        oss << "Parameter '" << name << "' has more than one value";
        throw eckit::UserError(oss.str());
    }

    return c[0];
}


void MarsRequest::getParams(std::vector<std::string>& p) const {
    p.clear();
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        p.push_back((*i).name());
    }
}

size_t MarsRequest::count() const {
    size_t result = 1;
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        result *= (*i).count();
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
void expand_along_keys(
    const MarsRequest& prototype,
    const std::vector<std::pair<std::string, std::vector<std::string>>>& expvalues,
    std::vector<MarsRequest>& requests,
    size_t i) {

    if(i == expvalues.size()) {
        requests.push_back(prototype);
        return;
    }

    const std::string& key = expvalues[i].first;
    const std::vector<std::string>& values = expvalues[i].second;

    MarsRequest req(prototype);
    for (auto& value : values) {
        req.setValue(key, value);
        expand_along_keys(req, expvalues, requests, i+1);
    }
}

std::vector<MarsRequest> MarsRequest::split(const std::vector<std::string>& keys) const {

    size_t n = 1;

    LOG_DEBUG_LIB(LibMetkit) << "Splitting request with keys" << keys << std::endl;

    std::vector<std::pair<std::string, std::vector<std::string>>> expvalues;
    for (auto& key : keys) {
        std::vector<std::string> v = values(key, true); // ok to be empty
        LOG_DEBUG_LIB(LibMetkit) << "splitting along key " << key << " n values " << v.size() <<  " values " << v << std::endl;
        if (v.empty()) continue;
        n *= v.size();
        expvalues.emplace_back(std::make_pair(key, v));
    }

    std::vector<MarsRequest> requests;
    requests.reserve(n);

    if(n == 1) {
        requests.push_back(*this);
        return requests;
    }

    expand_along_keys(*this, expvalues, requests, 0);

    return requests;
}

std::vector<MarsRequest> MarsRequest::split(const std::string& key) const {
    std::vector<std::string> keys = { key };
    return split( keys );
}

void MarsRequest::merge(const MarsRequest& other) {
    for (auto& param : params_) {
        LOG_DEBUG_LIB(LibMetkit) << "Merging parameter " << param << std::endl;
        auto it = other.find(param.name());
        if (it != other.params_.end())
            param.merge(*it);
    }
}

MarsRequest MarsRequest::subset(const std::set<std::string>& keys) const {
    MarsRequest req(verb_);
    for (std::list<Parameter>::const_iterator it = params_.begin(); it != params_.end(); ++it) {
        if (keys.find(it->name()) != keys.end()) {
            req.params_.push_back(*it);
        }
    }
    return req;
}


MarsRequest MarsRequest::extract(const std::string& category) const {
    MarsRequest req(verb_);
    for (std::list<Parameter>::const_iterator it = params_.begin(); it != params_.end(); ++it) {
        if (it->type().category() == category) {
            req.params_.push_back(*it);
        }
    }
    return req;
}

void MarsRequest::verb(const std::string& verb) {
    verb_ = verb;
}

bool MarsRequest::operator<(const MarsRequest& other) const {
    if (verb_ != other.verb_) {
        return verb_ < other.verb_;
    }
    return params_ < other.params_;
}


void MarsRequest::setValue(const std::string& name, const char* value) {
    std::string v(value);
    setValue(name, v);
}


const std::string& MarsRequest::verb() const {
    return verb_;
}

std::list<Parameter>::const_iterator MarsRequest::find(const std::string& name) const {
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).name() == name) {
            return i;
        }
    }
    return params_.end();
}

std::list<Parameter>::iterator MarsRequest::find(const std::string& name) {
    for (std::list<Parameter>::iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).name() == name) {
            return i;
        }
    }
    return params_.end();
}

void MarsRequest::erase(const std::string& name) {
    auto it = find(name);
    if (it != params_.end()) {
        params_.erase(it);
    }
}

std::string MarsRequest::asString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}
//----------------------------------------------------------------------------------------------------------------------

std::vector<MarsRequest> MarsRequest::parse(std::istream& in, bool strict) {
    MarsParser parser(in);
    MarsExpension expand(true, strict);
    return expand.expand(parser.parse());
}

MarsRequest MarsRequest::parse(const std::string& s, bool strict) {
    std::istringstream in(s);
    std::vector<MarsRequest> v = parse(in, strict);
    ASSERT(v.size() == 1);
    return v[0];
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
