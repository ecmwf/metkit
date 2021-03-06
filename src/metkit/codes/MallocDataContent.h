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
/// @author Tiago Quintino
/// @date   Jun 2020

#pragma once

#include "metkit/codes/DataContent.h"
#include "eckit/io/Offset.h"


namespace metkit {
namespace codes {

class MallocDataContent : public DataContent {
public:

    MallocDataContent(void* data, size_t size, const eckit::Offset& offset);
    ~MallocDataContent();

private:  // methods
    void print(std::ostream& s) const override;
    eckit::Offset offset() const override;

    eckit::message::MessageContent* transform(const eckit::StringDict&) const override;

private:  // members
    void* buffer_;
    eckit::Offset offset_;
};


}  // namespace codes
}  // namespace metkit
