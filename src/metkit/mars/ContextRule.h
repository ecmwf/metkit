/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

ContextRule::ContextRule(const std::string& k) : key_(k) {}

std::ostream& operator<<(std::ostream& s, const ContextRule& r) {
    r.print(s);
    return s;
}

//----------------------------------------------------------------------------------------------------------------------

class Include : public ContextRule {
public:

    Include(const std::string& k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}

    bool matches(MarsRequest req) const override {
        if (key_ == "_verb") {
            return (vals_.find(req.verb()) != vals_.end());
        }
        if (!req.has(key_)) {
            return false;
        }
        for (const std::string& v : req.values(key_)) {
            if (vals_.find(v) != vals_.end()) {
                return true;
            }
        }
        return false;
    }

private:  // methods

    void print(std::ostream& out) const override { out << "Include[key=" << key_ << ",vals=[" << vals_ << "]]"; }

private:

    std::set<std::string> vals_;
};

class Exclude : public ContextRule {
public:

    Exclude(const std::string& k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}
    bool matches(MarsRequest req) const override {
        if (!req.has(key_)) {
            return false;
        }
        for (const std::string& v : req.values(key_)) {
            if (vals_.find(v) != vals_.end()) {
                return false;
            }
        }
        return true;
    }

private:  // methods

    void print(std::ostream& out) const override { out << "Exclude[key=" << key_ << ",vals=[" << vals_ << "]]"; }

private:

    std::set<std::string> vals_;
};

class Undef : public ContextRule {
public:

    Undef(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override { return !req.has(key_); }

private:  // methods

    void print(std::ostream& out) const override { out << "Undef[key=" << key_ << "]"; }
};

class Def : public ContextRule {
public:

    Def(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override { return req.has(key_); }

private:  // methods

    void print(std::ostream& out) const override { out << "Def[key=" << key_ << "]"; }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
