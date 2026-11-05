// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/codes/api/CodesTypes.h"

namespace metkit::codes {

CodesException::CodesException(const std::string& reason, const eckit::CodeLocation& l) :
    eckit::Exception(std::string("CodesException: ") + reason, l) {};

CodesWrongLength::CodesWrongLength(const std::string& reason, const eckit::CodeLocation& l) :
    CodesException(reason, l) {};

}  // namespace metkit::codes
