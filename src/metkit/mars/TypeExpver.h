/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
    TypeExpver(Keyword keyword, const eckit::Value& settings);
    // static std::shared_ptr<TypeExpver> create(Keyword keyword, const eckit::Value& settings){
    //     return std::shared_ptr<TypeExpver>(new TypeExpver(keyword, settings));
    // }
    ~TypeExpver() noexcept override = default;

    bool expand(std::string& value, const MarsRequest& request) const override;

private:  // methods

    void print(std::ostream& out) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
