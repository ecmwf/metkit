/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeToByList.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeToByList_H
#define metkit_TypeToByList_H

#include "metkit/mars/TypeInteger.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeToByList : public TypeInteger {

public:  // methods

    TypeToByList(const std::string& name, const eckit::Value& settings);

    virtual ~TypeToByList() override;

private:  // methods

    virtual void print(std::ostream& out) const override;
    virtual void expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const override;

    long by_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
