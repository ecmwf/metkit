// metkit CodesHandle bridge — implementation.

#include "metkit_exceptions.h"

#include "CodesHandle.h"

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

bool CodesHandleWrapper::is_defined(rust::Str key) const {
    return handle_->isDefined(std::string(key));
}

bool CodesHandleWrapper::is_missing(rust::Str key) const {
    return handle_->isMissing(std::string(key));
}

bool CodesHandleWrapper::has(rust::Str key) const {
    return handle_->has(std::string(key));
}

rust::String CodesHandleWrapper::get_string(rust::Str key) const {
    return rust::String(handle_->getString(std::string(key)));
}

int64_t CodesHandleWrapper::get_long(rust::Str key) const {
    return static_cast<int64_t>(handle_->getLong(std::string(key)));
}

double CodesHandleWrapper::get_double(rust::Str key) const {
    return handle_->getDouble(std::string(key));
}

rust::Vec<double> CodesHandleWrapper::get_double_array(rust::Str key) const {
    auto vec = handle_->getDoubleArray(std::string(key));
    rust::Vec<double> result;
    result.reserve(vec.size());
    for (double v : vec) {
        result.push_back(v);
    }
    return result;
}

rust::Vec<int64_t> CodesHandleWrapper::get_long_array(rust::Str key) const {
    auto vec = handle_->getLongArray(std::string(key));
    rust::Vec<int64_t> result;
    result.reserve(vec.size());
    for (long v : vec) {
        result.push_back(static_cast<int64_t>(v));
    }
    return result;
}

void CodesHandleWrapper::set_string(rust::Str key, rust::Str value) {
    handle_->set(std::string(key), std::string(value));
}

void CodesHandleWrapper::set_long(rust::Str key, int64_t value) {
    handle_->set(std::string(key), static_cast<long>(value));
}

void CodesHandleWrapper::set_double(rust::Str key, double value) {
    handle_->set(std::string(key), value);
}

void CodesHandleWrapper::set_double_array(rust::Str key, rust::Slice<const double> values) {
    handle_->set(std::string(key), metkit::codes::Span<const double>(values.data(), values.size()));
}

void CodesHandleWrapper::set_missing(rust::Str key) {
    handle_->setMissing(std::string(key));
}

size_t CodesHandleWrapper::value_count(rust::Str key) const {
    return handle_->size(std::string(key));
}

size_t CodesHandleWrapper::message_size() const {
    return handle_->messageSize();
}

rust::Slice<const uint8_t> CodesHandleWrapper::message_data() const {
    auto span = handle_->messageData();
    return {span.data(), span.size()};
}

std::unique_ptr<CodesHandleWrapper> CodesHandleWrapper::clone() const {
    return std::make_unique<CodesHandleWrapper>(handle_->clone());
}

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<CodesHandleWrapper> CodesHandleWrapper::from_message(rust::Slice<const uint8_t> data) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromMessageCopy(metkit::codes::Span<const uint8_t>(data.data(), data.size())));
}

std::unique_ptr<CodesHandleWrapper> CodesHandleWrapper::from_file(rust::Str path) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromFile(std::string(path), metkit::codes::Product::GRIB));
}

std::unique_ptr<CodesHandleWrapper> CodesHandleWrapper::from_file_at_offset(rust::Str path, int64_t offset) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromFile(std::string(path), metkit::codes::Product::GRIB, offset));
}

std::unique_ptr<CodesHandleWrapper> CodesHandleWrapper::from_sample(rust::Str sample) {
    return std::make_unique<CodesHandleWrapper>(metkit::codes::codesHandleFromSample(std::string(sample)));
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
