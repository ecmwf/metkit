/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_OdbToRequest_H
#define metkit_OdbToRequest_H

#include <set>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"

#include "metkit/MarsRequest.h"

namespace eckit { class DataHandle; }

namespace metkit {
namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class OdbToRequest {

public: // methods

    OdbToRequest(bool mergeSimilar=true, bool constant=true);
    ~OdbToRequest();

    std::vector<MarsRequest> odbToRequest(eckit::DataHandle& dh) const;

private: // members

    // Mapping between column names in ODB and keys in MARS requests
    std::map<std::string, std::string> mapping_;

    bool mergeSimilarBlocks_;
    bool onlyConstantColumns_;
};

///class OdbToRequest {
///
///public: // methods
///
///    OdbToRequest(bool mergeSimilar=true, bool constant=true);
///    ~OdbToRequest();
///
///    // Configure what this request parser is looking for
///
///	void parseConfig(const std::string& s);
///	void addColumn(const std::string& keyword, const std::string& columnName);
///
///    // Scan the file
///
///    bool scanFile(const eckit::PathName& path);
///    bool scanFile(const eckit::PathName& path, std::vector<ODBSpan>& spans);
///
///    // Generate the (combined) MARS request for all the data observed
///
///	std::string genRequest() const;
///
///	unsigned long long rowsNumber() { return rowsNumber_; }
///
///protected:
///    bool collectValues(const core::Table&, ODAHandle&);
///
///    std::string columnIsNotConstant(const core::Column& column) { return T::columnIsNotConstant(column); }
///	std::string columnNotFound(const std::string& columnName) { return T::columnNotFound(columnName); }
///	bool duplicateCombination(const std::string& errorMessage) { return T::duplicateCombination(errorMessage); }
///
///	std::string patchValue(const std::string& k, const std::string& value) const;
///
///private: // members
///
///    // The keyword/column mapping
///	std::vector<std::string> keywords_;
///	std::vector<std::string> columnNames_;
///
///	unsigned long long rowsNumber_;
///
///    // Describe the overall span that has been observed.
///    ODBSpan request_;
///
///    bool mergeSimilarBlocks_;
///    bool onlyConstantColumns_;
///};
///
///
///struct ODA2RequestServerTraits {
///    static std::string columnIsNotConstant(const core::Column& column)
///	{
///        std::stringstream ss;
///		ss << "Column '" << column.name() << "' is not constant"
///			<< " (min=" << column.min() << ", max=" << column.max() << ")";
///		throw eckit::UserError(ss.str());
///	}
///
///	static std::string columnNotFound(const std::string& columnName)
///	{ throw eckit::UserError(std::string("Column '") + columnName + "' not found."); }
///
///	static bool duplicateCombination(const std::string& errorMessage)
///	{ throw eckit::UserError(errorMessage); return false; }
///};
///
///struct ODA2RequestClientTraits {
///    static std::string columnIsNotConstant(const core::Column& column) { return std::string("MULTIPLE"); }
///	static std::string columnNotFound(const std::string& columnName) { return std::string("MISSING"); }
///	static bool duplicateCombination(const std::string& errorMessage)
///	{ eckit::Log::error() << errorMessage << std::endl; return false; }
///};

//----------------------------------------------------------------------------------------------------------------------

} // namespace odb
} // namespace metkit

#endif

