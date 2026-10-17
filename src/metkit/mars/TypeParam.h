// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeParam.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeParam : public Type {

public:  // methods

    TypeParam(const std::string& type, Keyword key, const eckit::Value& val);
    TypeParam(const std::string& type, Keyword key, MemFile& file);

    ~TypeParam() noexcept override = default;

private:  // methods

    void print(std::ostream& out) const override;
    void pass2(MarsRequest& request) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
