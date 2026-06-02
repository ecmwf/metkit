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

// System includes
#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Project includes
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::normalization::per_key {

namespace model_detail {

inline std::string scalar_to_string(const eckit::Value& v) {
    if (v.isString()) {
        return static_cast<std::string>(v);
    }
    if (v.isNumber()) {
        return std::to_string(static_cast<long long>(v));
    }
    return static_cast<std::string>(v);
}

inline std::vector<std::string> parse_string_list(const eckit::Value& v) {
    std::vector<std::string> result;
    if (v.isNil()) {
        return result;
    }
    if (v.isList()) {
        for (size_t i = 0; i < v.size(); ++i) {
            result.push_back(scalar_to_string(v[i]));
        }
    } else {
        result.push_back(scalar_to_string(v));
    }
    return result;
}

/// One context-dependent branch of the model enum, as parsed from language.yaml.
struct ModelBranch {
    /// Context requirements: each entry maps a MARS key name to the set of
    /// canonical values that the output dict must contain for this branch to
    /// apply.  An empty map means the branch matches unconditionally.
    std::unordered_map<std::string, std::vector<std::string>> requiredContext;
    std::unordered_map<std::string, std::string> aliases;  ///< alias -> canonical
};

///
/// @brief Lazily build the ordered list of model branches from language.yaml.
///
/// The @c model key in language.yaml is a list of context-dependent enum
/// branches.  Each branch has an optional @c context map specifying which
/// @c class and/or @c country values it applies to, followed by a @c values
/// list of @c [canonical, alias1, alias2, ...] rows.  The last branch
/// typically has no context (default/fallthrough).
///
inline const std::vector<ModelBranch>& model_branches() {
    static const std::vector<ModelBranch> branches = []() {
        std::vector<ModelBranch> result;
        const eckit::Value all =
            eckit::YAMLParser::decodeFile(metkit::LibMetkit::languageYamlFile());
        const eckit::Value retrieve = all["retrieve"];
        if (retrieve.isNil() || !retrieve.isMap() || !retrieve.contains("model")) {
            return result;
        }
        const eckit::Value modelDef = retrieve["model"];
        if (!modelDef.isMap() || !modelDef.contains("type")) {
            return result;
        }
        const eckit::Value typeList = modelDef["type"];
        if (!typeList.isList()) {
            return result;
        }
        for (size_t b = 0; b < typeList.size(); ++b) {
            const eckit::Value branch = typeList[b];
            if (!branch.isMap() || !branch.contains("values")) {
                continue;
            }
            ModelBranch mb;
            // Parse optional context conditions: iterate every key in the
            // context map so any future context dimension is handled automatically.
            if (branch.contains("context")) {
                const eckit::Value ctx = branch["context"];
                if (ctx.isMap()) {
                    const eckit::Value ctxKeys = ctx.keys();
                    for (size_t k = 0; k < ctxKeys.size(); ++k) {
                        const std::string ctxKey = scalar_to_string(ctxKeys[k]);
                        mb.requiredContext[ctxKey] = parse_string_list(ctx[ctxKey]);
                    }
                }
            }
            // Parse values: [canonical, alias1, alias2, ...] or bare scalar
            const eckit::Value values = branch["values"];
            if (values.isList()) {
                for (size_t i = 0; i < values.size(); ++i) {
                    const eckit::Value row = values[i];
                    if (row.isList() && row.size() > 0) {
                        const std::string canonical = scalar_to_string(row[0]);
                        for (size_t j = 0; j < row.size(); ++j) {
                            mb.aliases.emplace(scalar_to_string(row[j]), canonical);
                        }
                    } else if (!row.isList()) {
                        const std::string canonical = scalar_to_string(row);
                        mb.aliases.emplace(canonical, canonical);
                    }
                }
            }
            result.push_back(std::move(mb));
        }
        return result;
    }();
    return branches;
}

/// Return true when every context requirement of @p branch is satisfied.
///
/// For each (key, required_values) pair in @c branch.requiredContext the
/// normalized value of that key is read from @p out (scratch).  The branch
/// matches only when @p out contains each required key with one of the
/// listed canonical values.
template <typename MarsDict_t>
bool context_matches(const ModelBranch& branch, const MarsDict_t& out) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    for (const auto& [key, required_vals] : branch.requiredContext) {
        const std::string actual = get_opt<std::string>(out, key).value_or("");
        if (std::find(required_vals.begin(), required_vals.end(), actual) == required_vals.end()) {
            return false;
        }
    }
    return true;
}

/// Resolve @p value to its canonical model name given the current context.
/// @p out  The already-populated output dict (scratch); every context key
///          is read from here so that sanitized canonical values are used.
template <typename MarsDict_t>
inline std::string resolve_or_throw(const std::string& value, const MarsDict_t& out) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    for (const ModelBranch& branch : model_branches()) {
        if (!context_matches(branch, out)) {
            continue;
        }
        // First matching branch is authoritative — validate strictly within it.
        auto it = branch.aliases.find(value);
        if (it != branch.aliases.end()) {
            return it->second;
        }
        // Build a readable description of the active context for the error.
        std::ostringstream ctx_desc;
        for (const auto& [key, required_vals] : branch.requiredContext) {
            ctx_desc << " " << key << "='" << get_opt<std::string>(out, key).value_or("") << "'";
        }
        std::ostringstream msg;
        msg << "model: value '" << value << "' is not valid";
        if (ctx_desc.tellp() > 0) {
            msg << " for" << ctx_desc.str();
        }
        msg << " (not found in the applicable language.yaml model branch)";
        throw Mars2GribGenericException(msg.str(), Here());
    }

    std::ostringstream msg;
    msg << "model: no applicable context branch found for value '" << value << "'";
    throw Mars2GribGenericException(msg.str(), Here());
}

}  // namespace model_detail

///
/// @brief Sanitize the MARS key: model.
///
/// Selects the applicable context branch from the @c model enum in
/// @c language.yaml by inspecting the normalized @c class already present
/// in @p out (scratch) and the raw @c country in @p in, then validates the
/// supplied value against that branch only.
///
/// @note Ordering constraints (enforced in SanitizationRegistry.h):
///   Context keys are looked up in @p out (scratch), so each key named in
///   any @c context block of the model branches must have been sanitized
///   before this rule runs.  Currently those keys are:
///   - @c class  — must precede model (already enforced in registry)
///   - @c country — must precede model IF country is ever added to the registry
///     (currently country is not sanitized so it will always be absent from
///     @p out, meaning country-gated branches will never match until that
///     changes — which is intentional: add a country sanitizer and register
///     it before model when needed).
///
/// No default is applied; no-op if the key is absent.
///
template <typename MarsDict_t>
void sanitise_model_or_throw(const MarsDict_t& in, MarsDict_t& out, const eckit::Value& /*language*/) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    if (auto v = get_opt<std::string>(in, "model")) {
        set_or_throw<std::string>(out, "model", model_detail::resolve_or_throw(*v, out));
        return;
    }
    if (auto v = get_opt<long>(in, "model")) {
        set_or_throw<std::string>(out, "model",
            model_detail::resolve_or_throw(std::to_string(*v), out));
    }
}

}  // namespace metkit::mars2grib::frontend::normalization::per_key
