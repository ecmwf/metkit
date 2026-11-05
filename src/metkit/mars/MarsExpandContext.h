// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include <iosfwd>

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsExpandContext {

    void info(std::ostream&) const {}

public:

    friend std::ostream& operator<<(std::ostream& s, const MarsExpandContext& r) { return s; }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
