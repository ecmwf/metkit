/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/KeyIterator.h"

#include "eccodes.h"


namespace metkit::codes {


KeyIteratorFlags operator|(KeyIteratorFlags a, KeyIteratorFlags b) {
    return static_cast<KeyIteratorFlags>(static_cast<std::underlying_type_t<KeyIteratorFlags> >(a) |
                                         static_cast<std::underlying_type_t<KeyIteratorFlags> >(b));
}

KeyIteratorFlags operator&(KeyIteratorFlags a, KeyIteratorFlags b) {
    return static_cast<KeyIteratorFlags>(static_cast<std::underlying_type_t<KeyIteratorFlags> >(a) &
                                         static_cast<std::underlying_type_t<KeyIteratorFlags> >(b));
}

KeyIteratorFlags operator^(KeyIteratorFlags a, KeyIteratorFlags b) {
    return static_cast<KeyIteratorFlags>(static_cast<std::underlying_type_t<KeyIteratorFlags> >(a) ^
                                         static_cast<std::underlying_type_t<KeyIteratorFlags> >(b));
}

KeyIteratorFlags operator~(KeyIteratorFlags a) {
    return static_cast<KeyIteratorFlags>(~static_cast<std::underlying_type_t<KeyIteratorFlags> >(a));
}

bool hasFlag(KeyIteratorFlags value, KeyIteratorFlags flag) {
    return (static_cast<std::underlying_type_t<KeyIteratorFlags> >(value) &
            static_cast<std::underlying_type_t<KeyIteratorFlags> >(flag)) != 0;
}



//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
