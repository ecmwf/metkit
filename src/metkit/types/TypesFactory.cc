/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/thread/AutoLock.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/value/Value.h"

#include "metkit/types/TypesFactory.h"



namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypesFactory::TypesFactory(const std::string &name) :
    name_(name) {
    TypesRegistry::instance().add(name, this);
}

TypesFactory::~TypesFactory() {
    TypesRegistry::instance().remove(name_);
}

Type* TypesRegistry::build(const std::string &keyword, const eckit::Value& settings) {

    std::string name;

    if (settings["type"].isList()) {
        name = "mixed";
    }
    else {

        name = std::string(settings["type"]);
    }

    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    std::map<std::string, TypesFactory *>::const_iterator j = m_.find(name);

    if (j == m_.end()) {
        eckit::Log::error() << "No TypesFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "KeywordTypes are:" << std::endl;
        for (j = m_.begin() ; j != m_.end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No TypesFactory called ") + name);
    }

    return (*j).second->make(keyword, settings);
}

void TypesRegistry::list(std::ostream& s) {

    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    s << "[";

    bool first = true;
    std::map<std::string, TypesFactory*>::const_iterator j = m_.begin();
    while (j != m_.end()) {
        if (!first) s << ",";
        s << (j++)->first;
        first = false;
    }

    s << "]";
}

Type* TypesFactory::build(const std::string &keyword, const eckit::Value& settings) {
    return TypesRegistry::instance().build(keyword, settings);
}

void TypesFactory::list(std::ostream& s) {
    TypesRegistry::instance().list(s);
}

TypesRegistry& TypesRegistry::instance()
{
    static TypesRegistry instance;
    return instance;
}

void TypesRegistry::add(const std::string& name, TypesFactory* f) {

    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(m_.find(name) == m_.end());
    m_[name] = f;
}

void TypesRegistry::remove(const std::string& name) {

    eckit::AutoLock<eckit::Mutex> lock(mutex_);
    m_.erase(name);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
