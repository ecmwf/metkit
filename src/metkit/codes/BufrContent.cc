/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Emanuele Danovaro
/// @date   Mar 2022

#include "metkit/codes/BufrContent.h"

#include "metkit/codes/GribHandle.h"

#include "eccodes.h"

namespace metkit {
namespace codes {


//----------------------------------------------------------------------------------------------------------------------

BufrContent::BufrContent(codes_handle* handle, bool delete_handle): CodesContent(handle, delete_handle) {}

BufrContent::BufrContent(const codes_handle* handle):
    BufrContent(const_cast<codes_handle*>(handle), false) {
}

BufrContent::~BufrContent() {}


//----------------------------------------------------------------------------------------------------------------------

eckit::message::MessageContent* BufrContent::transform(const eckit::StringDict& dict) const {
    codes_handle* h = codes_handle_clone(handle_);

    std::vector<codes_values> values;

    for (auto& kv : dict) {
        codes_values v;
        v.name         = kv.first.c_str();
        v.long_value   = std::stol(kv.second);
        v.type         = GRIB_TYPE_LONG;

        values.push_back(v);
    }

    try {
        CODES_CALL(codes_set_values(h, values.data(), values.size()));
    }
    catch(...) {
        codes_handle_delete(h);
        throw;
    }

    return new BufrContent(h);
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace close
}  // namespace metkit

