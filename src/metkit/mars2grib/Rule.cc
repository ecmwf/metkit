/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars2grib/Rule.h"
#include "metkit/mars2grib/Mars2GribException.h"
#include "metkit/config/LibMetkit.h"

#include <ostream>


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------


RuleList::RuleList(Rules&& rules) :
    rules_{std::move(rules)} {};

namespace {

Rules buildRules(const std::vector<eckit::LocalConfiguration>& conf) {
    Rules rules;
    std::size_t i=0;
    for (const auto& subConf : conf) {
        ++i;
        if (!subConf.has("type")) {
            std::ostringstream oss;
            oss << "No key \"type\" in entry " << i << " of rule list: " << subConf;
            throw Mars2GribException(oss.str(), Here());
        }
        rules.push_back(RuleFactory::instance().build(subConf.getString("type"), RuleConfiguration{subConf}));
    };
    return rules;
}

std::vector<eckit::LocalConfiguration> buildRules(const eckit::Configuration& conf) {
    return conf.getSubConfigurations();
};


}  // namespace

RuleList::RuleList(const std::vector<eckit::LocalConfiguration>& conf) :
    RuleList(buildRules(conf)){};
RuleList::RuleList(const eckit::Configuration& conf) :
    RuleList(buildRules(conf)) {}


void RuleList::apply(const eckit::ValueMap& initial, KeySetter& out) const {
    eckit::ValueMap workdict{initial};

    for (auto& rule : rules_) {
        rule->apply(initial, workdict, out);
    }
};

//----------------------------------------------------------------------------------------------------------------------

RuleConfiguration::RuleConfiguration(const eckit::Configuration& config) :
    eckit::LocalConfiguration{config} {}

//----------------------------------------------------------------------------------------------------------------------

RuleFactory& RuleFactory::instance() {
    static RuleFactory singleton;
    return singleton;
}

void RuleFactory::enregister(const std::string& name, const RuleBuilderBase* builder) {
    std::lock_guard<std::recursive_mutex> lock{mutex_};
    ASSERT(factories_.find(name) == factories_.end());
    factories_[name] = builder;
}

void RuleFactory::deregister(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock{mutex_};
    ASSERT(factories_.find(name) != factories_.end());
    factories_.erase(name);
}

void RuleFactory::list(std::ostream& out) {
    std::lock_guard<std::recursive_mutex> lock{mutex_};

    const char* sep = "";
    for (auto const& sinkFactory : factories_) {
        out << sep << sinkFactory.first;
        sep = ", ";
    }
}

std::unique_ptr<GenericRule> RuleFactory::build(const std::string& name, const RuleConfiguration& compConf) {
    std::lock_guard<std::recursive_mutex> lock{mutex_};

    LOG_DEBUG_LIB(LibMetkit) << "Looking for RuleFactory [" << name << "]" << std::endl;

    auto f = factories_.find(name);

    if (f != factories_.end())
        return f->second->make(compConf);

    eckit::Log::error() << "No RuleFactory for [" << name << "]" << std::endl;
    eckit::Log::error() << "RuleFactories are:" << std::endl;
    for (auto const& factory : factories_) {
        eckit::Log::error() << "   " << factory.first << std::endl;
    }
    throw eckit::SeriousBug(std::string("No RuleFactory called ") + name);
}


RuleBuilderBase::RuleBuilderBase(const std::string& name) :
    name_(name) {
    RuleFactory::instance().enregister(name, this);
}

RuleBuilderBase::~RuleBuilderBase() {
    RuleFactory::instance().deregister(name_);
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
