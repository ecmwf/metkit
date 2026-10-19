// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeExpver.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeExpver : public Type {
public:  // methods

    TypeExpver(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeExpver(const std::string& type, MarsKeyword key, MemFile& file);
    ~TypeExpver() noexcept override = default;

    bool expand(std::string& value, const MarsRequest& request) const override;

private:  // methods

    void print(std::ostream& out) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
