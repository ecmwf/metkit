/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeAny.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeAny : public Type {

public:  // methods

    TypeAny(const std::string& name, const eckit::Value& settings = eckit::Value());

    ~TypeAny() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
