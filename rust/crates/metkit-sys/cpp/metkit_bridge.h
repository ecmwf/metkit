// metkit C++ bridge for Rust FFI
#pragma once

// Use eckit's auto-generated exception handler
#include "eckit_exceptions.h"

// eckit-sys bridge — provides StreamWrapper
#include "eckit_bridge.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/MarsRequestHandle.h"
#include "metkit/mars/ParamID.h"
#include "metkit/mars/RequestEnvironment.h"

#include "rust/cxx.h"

#include <memory>
#include <string>
#include <vector>

namespace metkit_bridge {

// ==================== MarsRequest ====================

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

    // Access underlying
    const metkit::mars::MarsRequest& inner() const { return request_; }
    metkit::mars::MarsRequest& inner() { return request_; }
};

// ==================== CodesHandle ====================

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

    // Clone
    std::unique_ptr<CodesHandleWrapper> clone() const;

    // Access underlying
    const metkit::codes::CodesHandle& inner() const { return *handle_; }
    metkit::codes::CodesHandle& inner() { return *handle_; }
};

// CodesHandle factory functions
std::unique_ptr<CodesHandleWrapper> codes_handle_from_message(rust::Slice<const uint8_t> data);
std::unique_ptr<CodesHandleWrapper> codes_handle_from_file(rust::Str path);
std::unique_ptr<CodesHandleWrapper> codes_handle_from_file_at_offset(rust::Str path, int64_t offset);
std::unique_ptr<CodesHandleWrapper> codes_handle_from_sample(rust::Str sample);

// ==================== HyperCube ====================

/// Wraps `metkit::hypercube::HyperCube` for Rust FFI.
class HyperCubeWrapper {
    std::unique_ptr<metkit::hypercube::HyperCube> cube_;

public:

    explicit HyperCubeWrapper(const MarsRequestWrapper& request);

    size_t size() const;
    size_t count() const;
    size_t count_vacant() const;
    bool contains(const MarsRequestWrapper& request) const;
    bool clear(const MarsRequestWrapper& request);
    size_t field_ordinal(const MarsRequestWrapper& request) const;

    // Access underlying
    const metkit::hypercube::HyperCube& inner() const { return *cube_; }
};

std::unique_ptr<HyperCubeWrapper> hypercube_create(const MarsRequestWrapper& request);

// ==================== MarsRequest factory ====================

std::unique_ptr<MarsRequestWrapper> request_create(rust::Str verb);
std::unique_ptr<MarsRequestWrapper> request_from_message(const eckit_bridge::MessageWrapper& msg);
std::unique_ptr<MarsRequestWrapper> request_decode(eckit_bridge::StreamWrapper& stream);

/// Create a `MarsRequestHandle` — DataHandle for Hermes retrieve/list/get.
std::unique_ptr<eckit_bridge::DataHandleWrapper> mars_request_handle(const MarsRequestWrapper& request,
                                                                     const eckit_bridge::ConfigWrapper& config);

/// Holds parsed requests — parse once, iterate by index.
class ParsedRequestsWrapper {
    std::vector<metkit::mars::MarsRequest> requests_;

public:

    ParsedRequestsWrapper() = default;
    ParsedRequestsWrapper(rust::Str input, bool strict);
    void push(const metkit::mars::MarsRequest& r) { requests_.push_back(r); }
    size_t count() const;
    std::unique_ptr<MarsRequestWrapper> at(size_t index) const;
};

std::unique_ptr<ParsedRequestsWrapper> parse_requests(rust::Str input, bool strict);

/// Raw parse without verb validation — uses MarsParser directly.
std::unique_ptr<ParsedRequestsWrapper> parse_requests_raw(rust::Str input);

// ==================== RequestEnvironment ====================

void request_environment_init(rust::Vec<rust::String> keys, rust::Vec<rust::String> values);
std::unique_ptr<MarsRequestWrapper> request_environment_request();

// ==================== MarsLanguage ====================

class MarsLanguageWrapper {
    std::unique_ptr<metkit::mars::MarsLanguage> lang_;

public:

    explicit MarsLanguageWrapper(rust::Str verb);

    rust::Vec<rust::String> sink_keywords() const;
    bool is_data(rust::Str keyword) const;
};

std::unique_ptr<MarsLanguageWrapper> language_create(rust::Str verb);

// ==================== ParamID / WindFamily ====================

size_t wind_family_count();
rust::String wind_family_u(size_t index);
rust::String wind_family_v(size_t index);
rust::String wind_family_vo(size_t index);
rust::String wind_family_d(size_t index);

}  // namespace metkit_bridge
