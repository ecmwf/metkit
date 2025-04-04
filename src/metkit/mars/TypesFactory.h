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

#ifndef metkit_TypesFactory_H
#define metkit_TypesFactory_H

#include <string>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"

namespace eckit {
class Value;
}

namespace metkit {
namespace mars {

class Type;
class TypesFactory;

//----------------------------------------------------------------------------------------------------------------------


class TypesRegistry : private eckit::NonCopyable {
    eckit::Mutex mutex_;
    std::map<std::string, TypesFactory*> m_;

public:

    static TypesRegistry& instance();

    void add(const std::string& name, TypesFactory* f);
    void remove(const std::string& name);

    Type* build(const std::string& keyword, const eckit::Value&);

    void list(std::ostream& s);
};

/// A self-registering factory for producing TypesFactory instances

class TypesFactory {
public:

    virtual Type* make(const std::string& keyword, const eckit::Value& settings) const = 0;

    static Type* build(const std::string& keyword, const eckit::Value& settings);

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
    virtual Type* make(const std::string& keyword, const eckit::Value& settings) const override {
        return new T(keyword, settings);
    }

public:

    TypeBuilder(const std::string& name) : TypesFactory(name) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
