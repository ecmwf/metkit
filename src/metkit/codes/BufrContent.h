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

#pragma once

#include "metkit/codes/CodesContent.h"

namespace metkit {
namespace codes {


//----------------------------------------------------------------------------------------------------------------------

class BufrContent : public CodesContent {
public:

    BufrContent(codes_handle* handle, bool delete_handle);
    explicit BufrContent(const codes_handle* handle);

    ~BufrContent();

private:

    eckit::message::MessageContent* transform(const eckit::StringDict& dict) const override;
};

//----------------------------------------------------------------------------------------------------------------------


}  // namespace codes
}  // namespace metkit
