#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Logging
#include "metkit/config/LibMetkit.h"

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/tables/tables_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/localTablesVersion.h"
#include "metkit/mars2grib/backend/deductions/tablesVersion.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, TablesType Variant>
constexpr bool tablesApplicable() {

    // Conditions to apply concept
    return ((Stage == StageAllocate) && (Section == SecIdentificationSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, TablesType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void TablesOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
              OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (tablesApplicable<Stage, Section, Variant>()) {

        try {

            // Debug output
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Tables] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(tablesTypeName<Variant>()) << std::endl;

            long localTablesVersionVal = deductions::localTablesVersion<MarsDict_t, ParDict_t>(mars, par);

            // set in output dictionary
            if constexpr (Variant == TablesType::Custom) {
                long tablesVersionVal = get_or_throw<long>(par, "tablesVersion");
                set_or_throw<long>(out, "tablesVersion", tablesVersionVal);
                set_or_throw<long>(out, "localTablesVersion", localTablesVersionVal);
            }
            else if constexpr (Variant == TablesType::Default) {
                // deduce tablesVersion and localTablesVersion
                long tablesVersionVal = deductions::tablesVersion<MarsDict_t, ParDict_t>(mars, par);
                set_or_throw<long>(out, "tablesVersion", tablesVersionVal);
                set_or_throw<long>(out, "localTablesVersion", localTablesVersionVal);
            }
        }
        catch (...) {

            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(tablesName), std::string(tablesTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `tables` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( tablesApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(tablesName), std::string(tablesTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts
