/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
    bool expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request) const override;

    std::set<long> denominators_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
