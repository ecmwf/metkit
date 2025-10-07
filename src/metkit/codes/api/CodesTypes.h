/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "eckit/exception/Exceptions.h"


namespace metkit::codes {

/// Exception that is thrown by the API to wrap eccodes errors
class CodesException : public eckit::Exception {
public:

    CodesException(const std::string& reason, const eckit::CodeLocation& l = eckit::CodeLocation());
};


/// Enum to classify types of handles
enum class Product {
    GRIB,
    BUFR,
};


/// Enum that redefines eccodes specific key types
enum class NativeType {
    Undefined,
    Long,
    Double,
    String,
    Bytes,
    Section,
    Label,
    Missing,
};


/// Sum type of possible values returned by eccodes.
using CodesValue = std::variant<long, double, float, std::string, std::vector<long>, std::vector<double>,
                                std::vector<float>, std::vector<std::string>, std::vector<uint8_t>>;


//----------------------------------------------------------------------------------------------------------------------

/// To be replaced with std::span in C++20.
template <typename T>
struct Span {
    using element_type = T;
    using value_type   = std::remove_cv_t<T>;

    const value_type* data_ = nullptr;
    std::size_t size_       = 0;

    const value_type* data() { return data_; }
    std::size_t size() { return size_; }

    Span() = default;

    Span(const T* p, std::size_t n) : data_(p), size_(n) {}

    // from std::vector
    template <typename Alloc>
    Span(const std::vector<value_type, Alloc>& v) : data_(v.data()), size_(v.size()) {}

    // from std::array
    template <std::size_t N>
    Span(const std::array<value_type, N>& arr) : data_(arr.data()), size_(N) {}

    // from C array
    template <std::size_t N>
    Span(const value_type (&arr)[N]) : data_(arr), size_(N) {}

    // from std::basic_string
    template <typename CharT = value_type, std::enable_if_t<std::is_same<T, CharT>::value, bool> = true>
    Span(const std::basic_string<CharT>& s) : data_(s.data()), size_(s.size()) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
