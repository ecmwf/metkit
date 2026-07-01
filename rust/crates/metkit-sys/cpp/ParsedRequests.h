// metkit MarsRequest batch-parse bridge — holds parsed requests.
#pragma once

#include "MarsRequest.h"
#include "metkit/mars/MarsRequest.h"

#include "rust/cxx.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Holds parsed requests — parse once, iterate by index.
class ParsedRequestsWrapper {
    std::vector<metkit::mars::MarsRequest> requests_;

public:

    ParsedRequestsWrapper() = default;
    ParsedRequestsWrapper(rust::Str input, bool strict);

    void push(const metkit::mars::MarsRequest& r) { requests_.push_back(r); }
    size_t count() const;
    std::unique_ptr<MarsRequestWrapper> at(size_t index) const;

    // ============== Factories ==============

    /// Parse MARS requests with verb validation.
    static std::unique_ptr<ParsedRequestsWrapper> parse(rust::Str input, bool strict);

    /// Raw parse without verb validation — uses `MarsParser` directly.
    static std::unique_ptr<ParsedRequestsWrapper> parse_raw(rust::Str input);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
