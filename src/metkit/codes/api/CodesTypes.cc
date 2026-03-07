/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/CodesTypes.h"

namespace metkit::codes {

CodesException::CodesException(const std::string& reason, const eckit::CodeLocation& l) :
    eckit::Exception(std::string("CodesException: ") + reason, l) {};

CodesWrongLength::CodesWrongLength(const std::string& reason, const eckit::CodeLocation& l) :
    CodesException(reason, l) {};

}  // namespace metkit::codes
