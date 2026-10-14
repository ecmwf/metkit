// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeRange.h
/// @author Emanuele Danovaro
/// @date   February 2022

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

class StepRange;

//----------------------------------------------------------------------------------------------------------------------

class TypeRange : public Type {

public:  // methods

    TypeRange(const std::string& name, const eckit::Value& settings);

    ~TypeRange() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
