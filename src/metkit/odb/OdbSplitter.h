// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/io/SeekableHandle.h"
#include "eckit/message/Splitter.h"

#include "odc/api/Odb.h"

namespace eckit {
class PeekHandle;
}

namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------


class OdbSplitter : public eckit::message::Splitter {
public:

    explicit OdbSplitter(eckit::PeekHandle&);
    ~OdbSplitter() override;

private:  // members

    eckit::SeekableHandle handleWrapper_;
    odc::api::Reader reader_;
    odc::api::Frame lastFrame_;

private:  // methods

    virtual eckit::message::Message next() override;
    virtual void print(std::ostream&) const override;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
