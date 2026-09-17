/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Type of Ensemble Forecast.
///
/// This enumeration represents the GRIB code values defined in
/// **GRIB2 Code Table 4.6 – Type of ensemble forecast**.
///
/// The values describe how a forecast member relates to the ensemble
/// generation strategy (control, perturbation type, multi-model, etc.).
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table and must not be modified manually.
///
/// @note
/// The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-level representation** only.
/// No policy, deduction, or defaulting logic must be embedded here.
/// All semantic decisions must be implemented in the corresponding
/// deduction functions.
///
/// @section Source of truth
/// The authoritative definition of this table is:
///
/// GRIB2 Code Table 4.6 – Type of ensemble forecast
///
/// as implemented in ecCodes.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and its conversion helpers automatically from
/// ecCodes GRIB code tables at build or configure time to avoid drift.
///
enum class TypeOfEnsembleForecast : long {
    UnperturbedHighResControl           = 0,
    UnperturbedLowResControl            = 1,
    NegativelyPerturbed                 = 2,
    PositivelyPerturbed                 = 3,
    MultiModel                          = 4,
    Unperturbed                         = 5,
    Perturbed                           = 6,
    InitialConditionsPerturbations      = 7,
    ModelPhysicsPerturbations           = 8,
    InitialAndModelPhysicsPerturbations = 9,
    Missing                             = 255
};


///
/// @brief Convert `TypeOfEnsembleForecast` to its canonical string name.
///
/// The returned string corresponds to the symbolic GRIB meaning of the
/// enumerator and is intended for logging, diagnostics, and validation.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string representation
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enum value is not recognized.
///
template <class Cntx_t>
inline std::string enum2name_TypeOfEnsembleForecast_or_throw(TypeOfEnsembleForecast value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfEnsembleForecast::UnperturbedHighResControl:
            {
                std::string result = "unperturbed-high-res-control";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::UnperturbedLowResControl:
            {
                std::string result = "unperturbed-low-res-control";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::NegativelyPerturbed:
            {
                std::string result = "negatively-perturbed";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::PositivelyPerturbed:
            {
                std::string result = "positively-perturbed";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::MultiModel:
            {
                std::string result = "multi-model";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::Unperturbed:
            {
                std::string result = "unperturbed";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::Perturbed:
            {
                std::string result = "perturbed";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::InitialConditionsPerturbations:
            {
                std::string result = "initial-conditions-perturbations";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::ModelPhysicsPerturbations:
            {
                std::string result = "model-physics-perturbations";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations:
            {
                std::string result = "initial-and-model-physics-perturbations";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case TypeOfEnsembleForecast::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid TypeOfEnsembleForecast enum value", Here());
    }
}


///
/// @brief Convert a canonical string name to `TypeOfEnsembleForecast`.
///
/// This function performs a strict mapping from a string identifier
/// to the corresponding GRIB enumeration value.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding `TypeOfEnsembleForecast` value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided name is not supported.
///
template <class Cntx_t>
inline TypeOfEnsembleForecast name2enum_TypeOfEnsembleForecast_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "unperturbed-high-res-control") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::UnperturbedHighResControl;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "unperturbed-low-res-control") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::UnperturbedLowResControl;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "negatively-perturbed") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::NegativelyPerturbed;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "positively-perturbed") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::PositivelyPerturbed;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "multi-model") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::MultiModel;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "unperturbed") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Unperturbed;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "perturbed") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Perturbed;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "initial-conditions-perturbations") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::InitialConditionsPerturbations;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "model-physics-perturbations") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::ModelPhysicsPerturbations;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "initial-and-model-physics-perturbations") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "missing") {
        {
            TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else {
        throw Mars2GribTableException("Invalid TypeOfEnsembleForecast name: '" + name + "'", Here());
    }
}

///
/// @brief Convert a numeric GRIB code to `TypeOfEnsembleForecast`.
///
/// This function validates and converts a raw numeric value associated
/// with **GRIB2 Code Table 4.6 – Type of ensemble forecast** into the
/// corresponding `TypeOfEnsembleForecast` enumeration.
///
/// Only officially defined GRIB values are accepted. Any other value
/// is considered invalid and results in an exception.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding `TypeOfEnsembleForecast` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the provided value does not correspond to a valid
/// GRIB Code Table 4.6 entry.
///
/// @note
/// - The value `255` corresponds to the GRIB *missing* value.
/// - No implicit normalization, fallback, or defaulting is performed.
///
/// @important
/// This function performs **pure table validation**.
/// Any semantic interpretation must be handled by the calling deduction.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes GRIB code tables.
///
template <class Cntx_t>
inline TypeOfEnsembleForecast long2enum_TypeOfEnsembleForecast_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::UnperturbedHighResControl;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::UnperturbedLowResControl;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::NegativelyPerturbed;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::PositivelyPerturbed;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::MultiModel;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Unperturbed;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 6:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Perturbed;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 7:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::InitialConditionsPerturbations;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 8:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::ModelPhysicsPerturbations;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 9:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                TypeOfEnsembleForecast result = TypeOfEnsembleForecast::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid TypeOfEnsembleForecast numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..9,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables