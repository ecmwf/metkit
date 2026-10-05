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

#include <bitset>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "eckit/memory/Counted.h"
#include "eckit/value/Value.h"

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class NotInSet {
    std::set<std::string> set_;

public:

    NotInSet(const std::vector<std::string>& f) : set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const { return set_.find(s) == set_.end(); }
};

//----------------------------------------------------------------------------------------------------------------------

/// @brief abstract class - ContextRule subclasses are used to define a context. A MarsRequest matches a context, if it
/// matches all its ContextRules
class ContextRule {
public:

    ContextRule(Keyword k) : key_(k) {}

    virtual ~ContextRule() = default;

    Keyword key() const { return key_; }

    virtual bool matches(const MarsRequest& req) const = 0;

    bool operator<(const ContextRule& other) const;
    bool operator==(const ContextRule& other) const;

    virtual void write(std::ofstream& file) const = 0;

    static std::unique_ptr<ContextRule> parse(MemFile& file);

    friend std::ostream& operator<<(std::ostream& s, const ContextRule& r) {
        r.print(s);
        return s;
    }

protected:

    Keyword key_;

private:  // methods

    virtual void print(std::ostream& out) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

/// @brief A MarsRequest matches an Include ContextRule if at least one of the mars request values matches with the
/// values associated with the Include rule
class Include : public ContextRule {
public:

    Include(Keyword k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}
    bool matches(const MarsRequest& req) const override;

    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override { out << "Include[key=" << key_ << ",vals=[" << vals_ << "]]"; }

private:

    std::set<std::string> vals_;
};

/// @brief A MarsRequest matches an Exclude ContextRule if none of the mars request values matches with the values
/// associated with the Exclude rule
class Exclude : public ContextRule {
public:

    Exclude(Keyword k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}
    bool matches(const MarsRequest& req) const override;
    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override { out << "Exclude[key=" << key_ << ",vals=[" << vals_ << "]]"; }

private:

    std::set<std::string> vals_;
};

/// @brief A MarsRequest matches an Undef ContextRule if the specified keyword is not defined in the mars request
class Undef : public ContextRule {
public:

    Undef(Keyword k) : ContextRule(k) {}
    bool matches(const MarsRequest& req) const override;
    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override { out << "Undef[key=" << key_ << "]"; }
};

/// @brief A MarsRequest matches an Undef ContextRule if the specified keyword is defined in the mars request
class Def : public ContextRule {
public:

    Def(Keyword k) : ContextRule(k) {}
    bool matches(const MarsRequest& req) const override;
    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override { out << "Def[key=" << key_ << "]"; }
};


//----------------------------------------------------------------------------------------------------------------------

/// @brief a Context contains a list of ContextRule. A MarsRequest matches a context, if it matches all the ContextRules
/// associated
class Context {
public:

    Context(size_t id, const eckit::Value& c);
    Context(size_t id, MemFile& file);

    size_t id() const { return id_; }

    /// @note takes ownership of the rule
    void add(std::unique_ptr<ContextRule> rule);

    Keyword maxAxisIndex() const;

    bool matches(const MarsRequest& req) const;

    bool operator<(const Context& other) const;
    bool operator==(const Context& other) const;

    void write(std::ofstream& file) const;

    friend std::ostream& operator<<(std::ostream& s, const Context& x);

private:  // methods

    void print(std::ostream& out) const;

private:

    size_t id_;
    std::vector<std::unique_ptr<ContextRule>> rules_;
};

//----------------------------------------------------------------------------------------------------------------------

class ITypeToByList {
public:

    virtual ~ITypeToByList()                                                                      = default;
    virtual void write(std::ofstream& file) const                                                 = 0;
    virtual void expandRanges(std::vector<std::string>& values, const MarsRequest& request) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

enum class Category : uint8_t {
    None = 0,
    Data,
    Derived,
    PostProc,
    Sink
};

//----------------------------------------------------------------------------------------------------------------------

class TypesFactory;

class Type : public std::enable_shared_from_this<Type> {

public:  // methods

    Type(const std::string& type, Keyword key, const eckit::Value& settings);
    Type(const std::string& type, Keyword key, MemFile& file);

    virtual ~Type() = default;

    std::shared_ptr<Type> getptr() { return shared_from_this(); }
    std::shared_ptr<const Type> getptr() const { return shared_from_this(); }

    virtual bool expand(std::string& value, const MarsRequest& request = {}) const;
    void expand(std::vector<std::string>& values, const MarsRequest& request = {}) const;

    std::string tidy(const std::string& value, const MarsRequest& request = {}) const;

    virtual void setDefaults(MarsRequest& request) const;
    virtual void check(const std::vector<std::string>& values) const;
    virtual void clearDefaults();

    virtual void pass2(MarsRequest& request) const;
    virtual void finalise(MarsRequest& request, bool strict) const;

    virtual const std::vector<std::string>& flattenValues(const MarsRequest& request) const;
    virtual bool flatten() const;
    virtual bool multiple() const;

    virtual bool filter(const std::vector<std::string>& filter, std::vector<std::string>& values) const;
    virtual bool filter(Keyword keyword, const std::vector<std::string>& filter,
                        std::vector<std::string>& values) const;
    virtual bool matches(const std::vector<std::string>& filter, const std::vector<std::string>& values) const;

    Keyword id() const;
    const std::string& name() const;

    const Category& category() const;

    friend std::ostream& operator<<(std::ostream& s, const Type& x);

    virtual size_t count(const std::vector<std::string>& values) const;

    virtual void write(std::ofstream& file) const;

protected:  // methods

    virtual bool hasGroups() const { return false; }
    virtual std::optional<std::reference_wrapper<const std::vector<std::string>>> group(const std::string&) const {
        NOTIMP;
    }

    friend class MarsLanguage;

    // write() = writeCommon() + writeToByList(). Subclasses that serialise extra state must write it in the same
    // order in which their file constructor reads it (base class first, then the to-by-list, then their own state)
    void writeCommon(std::ofstream& file) const;
    void writeToByList(std::ofstream& file) const;

    void defaults(const Context& context, const std::vector<std::string>& values);
    void set(const Context& context, const std::vector<std::string>& values);
    void unset(const Context& context);

protected:  // members

    std::string typeName_;

    Keyword id_;

    std::bitset<8> flags_;
    // flags_[0] --> flatten
    // flags_[1] --> multiple
    // flags_[2] --> duplicates
    // flags_[3] --> uppercase (enum/regex)
    // flags_[4] --> firstRule (param)
    // flags_[5] --> hasGroups: only meaningful in the binary file, use hasGroups() at runtime

    Category category_;

    std::list<std::pair<std::reference_wrapper<const Context>, std::vector<std::string>>> defaults_;
    std::list<std::pair<std::reference_wrapper<const Context>, std::vector<std::string>>> sets_;
    std::list<std::reference_wrapper<const Context>> unsets_;

    std::unique_ptr<ITypeToByList> toByList_;

    std::map<Keyword, std::function<bool(const std::vector<std::string>&, std::vector<std::string>&)>> filters_;

private:  // methods

    virtual void print(std::ostream& out) const {}
    void patchRequest(MarsRequest& request, const std::vector<std::string>& values) const;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
