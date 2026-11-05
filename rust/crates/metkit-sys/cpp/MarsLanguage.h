// metkit MarsLanguage bridge — wraps `metkit::mars::MarsLanguage`.
#pragma once

#include "metkit/mars/MarsLanguage.h"

#include "rust/cxx.h"

#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `metkit::mars::MarsLanguage` for Rust FFI.
class MarsLanguageWrapper {
    std::unique_ptr<metkit::mars::MarsLanguage> lang_;

public:

    explicit MarsLanguageWrapper(rust::Str verb);

    rust::Vec<rust::String> sink_keywords() const;
    bool is_data(rust::Str keyword) const;

    // ============== Factories ==============

    /// Construct a MarsLanguage for the given verb.
    static std::unique_ptr<MarsLanguageWrapper> create(rust::Str verb);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
