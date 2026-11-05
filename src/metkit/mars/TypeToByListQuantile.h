// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeToByListQuantile.h
/// @author Emanuele Danovaro
/// @date   February 2022

#pragma once

#include "metkit/mars/Quantile.h"
#include "metkit/mars/Type.h"
namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeToByListQuantile : public Type {

public:  // methods

    TypeToByListQuantile(const std::string& name, const eckit::Value& settings);

    ~TypeToByListQuantile() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::set<long> denominators_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
