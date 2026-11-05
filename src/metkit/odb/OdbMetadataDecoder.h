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

#include <functional>
#include <map>
#include <set>
#include <string>

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

    void finalize();

    static const std::vector<std::string>& columnNames();
    static std::vector<std::string> columnNames(const odc::api::Frame& frame);

private:  // methods

    template <typename T>
    void visit(const std::string& columnName, const std::set<T>& vals, const metkit::mars::MarsLanguage& language);

    /// If the column belongs to a conditional rule, store it for evaluation in finalize() and return true.
    template <typename T>
    bool deferIfConditional(const std::string& columnName, const std::set<T>& vals);

    /// Decode the values of a column (type lookup, value mapping, tidy) and pass them to the gatherer.
    void process(const std::string& columnName, const std::set<long>& vals);
    void process(const std::string& columnName, const std::set<double>& vals);
    void process(const std::string& columnName, const std::set<std::string>& vals);

private:  // members

    metkit::mars::MarsLanguage language_;
    eckit::message::MetadataGatherer& gather_;
    eckit::message::GetMetadataOptions options_;

    std::map<std::string, std::set<std::string>> keywordValues_;  ///< Decoded MARS values seen per keyword
    std::map<std::string, std::function<void()>> deferred_;       ///< Pending conditional columns
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit

#endif
