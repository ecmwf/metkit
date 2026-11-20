// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020


#pragma once

#include "eckit/io/Buffer.h"
#include "eckit/message/MessageContent.h"

namespace metkit::codes {

class OdbContent : public eckit::message::MessageContent {
public:

    OdbContent(eckit::Buffer&&);
    OdbContent(eckit::DataHandle&, size_t size);
    ~OdbContent();

private:

    eckit::Buffer frame_;

    virtual size_t length() const override;
    const void* data() const override;
    virtual eckit::DataHandle* readHandle() const override;

    virtual void write(eckit::DataHandle& handle) const override;

    virtual void print(std::ostream& s) const override;
};


}  // namespace metkit::codes
