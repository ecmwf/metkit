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
#include "metkit/mars/MarsParser.h"

namespace metkit::mars {

MarsParsedRequest::MarsParsedRequest(const std::string& verb, size_t line) : verb_(verb), line_(line) {}

const std::string& MarsParsedRequest::verb() const {
    return verb_;
}

void MarsParsedRequest::getParams(std::vector<std::string>& keys) const {
    keys.clear();
    for (const auto& p : params_) {
        keys.push_back(p.name());
    }
}
const std::vector<std::string>& MarsParsedRequest::values(const std::string& key, bool emptyOk) const {
    auto p = find(key);
    if (p) {
        return p->get().values();
    }
    static const std::vector<std::string> empty;
    return empty;
}

void MarsParsedRequest::verb(const std::string& v) {
    verb_ = v;
}

void MarsParsedRequest::values(const std::string& key, const std::vector<std::string>& vals) {
    auto p = find(key);
    if (p) {
        p->get().values(vals);
    }
    else {
        params_.push_back(StringParameter{key, vals});
    }
}

std::optional<std::reference_wrapper<const Parameter>> MarsParsedRequest::find(const std::string& name) const {
    for (auto i = params_.begin(); i != params_.end(); ++i) {
        if (i->name() == name) {
            return std::cref(*i);
        }
    }
    return std::nullopt;
}
std::optional<std::reference_wrapper<Parameter>> MarsParsedRequest::find(const std::string& name) {
    for (auto i = params_.begin(); i != params_.end(); ++i) {
        if (i->name() == name) {
            return std::ref(*i);
        }
    }
    return std::nullopt;
}

void MarsParsedRequest::erase(const std::string& key) {
    for (auto i = params_.begin(); i != params_.end(); ++i) {
        if (i->name() == key) {
            params_.erase(i);
            return;
        }
    }
}

const std::list<StringParameter>& MarsParsedRequest::params() const {
    return params_;
}

void MarsParsedRequest::dump(std::ostream& s, const char* cr, const char* tab, bool verb) const {
    if (verb) {
        s << verb_ << ',';
    }
    std::string separator = "";
    if (!params_.empty()) {
        s << separator << cr << tab;
        separator = ",";

        int a = 0;
        for (const auto& p : params_) {
            if (a++) {
                s << ',' << cr << tab;
            }

            int b = 0;
            s << p.name() << "=";

            for (const auto& k : p.values()) {
                if (b++) {
                    s << '/';
                }
                MarsParser::quoted(s, k);
            }
        }
    }

    s << cr << cr;
}

void MarsParsedRequest::info(std::ostream& out) const {
    out << " Request starting line " << line_;
}

}  // namespace metkit::mars
