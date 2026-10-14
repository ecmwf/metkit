// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/message/Message.h"

#include "metkit/mars/MarsLanguage.h"

#include "odc/api/Odb.h"


namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------

class OdbMetadataDecoder : public odc::api::SpanVisitor {

public:

    OdbMetadataDecoder(eckit::message::MetadataGatherer& gatherer, const eckit::message::GetMetadataOptions& options,
                       const metkit::mars::MarsLanguage& language);

    virtual void operator()(const std::string& columnName, const std::set<long>& vals);
    virtual void operator()(const std::string& columnName, const std::set<double>& vals);
    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals);

    static const std::vector<std::string>& columnNames();

private:  // methods

    template <typename T>
    void visit(const std::string& columnName, const std::set<T>& vals, const metkit::mars::MarsLanguage& language);

private:  // members

    const metkit::mars::MarsLanguage& language_;
    eckit::message::MetadataGatherer& gather_;
    eckit::message::GetMetadataOptions options_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
