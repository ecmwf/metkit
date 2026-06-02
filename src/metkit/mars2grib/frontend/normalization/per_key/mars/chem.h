/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 */

#pragma once

// System includes
#include <sstream>
#include <string>
#include <unordered_map>

// Project includes
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

namespace chem_detail {

/// Process-wide lazy-loaded lookup tables built from chemids.yaml.
/// Row format: [long_id, string_alias, optional_description]
struct ChemTable {
    std::unordered_map<long, long>        byId;
    std::unordered_map<std::string, long> byAlias;
};

inline const ChemTable& chem_table() {
    static const ChemTable table = []() {
        ChemTable t;
        const eckit::Value rows =
            eckit::YAMLParser::decodeFile(metkit::LibMetkit::configFile("chemids.yaml"));
        for (size_t i = 0; i < rows.size(); ++i) {
            const eckit::Value row = rows[i];
            const long id          = static_cast<long>(row[0]);
            t.byId[id]             = id;
            if (row.size() >= 2) {
                const std::string alias = static_cast<std::string>(row[1]);
                if (!alias.empty()) {
                    t.byAlias[alias] = id;
                }
            }
        }
        return t;
    }();
    return table;
}

inline long resolve_or_throw(long id) {
    const ChemTable& t = chem_table();
    auto it            = t.byId.find(id);
    if (it == t.byId.end()) {
        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;
        std::ostringstream msg;
        msg << "chem: unknown constituent id " << id << " (not found in chemids.yaml)";
        throw Mars2GribGenericException(msg.str(), Here());
    }
    return it->second;
}

inline long resolve_or_throw(const std::string& alias) {
    const ChemTable& t = chem_table();
    auto it            = t.byAlias.find(alias);
    if (it == t.byAlias.end()) {
        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;
        std::ostringstream msg;
        msg << "chem: unknown constituent alias '" << alias << "' (not found in chemids.yaml)";
        throw Mars2GribGenericException(msg.str(), Here());
    }
    return it->second;
}

}  // namespace chem_detail

///
/// @brief Sanitize the MARS key: chem (constituent type).
///
/// Validates the supplied value against @c chemids.yaml (loaded lazily on
/// first use). Accepts either:
/// - a @c long constituent id (e.g. 25 for CO2), or
/// - a @c std::string short-name alias (e.g. "CO2").
/// Resolves both forms to the canonical @c long id expected by the encoder.
/// No-op if the key is absent.
///
template <typename MarsDict_t>
void sanitise_chem_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& /*language*/) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    if (auto v = get_opt<long>(in, "chem")) {
        set_or_throw<long>(out, "chem", chem_detail::resolve_or_throw(*v));
        return;
    }
    if (auto v = get_opt<std::string>(in, "chem")) {
        set_or_throw<long>(out, "chem", chem_detail::resolve_or_throw(*v));
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key
