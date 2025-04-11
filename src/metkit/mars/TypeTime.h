/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeTime.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Emanuele Danovaro
/// @date   April 2016

#ifndef metkit_TypeTime_H
#define metkit_TypeTime_H

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeTime : public Type {

public:  // methods

    TypeTime(const std::string& name, const eckit::Value& settings);

    ~TypeTime() noexcept override = default;

private:  // methods

    virtual void print(std::ostream& out) const override;
    virtual bool expand(const MarsExpandContext& ctx, std::string& value) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

#endif
