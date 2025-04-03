/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeParam.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeParam_H
#define metkit_TypeParam_H

#include "metkit/mars/Type.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeParam : public Type {

public:  // methods

    TypeParam(const std::string& name, const eckit::Value& settings);

    ~TypeParam() noexcept override = default;
    ;

protected:

    virtual bool expand(const MarsExpandContext& ctx, const MarsRequest& request, std::vector<std::string>& values,
                        bool fail) const;

private:  // methods

    eckit::ValueMap expandWith_;
    bool firstRule_;

    virtual void print(std::ostream& out) const override;
    virtual void reset() override;
    virtual void pass2(const MarsExpandContext& ctx, MarsRequest& request) override;
    virtual void expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
