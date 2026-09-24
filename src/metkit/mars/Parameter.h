/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/TypesFactory.h"

namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace metkit::mars {

class Type;
class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------

class Parameter {
public:

    Parameter() = default;
    Parameter(const std::vector<std::string>& values) : values_(values) {}
    Parameter(std::vector<std::string>&& values) : values_(std::move(values)) {}

    virtual ~Parameter() = default;

    virtual const std::string& name() const = 0;

    const std::vector<std::string>& values() const { return values_; }
    void values(const std::vector<std::string>& values);

    virtual bool filter(const std::vector<std::string>& filter);
    virtual bool filter(Keyword keyword, const std::vector<std::string>& filter);
    virtual bool matches(const std::vector<std::string>& matches) const;

    void merge(const Parameter& p);

    virtual size_t count() const;

protected:

    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Parameter& p) {
        p.print(s);
        return s;
    }

protected:

    std::vector<std::string> values_;
};

class StringParameter : public Parameter {

public:

    StringParameter(const std::string& name) : name_(name) {}
    StringParameter(const std::string& name, const std::vector<std::string>& values) : name_(name) { values_ = values; }

    const std::string& name() const override { return name_; }

private:  // methods

    void print(std::ostream&) const override;

private:  // members

    std::string name_;
};

class TypeParameter : public Parameter {
public:  // methods

    TypeParameter();
    ~TypeParameter() override;

    TypeParameter(const std::vector<std::string>& values, const Type* = 0);
    TypeParameter(const TypeParameter&);

    TypeParameter& operator=(const TypeParameter&);
    bool operator<(const TypeParameter&) const;

    bool filter(const std::vector<std::string>& filter) override;
    bool filter(Keyword keyword, const std::vector<std::string>& filter) override;
    bool matches(const std::vector<std::string>& matches) const override;

    size_t count() const override;

    const Type& type() const { return *type_; }
    Keyword id() const;
    const std::string& name() const override;

private:  // methods

    void print(std::ostream&) const override;

private:  // members

    const Type* type_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
