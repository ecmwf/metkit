/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file waveStreams.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <string>
#include <unordered_map>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::rules::impl {

/// @brief Wave streams and the atmospheric streams they are converted to.
///
/// Pairs follow the ecCodes stream table (definitions/mars/stream.table). The legacy seasonal and monthly
/// forecast wave streams are not converted.
inline const std::unordered_map<std::string, std::string>& waveToAtmosphericStreams() {
    static const std::unordered_map<std::string, std::string> streams{
        // Deterministic and data assimilation
        {"wave", "oper"},
        {"scwv", "scda"},
        {"dcwv", "dcda"},
        {"lwwv", "lwda"},
        {"ewda", "enda"},
        {"ewla", "elda"},
        {"fsow", "fsob"},
        // Ensemble forecasts, extended range and hindcasts
        {"waef", "enfo"},
        {"enwh", "enfh"},
        {"weef", "eefo"},
        {"weeh", "eefh"},
        {"ewho", "efho"},
        {"weov", "efov"},
        {"ewhc", "efhc"},
        // Hindcast statistics
        {"wehs", "efhs"},
        {"wees", "eehs"},
        // Monthly means and climatology
        {"wamo", "mnth"},
        {"wamd", "moda"},
        {"ewmm", "edmm"},
        {"ewmo", "edmo"},
        {"dacw", "dacl"},
    };
    return streams;
}

/// @brief Convert wave streams
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void convertWaveStreams(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc,
                               const OptDict_t& opts) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        (void)opts;

        const auto& streams = waveToAtmosphericStreams();
        const auto it       = streams.find(get_or_throw<std::string>(in, "stream"));
        if (it != streams.end()) {
            set_or_throw<std::string>(out, "stream", it->second);
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertWaveStreams", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
