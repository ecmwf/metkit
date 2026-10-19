// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeFloat.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Emanuele Danovaro
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeFloat : public Type {
public:  // methods

    TypeFloat(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeFloat(const std::string& type, MarsKeyword key, MemFile& file);
    ~TypeFloat() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
