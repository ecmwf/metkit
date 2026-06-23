// metkit MarsRequest bridge — wraps `metkit::mars::MarsRequest`.
#pragma once

#include "EckitBridge.h"
#include "metkit/mars/MarsRequest.h"

#include "rust/cxx.h"

#include <cstdint>
#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `metkit::mars::MarsRequest` for Rust FFI.
class MarsRequestWrapper {
    metkit::mars::MarsRequest request_;

public:

    MarsRequestWrapper() = default;
    explicit MarsRequestWrapper(metkit::mars::MarsRequest r) : request_(std::move(r)) {}

    // Query
    rust::String verb() const;
    bool has(rust::Str key) const;
    rust::Vec<rust::String> values(rust::Str key) const;
    rust::String get_first(rust::Str key) const;
    bool empty() const;
    size_t count() const;
    bool matches(const MarsRequestWrapper& filter) const;
    rust::Vec<rust::String> params() const;

    // Mutation
    void set_verb(rust::Str verb);
    void set_value_string(rust::Str key, rust::Str value);
    void set_values(rust::Str key, rust::Vec<rust::String> values);
    void set_value_long(rust::Str key, int64_t value);
    void unset_values(rust::Str key);

    // Extract parameters by category
    std::unique_ptr<MarsRequestWrapper> extract(rust::Str category) const;

    // Expansion
    std::unique_ptr<MarsRequestWrapper> expand(bool inherit, bool strict) const;

    // Stream serialization
    void encode(eckit_bridge::StreamWrapper& stream) const;

    // Output
    rust::String to_json() const;
    rust::String dump() const;

    /// Create a `MarsRequestHandle` — DataHandle for Hermes retrieve/list/get.
    std::unique_ptr<eckit_bridge::DataHandleWrapper> make_handle(const eckit_bridge::ConfigWrapper& config) const;

    // Access underlying
    const metkit::mars::MarsRequest& inner() const { return request_; }
    metkit::mars::MarsRequest& inner() { return request_; }

    // ============== Factories ==============

    /// Create a MarsRequest with the given verb.
    static std::unique_ptr<MarsRequestWrapper> create(rust::Str verb);

    /// Build a MarsRequest from an eckit message.
    static std::unique_ptr<MarsRequestWrapper> from_message(const eckit_bridge::MessageWrapper& msg);

    /// Decode a MarsRequest from a stream.
    static std::unique_ptr<MarsRequestWrapper> decode(eckit_bridge::StreamWrapper& stream);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
