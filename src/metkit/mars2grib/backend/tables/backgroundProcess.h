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

///
/// @brief GRIB background process classification.
///
/// This enumeration represents the GRIB code values associated with the
/// `backgroundProcess` key in the Product Definition Section.
///
/// Each enumerator corresponds to a distinct model configuration or
/// post-processing workflow as defined by ECMWF conventions and encoded
/// in GRIB local concepts.
///
/// The numeric values of the enumerators map **directly** to the GRIB
/// code table values and must not be changed manually.
///
/// @note
/// The value `255` usually corresponds to the GRIB *missing* value.
/// In this specific case, it is semantically interpreted as the
/// deterministic IFS workflow (`ifs`) for compatibility with existing
/// production data.
///
/// @important
/// This enum is a **GRIB-level representation**, not a policy decision.
/// All semantic validation, defaulting, and resolution logic must be
/// implemented in the corresponding deduction functions.
///
/// @section Source of truth
/// The authoritative definition of supported background processes and
/// their GRIB encodings is maintained in:
///
/// `definitions/grib2/localConcepts/ecmf/modelNameConcept.def`
///
/// This enumeration must remain consistent with that definition.
///
/// @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: critical]
/// - Replace this manually maintained enumeration with code generated
/// automatically from ecCodes GRIB code tables / definitions.
/// - The generation should occur at build or configure time (e.g. via a
/// Python code-generation step) to prevent silent divergence between
/// the encoder and the ecCodes library.
///
enum class BackgroundProcess : long {
    aifs_single           = 1,
    aifs_ens              = 2,
    aifs_single_mse       = 3,
    aifs_ens_crps         = 4,
    aifs_ens_diff         = 5,
    aifs_compo_single     = 6,
    aifs_compo_ens        = 7,
    aifs_compo_single_mse = 8,
    aifs_compo_ens_crps   = 9,
    ifs                   = 255
};

///
/// @brief Map a MARS model identifier to a GRIB `BackgroundProcess` enumeration.
///
/// This function converts a string-based model identifier, typically obtained
/// from the MARS key `mars::model`, into the corresponding GRIB
/// `BackgroundProcess` enumeration value.
///
/// The mapping is explicit and strict. Only the following identifiers are
/// supported:
///
/// - `"ifs"`                    → `BackgroundProcess::ifs`
/// - `"aifs-single"`            → `BackgroundProcess::aifs_single`
/// - `"aifs-ens"`               → `BackgroundProcess::aifs_ens`
/// - `"aifs-single-mse"`        → `BackgroundProcess::aifs_single_mse`
/// - `"aifs-ens-crps"`          → `BackgroundProcess::aifs_ens_crps`
/// - `"aifs-ens-diff"`          → `BackgroundProcess::aifs_ens_diff`
/// - `"aifs-compo-single"`      → `BackgroundProcess::aifs_compo_single`
/// - `"aifs-compo-ens"`         → `BackgroundProcess::aifs_compo_ens`
/// - `"aifs-compo-single-mse"`  → `BackgroundProcess::aifs_compo_single_mse`
/// - `"aifs-compo-ens-crps"`    → `BackgroundProcess::aifs_compo_ens_crps`
///
/// Any other value is considered invalid and results in a deduction error.
///
/// @param[in] value String value of the MARS `model` key to be mapped
///
/// @return The corresponding `BackgroundProcess` enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the provided string does not correspond to a supported
/// MARS model identifier.
///
/// @note
/// - This function performs a **pure mapping** from MARS semantics to
/// GRIB background process codes.
/// - No implicit normalization, fallback, or defaulting is performed.
///
/// @important
/// The authoritative definition of valid model identifiers and their
/// mapping to GRIB background process codes is maintained in:
///
/// `definitions/grib2/localConcepts/ecmf/modelNameConcept.def`
///
/// This function must remain consistent with that definition.
///
/// @todo [owner: mds,dgov][scope: deduction][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes definitions to prevent divergence between software stacks.
///
inline BackgroundProcess name2enum_BackgroundProcess_or_throw(const std::string& value) {
    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;
    if (value == "ifs") {
        return BackgroundProcess::ifs;
    }
    else if (value == "aifs-single") {
        return BackgroundProcess::aifs_single;
    }
    else if (value == "aifs-ens") {
        return BackgroundProcess::aifs_ens;
    }
    else if (value == "aifs-single-mse") {
        return BackgroundProcess::aifs_single_mse;
    }
    else if (value == "aifs-ens-crps") {
        return BackgroundProcess::aifs_ens_crps;
    }
    else if (value == "aifs-ens-diff") {
        return BackgroundProcess::aifs_ens_diff;
    }
    else if (value == "aifs-compo-single") {
        return BackgroundProcess::aifs_compo_single;
    }
    else if (value == "aifs-compo-ens") {
        return BackgroundProcess::aifs_compo_ens;
    }
    else if (value == "aifs-compo-single-mse") {
        return BackgroundProcess::aifs_compo_single_mse;
    }
    else if (value == "aifs-compo-ens-crps") {
        return BackgroundProcess::aifs_compo_ens_crps;
    }
    else {
        std::string errMsg = "Invalid BackgroundProcess value: ";
        errMsg += "actual='" + value + "', ";
        errMsg +=
            "expected={'ifs', 'aifs-single', 'aifs-ens', 'aifs-single-mse', "
            "'aifs-ens-crps', 'aifs-ens-diff', 'aifs-compo-single', 'aifs-compo-ens', "
            "'aifs-compo-single-mse', 'aifs-compo-ens-crps'}";
        throw Mars2GribTableException(errMsg, Here());
    }
    // Remove compiler warning
    __builtin_unreachable();
}

