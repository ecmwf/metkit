// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/message/Splitter.h"


namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

class CodesSplitter : public eckit::message::Splitter {
public:

    CodesSplitter(eckit::PeekHandle&);
    ~CodesSplitter();

private:  // methods

    eckit::message::Message next() override;
    void print(std::ostream&) const override;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
