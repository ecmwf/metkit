/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeInteger.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Emanuele Danovaro
/// @date   April 2016

#pragma once

#include <iosfwd>
#include <optional>

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeInteger : public Type {

public:  // methods

    TypeInteger(const std::string& name, const eckit::Value& settings);

protected:

    bool ok(const std::string& value, long& n) const;
    bool expand(const MarsExpandContext& ctx, std::string& value) const override;

private:  // methods

    void print(std::ostream& out) const override;

private:  // members

    struct Range {
        int lower_;
        int upper_;
    };

    std::optional<Range> range_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
