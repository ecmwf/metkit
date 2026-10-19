// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeRegex.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "eckit/utils/Regex.h"
#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeRegex : public Type {

public:  // methods

    TypeRegex(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeRegex(const std::string& type, MarsKeyword key, MemFile& file);

    ~TypeRegex() noexcept override = default;

    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::vector<eckit::Regex> regex_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
