// metkit CodesHandle bridge — wraps `metkit::codes::CodesHandle`.
#pragma once

#include "metkit/codes/api/CodesAPI.h"

#include "rust/cxx.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `metkit::codes::CodesHandle` for Rust FFI.
class CodesHandleWrapper {
    std::unique_ptr<metkit::codes::CodesHandle> handle_;

public:

    explicit CodesHandleWrapper(std::unique_ptr<metkit::codes::CodesHandle> h) : handle_(std::move(h)) {}

    // Query
    bool is_defined(rust::Str key) const;
    bool is_missing(rust::Str key) const;
    bool has(rust::Str key) const;

    // Scalar get
    rust::String get_string(rust::Str key) const;
    int64_t get_long(rust::Str key) const;
    double get_double(rust::Str key) const;

    // Array get
    rust::Vec<double> get_double_array(rust::Str key) const;
    rust::Vec<int64_t> get_long_array(rust::Str key) const;

    // Scalar set
    void set_string(rust::Str key, rust::Str value);
    void set_long(rust::Str key, int64_t value);
    void set_double(rust::Str key, double value);

    // Array set
    void set_double_array(rust::Str key, rust::Slice<const double> values);

    // Missing
    void set_missing(rust::Str key);

    // Size and data
    size_t value_count(rust::Str key) const;
    size_t message_size() const;
    rust::Slice<const uint8_t> message_data() const;

    /// Deep-copy this handle (wraps `metkit::codes::CodesHandle::clone()`).
    std::unique_ptr<CodesHandleWrapper> clone() const;

    // Access underlying
    const metkit::codes::CodesHandle& inner() const { return *handle_; }
    metkit::codes::CodesHandle& inner() { return *handle_; }

    // ============== Factories ==============

    static std::unique_ptr<CodesHandleWrapper> from_message(rust::Slice<const uint8_t> data);
    static std::unique_ptr<CodesHandleWrapper> from_file(rust::Str path);
    static std::unique_ptr<CodesHandleWrapper> from_file_at_offset(rust::Str path, int64_t offset);
    static std::unique_ptr<CodesHandleWrapper> from_sample(rust::Str sample);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
