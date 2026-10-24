// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

    TypeInteger(const std::string& type, MarsKeyword key, const eckit::Value& val);
    TypeInteger(const std::string& type, MarsKeyword key, MMapFileReader& file);
    ~TypeInteger() noexcept override = default;

    void write(std::ofstream& file) const override;

    static long parse(const std::string_view value, bool acceptNegative = true);

protected:

    bool ok(const std::string& value, long& n) const;
    bool expand(std::string& value, const MarsRequest& request) const override;

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
