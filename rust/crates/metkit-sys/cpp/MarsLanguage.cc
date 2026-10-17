// metkit MarsLanguage bridge — implementation.

#include "metkit_exceptions.h"

#include "MarsLanguage.h"

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

MarsLanguageWrapper::MarsLanguageWrapper(rust::Str verb) :
    lang_(metkit::mars::MarsLanguage::get(std::string(verb))) {}

bool MarsLanguageWrapper::is_data(rust::Str keyword) const {
    return lang_.isData(std::string(keyword));
}

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<MarsLanguageWrapper> MarsLanguageWrapper::create(rust::Str verb) {
    return std::make_unique<MarsLanguageWrapper>(verb);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
