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

#ifndef metkit_OdbMetadataDecoder_h
#define metkit_OdbMetadataDecoder_h

#include "eckit/message/Message.h"

#include "metkit/mars/MarsLanguage.h"

#include "odc/api/Odb.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------

class OdbMetadataDecoder : public odc::api::SpanVisitor {

public:

    OdbMetadataDecoder(eckit::message::MetadataGatherer& gatherer, const eckit::message::GetMetadataOptions& options,
                       const std::string& verb = "retrieve");

    virtual void operator()(const std::string& columnName, const std::set<long>& vals);
    virtual void operator()(const std::string& columnName, const std::set<double>& vals);
    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals);

    static const std::vector<std::string>& columnNames();

private:  // methods

    template <typename T>
    void visit(const std::string& columnName, const std::set<T>& vals, const metkit::mars::MarsLanguage& language);

private:  // members

    metkit::mars::MarsLanguage language_;
    eckit::message::MetadataGatherer& gather_;
    eckit::message::GetMetadataOptions options_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit

#endif
