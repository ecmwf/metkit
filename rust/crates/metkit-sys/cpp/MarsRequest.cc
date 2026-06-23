// metkit MarsRequest bridge — implementation.

#include "metkit_exceptions.h"

#include "MarsRequest.h"

#include "eckit/log/JSON.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequestHandle.h"

#include <sstream>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

rust::String MarsRequestWrapper::verb() const {
    return rust::String(request_.verb());
}

bool MarsRequestWrapper::has(rust::Str key) const {
    return request_.has(std::string(key));
}

rust::Vec<rust::String> MarsRequestWrapper::values(rust::Str key) const {
    const auto& vals = request_.values(std::string(key));
    rust::Vec<rust::String> result;
    result.reserve(vals.size());
    for (const auto& v : vals) {
        result.push_back(rust::String(v));
    }
    return result;
}

rust::String MarsRequestWrapper::get_first(rust::Str key) const {
    return rust::String(request_[std::string(key)]);
}

bool MarsRequestWrapper::empty() const {
    return request_.empty();
}

size_t MarsRequestWrapper::count() const {
    return request_.count();
}

bool MarsRequestWrapper::matches(const MarsRequestWrapper& filter) const {
    return request_.matches(filter.request_);
}

rust::Vec<rust::String> MarsRequestWrapper::params() const {
    auto p = request_.params();
    rust::Vec<rust::String> result;
    result.reserve(p.size());
    for (const auto& name : p) {
        result.push_back(rust::String(name));
    }
    return result;
}

void MarsRequestWrapper::set_verb(rust::Str verb) {
    request_.verb(std::string(verb));
}

void MarsRequestWrapper::set_value_string(rust::Str key, rust::Str value) {
    request_.setValue(std::string(key), std::string(value));
}

void MarsRequestWrapper::set_values(rust::Str key, rust::Vec<rust::String> values) {
    std::vector<std::string> vec;
    vec.reserve(values.size());
    for (const auto& v : values) {
        vec.emplace_back(std::string(v));
    }
    request_.values(std::string(key), vec);
}

void MarsRequestWrapper::set_value_long(rust::Str key, int64_t value) {
    request_.setValue(std::string(key), static_cast<long>(value));
}

void MarsRequestWrapper::unset_values(rust::Str key) {
    request_.unsetValues(std::string(key));
}

std::unique_ptr<MarsRequestWrapper> MarsRequestWrapper::extract(rust::Str category) const {
    return std::make_unique<MarsRequestWrapper>(request_.extract(std::string(category)));
}

std::unique_ptr<MarsRequestWrapper> MarsRequestWrapper::expand(bool inherit, bool strict) const {
    metkit::mars::MarsLanguage lang(request_.verb());
    auto expanded = lang.expand(request_, inherit, strict);
    return std::make_unique<MarsRequestWrapper>(std::move(expanded));
}

rust::String MarsRequestWrapper::to_json() const {
    std::ostringstream oss;
    eckit::JSON json(oss);
    json << request_;
    return rust::String(oss.str());
}

rust::String MarsRequestWrapper::dump() const {
    std::ostringstream oss;
    request_.dump(oss);
    return rust::String(oss.str());
}

void MarsRequestWrapper::encode(eckit_bridge::StreamWrapper& stream) const {
    stream.inner() << request_;
}

std::unique_ptr<eckit_bridge::DataHandleWrapper> MarsRequestWrapper::make_handle(
    const eckit_bridge::ConfigWrapper& config) const {
    return std::make_unique<eckit_bridge::DataHandleWrapper>(
        new metkit::mars::MarsRequestHandle(request_, config.inner()));
}

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<MarsRequestWrapper> MarsRequestWrapper::create(rust::Str verb) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(std::string(verb)));
}

std::unique_ptr<MarsRequestWrapper> MarsRequestWrapper::from_message(const eckit_bridge::MessageWrapper& msg) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(msg.inner()));
}

std::unique_ptr<MarsRequestWrapper> MarsRequestWrapper::decode(eckit_bridge::StreamWrapper& stream) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(stream.inner()));
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
