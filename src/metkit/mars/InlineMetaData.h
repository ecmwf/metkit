/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File InlineMetaData.h
// Claude Gibert - ECMWF May 1998
// Chris Bradley - ECMWF May 2025

#ifndef InlineMetaData_H
#define InlineMetaData_H

#include <memory>

#include "eckit/io/Length.h"
#include "eckit/serialisation/Streamable.h"

#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

// The server and the client have different implementations of the MarsRequest class. This causes us a problem when we want to deserialise.
// The MarsRequest object on the mars server is registered with the ClassSpec under the name "MarsRequest", and expected to be deserialised via the reanimator.
// The use of class spec and reanimator is completely unnecessary, as the MarsRequest object is not derived from anything other than Streamable, so
// we do not need RTTI.
// The C++ client wants to create a metkit::mars::MarsRequest object.
// Yet, we cannot change the serialisation while the old mars client is still in use. So we must work around it for now.

template <typename MARS_REQUEST>
class InlineMetaDataImpl : public eckit::Streamable {
public:
    // -- Exceptions
    // None

    // -- Contructors

    InlineMetaDataImpl(const eckit::StringList& names, const std::vector<eckit::StringList>& values,
                   const eckit::Length& dataLength);
    InlineMetaDataImpl(MARS_REQUEST* r, const eckit::Length& dataLength);
    InlineMetaDataImpl(eckit::Stream&);

    // not copyable
    InlineMetaDataImpl(const InlineMetaDataImpl&) = delete;
    InlineMetaDataImpl& operator=(const InlineMetaDataImpl&) = delete;

    // -- Destructor
    virtual ~InlineMetaDataImpl() override;

    // From Streamable
    virtual void encode(eckit::Stream&) const override;

    // virtual const eckit::ReanimatorBase& reanimator() const override { return reanimator_; }
    // static const eckit::ClassSpec& classSpec() { return classSpec_; }

    eckit::Length length() const { return length_; }
    const MARS_REQUEST& request() const { return *request_; }

protected:

    void print(std::ostream&) const;

private:

    // -- Members
    std::unique_ptr<MARS_REQUEST> request_;
    eckit::Length length_;

    // static eckit::ClassSpec classSpec_;
    // static eckit::Reanimator<InlineMetaData> reanimator_;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& s, const InlineMetaDataImpl<MARS_REQUEST>& r) {
        r.print(s);
        return s;
    }
};

using InlineMetaData = InlineMetaDataImpl<MarsRequest>;

} // namespace metkit::mars

#endif  // InlineMetaData_H