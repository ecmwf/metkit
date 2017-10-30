/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/Parameter.h"
#include "metkit/types/Type.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class UndefinedType : public Type {
    virtual void print( std::ostream &out ) const  {
        out << "<undefined type>";
    }

    virtual bool filter(const std::vector< std::string >& filter,
                        std::vector<std::string>& values) const {
        NOTIMP;
    }

public:
    UndefinedType() : Type("<undefined>", eckit::Value()) { attach(); }
};


static UndefinedType undefined;


//----------------------------------------------------------------------------------------------------------------------


Parameter::Parameter():
    type_(&undefined) {
    type_->attach();
}

Parameter::~Parameter() {
    type_->detach();
}

Parameter::Parameter(const std::vector<std::string>& values, Type* type):
    type_(type),
    values_(values) {
    if (!type) {
        type_ = &undefined;
    }
    type_->attach();
}


Parameter::Parameter(const Parameter& other):
    type_(other.type_),
    values_(other.values_) {
    type_->attach();
}

Parameter& Parameter::operator=(const Parameter& other) {
    Type *old = type_;
    type_ = other.type_;
    type_->attach();
    old->detach();

    values_ = other.values_;
    return *this;
}

void Parameter::values(const std::vector<std::string>& values) {
    values_ = values;
}

bool Parameter::filter(const std::vector<std::string> &filter)  {
    return type_->filter(filter, values_);
}


bool Parameter::matches(const std::vector<std::string> &match) const {
    return type_->matches(match, values_);
}


const std::string& Parameter::name() const {
    return type_->name();
}

size_t Parameter::count() const {
    return type_->count(values_);
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values_ < other.values_;
}

//----------------------------------------------------------------------------------------------------------------------


} // namespace metkit
