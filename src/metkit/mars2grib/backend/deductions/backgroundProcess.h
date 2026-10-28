/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file backgroundProcess.h
/// @brief Deduction of the GRIB `backgroundProcess` attribute.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **GRIB Background Process** from input dictionaries.
///
/// The deduction maps the MARS model identifier to a GRIB-compliant
/// `BackgroundProcess` value according to predefined, authoritative
/// mapping rules.
///
/// Deductions are responsible for:
/// - extracting values from MARS, parameter, and option dictionaries
/// - applying deterministic deduction logic
/// - returning strongly typed values to concept operations
///
/// Deductions:
/// - do NOT encode GRIB keys directly
/// - do NOT apply semantic defaults beyond explicit rules
/// - do NOT perform GRIB table validation
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or unsupported inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value resolved via deduction logic from input dictionaries
/// - DEFAULT: value defaulted to a predefined constant due to missing input
///
/// @section References
/// Concept:
/// - @ref generatingProcessEncoding.h
///
/// Related deductions:
/// - @ref generatingProcessIdentifier.h
/// - @ref typeOfGeneratingProcess.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <optional>
#include <string>

// Tables
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `backgroundProcess` value from input dictionaries.
///
/// @section Deduction contract
/// - Reads: `mars["model"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE or DEFAULT)
/// - Failure mode: throws
///
/// This deduction resolves the GRIB `backgroundProcess` by mapping the
/// MARS model identifier to the corresponding
/// `tables::BackgroundProcess` enumeration value.
///
/// The mapping is explicit and strict: only supported model identifiers
/// are accepted. Unsupported or unknown values result in an immediate
/// deduction failure.
/// If the key `model` is missing, a default value of "ifs" is used,
/// which is then mapped to the corresponding `backgroundProcess`.
///
/// This function acts as the single authoritative deduction point for
/// `backgroundProcess`. All mapping rules and consistency checks for
/// this GRIB key must be implemented here.
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support keyed access to `model`
/// and conversion to `std::string`. If the key `model` is missing,
/// a default value of "ifs" is used.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @tparam OptDict_t
/// Type of the options dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary from which the model identifier is read.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved GRIB `BackgroundProcess` enumeration value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If the key `model` cannot be mapped to a
/// valid GRIB background process, or if any unexpected error occurs
/// during deduction.
///
/// @note
/// This deduction is deterministic and does not rely on any
/// pre-existing GRIB header state.
///
///
/// @brief Component of the coupled system that produced a field, from the MARS keys of the field.
///
/// The legacy archive tells the components apart by MARS keys only: the fire emissions are `stream=gfas`,
/// the land surface analysis is `stream=ldas`, atmospheric composition is `class=mc` (IFS-compo), the native
/// ocean fields are on `levtype=o2d`/`o3d` (nemo), the wave fields carry table-140 params (ecwam), and
/// everything else is the atmosphere (IFS). The IFS ocean diagnostics (t20d, pt300m, sc300m, ...) are on
/// `levtype=sfc` and therefore IFS, which is what makes ecCodes keep them on `sfc`.
///
template <class MarsDict_t>
tables::BackgroundProcess componentOfField(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;

    const std::optional<std::string> stream = get_opt<std::string>(mars, "stream");
    if (stream && *stream == "gfas") {
        return tables::BackgroundProcess::gfas;
    }
    if (stream && *stream == "ldas") {
        return tables::BackgroundProcess::ldas;
    }

    const std::optional<std::string> marsClass = get_opt<std::string>(mars, "class");
    if (marsClass && *marsClass == "mc") {
        return tables::BackgroundProcess::ifs_compo;
    }

    const std::optional<std::string> levtype = get_opt<std::string>(mars, "levtype");
    if (levtype && (*levtype == "o2d" || *levtype == "o3d")) {
        return tables::BackgroundProcess::nemo;
    }

    const std::optional<long> param = get_opt<long>(mars, "param");
    if (param && *param >= 140000 && *param < 141000) {
        return tables::BackgroundProcess::ecwam;
    }

    return tables::BackgroundProcess::ifs;
}

///
/// @brief Resolve the GRIB `backgroundProcess` from the MARS dictionary.
///
/// - `mars::model` present and an AIFS model, or one of the component names
///   (`gfas`, `ldas`, `ecwam`, `nemo`, `IFS-compo`, `IFS-legacy`): that code.
/// - `mars::model` `IFS`, `IFS-NEMO` or `IFS-FESOM` (the coupled system): the component of
///   the field, see @ref componentOfField.
/// - `mars::model` absent: the component of the field. The `backgroundProcess` of an input
///   GRIB2 header is never taken over: the value is always deduced from the MARS keys.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::BackgroundProcess resolve_BackgroundProcess_or_throw(const MarsDict_t& mars,
                                                             [[maybe_unused]] const ParDict_t& par,
                                                             [[maybe_unused]] const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        if (has(mars, "model")) {
            const std::string marsModelVal = get_or_throw<std::string>(mars, "model");

            const bool coupledSystem =
                marsModelVal == "IFS" || marsModelVal == "IFS-NEMO" || marsModelVal == "IFS-FESOM";
            const tables::BackgroundProcess backgroundProcess =
                coupledSystem ? componentOfField(mars) : tables::name2enum_BackgroundProcess_or_throw(marsModelVal);

            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg =
                    "`backgroundProcess` resolved from input dictionaries: model='" + marsModelVal + "', value='";
                logMsg += tables::enum2name_BackgroundProcess_or_throw(backgroundProcess) + "'";
                return logMsg;
            }());

            return backgroundProcess;
        }
        else {
            const tables::BackgroundProcess backgroundProcess = componentOfField(mars);

            MARS2GRIB_LOG_DEFAULT([&]() {
                std::string logMsg = "`backgroundProcess` defaulted to the component of the field: value='";
                logMsg += tables::enum2name_BackgroundProcess_or_throw(backgroundProcess) + "'";
                return logMsg;
            }());

            return backgroundProcess;
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `backgroundProcess` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
