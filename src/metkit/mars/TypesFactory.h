// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypesFactory.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include <string>

#include "eckit/types/Types.h"

#include "metkit/mars/SymbolTable.h"
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

    std::unique_ptr<Type> build(MarsKeyword key, const eckit::Value& val);
    std::unique_ptr<Type> build(const std::string& name, MarsKeyword key, MMapFileReader& file);

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

    virtual std::unique_ptr<Type> make(MarsKeyword key, const eckit::Value& val) const = 0;
    virtual std::unique_ptr<Type> make(MarsKeyword key, MMapFileReader& file) const    = 0;

    static std::unique_ptr<Type> build(MarsKeyword key, const eckit::Value& val);
    static std::unique_ptr<Type> build(const std::string& name, MarsKeyword key, MMapFileReader& file);

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
    std::unique_ptr<Type> make(MarsKeyword key, const eckit::Value& settings) const override {
        return std::make_unique<T>(name_, key, settings);
    }
    std::unique_ptr<Type> make(MarsKeyword key, MMapFileReader& file) const override {
        return std::make_unique<T>(name_, key, file);
    }

public:

    TypeBuilder(const std::string& name) : TypesFactory(name) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
