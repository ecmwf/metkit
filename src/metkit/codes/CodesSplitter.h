/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/message/Splitter.h"


namespace metkit {
namespace codes {

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

}  // namespace codes
}  // namespace metkit
