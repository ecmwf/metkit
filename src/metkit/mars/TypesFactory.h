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
#include "metkit/mars/Type.h"

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

    std::shared_ptr<Type> build(Keyword key, const eckit::Value& val);
    std::shared_ptr<Type> build(const std::string& name, Keyword key, MemFile& file);

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

    virtual std::shared_ptr<Type> make(Keyword key, const eckit::Value& val) const = 0;
    virtual std::shared_ptr<Type> make(Keyword key, MemFile& file) const           = 0;

    static std::shared_ptr<Type> build(Keyword key, const eckit::Value& val);
    static std::shared_ptr<Type> build(const std::string& name, Keyword key, MemFile& file);

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
    std::shared_ptr<Type> make(Keyword key, const eckit::Value& settings) const override {
        return std::make_shared<T>(name_, key, settings);
    }
    std::shared_ptr<Type> make(Keyword key, MemFile& file) const override {
        return std::make_shared<T>(name_, key, file);
    }

public:

    TypeBuilder(const std::string& name) : TypesFactory(name) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
