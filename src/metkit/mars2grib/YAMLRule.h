/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   YAMLRule.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_mars2grib_YAMLRule_H
#define metkit_mars2grib_YAMLRule_H

#include "metkit/mars2grib/Rule.h"

namespace metkit::mars2grib {

namespace YAMLAction {
struct Action;

struct Printable {
    virtual void print(std::ostream&) const = 0;
};

struct Log {
    const Printable* printable;  // May be null
    std::optional<std::string> customMessage;
};


struct Action: Printable {
    virtual ~Action()                                                                                                              = default;
    virtual void apply(std::vector<Log>& logTrace, const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& out) const = 0;
};
};  // namespace YAMLAction


/**
 * Rule for any declarative formate that can be parsed to an `eckit::LocalConfiguration` (YAML & JSON)...
 *
 * Outer object is expected to be a dict and represents an 'action'.
 *
 * Type of actions:
 *  - Mapping:
 *    Keys:
 *      - `key`:
 *          Key which to look up.
 *      - `dict` (optional):
 *          Where to look up the key (`work` or `initial`). Default is `work`.
 *      - `value-map`:
 *          Map whose keys are possible values for the looked up `key`.
 *          Maps to another action.
 *      - `default` (optional):
 *          Default action if the looked up value is not listed in `value-map`.
 *  - Failure (throw exception)
 *    Keys: `fail` (String with error message)
 *  - Write (write keys to workdir and/or keysetter)
 *    Keys: `write`, `write-out`, `write-work`
 *  - Null/Pass (do dothing)
 *    keys: `pass` (null or String with log message)
 */
class YAMLRule : public GenericRule {
public:
    virtual ~YAMLRule() {}

    YAMLRule(const eckit::LocalConfiguration&, const std::string& ruleSource);
    YAMLRule(const eckit::LocalConfiguration&);
    YAMLRule(const eckit::PathName&);
    YAMLRule(const RuleConfiguration& ruleConf);

    void apply(const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& out) const override;


private:
    std::unique_ptr<YAMLAction::Action> action_;
    std::string ruleSource_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
