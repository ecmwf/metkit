// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
