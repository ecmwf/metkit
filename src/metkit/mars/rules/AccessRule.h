/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date August 2025

#pragma once

#include "metkit/mars/rules/MarsExpression.h"
#include "metkit/mars/rules/MarsTaskProxy.h"

#include <map>
#include <memory>


namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

class AccessRule {

    typedef std::map<std::string, std::set<std::string> > Access;

    std::unique_ptr<MarsExpression> expr_;
    std::unique_ptr<AccessRule> next_;
    std::string name_;
    Access access_;
    std::string url_;

    void print(std::ostream&) const;

public:
    AccessRule(const std::string& name, MarsExpression* e, const eckit::Value& access, const std::string& url = "");

    eckit::Value eval(const MarsTaskProxy& t) const { return expr_->eval(t); }
    const std::string& name() const { return name_; }
    const std::string& url() const { return url_; }
    const Access& access() const { return access_; }

    friend std::ostream& operator<<(std::ostream& s, const AccessRule& p) {
        p.print(s);
        return s;
    }

    const AccessRule* next() const { return next_.get(); }

    // For use by yacc
    void next(AccessRule* n) { next_.reset(n); }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules
