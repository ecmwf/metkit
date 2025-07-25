/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Type.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Emanuele Danovaro
/// @date   April 2016

#pragma once

#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "eckit/memory/Counted.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

/// @brief abstract class - ContextRule subclasses are used to define a context. A MarsRequest matches a context, if it
/// matches all its ContextRules
class ContextRule {
public:

    ContextRule(const std::string& k) : key_(k) {}

    virtual ~ContextRule() = default;

    virtual bool matches(MarsRequest req) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const ContextRule& r) {
        r.print(s);
        return s;
    }

protected:

    std::string key_;

private:  // methods

    virtual void print(std::ostream& out) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

/// @brief A MarsRequest matches an Include ContextRule if at least one of the mars request values matches with the
/// values associated with the Include rule
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

/// @brief A MarsRequest matches an Exclude ContextRule if none of the mars request values matches with the values
/// associated with the Exclude rule
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

/// @brief A MarsRequest matches an Undef ContextRule if the specified keyword is not defined in the mars request
class Undef : public ContextRule {
public:

    Undef(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override { return !req.has(key_); }

private:  // methods

    void print(std::ostream& out) const override { out << "Undef[key=" << key_ << "]"; }
};

/// @brief A MarsRequest matches an Undef ContextRule if the specified keyword is defined in the mars request
class Def : public ContextRule {
public:

    Def(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override { return req.has(key_); }

private:  // methods

    void print(std::ostream& out) const override { out << "Def[key=" << key_ << "]"; }
};


//----------------------------------------------------------------------------------------------------------------------

/// @brief a Context contains a list of ContextRule. A MarsRequest matches a context, if it matches all the ContextRules
/// associated
class Context {
public:

    static std::unique_ptr<Context> parseContext(eckit::Value c);

    /// @note takes ownership of the rule
    void add(std::unique_ptr<ContextRule> rule);

    bool matches(MarsRequest req) const;

    friend std::ostream& operator<<(std::ostream& s, const Context& x);

private:  // methods

    void print(std::ostream& out) const;

private:

    std::vector<std::unique_ptr<ContextRule>> rules_;
};

//----------------------------------------------------------------------------------------------------------------------

class ITypeToByList {
public:

    virtual ~ITypeToByList()                                    = default;
    virtual void expandRanges(const MarsExpandContext& ctx, std::vector<std::string>& values,
                              const MarsRequest& request) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

class Type : public eckit::Counted {
public:  // methods

    Type(const std::string& name, const eckit::Value& settings);

    ~Type() noexcept override = default;

    virtual bool expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request = {}) const;
    virtual void expand(const MarsExpandContext& ctx, std::vector<std::string>& values,
                        const MarsRequest& request = {}) const;

    std::string tidy(const std::string& value, const MarsExpandContext& ctx = DummyContext{},
                     const MarsRequest& request = {}) const;

    virtual void setDefaults(MarsRequest& request);
    virtual void setInheritance(const std::vector<std::string>& inheritance);
    virtual void check(const MarsExpandContext& ctx, const std::vector<std::string>& values) const;
    virtual void clearDefaults();
    virtual void reset();

    virtual void pass2(const MarsExpandContext& ctx, MarsRequest& request);
    virtual void finalise(const MarsExpandContext& ctx, MarsRequest& request, bool strict);

    virtual const std::vector<std::string>& flattenValues(const MarsRequest& request);
    virtual bool flatten() const;
    virtual bool multiple() const;

    virtual bool filter(const std::vector<std::string>& filter, std::vector<std::string>& values) const;
    virtual bool filter(const std::string& keyword, const std::vector<std::string>& filter,
                        std::vector<std::string>& values) const;
    virtual bool matches(const std::vector<std::string>& filter, const std::vector<std::string>& values) const;

    const std::string& name() const;
    const std::string& category() const;

    friend std::ostream& operator<<(std::ostream& s, const Type& x);

    virtual size_t count(const std::vector<std::string>& values) const;

protected:  // methods

    virtual bool hasGroups() const { return false; }
    virtual const std::vector<std::string>& group(const std::string&) const { NOTIMP; }

protected:  // members

    std::string name_;
    std::string category_;

    bool flatten_;
    bool multiple_;
    bool duplicates_;

    std::map<std::unique_ptr<Context>, std::vector<std::string>> defaults_;
    std::optional<std::vector<std::string>> inheritance_;
    std::set<std::unique_ptr<Context>> only_;
    std::map<std::unique_ptr<Context>, std::vector<std::string>> sets_;
    std::set<std::unique_ptr<Context>> unsets_;

    std::unique_ptr<ITypeToByList> toByList_;

    std::map<std::string, std::function<bool(const std::vector<std::string>&, std::vector<std::string>&)>> filters_;

private:  // methods

    virtual void print(std::ostream& out) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
