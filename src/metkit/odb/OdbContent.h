/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020


#ifndef metkit_codes_OdbContent_H
#define metkit_codes_OdbContent_H

#include "eckit/io/Buffer.h"
#include "eckit/message/MessageContent.h"

namespace metkit {
namespace codes {

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


}  // namespace codes
}  // namespace metkit


#endif
