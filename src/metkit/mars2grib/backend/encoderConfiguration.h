#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/backend/concepts/conceptRegistry.h"
#include "metkit/mars2grib/backend/sections/initializers/sectionRegistry.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

// Ordered recipes (vector-based)
#include "metkit/mars2grib/backend/sections/recipes/Recipes.h"

namespace metkit::mars2grib::backend::config {

// =================================================================================================
// Public data model
// =================================================================================================

using metkit::mars2grib::backend::concepts_::NUM_SECTIONS;
using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

struct ConceptCfg {
    std::string name;
    std::string type;
};

struct SectionCfg {
    uint16_t templateNumber;
    std::vector<ConceptCfg> concepts;  // ORDER IS SEMANTIC
};

struct EncoderCfg {
    std::array<SectionCfg, NUM_SECTIONS> sections_;
};

// =================================================================================================
// Implementation details
// =================================================================================================
namespace internal {

using metkit::mars2grib::backend::sections::recipes::findRecipe;
using metkit::mars2grib::backend::sections::recipes::SectionRecipe;
using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

// -------------------------------------------------------------------------------------------------
// Section names
// -------------------------------------------------------------------------------------------------

inline constexpr std::array<std::string_view, NUM_SECTIONS> sectionNames = {
    {"indicator-section", "identification-section", "local-use-section", "grid-definition-section",
     "product-definition-section", "data-representation-section"}};

// -------------------------------------------------------------------------------------------------
// Utilities
// -------------------------------------------------------------------------------------------------

inline std::string strip_descriptor(std::string_view name) {

    try {
        constexpr std::string_view suffix = "-configurator";
        if (name.size() >= suffix.size() && name.substr(name.size() - suffix.size()) == suffix) {
            return std::string{name.substr(0, name.size() - suffix.size())};
        }
        return std::string{name};
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Error stripping descriptor from concept name " + std::string(name), Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
}

// -------------------------------------------------------------------------------------------------
// YAML normalization (SELF-CONTAINED)
// -------------------------------------------------------------------------------------------------

inline std::string resolveReferenceTimeType(const eckit::LocalConfiguration& cfg) {

    try {
        if (!cfg.has("product-definition-section")) {
            throw Mars2GribGenericException("No product definition section in configuration", Here());
        }

        const auto pds = cfg.getSubConfiguration("product-definition-section");

        if (!pds.has("template-number")) {
            throw Mars2GribGenericException("No product definition template number in configuration", Here());
        }

        const long tmpl = pds.getLong("template-number");
        return (tmpl == 60 || tmpl == 61) ? "reforecast" : "standard";
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error resolving reference time type", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
}

inline std::string mapStatisticsType(const std::string& stat, const std::string& name) {

    try {
        static const std::map<std::string, std::string> table = {
            {"average", "average"},  {"accumul", "accumulation"},     {"max", "maximum"},
            {"min", "minimum"},      {"stddev", "standardDeviation"}, {"mode", "mode"},
            {"severity", "severity"}};

        auto it = table.find(stat);
        if (it == table.end()) {
            throw Mars2GribGenericException(
                "Unsupported type-of-statistical-processing " + stat + " for concept " + name, Here());
        }

        return it->second;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error mapping statistics type", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
}

// -------------------------------------------------------------------------------------------------
// Populate normalized YAML overrides
// -------------------------------------------------------------------------------------------------

inline void populateConceptsFromYaml(const eckit::LocalConfiguration& cfg, const eckit::LocalConfiguration& sectionCfg,
                                     std::map<std::string, std::string>& out) {
    try {
        for (const auto& name : sectionCfg.keys()) {

            if (name == "template-number" || name == "product-categories") {
                continue;
            }

            const auto conceptCfg = sectionCfg.getSubConfiguration(name);

            if (!conceptCfg.has("type")) {
                throw Mars2GribGenericException("No type found for concept " + name, Here());
            }

            std::string key  = strip_descriptor(name);
            std::string type = conceptCfg.getString("type");

            // ----------------------------
            // Semantic normalization rules
            // ----------------------------
            if (key == "model") {
                out["generatingProcess"] = type;
            }
            else if (key == "data-type") {
                out["dataType"] = type;
            }
            else if (key == "reference-time") {
                out["referenceTime"] = resolveReferenceTimeType(cfg);
            }
            else if (key == "direction-frequency") {
                out["wave"] = "spectra";
            }
            else if (key == "period") {
                out["wave"] = "period";
            }
            else if (key == "ensemble") {
                out["ensemble"] = "individual";
            }
            else if (key == "random-patterns") {
                out["ensemble"] = "randomPatterns";
            }
            else if (key == "point-in-time") {
                out["pointInTime"] = type;
            }
            else if (key == "chemistry") {
                out["composition"] = type;
            }
            else if (key == "param") {
                out["param"] = "default";
            }
            else if (key == "time-statistics") {
                const std::string stat = conceptCfg.getString("type-of-statistical-processing");
                out["statistics"]      = mapStatisticsType(stat, name);
            }
            else {
                out[key] = type;
            }
        }
        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error populating concepts from YAML", Here()));
    }

    // Remove compiler warning
    __builtin_unreachable();
}

// -------------------------------------------------------------------------------------------------
// Build one section from YAML + recipe (ORDERED)
// -------------------------------------------------------------------------------------------------

inline SectionCfg buildSection(const eckit::LocalConfiguration& cfg, uint16_t secId) {

    try {
        const std::string sectionName(sectionNames[secId]);

        if (!cfg.has(sectionName)) {
            throw Mars2GribGenericException(sectionName + " is missing", Here());
        }

        const auto sectionCfg = cfg.getSubConfiguration(sectionName);

        if (!sectionCfg.has("template-number")) {
            throw Mars2GribGenericException(sectionName + " has no template number", Here());
        }

        const uint16_t tmpl = static_cast<uint16_t>(sectionCfg.getLong("template-number"));

        const SectionRecipe* recipe = findRecipe(secId, tmpl);

        if (!recipe) {
            throw Mars2GribGenericException(
                "No recipe found for section " + sectionName + " template " + std::to_string(tmpl), Here());
        }

        std::map<std::string, std::string> overrides;
        populateConceptsFromYaml(cfg, sectionCfg, overrides);

        SectionCfg out;
        out.templateNumber = tmpl;

        for (const auto& spec : recipe->concepts) {

            std::string finalType = std::string(spec.type);

            auto it = overrides.find(std::string(spec.name));
            if (it != overrides.end()) {
                if (spec.type == "default") {
                    finalType = it->second;
                }
                else if (it->second != spec.type) {
                    throw Mars2GribGenericException("Concept type mismatch for concept " + std::string(spec.name) +
                                                        " - expected " + std::string(spec.type) + ", got " + it->second,
                                                    Here());
                }
            }

            out.concepts.push_back(ConceptCfg{std::string(spec.name), std::move(finalType)});
        }

        return out;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error building section configuration", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

// -------------------------------------------------------------------------------------------------
// Cross-section propagation (EXTENSIBLE)
// -------------------------------------------------------------------------------------------------

inline void applyCrossPropagationRules(EncoderCfg& cfg) {

    try {
        // RULE 1:
        // referenceTime from section 4 → section 1
        for (const auto& c : cfg.sections_[4].concepts) {
            if (c.name == "referenceTime") {
                for (auto& d : cfg.sections_[1].concepts) {
                    if (d.name == "referenceTime") {
                        d.type = c.type;
                    }
                }
            }
        }
        // RULE 2:
        // referenceTime from section 4 → section 1
        for (const auto& c : cfg.sections_[4].concepts) {
            if (c.name == "satellite") {
                for (auto& d : cfg.sections_[1].concepts) {
                    if (d.name == "satellite") {
                        d.type = c.type;
                    }
                }
            }
        }

        // Future rules go here
        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error applying cross-propagation rules", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace internal

// =================================================================================================
// Public API
// =================================================================================================

inline EncoderCfg makeEncoderConfiguration(const eckit::LocalConfiguration& cfg) {
    try {
        EncoderCfg out;

        for (uint16_t i = 0; i < NUM_SECTIONS; ++i) {
            out.sections_[i] = internal::buildSection(cfg, i);
        }

        internal::applyCrossPropagationRules(out);
        return out;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error making encoder configuration", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline auto makeEncoderCallbacks(const EncoderCfg& cfg) {
    // ---------------------------------------------------------------------------------------------
    // Type aliases (readability)
    // ---------------------------------------------------------------------------------------------
    using Fn_t = metkit::mars2grib::backend::concepts_::Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
    using metkit::mars2grib::backend::concepts_::NUM_SECTIONS;
    using metkit::mars2grib::backend::concepts_::NUM_STAGES;
    using StageTable  = std::array<std::vector<Fn_t>, NUM_SECTIONS>;
    using CallbackTbl = std::array<StageTable, NUM_STAGES + 1>;

    using metkit::mars2grib::backend::concepts_::concept_registry_instance;
    using metkit::mars2grib::backend::sections::initializers::getSectionInitializerFn;

    try {

        // -----------------------------------------------------------------------------------------
        // Registry
        // -----------------------------------------------------------------------------------------
        const auto& registry = concept_registry_instance<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>();

        CallbackTbl callbacks;

        // -----------------------------------------------------------------------------------------
        // Build callbacks directly from EncoderCfg
        // -----------------------------------------------------------------------------------------
        for (uint16_t sid = 0; sid < NUM_SECTIONS; ++sid) {

            // ---- Stage 0: section initializer (exactly one) ----
            try {
                const long tmpl = cfg.sections_[sid].templateNumber;
                callbacks[0][sid].push_back(
                    getSectionInitializerFn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(sid, tmpl));
            }
            catch (...) {
                std::throw_with_nested(Mars2GribGenericException("Error getting section initializer for section " +
                                                                     std::to_string(sid) + " template " +
                                                                     std::to_string(cfg.sections_[sid].templateNumber),
                                                                 Here()));
            }

            // ---- Stage >=1: concept callbacks (ORDER PRESERVED) ----
            for (const auto& cs : cfg.sections_[sid].concepts) {

                const auto key = std::make_pair(std::string(cs.name), std::string(cs.type));

                auto it = registry.map.find(key);
                if (it == registry.map.end()) {
                    throw Mars2GribGenericException("Concept not found in registry: " + cs.name + " / " + cs.type +
                                                        " (section " + std::to_string(sid) + ")",
                                                    Here());
                }
                const auto& fnByStageAndSection = it->second;

                try {
                    for (std::size_t stage = 1; stage <= NUM_STAGES; ++stage) {
                        if (auto fn = fnByStageAndSection[stage - 1][sid]) {
                            callbacks[stage][sid].push_back(fn);
                        }
                    }
                }
                catch (...) {
                    std::throw_with_nested(
                        Mars2GribGenericException("Error populating callbacks for concept: " + cs.name + " / " +
                                                      cs.type + " (section " + std::to_string(sid) + ")",
                                                  Here()));
                }
            }
        }

        return callbacks;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error making encoder callbacks", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

inline void printEncoderConfiguration(const EncoderCfg& cfg, std::ostream& os = std::cout) {

    try {
        os << "Encoder configuration\n";
        os << "=====================\n";

        for (uint16_t sid = 0; sid < NUM_SECTIONS; ++sid) {

            const auto& section = cfg.sections_[sid];

            os << "\n";
            os << "Section [" << sid << "]\n";
            os << "  Template number: " << section.templateNumber << "\n";
            os << "  Concepts:\n";

            if (section.concepts.empty()) {
                os << "    (none)\n";
                continue;
            }

            for (const auto& cs : section.concepts) {
                os << "    - " << cs.name << " : " << cs.type << "\n";
            }
        }

        os << std::flush;

        return;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error printing encoder configuration", Here()));
    }
    // Remove compiler warning
    __builtin_unreachable();
}

inline std::string encoderConfiguration_to_json(const EncoderCfg& cfg) noexcept(true) {
    try {
        std::ostringstream os;

        os << "{\n";
        os << "  \"encoderConfiguration\": {\n";
        os << "    \"sections\": [\n";

        for (uint16_t sid = 0; sid < NUM_SECTIONS; ++sid) {

            const auto& section = cfg.sections_[sid];

            os << "      {\n";
            os << "        \"id\": " << sid << ",\n";
            os << "        \"templateNumber\": " << section.templateNumber << ",\n";
            os << "        \"concepts\": [";

            if (section.concepts.empty()) {
                os << "]";
            }
            else {
                os << "\n";
                for (size_t i = 0; i < section.concepts.size(); ++i) {
                    const auto& cs = section.concepts[i];

                    os << "          {\n";
                    os << "            \"name\": \"" << cs.name << "\",\n";
                    os << "            \"type\": \"" << cs.type << "\"\n";
                    os << "          }";

                    if (i + 1 < section.concepts.size()) {
                        os << ",";
                    }
                    os << "\n";
                }
                os << "        ]";
            }

            os << "\n      }";

            if (sid + 1 < NUM_SECTIONS) {
                os << ",";
            }
            os << "\n";
        }

        os << "    ]\n";
        os << "  }\n";
        os << "}\n";

        return os.str();
    }
    catch (...) {
        return R"({
  "encoderConfiguration": {
    "warning": "Failed to serialize encoder configuration to JSON"
  }
})";
    }
}

}  // namespace metkit::mars2grib::backend::config
