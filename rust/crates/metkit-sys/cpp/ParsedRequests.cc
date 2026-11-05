// metkit ParsedRequests bridge — implementation.

#include "metkit_exceptions.h"

#include "ParsedRequests.h"

#include "metkit/mars/MarsParser.h"

#include <sstream>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<ParsedRequestsWrapper> ParsedRequestsWrapper::parse(rust::Str input, bool strict) {
    return std::make_unique<ParsedRequestsWrapper>(input, strict);
}

std::unique_ptr<ParsedRequestsWrapper> ParsedRequestsWrapper::parse_raw(rust::Str input) {
    auto str = std::string(input);
    std::istringstream iss(str);
    auto parsed  = metkit::mars::MarsParser(iss).parse();
    auto wrapper = std::make_unique<ParsedRequestsWrapper>();
    for (auto& r : parsed) {
        wrapper->push(r);
    }
    return wrapper;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
