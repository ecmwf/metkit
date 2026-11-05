// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @author Emanuele Danovaro
/// @date   Mar 2022

#pragma once

#include "metkit/codes/CodesDataContent.h"

namespace metkit::codes {


//----------------------------------------------------------------------------------------------------------------------

class BufrContent : public CodesDataContent {
public:

    BufrContent(std::unique_ptr<CodesHandle> handle);

    virtual ~BufrContent() = default;

protected:

    using CodesDataContent::transform;
    void transform(const eckit::OrderedStringDict& dict) override;
};

//----------------------------------------------------------------------------------------------------------------------


}  // namespace metkit::codes
