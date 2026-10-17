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

    TypeMixed(const std::string& type, Keyword key, const eckit::Value& val);
    TypeMixed(const std::string& type, Keyword key, MemFile& file);

    ~TypeMixed() noexcept override = default;

    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::list<std::pair<std::reference_wrapper<const Context>, std::shared_ptr<Type>>> types_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
