/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeDate.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeDate_H
#define metkit_TypeDate_H

#include "metkit/mars/Type.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeDate : public Type {

public:  // methods

    TypeDate(const std::string& name, const eckit::Value& settings);

    virtual ~TypeDate() override;

private:  // methods

    virtual void print(std::ostream& out) const override;
    virtual void pass2(const MarsExpandContext& ctx, MarsRequest& request) override;
    virtual void expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const override;
    virtual bool expand(const MarsExpandContext& ctx, std::string& value) const override;

    long by_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