///
/// @brief Map a GRIB `BackgroundProcess` enumeration to its canonical MARS model identifier.
///
/// This function converts a GRIB-level `BackgroundProcess` enumeration value
/// into the corresponding canonical string identifier used by MARS
/// (e.g. `mars::model`).
///
/// The mapping is explicit and strict. Only officially supported enumeration
/// values are accepted.
///
/// Supported mappings:
///
/// - `BackgroundProcess::ifs`                    → `"ifs"`
/// - `BackgroundProcess::aifs_single`            → `"aifs-single"`
/// - `BackgroundProcess::aifs_ens`               → `"aifs-ens"`
/// - `BackgroundProcess::aifs_single_mse`        → `"aifs-single-mse"`
/// - `BackgroundProcess::aifs_ens_crps`          → `"aifs-ens-crps"`
/// - `BackgroundProcess::aifs_ens_diff`          → `"aifs-ens-diff"`
/// - `BackgroundProcess::aifs_compo_single`      → `"aifs-compo-single"`
/// - `BackgroundProcess::aifs_compo_ens`         → `"aifs-compo-ens"`
/// - `BackgroundProcess::aifs_compo_single_mse`  → `"aifs-compo-single-mse"`
/// - `BackgroundProcess::aifs_compo_ens_crps`    → `"aifs-compo-ens-crps"`
///
/// Any other enumeration value is considered invalid and results in a
/// table-mapping error.
///
/// @param[in] value GRIB `BackgroundProcess` enumeration value
///
/// @return Canonical string representation corresponding to the MARS
/// `model` identifier.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value does not correspond to a supported
/// background process.
///
/// @note
/// - This function performs a **pure reverse mapping** of
/// `mapto_BackgroundProcess_or_throw(const std::string&)`.
/// - No implicit normalization, fallback, or defaulting is performed.
///
/// @important
/// The authoritative definition of background process identifiers and their
/// string representations is maintained in:
///
/// `definitions/grib2/localConcepts/ecmf/modelNameConcept.def`
///
/// This function must remain strictly consistent with that definition.
///
/// @todo [owner: mds,dgov][scope: tables][reason: correctness][prio: medium]
/// - Replace this hard-coded mapping with code generated directly from
/// ecCodes definitions to guarantee bidirectional consistency.
///
inline std::string enum2name_BackgroundProcess_or_throw(BackgroundProcess value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case BackgroundProcess::ifs:
            return "ifs";
        case BackgroundProcess::aifs_single:
            return "aifs-single";
        case BackgroundProcess::aifs_ens:
            return "aifs-ens";
        case BackgroundProcess::aifs_single_mse:
            return "aifs-single-mse";
        case BackgroundProcess::aifs_ens_crps:
            return "aifs-ens-crps";
        case BackgroundProcess::aifs_ens_diff:
            return "aifs-ens-diff";
        case BackgroundProcess::aifs_compo_single:
            return "aifs-compo-single";
        case BackgroundProcess::aifs_compo_ens:
            return "aifs-compo-ens";
        case BackgroundProcess::aifs_compo_single_mse:
            return "aifs-compo-single-mse";
        case BackgroundProcess::aifs_compo_ens_crps:
            return "aifs-compo-ens-crps";
        default:
            std::string errMsg = "Invalid BackgroundProcess enum value: ";
            errMsg += std::to_string(static_cast<long>(value));
            throw Mars2GribTableException(errMsg, Here());
    }

    // Remove compiler warning
    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::backend::tables