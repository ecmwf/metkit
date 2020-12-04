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

#ifndef metkit_OdbDecoder_h
#define metkit_OdbDecoder_h

#include "eckit/message/Decoder.h"

#include "odc/api/Odb.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

class OdbMetadataSetter : public odc::api::SpanVisitor {

public:
    static odc::api::Span span(odc::api::Frame& frame);

    OdbMetadataSetter(eckit::message::MetadataGatherer& gather);

    virtual void operator()(const std::string& columnName, const std::set<long>& vals);
    virtual void operator()(const std::string& columnName, const std::set<double>& vals);
    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals);

private:

    eckit::message::MetadataGatherer& gather_;

};

//----------------------------------------------------------------------------------------------------------------------

class OdbDecoder : public eckit::message::Decoder {

private: // methods

    virtual bool match(const eckit::message::Message&) const override;
    virtual void print(std::ostream&) const override;
    virtual void getMetadata(const eckit::message::Message& msg,
                             eckit::message::MetadataGatherer&) const override;

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace codes
} // namespace metkit

#endif
