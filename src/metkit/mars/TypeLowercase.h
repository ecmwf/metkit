// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeLowercase.h
/// @author Emanuele Danovaro
/// @date   February 2024

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeLowercase : public Type {

public:  // methods

    TypeLowercase(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeLowercase(const std::string& type, MarsKeyword key, MMapFileReader& file);
    ~TypeLowercase() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
