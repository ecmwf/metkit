#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/Log.h"


#include "metkit/mars2grib/backend/concepts/concept_registry.h"
#include "metkit/mars2grib/backend/sections/section_registry.h"
#include "metkit/mars2grib/backend/sections/sections_recipes.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::utils::cfg {

// Namespaces shortcuts
using metkit::mars2grib::backend::cnpts::NUM_SECTIONS;
using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

// Namespaces shortcuts
inline constexpr std::array<std::string_view, NUM_SECTIONS> sectionNames = {
    {"indicator-section", "identification-section", "local-use-section", "grid-definition-section",
     "product-definition-section", "data-representation-section"}};


inline std::string strip_descriptor(std::string_view name) {
    constexpr std::string_view suffix = "-configurator";
    if (name.size() >= suffix.size() && name.substr(name.size() - suffix.size()) == suffix) {
        return std::string{name.substr(0, name.size() - suffix.size())};
    }
    return std::string{name};
}

struct Section {
    uint16_t templateNumber_;
    std::map<std::string, std::string> concepts_;
};

struct EncoderCfg {
    std::array<Section, NUM_SECTIONS> sec_;
};

Section lookupCfgSection(const eckit::LocalConfiguration& cfg, long SecId) {

    // Get section name from Id
    std::string sectionName = std::string(sectionNames[SecId]);

    // Verify the section is present
    if (!cfg.has(sectionName)) {
        throw Mars2GribGenericException(sectionName + " is missing", Here());
    }

    // Get section configuration
    eckit::LocalConfiguration sectionCfg = cfg.getSubConfiguration(sectionName);

    // Initialize the Section
    Section sec;

    // Verify template number is present
    if (!sectionCfg.has("template-number")) {
        throw Mars2GribGenericException(sectionName + " has no template number", Here());
    }

    // Get template number
    sec.templateNumber_ = static_cast<uint16_t>(sectionCfg.getLong("template-number"));

    // Populate concepts
    for (auto& name : sectionCfg.keys()) {

        // Skip template number
        if (name == "template-number") {
            continue;  // Skip template number
        };

        // Get the key in the concept map
        std::string key = strip_descriptor(name);

        // Get the type
        eckit::LocalConfiguration conceptCfg = sectionCfg.getSubConfiguration(name);
        if (!conceptCfg.has("type")) {
            throw Mars2GribGenericException("No type found for concept " + name, Here());
        }
        std::string type = conceptCfg.getString("type");

        // =========================================================================================
        // Modify type
        if (key == "model") {
            sec.concepts_.insert(std::make_pair("generatingProcess", std::string(type)));
        }
        else if (key == "data-type") {
            sec.concepts_.insert(std::make_pair("dataType", std::string(type)));
        }
        else if (key == "reference-time") {
            if (cfg.has("product-definition-section")) {
                auto tmp = cfg.getSubConfiguration("product-definition-section");
                if (tmp.has("template-number")) {
                    long tmplNum = tmp.getLong("template-number");
                    if (tmplNum == 60 || tmplNum == 61) {
                        type = "reforecast";
                    }
                    else {
                        type = "standard";
                    }
                }
                else {
                    throw Mars2GribGenericException("No product definition template number in configuration", Here());
                }
            }
            else {
                throw Mars2GribGenericException("No product definition template number in configuration", Here());
            }
            sec.concepts_.insert(std::make_pair("referenceTime", std::string(type)));
        }
        else if (key == "direction-frequency") {
            sec.concepts_.insert(std::make_pair("wave", "spectra"));
        }
        else if (key == "period") {
            sec.concepts_.insert(std::make_pair("wave", "period"));
        }
        else if (key == "ensemble") {
            sec.concepts_.insert(std::make_pair("ensemble", "individual"));
        }
        else if (key == "point-in-time") {
            sec.concepts_.insert(std::make_pair("pointInTime", std::string(type)));
        }
        else if (key == "chemistry") {
            sec.concepts_.insert(std::make_pair("composition", std::string(type)));
        }
        else if (key == "param") {
            sec.concepts_.insert(std::make_pair("param", "default"));
        }
        else if (key == "time-statistics") {
            std::string typeOfStatisticalProcess = conceptCfg.getString("type-of-statistical-processing");
            if (typeOfStatisticalProcess == "average") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "average"));
            }
            else if (typeOfStatisticalProcess == "accumul") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "accumulation"));
            }
            else if (typeOfStatisticalProcess == "max") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "maximum"));
            }
            else if (typeOfStatisticalProcess == "min") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "minimum"));
            }
            else if (typeOfStatisticalProcess == "stddev") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "standardDeviation"));
            }
            else if (typeOfStatisticalProcess == "mode") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "mode"));
            }
            else if (typeOfStatisticalProcess == "severity") {
                // Insert into the map
                sec.concepts_.insert(std::make_pair("statistics", "severity"));
            }
            else {
                throw Mars2GribGenericException(
                    "Unsupported type-of-statistical-processing " + typeOfStatisticalProcess + " for concept " + name,
                    Here());
            }
        }
        else {
            sec.concepts_.insert(std::make_pair(key, type));
        }

    }  // for concept name

    return sec;

};  // lookupCfgSection


