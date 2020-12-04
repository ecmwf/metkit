/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Emanuele Danovaro
/// @date   December 2020

#ifndef metkit_FieldIndexGatherer_h
#define metkit_FieldIndexGatherer_h

#include <iostream>

#include "eckit/message/Message.h"

#include "metkit/fields/FieldIndex.h"


namespace metkit {
namespace fields {

//----------------------------------------------------------------------------------------------------------------------


class FieldIndexGatherer : public metkit::fields::FieldIndex, public eckit::message::MetadataGatherer {

public:  // methods

    bool operator==(const FieldIndexGatherer& rhs);

private:  // methods

    void setValue(const std::string& name, double value) override;
    void setValue(const std::string& name, long value) override;
    void setValue(const std::string& name, const std::string& value) override;

    void print(std::ostream& s) const;

    friend std::ostream& operator<<(std::ostream& s, const FieldIndexGatherer& p)  {
        p.print(s);
        return s;
    }
};



//----------------------------------------------------------------------------------------------------------------------

} // namespace fields
} // namespace metkit

#endif
