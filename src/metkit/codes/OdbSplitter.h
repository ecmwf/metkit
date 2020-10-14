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

#ifndef metkit_OdbSplitter_h
#define metkit_OdbSplitter_h

#include "eckit/message/Splitter.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

class OdbSplitter : public eckit::message::Splitter {
public:

    explicit OdbSplitter(eckit::PeekHandle&);
    ~OdbSplitter() override;

private: // members

    bool first_;

private: // methods

    virtual eckit::message::Message next() override;
    virtual void print(std::ostream&) const override;

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace codes
} // namespace metkit

#endif