Section lookupExpectedSection(const eckit::LocalConfiguration& cfg, long SecId) {

    using metkit::mars2grib::backend::sections::ConceptList;
    using metkit::mars2grib::backend::sections::resolveSectionTemplateConcepts;

    // Get section name from Id
    std::string sectionName = std::string(sectionNames[SecId]);

    // Verify the section is present
    if (!cfg.has(sectionName)) {
        throw Mars2GribGenericException(sectionName + " is missing", Here());
    }

    // Get section configuration
    eckit::LocalConfiguration sectionCfg = cfg.getSubConfiguration(sectionName);

    // Initialize the Section
    Section sec;

    // Verify template number is present
    if (!sectionCfg.has("template-number")) {
        throw Mars2GribGenericException(sectionName + " has no template number", Here());
    }

    // Get template number
    sec.templateNumber_ = static_cast<uint16_t>(sectionCfg.getLong("template-number"));

    // Populate concepts
    const std::optional<ConceptList> concepts = resolveSectionTemplateConcepts(SecId, sec.templateNumber_);

    // Insert into the map
    if (concepts) {
        for (const auto& concept : *concepts) {
            std::string key  = std::string(concept.name);
            std::string type = concept.type ? std::string(*concept.type) : "default";
            sec.concepts_.insert(std::make_pair(key, type));
        }
    }
    else {
        throw Mars2GribGenericException(
            "No concepts found for " + sectionName + " template number " + std::to_string(sec.templateNumber_), Here());
    }

    return sec;
};


EncoderCfg parseEncoderCfg(const eckit::LocalConfiguration& cfg) {

    // Initialize EncoderCfg
    EncoderCfg encoderCfg;
    EncoderCfg expectedCfg;
    EncoderCfg combinedCfg;

    // Populate all sections
    for (size_t i = 0; i < NUM_SECTIONS; ++i) {
        encoderCfg.sec_[i]  = lookupCfgSection(cfg, i);
        expectedCfg.sec_[i] = lookupExpectedSection(cfg, i);
    }

    // Combine configurations
    for (size_t i = 0; i < NUM_SECTIONS; ++i) {

        // Combine template number
        if (encoderCfg.sec_[i].templateNumber_ != expectedCfg.sec_[i].templateNumber_) {
            throw Mars2GribGenericException("Template number mismatch for section ", Here());
        }
        combinedCfg.sec_[i].templateNumber_ = encoderCfg.sec_[i].templateNumber_;

        // Combine concepts
        for (const auto& [key, type] : expectedCfg.sec_[i].concepts_) {
            auto it = encoderCfg.sec_[i].concepts_.find(key);
            if (it != encoderCfg.sec_[i].concepts_.end()) {
                // Verify type matches
                if (type != "default" && it->second != type) {
                    throw Mars2GribGenericException("Concept type mismatch for concept ", Here());
                }
                combinedCfg.sec_[i].concepts_.insert(std::make_pair(key, type));
            }
            else {
                combinedCfg.sec_[i].concepts_.insert(std::make_pair(key, type));
            }
        }

        auto it = combinedCfg.sec_[4].concepts_.find("referenceTime");
        if (it != combinedCfg.sec_[4].concepts_.end()) {
            combinedCfg.sec_[1].concepts_["referenceTime"] = combinedCfg.sec_[4].concepts_["referenceTime"];
        }
    }


    // Validate concepts against expected
    return combinedCfg;
};

void print_encoder_cfg(const EncoderCfg& cfg) {
    for (uint32_t i = 0; i < NUM_SECTIONS; ++i) {
        std::cout << "Section " << i << " (template " << cfg.sec_[i].templateNumber_ << "):" << std::endl;
        for (const auto& [key, type] : cfg.sec_[i].concepts_) {
            std::cout << "  Concept: " << key << ", Type: " << type << std::endl;
        }
    }
};

}  // namespace metkit::mars2grib::utils::cfg
