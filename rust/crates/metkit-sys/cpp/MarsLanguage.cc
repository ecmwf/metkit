// metkit MarsLanguage bridge — implementation.

#include "metkit_exceptions.h"

#include "MarsLanguage.h"

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<MarsLanguageWrapper> MarsLanguageWrapper::create(rust::Str verb) {
    return std::make_unique<MarsLanguageWrapper>(verb);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
