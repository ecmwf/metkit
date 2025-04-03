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

namespace metkit::mars {

class MarsRequest;
class MarsExpandContext;

class ContextRule {
public:

    ContextRule(const std::string& k);

    virtual ~ContextRule() = default;

    virtual bool matches(MarsRequest req) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const ContextRule& x);

protected:

    std::string key_;

private:  // methods

    virtual void print(std::ostream& out) const = 0;
};

class Context {
public:

    /// @note takes ownership of the rule
    void add(ContextRule* rule);

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

    virtual ~ITypeToByList()                                                                        = default;
    virtual void expandRanges(const MarsExpandContext& ctx, std::vector<std::string>& values) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

class Type : public eckit::Counted {
public:  // methods

    Type(const std::string& name, const eckit::Value& settings);

    ~Type() noexcept override = default;

    virtual void expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const;
    virtual bool expand(const MarsExpandContext& ctx, std::string& value) const;

    std::string tidy(const MarsExpandContext& ctx, const std::string& value) const;
    std::string tidy(const std::string& value) const;
    std::vector<std::string> tidy(const std::vector<std::string>& values) const;

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

protected:  // members

    std::string name_;
    std::string category_;

    bool flatten_;
    bool multiple_;
    bool duplicates_;

    std::map<std::unique_ptr<Context>, std::vector<std::string>> defaults_;
    std::optional<std::vector<std::string>> inheritance_;
    std::set<std::unique_ptr<Context>> only_;
    std::map<std::unique_ptr<Context>, std::string> sets_;
    std::set<std::unique_ptr<Context>> unsets_;

    std::unique_ptr<ITypeToByList> toByList_;

    std::map<std::string, std::function<bool(const std::vector<std::string>&, std::vector<std::string>&)>> filters_;

private:  // methods

    virtual void print(std::ostream& out) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
