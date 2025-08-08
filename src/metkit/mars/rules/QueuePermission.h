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

#include <memory>

class MarsQueue;

namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

class QueuePermission {

    std::unique_ptr<MarsExpression> value_;
    std::unique_ptr<MarsExpression> expr_;
    std::unique_ptr<QueuePermission> next_;
    std::string info_;


    void print(std::ostream&) const;

public:
    QueuePermission(const std::string& info, MarsExpression* e, MarsExpression* v) : value_(v), expr_(e), info_(info) {}

    eckit::Value eval(const MarsTaskProxy& t) const { return expr_->eval(t); }
    double value(const MarsTaskProxy& t) const { return value_->eval(t); }
    std::string info(const MarsTaskProxy& t) const;
    const QueuePermission* next() const { return next_.get(); }

    friend std::ostream& operator<<(std::ostream& s, const QueuePermission& p) {
        p.print(s);
        return s;
    }

    friend class ::MarsQueue;

    // For use by yacc
    void next(QueuePermission* n) { next_.reset(n); }
};


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules
