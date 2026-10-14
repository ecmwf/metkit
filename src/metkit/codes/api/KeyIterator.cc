// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
