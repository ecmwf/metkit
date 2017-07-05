/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "Attribute.h"

#include "Endowed.h"
#include "Exceptions.h"
#include "Value.h"

#include <iostream>

Attribute::Attribute(Endowed &owner, const std::string &name, Value *value):
    owner_(owner), name_(name), value_(value)
{
}

Attribute::~Attribute()
{
    delete value_;
}

void Attribute::dump(std::ostream &out) const
{
    out << "\t\t" << fullName() << " = " ;
    value_->dump(out);
    out << " ;" << std::endl;
}

bool Attribute::sameAs(const Attribute &other) const {
    return name_ == other.name_;
}

std::string Attribute::fullName() const {
    return owner_.name() + ':' + name_;
}

std::string Attribute::asString() const {
    return value_->asString();
}

const std::string &Attribute::name() const {
    return name_;
}

const Value &Attribute::value() const {
    ASSERT(value_ != 0);
    return *value_;
}

void Attribute::invalidate() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::merge(const Attribute &) {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::clone(Endowed &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::create(int) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}
