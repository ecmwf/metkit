/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Oct 98

/// @author Simon Smart
/// @date August 2025

#pragma once

#include "metkit/mars/rules/Expression.h"
#include "metkit/mars/rules/MarsTaskProxy.h"
#include "metkit/config/LibMetkit.h"

namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

typedef Expression<const MarsTaskProxy> MarsExpression;

class QueuePermission;
class AccessRule;

class MarsRulesParser {
public:
    static int line();

    static void include(const eckit::PathName&);

    static QueuePermission* parsePermissionFile(const eckit::PathName&);

    static AccessRule* parseAccessFile(const eckit::PathName&);
};

//----------------------------------------------------------------------------------------------------------------------

class MarsExpressionFactory {
protected:
    MarsExpressionFactory(const std::string&);

    virtual MarsExpression* make(const eckit::Value&) = 0;

public:
    static MarsExpression* create(const std::string&, const eckit::Value&);
};

//----------------------------------------------------------------------------------------------------------------------

template <class T>
class ParamExpression : public MarsExpression {
    std::string str_;
    void print(std::ostream& s) const override { s << '%' << str_ << '%'; }

public:
    explicit ParamExpression(const std::string& str) : str_(str) {}
    eckit::Value eval(const MarsTaskProxy&) const override;
};

template <class T>
eckit::Value ParamExpression<T>::eval(const MarsTaskProxy& task) const {
    LOG_DEBUG_LIB(LibMetkit) << "ParamExpression<T>::eval" << std::endl;

    std::vector<T> v;
    long n = task.getRequestValues(str_, v);

    switch (n) {
        case 0:
#if 1
            throw EvalError(std::string("Missing value for ") + str_);
#endif
            eckit::Log::warning() << "Missing value for " << str_ << ", returning 'all'" << std::endl;
            return {"all"};
            break;

        case 1:
            break;

        default:
#if 0
            {
        std::vector<Value> values; values.reserve(v.size());
        for (int i = 0; i < v.size(); i++)
            values[i] = Value(v[i]);
        return Value(values);
    }
#endif
            // throw eckit::UserError(std::string("Too many values given for ") + str_ );
        {
            eckit::Log::warning() << "Too many values given for " << str_ << std::endl;
            for (int i = 0; i < v.size(); i++)
                eckit::Log::warning() << v[i] << ' ';
            eckit::Log::warning() << std::endl;
        }
            break;
    }

    return eckit::Value(v[0]);
}

template <class T>
class ParamDefinedExpression : public MarsExpression {
    std::string str_;
    void print(std::ostream& s) const override { s << '%' << str_ << '%'; }

public:
    explicit ParamDefinedExpression(const std::string& str) : str_(str) {}
    eckit::Value eval(const MarsTaskProxy&) const override;
};

template <class T>
eckit::Value ParamDefinedExpression<T>::eval(const MarsTaskProxy& task) const {
    LOG_DEBUG_LIB(LibMetkit) << "ParamDefined<T>::eval" << std::endl;

    std::vector<T> v;
    long n = task.getRequestValues(str_, v);

    return {n > 0};
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules