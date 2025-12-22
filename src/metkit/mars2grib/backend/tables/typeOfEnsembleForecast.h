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

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::tables {

/**
 * @brief GRIB Type of Ensemble Forecast.
 *
 * This enumeration represents the GRIB code values defined in
 * **GRIB2 Code Table 4.6 – Type of ensemble forecast**.
 *
 * The values describe how a forecast member relates to the ensemble
 * generation strategy (control, perturbation type, multi-model, etc.).
 *
 * The numeric values of the enumerators map **directly** to the GRIB
 * code table and must not be modified manually.
 *
 * @note
 * The value `255` corresponds to the GRIB *missing* value.
 *
 * @important
 * This enum is a **GRIB-level representation** only.
 * No policy, deduction, or defaulting logic must be embedded here.
 * All semantic decisions must be implemented in the corresponding
 * deduction functions.
 *
 * @section Source of truth
 * The authoritative definition of this table is:
 *
 *   GRIB2 Code Table 4.6 – Type of ensemble forecast
 *
 * as implemented in ecCodes.
 *
 * @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
 * - Generate this enum and its conversion helpers automatically from
 *   ecCodes GRIB code tables at build or configure time to avoid drift.
 */
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


/**
 * @brief Convert `TypeOfEnsembleForecast` to its canonical string name.
 *
 * The returned string corresponds to the symbolic GRIB meaning of the
 * enumerator and is intended for logging, diagnostics, and validation.
 *
 * @param[in] value Enumeration value
 *
 * @return Canonical string representation
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the enum value is not recognized.
 */
inline std::string enum2name_TypeOfEnsembleForecast_or_throw(TypeOfEnsembleForecast value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case TypeOfEnsembleForecast::UnperturbedHighResControl:
            return "unperturbed-high-res-control";
        case TypeOfEnsembleForecast::UnperturbedLowResControl:
            return "unperturbed-low-res-control";
        case TypeOfEnsembleForecast::NegativelyPerturbed:
            return "negatively-perturbed";
        case TypeOfEnsembleForecast::PositivelyPerturbed:
            return "positively-perturbed";
        case TypeOfEnsembleForecast::MultiModel:
            return "multi-model";
        case TypeOfEnsembleForecast::Unperturbed:
            return "unperturbed";
        case TypeOfEnsembleForecast::Perturbed:
            return "perturbed";
        case TypeOfEnsembleForecast::InitialConditionsPerturbations:
            return "initial-conditions-perturbations";
        case TypeOfEnsembleForecast::ModelPhysicsPerturbations:
            return "model-physics-perturbations";
        case TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations:
            return "initial-and-model-physics-perturbations";
        case TypeOfEnsembleForecast::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid TypeOfEnsembleForecast enum value", Here());
    }
}


/**
 * @brief Convert a canonical string name to `TypeOfEnsembleForecast`.
 *
 * This function performs a strict mapping from a string identifier
 * to the corresponding GRIB enumeration value.
 *
 * @param[in] name Canonical string name
 *
 * @return Corresponding `TypeOfEnsembleForecast` value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the provided name is not supported.
 */
inline TypeOfEnsembleForecast name2enum_TypeOfEnsembleForecast_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "unperturbed-high-res-control") {
        return TypeOfEnsembleForecast::UnperturbedHighResControl;
    }
    else if (name == "unperturbed-low-res-control") {
        return TypeOfEnsembleForecast::UnperturbedLowResControl;
    }
    else if (name == "negatively-perturbed") {
        return TypeOfEnsembleForecast::NegativelyPerturbed;
    }
    else if (name == "positively-perturbed") {
        return TypeOfEnsembleForecast::PositivelyPerturbed;
    }
    else if (name == "multi-model") {
        return TypeOfEnsembleForecast::MultiModel;
    }
    else if (name == "unperturbed") {
        return TypeOfEnsembleForecast::Unperturbed;
    }
    else if (name == "perturbed") {
        return TypeOfEnsembleForecast::Perturbed;
    }
    else if (name == "initial-conditions-perturbations") {
        return TypeOfEnsembleForecast::InitialConditionsPerturbations;
    }
    else if (name == "model-physics-perturbations") {
        return TypeOfEnsembleForecast::ModelPhysicsPerturbations;
    }
    else if (name == "initial-and-model-physics-perturbations") {
        return TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations;
    }
    else if (name == "missing") {
        return TypeOfEnsembleForecast::Missing;
    }
    else {
        throw Mars2GribTableException("Invalid TypeOfEnsembleForecast name: '" + name + "'", Here());
    }
}

/**
 * @brief Convert a numeric GRIB code to `TypeOfEnsembleForecast`.
 *
 * This function validates and converts a raw numeric value associated
 * with **GRIB2 Code Table 4.6 – Type of ensemble forecast** into the
 * corresponding `TypeOfEnsembleForecast` enumeration.
 *
 * Only officially defined GRIB values are accepted. Any other value
 * is considered invalid and results in an exception.
 *
 * @param[in] value Numeric GRIB code
 *
 * @return Corresponding `TypeOfEnsembleForecast` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
 *         If the provided value does not correspond to a valid
 *         GRIB Code Table 4.6 entry.
 *
 * @note
 * - The value `255` corresponds to the GRIB *missing* value.
 * - No implicit normalization, fallback, or defaulting is performed.
 *
 * @important
 * This function performs **pure table validation**.
 * Any semantic interpretation must be handled by the calling deduction.
 *
 * @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
 * - Replace this hard-coded mapping with code generated directly from
 *   ecCodes GRIB code tables.
 */
inline TypeOfEnsembleForecast long2enum_TypeOfEnsembleForecast_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return TypeOfEnsembleForecast::UnperturbedHighResControl;
        case 1:
            return TypeOfEnsembleForecast::UnperturbedLowResControl;
        case 2:
            return TypeOfEnsembleForecast::NegativelyPerturbed;
        case 3:
            return TypeOfEnsembleForecast::PositivelyPerturbed;
        case 4:
            return TypeOfEnsembleForecast::MultiModel;
        case 5:
            return TypeOfEnsembleForecast::Unperturbed;
        case 6:
            return TypeOfEnsembleForecast::Perturbed;
        case 7:
            return TypeOfEnsembleForecast::InitialConditionsPerturbations;
        case 8:
            return TypeOfEnsembleForecast::ModelPhysicsPerturbations;
        case 9:
            return TypeOfEnsembleForecast::InitialAndModelPhysicsPerturbations;
        case 255:
            return TypeOfEnsembleForecast::Missing;
        default:
            throw Mars2GribTableException("Invalid TypeOfEnsembleForecast numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..9,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables