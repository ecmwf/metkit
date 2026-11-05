// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "eckit/io/Offset.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/types/Types.h"

namespace metkit::fields {

class FieldIndex;

class FieldIndexList {
public:

    ~FieldIndexList();

    void readFrom(eckit::Stream& s);
    void sendTo(eckit::Stream& s) const;

    eckit::OffsetList offset_;
    eckit::LengthList length_;
    std::vector<FieldIndex*> fields_;
};

}  // namespace metkit::fields
