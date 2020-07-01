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
/// @date   Jun 2020



#ifndef metkit_codes_MallocDataContent_H
#define metkit_codes_MallocDataContent_H

#include "metkit/codes/DataContent.h"


namespace metkit {
namespace codes {

class MallocDataContent : public DataContent {
public:

    MallocDataContent(void* data, size_t size);
    ~MallocDataContent();

private:

    void* buffer_;

    virtual void print(std::ostream & s) const;


};


}  // namespace codes
}  // namespace metkit


#endif
