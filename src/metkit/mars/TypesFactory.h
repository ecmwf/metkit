/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypesFactory.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include <string>

#include "eckit/types/Types.h"

#include "metkit/mars/Dictionary.h"

namespace eckit {
class Value;
}

namespace metkit::mars {

class Type;
class TypesFactory;

//----------------------------------------------------------------------------------------------------------------------

class TypesRegistry {

public:

    static TypesRegistry& instance();

    TypesRegistry(const TypesRegistry&)            = delete;
    TypesRegistry(TypesRegistry&&)                 = delete;
    TypesRegistry& operator=(const TypesRegistry&) = delete;
    TypesRegistry& operator=(TypesRegistry&&)      = delete;

    void add(const std::string& name, TypesFactory* f);
    void remove(const std::string& name);

    Type* build(Keyword keyword, const eckit::Value&);

    void list(std::ostream& s);

private:  // methods

    TypesRegistry() = default;

private:  // members

    eckit::Mutex mutex_;
    std::map<std::string, TypesFactory*> m_;
};

/// A self-registering factory for producing TypesFactory instances

class TypesFactory {
public:

    virtual Type* make(Keyword keyword, const eckit::Value& settings) const = 0;

    static Type* build(Keyword keyword, const eckit::Value& settings);

    static void list(std::ostream& s);

protected:

    TypesFactory(const std::string&);

    ~TypesFactory();

    std::string name_;
};

/// Templated specialisation of the self-registering factory,
/// that does the self-registration, and the construction of each object.

template <class T>
class TypeBuilder : public TypesFactory {
    Type* make(Keyword keyword, const eckit::Value& settings) const override {
        return new T(keyword, settings);
    }

public:

    TypeBuilder(const std::string& name) : TypesFactory(name) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
