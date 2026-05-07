// metkit C++ bridge implementation
#include "metkit_bridge.h"
#include "eckit/log/JSON.h"

#include <sstream>

namespace metkit_bridge {

// ==================== MarsRequest ====================

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

// ==================== CodesHandle ====================

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

std::unique_ptr<CodesHandleWrapper> codes_handle_from_message(rust::Slice<const uint8_t> data) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromMessageCopy(metkit::codes::Span<const uint8_t>(data.data(), data.size())));
}

std::unique_ptr<CodesHandleWrapper> codes_handle_from_file(rust::Str path) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromFile(std::string(path), metkit::codes::Product::GRIB));
}

std::unique_ptr<CodesHandleWrapper> codes_handle_from_file_at_offset(rust::Str path, int64_t offset) {
    return std::make_unique<CodesHandleWrapper>(
        metkit::codes::codesHandleFromFile(std::string(path), metkit::codes::Product::GRIB, offset));
}

std::unique_ptr<CodesHandleWrapper> codes_handle_from_sample(rust::Str sample) {
    return std::make_unique<CodesHandleWrapper>(metkit::codes::codesHandleFromSample(std::string(sample)));
}

// ==================== HyperCube ====================

HyperCubeWrapper::HyperCubeWrapper(const MarsRequestWrapper& request) :
    cube_(std::make_unique<metkit::hypercube::HyperCube>(request.inner())) {}

size_t HyperCubeWrapper::size() const {
    return cube_->size();
}

size_t HyperCubeWrapper::count() const {
    return cube_->count();
}

size_t HyperCubeWrapper::count_vacant() const {
    return cube_->countVacant();
}

bool HyperCubeWrapper::contains(const MarsRequestWrapper& request) const {
    return cube_->contains(request.inner());
}

bool HyperCubeWrapper::clear(const MarsRequestWrapper& request) {
    return cube_->clear(request.inner());
}

size_t HyperCubeWrapper::field_ordinal(const MarsRequestWrapper& request) const {
    return cube_->fieldOrdinal(request.inner());
}

std::unique_ptr<HyperCubeWrapper> hypercube_create(const MarsRequestWrapper& request) {
    return std::make_unique<HyperCubeWrapper>(request);
}

// ==================== MarsRequest factory ====================

std::unique_ptr<MarsRequestWrapper> request_create(rust::Str verb) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(std::string(verb)));
}

std::unique_ptr<MarsRequestWrapper> request_from_message(const eckit_bridge::MessageWrapper& msg) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(msg.inner()));
}

std::unique_ptr<MarsRequestWrapper> request_decode(eckit_bridge::StreamWrapper& stream) {
    return std::make_unique<MarsRequestWrapper>(metkit::mars::MarsRequest(stream.inner()));
}

std::unique_ptr<eckit_bridge::DataHandleWrapper> mars_request_handle(const MarsRequestWrapper& request,
                                                                     const eckit_bridge::ConfigWrapper& config) {
    return std::make_unique<eckit_bridge::DataHandleWrapper>(
        new metkit::mars::MarsRequestHandle(request.inner(), config.inner()));
}

ParsedRequestsWrapper::ParsedRequestsWrapper(rust::Str input, bool strict) {
    auto str = std::string(input);
    std::istringstream iss(str);
    requests_ = metkit::mars::MarsRequest::parse(iss, strict);
}

size_t ParsedRequestsWrapper::count() const {
    return requests_.size();
}

std::unique_ptr<MarsRequestWrapper> ParsedRequestsWrapper::at(size_t index) const {
    return std::make_unique<MarsRequestWrapper>(requests_.at(index));
}

std::unique_ptr<ParsedRequestsWrapper> parse_requests(rust::Str input, bool strict) {
    return std::make_unique<ParsedRequestsWrapper>(input, strict);
}

std::unique_ptr<ParsedRequestsWrapper> parse_requests_raw(rust::Str input) {
    auto str = std::string(input);
    std::istringstream iss(str);
    auto parsed  = metkit::mars::MarsParser(iss).parse();
    auto wrapper = std::make_unique<ParsedRequestsWrapper>();
    for (auto& r : parsed) {
        wrapper->push(r);
    }
    return wrapper;
}

// ==================== RequestEnvironment ====================

void request_environment_init(rust::Vec<rust::String> keys, rust::Vec<rust::String> values) {
    std::map<std::string, std::string> env;
    for (size_t i = 0; i < keys.size() && i < values.size(); ++i) {
        env[std::string(keys[i])] = std::string(values[i]);
    }
    metkit::mars::RequestEnvironment::initialize(env);
}

std::unique_ptr<MarsRequestWrapper> request_environment_request() {
    const auto& env = metkit::mars::RequestEnvironment::instance();
    return std::make_unique<MarsRequestWrapper>(env.request());
}

// ==================== MarsLanguage ====================

MarsLanguageWrapper::MarsLanguageWrapper(rust::Str verb) :
    lang_(std::make_unique<metkit::mars::MarsLanguage>(std::string(verb))) {}

rust::Vec<rust::String> MarsLanguageWrapper::sink_keywords() const {
    const auto& kw = lang_->sinkKeywords();
    rust::Vec<rust::String> result;
    result.reserve(kw.size());
    for (const auto& k : kw) {
        result.push_back(rust::String(k));
    }
    return result;
}

bool MarsLanguageWrapper::is_data(rust::Str keyword) const {
    return lang_->isData(std::string(keyword));
}

std::unique_ptr<MarsLanguageWrapper> language_create(rust::Str verb) {
    return std::make_unique<MarsLanguageWrapper>(verb);
}

// ==================== ParamID / WindFamily ====================

size_t wind_family_count() {
    return metkit::ParamID::getWindFamilies().size();
}

rust::String wind_family_u(size_t index) {
    return rust::String(std::string(metkit::ParamID::getWindFamilies().at(index).u_));
}

rust::String wind_family_v(size_t index) {
    return rust::String(std::string(metkit::ParamID::getWindFamilies().at(index).v_));
}

rust::String wind_family_vo(size_t index) {
    return rust::String(std::string(metkit::ParamID::getWindFamilies().at(index).vo_));
}

rust::String wind_family_d(size_t index) {
    return rust::String(std::string(metkit::ParamID::getWindFamilies().at(index).d_));
}

}  // namespace metkit_bridge
