// metkit MarsLanguage bridge — wraps `metkit::mars::MarsLanguage`.
#pragma once

#include "metkit/mars/MarsLanguage.h"

#include "rust/cxx.h"

#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps a `metkit::mars::MarsLanguage` for Rust FFI (the languages are cached and shared by C++).
class MarsLanguageWrapper {
    const metkit::mars::MarsLanguage& lang_;

public:

    explicit MarsLanguageWrapper(rust::Str verb);

    bool is_data(rust::Str keyword) const;

    // ============== Factories ==============

    /// Look up the MarsLanguage for the given verb.
    static std::unique_ptr<MarsLanguageWrapper> create(rust::Str verb);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
