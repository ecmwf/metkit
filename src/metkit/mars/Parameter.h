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

namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace metkit::mars {

class Type;
class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------

class Parameter;
class ParameterBase {
public:

    ParameterBase() = default;
    ParameterBase(const Parameter& other);
    ParameterBase(const std::vector<std::string>& values) : values_(values) {}
    ParameterBase(std::vector<std::string>&& values) : values_(std::move(values)) {}

    virtual ~ParameterBase() = default;

    virtual Keyword id() const = 0;
    virtual const std::string& name() const = 0;

    const std::vector<std::string>& values() const { return values_; }
    void values(const std::vector<std::string>& values);

    virtual bool multiple() const;

    virtual bool filter(const std::vector<std::string>& filter);
    virtual bool filter(Keyword keyword, const std::vector<std::string>& filter);
    virtual bool matches(const std::vector<std::string>& matches) const;

    void merge(const Parameter& p);

    virtual size_t count() const;
    
    virtual const Type& type() const { NOTIMP; }

    virtual void print(std::ostream&) const = 0;

protected:

    std::vector<std::string> values_;
};


class Parameter {
public:

    Parameter() = default;
    Parameter(const Parameter& other);
    // Parameter(const std::vector<std::string>& values) : im(values) {}
    Parameter(const std::string& name, const std::vector<std::string>& values);
    Parameter(std::unique_ptr<ParameterBase>&& param);
    // Parameter(std::vector<std::string>&& values) : values_(std::move(values)) {}

    Parameter& operator=(Parameter&& other) = default;

    Keyword id() const { return impl_->id(); }
    const std::string& name() const { return impl_->name(); }

    const std::vector<std::string>& values() const { return impl_->values(); }
    void values(const std::vector<std::string>& values) { impl_->values(values); }

    bool multiple() const { return impl_->multiple(); }

    bool filter(const std::vector<std::string>& filter) { return impl_->filter(filter); }
    bool filter(Keyword keyword, const std::vector<std::string>& filter) { return impl_->filter(keyword, filter); }
    bool matches(const std::vector<std::string>& matches) const { return impl_->matches(matches); }

    void merge(const Parameter& p) { impl_->merge(p); }

    size_t count() const { return impl_->count(); }

    const Type& type() const { return impl_->type(); }

protected:

    void print(std::ostream& s) const { impl_->print(s); }

    friend std::ostream& operator<<(std::ostream& s, const Parameter& p) {
        p.print(s);
        return s;
    }

private:
    std::unique_ptr<ParameterBase> impl_;
};

class StringParameter : public ParameterBase {

public:

    StringParameter(const Parameter& other) : ParameterBase(other.values()), name_(other.name()) {}
    StringParameter& operator=(const StringParameter&);

    StringParameter(const std::string& name) : name_(name) {}
    StringParameter(const std::string& name, const std::vector<std::string>& values) : ParameterBase(values), name_(name) {}
    StringParameter(const std::string& name, std::vector<std::string>&& values) : ParameterBase(std::move(values)), name_(name) {}

    Keyword id() const override;
    const std::string& name() const override { return name_; }

    void print(std::ostream&) const override;

private:  // members

    std::string name_;
};

class TypeParameter : public ParameterBase {
public:  // methods

    TypeParameter();
    TypeParameter(const std::vector<std::string>& values, std::shared_ptr<const Type> = 0);
    TypeParameter(const TypeParameter&);
    ~TypeParameter() override;
    
    TypeParameter& operator=(const TypeParameter&);
    bool operator<(const TypeParameter&) const;

    Keyword id() const override;
    const std::string& name() const override;

    bool multiple() const override;

    bool filter(const std::vector<std::string>& filter) override;
    bool filter(Keyword keyword, const std::vector<std::string>& filter) override;
    bool matches(const std::vector<std::string>& matches) const override;

    size_t count() const override;

    const Type& type() const override { return *type_; }

    void print(std::ostream&) const override;

private:  // members

    std::shared_ptr<const Type> type_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
