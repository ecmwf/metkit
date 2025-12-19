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
#include "metkit/mars2grib/backend/concepts/data-type/data_type_enum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/productionStatusOfProcessedData.h"
#include "metkit/mars2grib/backend/deductions/typeOfProcessedData.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, DataTypeType Variant>
constexpr bool data_typeApplicable() {

    return ((Variant == DataTypeType::Default) && (Stage == StagePreset) && (Section == SecIdentificationSection));
}

// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, DataTypeType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void DataTypeOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (data_typeApplicable<Stage, Section, Variant>()) {

        // =============================================================
        // Logging
        LOG_DEBUG_LIB(LibMetkit) << "[Concept DataType] Op called: "
                                 << "Stage=" << Stage << ", Section=" << Section
                                 << ", Variant=" << std::string(data_typeTypeName<Variant>()) << std::endl;

        // Options whether to set to Missing if deduction fails
        // Default is false to mimic the behaviour of all the other encoders we have in place
        bool setTypeOfProcessedDataMissingIfNotFound =
            get_opt<bool>(opt, "setTypeOfProcessedDataToMissingIfNotFound").value_or(false);

        bool setProductionStatusOfProcessedDataMissingIfNotFound =
            get_opt<bool>(opt, "setProductionStatusOfProcessedDataToMissingIfNotFound").value_or(false);

        // Deductions
        auto typeOfProcessedData = deductions::typeOfProcessed<MarsDict_t, ParDict_t>(mars, par);
        auto productionStatusOfProcessedData =
            deductions::productionStatusOfProcessed<MarsDict_t, ParDict_t>(mars, par);


        // Set values in output dictionary (grib sample)
        if (typeOfProcessedData != deductions::TypeOfProcessedData::Missing) {
            set_or_throw<long>(out, "typeOfProcessedData", static_cast<long>(typeOfProcessedData));
        }
        else if (setTypeOfProcessedDataMissingIfNotFound) {

            // WARNING MIVAL: Setting to Missing might not be the best option here. The encoder relies on whatever is
            // set here to decide how to encode the field. If Missing is not appropriate for the field being
            set_or_throw<long>(out, "typeOfProcessedData", static_cast<long>(deductions::TypeOfProcessedData::Missing));
        }

        if (productionStatusOfProcessedData != deductions::ProductionStatusOfProcessedData::Missing) {
            set_or_throw<long>(out, "productionStatusOfProcessedData",
                               static_cast<long>(productionStatusOfProcessedData));
        }
        else if (setProductionStatusOfProcessedDataMissingIfNotFound) {

            // WARNING MIVAL: Setting to Missing might not be the best option here. The encoder relies on whatever is
            // set here to decide how to encode the field. If Missing is not appropriate for the field being
            set_or_throw<long>(out, "productionStatusOfProcessedData",
                               static_cast<long>(deductions::ProductionStatusOfProcessedData::Missing));
        }

        // Successful operation
        return;

    }  // if constexpr ( data_typeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(dataTypeName), std::string(data_typeTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts
