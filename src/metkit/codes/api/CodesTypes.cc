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

CodesException::CodesException(std::string msg) : message_(std::string("CodesException: ") + std::move(msg)) {}

const char* CodesException::what() const noexcept {
    return message_.c_str();
}



/// Contiguous array access
unsigned char* ByteArray::data() {
    return ptr.get();
}
const unsigned char* ByteArray::data() const {
    return ptr.get();
}
std::size_t ByteArray::size() const {
    return allocatedSize;
};

ByteArray ByteArray::makeForOverwrite(std::size_t size) {
    /// To be replaced with make_unique_for_overwrite in C++20
    return ByteArray{std::unique_ptr<unsigned char[]>(new unsigned char[size]), size};
}

ByteArray::operator Span<unsigned char>() {
    return Span<unsigned char>{data(), size()};
}
ByteArray::operator Span<const unsigned char>() const {
    return Span<const unsigned char>{data(), size()};
}

//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
