// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   TypeToByListQuantile.h
/// @author Emanuele Danovaro
/// @date   February 2022

#pragma once

#include "metkit/mars/Quantile.h"
#include "metkit/mars/Type.h"

class MMapFileReader;

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeToByListQuantile : public Type {

public:

    TypeToByListQuantile(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeToByListQuantile(const std::string& type, MarsKeyword key, MMapFileReader& file);

    ~TypeToByListQuantile() noexcept override = default;

    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::set<uint32_t> denominators_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
