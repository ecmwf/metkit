#pragma once

#include <iostream>
#include <string>
#include <string_view>


// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/nil/nil_enum.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, NilType Variant>
constexpr bool nilApplicable() {
    return true;
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, NilType Variant, class MarsDict_t, class GeoDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void NilOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
           OutDict_t& out) noexcept(false) {


    if constexpr (nilApplicable<Stage, Section, Variant>()) {

        // Debug output
        LOG_DEBUG_LIB(LibMetkit) << "[Concept Nil] Op called: "
                                 << "Stage=" << Stage << ", Section=" << Section
                                 << ", Variant=" << std::string(nilTypeName<Variant>()) << std::endl;
        // No operation for Nil concept
    }

    // Successful no-op
    return;
}

}  // namespace metkit::mars2grib::backend::cnpts
