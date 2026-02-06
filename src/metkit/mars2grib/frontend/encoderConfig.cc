/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "encoderConfig.h"

#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/geo/Grid.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/log/Log.h"
#include "eckit/spec/Custom.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "common.h"
#include "level.h"
#include "pdt.h"
#include "statistical.h"
#include "time.h"

using metkit::mars2grib::utils::dict_traits::get_opt;
using metkit::mars2grib::utils::dict_traits::get_or_throw;
using metkit::mars2grib::utils::dict_traits::has;

namespace metkit::mars2grib::frontend {

//================================= Defaults =================================//

void setDefaults(eckit::LocalConfiguration& sections) {
    setRecursive(sections, "type", "grib2");

    // Section 0 -- Indicator
    setRecursive(sections, "indicator-section.template-number", 0);

    // Section 1 -- Identification
    setRecursive(sections, "identification-section.template-number", 0);
    setRecursive(sections, "identification-section.tables-configurator.type", "default");
    setRecursive(sections, "identification-section.tables-configurator.local-tables-version", 0);
    setRecursive(sections, "identification-section.origin-configurator.type", "default");
    setRecursive(sections, "identification-section.origin-configurator.sub-centre", 0);
    setRecursive(sections, "identification-section.data-type-configurator.type", "default");
    setRecursive(sections, "identification-section.reference-time-configurator.type", "default");

    // Section 2 -- Local use
    setRecursive(sections, "local-use-section.template-number", 0);

    // Section 3 -- Grid
    setRecursive(sections, "grid-definition-section.template-number", 0);

    // Section 4 -- Product definition
    // TODO -- Some stuff still missing?
    setRecursive(sections, "product-definition-section.param-configurator.type", "paramId");
    setRecursive(sections, "product-definition-section.model-configurator.type", "default");
    // PDT
    setRecursive(sections, "product-definition-section.product-categories.timeExtent", "None");
    setRecursive(sections, "product-definition-section.product-categories.timeFormat", "None");
    setRecursive(sections, "product-definition-section.product-categories.spatialExtent", "None");
    setRecursive(sections, "product-definition-section.product-categories.processType", "None");
    setRecursive(sections, "product-definition-section.product-categories.processSubType", "None");
    setRecursive(sections, "product-definition-section.product-categories.productCategory", "None");
    setRecursive(sections, "product-definition-section.product-categories.productSubCategory", "None");

    // Section 5 -- Data representation
    setRecursive(sections, "data-representation-section.template-number", 0);
}

//========================= Grid Definition Section ==========================//

void setGridDefinitionSection(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    if (const auto& marsGrid = get_opt<std::string>(mars, "grid"); marsGrid.has_value()) {
        const auto gridType = eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", *marsGrid}})->type();
        if (gridType == "regular-gg") {
            setRecursive(sections, "grid-definition-section.template-number", 40);  // Gaussian grid (GG)
            setRecursive(sections, "grid-definition-section.representation.type", "regularGaussian");
        }
        else if (gridType == "reduced-gg") {
            setRecursive(sections, "grid-definition-section.template-number", 40);  // Gaussian grid (GG)
            setRecursive(sections, "grid-definition-section.representation.type", "reducedGaussian");
        }
        else if (gridType == "regular-ll") {
            setRecursive(sections, "grid-definition-section.template-number", 0);  // Lat-long grid (LL)
        }
        else {
            throw eckit::Exception{"Cannot encode grid \"" + *marsGrid + "\" with grid type \"" + gridType + "\"! ",
                                   Here()};
        }
    }
    else if (has(mars, "truncation")) {
        setRecursive(sections, "grid-definition-section.template-number", 50);  // Spherical-harmonics
    }
    else {
        throw eckit::Exception("Unknown grid!", Here());
    }
}

//============================ Local Use Section =============================//

void setLocalUseSection(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    if (has(mars, "anoffset")) {
        if (get_opt<std::string>(mars, "class") == "d1") {
            setRecursive(sections, "local-use-section.template-number", 1036);
        }
        else {  // class != d1
            setRecursive(sections, "local-use-section.template-number", 36);
        }
    }
    else {  // anoffset missing
        if (get_opt<std::string>(mars, "class") == "d1") {
            setRecursive(sections, "local-use-section.template-number", 1001);
        }
        else {  // class != d1
            if (has(mars, "method")) {
                setRecursive(sections, "local-use-section.template-number", 15);
            }
            else {  // method missing
                if (has(mars, "channel")) {
                    const auto& type = get_opt<std::string>(mars, "type").value_or("None");
                    if (type == "em" || type == "es" || type == "ssd") {
                        setRecursive(sections, "local-use-section.template-number", 24);
                    }
                    else {
                        throw eckit::Exception("Unsupported type \"" + type + "\"!", Here());
                    }
                }
                else {  // channel missing
                    setRecursive(sections, "local-use-section.template-number", 1);
                }
            }
        }
    }
}

//=============================== Process Type ===============================//

void setProcessType(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    if (has(mars, "channel")) {
        return;  // Satellite field
    }

    if (get_or_throw<std::string>(mars, "levtype") == "al") {
        // Large ensemble
        if (!has(mars, "number")) {
            throw eckit::Exception{"Expected mars keyword \"number\"", Here()};
        }
        setPDT(sections, "processSubType", "largeEnsemble");
        setRecursiveDefault(sections, "product-definition-section.ensemble-configurator.type", "default");
        if (has(mars, "hdate")) {
            setPDT(sections, "processType", "reforecast");
        }
    }
    else {
        // Ensemble
        if (has(mars, "number")) {
            if (has(mars, "hdate")) {
                setPDT(sections, "processType", "reforecast");
            }
            setPDT(sections, "processSubType", "ensemble");
            setRecursiveDefault(sections, "product-definition-section.ensemble-configurator.type", "default");
        }
        else {
            if (has(mars, "hdate")) {
                throw eckit::Exception{"unexpected mars keyword \"hdate\"", Here()};
            }
            if (const auto& type = get_opt<std::string>(mars, "type"); type && (*type == "em" || *type == "es")) {
                // Derived ensemble forecast
                setPDT(sections, "processType", "derivedForecast");
                setPDT(sections, "processSubType", "ensemble");
                setRecursive(sections, "product-definition-section.ensemble-configurator.type", "derived");
            }
            // Else, just pass through...
        }
    }
}

//================================ Horizontal ================================//

bool matchChemical(const eckit::LocalConfiguration& mars) {
    return (has(mars, "chem") && !has(mars, "wavelength") && get_or_throw<long>(mars, "chem") < 900);
}

void setChemical(eckit::LocalConfiguration& sections) {
    setRecursiveDefault(sections, "product-definition-section.chemistry-configurator.type", "chemical");
    setPDT(sections, "productCategory", "chemical");
}

// These rules had to be ported manually as they don't follow the same pattern as most level/time/statistical rules
bool setMiscHorizontal(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    const auto param = get_or_throw<long>(mars, "param");

    if (has(mars, "channel")) {
        // Satellite field
        if (const auto& type = get_opt<std::string>(mars, "type"); type && (*type == "em" || *type == "es")) {
            // Derived ensemble forecast satellite
            if (const auto param = get_or_throw<long>(mars, "param"); matchAny(param, 194)) {
                setTypeOfLevel(sections, "surface");
                setPointInTime(sections);
                return true;
            }
        }
        else {
            // Single satellite
            if (const auto param = get_or_throw<long>(mars, "param"); matchAny(param, range(260510, 260512))) {
                setPointInTime(sections);
                setRecursiveDefault(sections, "product-definition-section.satellite-configurator.type", "default");
                setPDT(sections, "productCategory", "satellite");
                return true;
            }
        }
        throw eckit::Exception{"Unhandled satellite field!", Here()};
    }

    // Not a satellite field
    if (get_or_throw<std::string>(mars, "levtype") == "sfc") {
        if (matchChemical(mars)) {
            if (matchAny(param, range(228080, 228082), range(233032, 233035), range(235062, 235064))) {
                setTypeOfLevel(sections, "surface");
                setSinceLastPostProcessingStep(sections);
                setTypeOfStatisticalProcessing(sections, "accumul");
                setChemical(sections);
                setRecursive(sections, "identification-section.tables-configurator.type", "custom");
                setRecursive(sections, "identification-section.tables-configurator.tables-version", 30);
                setRecursive(sections, "identification-section.tables-configurator.local-tables-version", 0);
                return true;
            }
            else if (matchAny(param, range(228083, 228085))) {
                setTypeOfLevel(sections, "surface");
                setPointInTime(sections);
                setChemical(sections);
                return true;
            }
        }
        else if (matchAny(param, range(140114, 140120))) {
            setTypeOfLevel(sections, "surface");
            setPointInTime(sections);

            // Note: this param is a period-range!
            setRecursiveDefault(sections, "product-definition-section.period-configurator.type", "default");
            setPDT(sections, "productCategory", "wave");
            setPDT(sections, "productSubCategory", "periodRange");
            return true;
        }
        else if (matchAny(param, 140251)) {
            setPointInTime(sections);

            // Note: this param does not have a typeOfLevel, but direction and frequency instead!
            setRecursiveDefault(sections, "product-definition-section.directions-frequencies-configurator.type",
                                "default");
            setPDT(sections, "productCategory", "wave");
            setPDT(sections, "productSubCategory", "spectraList");
            return true;
        }
    }
    return false;
}

void setHorizontal(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    if (setMiscHorizontal(mars, sections)) {
        return;  // Bypass for special rules not captured below
    }

    setLevel(mars, sections);
    setTime(mars, sections);
    setStatistical(mars, sections);
}

//======================= Data Representation Section ========================//

void setDataRepresentationSection(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    const auto& packing = get_opt<std::string>(mars, "packing");
    if (!packing) {
        throw eckit::Exception{"Mars keyword \"packing\" is missing!", Here()};
    }
    if (*packing == "simple") {
        setRecursive(sections, "data-representation-section.template-number", 0);
    }
    else if (*packing == "ccsds") {
        setRecursive(sections, "data-representation-section.template-number", 42);
    }
    else if (*packing == "complex") {
        setRecursive(sections, "data-representation-section.template-number", 51);
    }
    else {
        throw eckit::Exception{"Unknown value \"" + *packing + "\" for mars keyword \"packing\"!", Here()};
    }
}

void setAll(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    setDefaults(sections);

    setGridDefinitionSection(mars, sections);
    setLocalUseSection(mars, sections);

    setProcessType(mars, sections);
    setHorizontal(mars, sections);

    setDataRepresentationSection(mars, sections);

    setRecursive(sections, "product-definition-section.template-number",
                 templateNumberFromPDT(get_or_throw<eckit::LocalConfiguration>(
                     sections, "product-definition-section.product-categories")));
}

eckit::LocalConfiguration buildEncoderConfig(const eckit::LocalConfiguration& mars) {
    try {
        eckit::LocalConfiguration sections;
        setAll(mars, sections);
        return sections;
    }
    catch (...) {
        LOG_DEBUG_LIB(LibMetkit) << "Could not create encoder configuration from mars: " << mars << std::endl;
        throw;
    }
}

}  // namespace metkit::mars2grib::frontend
