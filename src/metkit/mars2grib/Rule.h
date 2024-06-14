/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Rule.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_Rule_H
#define metkit_Rule_H

#include "eckit/value/Value.h"
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/KeySetter.h"

namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

class GenericRule {
public:
    virtual ~GenericRule(){};
    virtual void apply(const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& out) = 0;
};


using Rules = std::vector<std::unique_ptr<GenericRule>>;

class RuleList {
public:
    RuleList(Rules&&);
    RuleList(const std::vector<eckit::LocalConfiguration>&);
    RuleList(const eckit::Configuration&);
    
    void apply(const eckit::ValueMap& inital, KeySetter& out);

private:
    Rules rules_;
};


//----------------------------------------------------------------------------------------------------------------------

class RuleConfiguration: public eckit::LocalConfiguration {
public:
    RuleConfiguration() = default;
    RuleConfiguration(const eckit::Configuration& config);   
};

//----------------------------------------------------------------------------------------------------------------------

class RuleBuilderBase;

class RuleFactory : private eckit::NonCopyable {
private:  // methods
    RuleFactory() {}

public:  // methods
    static RuleFactory& instance();

    void enregister(const std::string& name, const RuleBuilderBase* builder);
    void deregister(const std::string& name);

    void list(std::ostream&);

    std::unique_ptr<GenericRule> build(const std::string&, const RuleConfiguration& conf);

private:  // members
    std::map<std::string, const RuleBuilderBase*> factories_;

    std::recursive_mutex mutex_;
};

class RuleBuilderBase : private eckit::NonCopyable {
public:  // methods
    virtual std::unique_ptr<GenericRule> make(const RuleConfiguration& conf) const = 0;

protected:  // methods
    RuleBuilderBase(const std::string&);

    virtual ~RuleBuilderBase();

    std::string name_;
};

template <class T>
class RuleBuilder final : public RuleBuilderBase {
    std::unique_ptr<GenericRule> make(const RuleConfiguration& conf) const override {
        return std::make_unique<T>(conf);
    }

public:
    RuleBuilder(const std::string& name) :
        RuleBuilderBase(name) {}
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
