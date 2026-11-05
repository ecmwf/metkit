// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeMixed.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeMixed : public Type {

public:  // methods

    TypeMixed(const std::string& name, const eckit::Value& settings);

    ~TypeMixed() noexcept override;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::vector<std::pair<std::unique_ptr<Context>, Type*>> types_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
