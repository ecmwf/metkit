// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeDate.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Emanuele Danovaro
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeDate : public Type {

public:  // methods

    TypeDate(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeDate(const std::string& type, MarsKeyword key, MMapFileReader& file);
    ~TypeDate() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    void pass2(MarsRequest& request) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
