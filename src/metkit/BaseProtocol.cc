/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File BaseProtocol.cc
// Baudouin Raoult - (c) ECMWF Feb 12


#include "metkit/BaseProtocol.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/config/Configuration.h"

namespace metkit {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, ProtocolFactory *> *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, ProtocolFactory *>();
}


BaseProtocol::BaseProtocol() {
}

BaseProtocol::BaseProtocol(eckit::Stream& s) :
    eckit::Streamable(s) {
}


BaseProtocol::BaseProtocol(const eckit::Configuration&) {
}


BaseProtocol::~BaseProtocol() {
}

const eckit::ClassSpec& BaseProtocol::classSpec() {
    static eckit::ClassSpec spec = { &Streamable::classSpec(), "BaseProtocol" };
    return spec;
}

void BaseProtocol::encode(eckit::Stream& s) const {
    eckit::Streamable::encode(s);
}

//========================================================================

ProtocolFactory::ProtocolFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("ProtocolFactory: duplication action: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ProtocolFactory::~ProtocolFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


BaseProtocol *ProtocolFactory::build( const eckit::Configuration& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const std::string& name = params.getString("class");

    std::map<std::string, ProtocolFactory *>::const_iterator j = m->find(name);
    if (j == m->end()) {
        if (j == m->end()) {
            eckit::Log::error() << "No ProtocolFactory for [" << name << "]" << std::endl;
            eckit::Log::error() << "ProtocolFactories are:" << std::endl;
            for (j = m->begin() ; j != m->end() ; ++j)
                eckit::Log::error() << "   " << (*j).first << std::endl;
            throw eckit::SeriousBug(std::string("No ProtocolFactory called ") + name);
        }
    }

    return (*j).second->make(params);
}


void ProtocolFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, ProtocolFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


}
