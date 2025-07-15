/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeEnum.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeEnum : public Type {

public:  // methods

    TypeEnum(const std::string& name, const eckit::Value& settings);

    ~TypeEnum() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    void reset() override;
    std::vector<std::string> expand(const MarsExpandContext& ctx, const std::string& value, const MarsRequest& request) const override;

    std::vector<std::string> parseEnumValue(const std::string& name, const eckit::Value& val, std::set<std::string>& values, bool uppercase, bool allowDuplicates = false);

private:  // members

    StringManyMap mapping_;
    std::vector<std::string> values_;

    mutable StringManyMap cache_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
