/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// backgroundProcess per component of the coupled system (ecCodes ECC-2354), deduced from the MARS keys.
///
/// | MARS keys of the field                          | component | backgroundProcess |
/// |-------------------------------------------------|-----------|-------------------|
/// | stream=gfas or stream=gfra                      | gfas      | 144               |
/// | stream=ldas                                     | ldas      | 246               |
/// | class=mc, class=gg, class=cr or class=a5        | IFS-compo | 252               |
/// | levtype=o2d or levtype=o3d                      | nemo      | 250               |
/// | param 140xxx, 141xxx, 143xxx, 144xxx or 145xxx  | ecwam     | 248               |
/// | anything else                                   | IFS       | 254               |
///
/// The rules are tested in this order. `generatingProcessIdentifier` is not deduced and not translated: it
/// is written as given in the misc dictionary.

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/backend/deductions/backgroundProcess.h"
#include "metkit/mars2grib/backend/tables/backgroundProcess.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

using metkit::mars2grib::backend::deductions::componentOfField;
using metkit::mars2grib::backend::tables::BackgroundProcess;

/// An od oper forecast field at step 6, as mars2mars hands it to the encoder.
/// It has no `model` key, as the operational fields have none.
eckit::LocalConfiguration operRequest(long param, const std::string& levtype) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", levtype);
    mars.set("date", 2026'10'13);
    mars.set("time", 00'00);
    mars.set("step", 6);
    mars.set("timespan", "none");
    return mars;
}

/// A misc dictionary that sets `generatingProcessIdentifier` explicitly (what the `ncycle` override does).
eckit::LocalConfiguration miscWithGeneratingProcessIdentifier(long generatingProcessIdentifier) {
    eckit::LocalConfiguration misc;
    misc.set("generatingProcessIdentifier", generatingProcessIdentifier);
    return misc;
}

/// Encode a field of constant values with the given MARS and misc dictionaries.
std::unique_ptr<metkit::codes::CodesHandle> encode(const eckit::LocalConfiguration& mars,
                                                   const eckit::LocalConfiguration& misc) {
    auto encoder = metkit::mars2grib::Mars2Grib();
    return encoder.encode(std::vector<double>(213988, 1.0), mars, misc);
}

/// Print the exception stack of the encoder if a case throws.
template <typename F>
void run(F&& f) {
    try {
        f();
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

}  // namespace

//----------------------------------------------------------------------------------------------------------------------
// Atmosphere (IFS, 254)
//----------------------------------------------------------------------------------------------------------------------

CASE("atmosphere: 2t (167) on levtype=sfc is IFS (254)") {
    run([] {
        const auto mars = operRequest(167, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
    });
}

//----------------------------------------------------------------------------------------------------------------------
// Ocean diagnostics of the IFS (254) versus native ocean fields (nemo, 250)
//
// The same params exist on both level types. On levtype=sfc they are IFS diagnostics and have to stay on `sfc`,
// on levtype=o2d they are native ocean fields and have to stay on `o2d`.
//----------------------------------------------------------------------------------------------------------------------

CASE("IFS ocean diagnostic: t20d (262104) on levtype=sfc is IFS (254) and stays on sfc") {
    run([] {
        const auto mars = operRequest(262104, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
        EXPECT_EQUAL(handle->getLong("paramId"), 262104);
    });
}

CASE("IFS ocean diagnostic: sc300m (262118) on levtype=sfc is IFS (254) and stays on sfc") {
    run([] {
        const auto mars = operRequest(262118, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
        EXPECT_EQUAL(handle->getLong("paramId"), 262118);
    });
}

CASE("IFS ocean diagnostic: pt300m (262144) on levtype=sfc is IFS (254) and stays on sfc") {
    run([] {
        const auto mars = operRequest(262144, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
        EXPECT_EQUAL(handle->getLong("paramId"), 262144);
    });
}

CASE("native ocean field: t20d (262104) on levtype=o2d is nemo (250) and stays on o2d") {
    run([] {
        const auto mars = operRequest(262104, "o2d");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 250);
        EXPECT_EQUAL(handle->getString("modelName"), "nemo");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "o2d");
        EXPECT_EQUAL(handle->getLong("paramId"), 262104);
    });
}

CASE("native ocean field: sc300m (262118) on levtype=o2d is nemo (250) and stays on o2d") {
    run([] {
        const auto mars = operRequest(262118, "o2d");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 250);
        EXPECT_EQUAL(handle->getString("modelName"), "nemo");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "o2d");
        EXPECT_EQUAL(handle->getLong("paramId"), 262118);
    });
}

CASE("native ocean field: pt300m (262144) on levtype=o2d is nemo (250) and stays on o2d") {
    run([] {
        const auto mars = operRequest(262144, "o2d");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 250);
        EXPECT_EQUAL(handle->getString("modelName"), "nemo");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "o2d");
        EXPECT_EQUAL(handle->getLong("paramId"), 262144);
    });
}

CASE("native ocean field: levtype=o3d is nemo (deduction only)") {
    const auto mars = operRequest(262500, "o3d");

    EXPECT(componentOfField(mars) == BackgroundProcess::nemo);
}

//----------------------------------------------------------------------------------------------------------------------
// Waves (ecwam, 248)
//----------------------------------------------------------------------------------------------------------------------

CASE("waves: swh (140229) is ecwam (248)") {
    run([] {
        const auto mars = operRequest(140229, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 248);
        EXPECT_EQUAL(handle->getString("modelName"), "ecwam");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
    });
}

CASE("waves: the time statistics of the wave params are ecwam (deduction only)") {
    // 141xxx: time-mean
    EXPECT(componentOfField(operRequest(141229, "sfc")) == BackgroundProcess::ecwam);
    // 143xxx: time-maximum
    EXPECT(componentOfField(operRequest(143229, "sfc")) == BackgroundProcess::ecwam);
    // 144xxx: time-minimum
    EXPECT(componentOfField(operRequest(144229, "sfc")) == BackgroundProcess::ecwam);
    // 145xxx: time-standard-deviation
    EXPECT(componentOfField(operRequest(145229, "sfc")) == BackgroundProcess::ecwam);
}

CASE("waves: the neighbouring param tables are not ecwam (deduction only)") {
    EXPECT(componentOfField(operRequest(139229, "sfc")) == BackgroundProcess::ifs);
    EXPECT(componentOfField(operRequest(142229, "sfc")) == BackgroundProcess::ifs);
    EXPECT(componentOfField(operRequest(146229, "sfc")) == BackgroundProcess::ifs);
}

//----------------------------------------------------------------------------------------------------------------------
// Fire emissions (gfas, 144)
//----------------------------------------------------------------------------------------------------------------------

CASE("fire emissions: stream=gfas is gfas (144), whatever the class") {
    run([] {
        auto mars = operRequest(167, "sfc");
        mars.set("class", "mc");
        mars.set("stream", "gfas");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 144);
        EXPECT_EQUAL(handle->getString("modelName"), "gfas");
    });
}

CASE("fire emissions: stream=gfra (gfas in reanalysis mode) is gfas (deduction only)") {
    auto mars = operRequest(167, "sfc");
    mars.set("stream", "gfra");

    EXPECT(componentOfField(mars) == BackgroundProcess::gfas);
}

CASE("fire emissions: stream=gfas with class=we (wildfire emissions) is gfas (deduction only)") {
    auto mars = operRequest(167, "sfc");
    mars.set("class", "we");
    mars.set("stream", "gfas");

    EXPECT(componentOfField(mars) == BackgroundProcess::gfas);
}

CASE("fire emissions: the stream is tested before the class (deduction only)") {
    auto mars = operRequest(167, "sfc");
    mars.set("class", "mc");
    mars.set("stream", "gfra");

    EXPECT(componentOfField(mars) == BackgroundProcess::gfas);
}

//----------------------------------------------------------------------------------------------------------------------
// Land surface analysis (ldas, 246)
//----------------------------------------------------------------------------------------------------------------------

CASE("land surface analysis: stream=ldas is ldas (246)") {
    run([] {
        auto mars = operRequest(167, "sfc");
        mars.set("stream", "ldas");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 246);
        EXPECT_EQUAL(handle->getString("modelName"), "ldas");
    });
}

//----------------------------------------------------------------------------------------------------------------------
// Atmospheric composition (IFS-compo, 252)
//----------------------------------------------------------------------------------------------------------------------

CASE("atmospheric composition: class=mc is IFS-compo (252)") {
    run([] {
        auto mars = operRequest(167, "sfc");
        mars.set("class", "mc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 252);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS-compo");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
    });
}

CASE("atmospheric composition: class=cr is IFS-compo (deduction only)") {
    auto mars = operRequest(167, "sfc");
    mars.set("class", "cr");

    EXPECT(componentOfField(mars) == BackgroundProcess::ifs_compo);
}

CASE("atmospheric composition: class=a5 is IFS-compo (deduction only)") {
    auto mars = operRequest(167, "sfc");
    mars.set("class", "a5");

    EXPECT(componentOfField(mars) == BackgroundProcess::ifs_compo);
}

//----------------------------------------------------------------------------------------------------------------------
// The `model` key
//----------------------------------------------------------------------------------------------------------------------

CASE("model=IFS-NEMO, levtype=sfc: the component of the field, IFS (254)") {
    run([] {
        auto mars = operRequest(262104, "sfc");
        mars.set("model", "IFS-NEMO");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
    });
}

CASE("model=IFS-NEMO, levtype=o2d: the component of the field, nemo (250)") {
    run([] {
        auto mars = operRequest(262104, "o2d");
        mars.set("model", "IFS-NEMO");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 250);
        EXPECT_EQUAL(handle->getString("modelName"), "nemo");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "o2d");
    });
}

CASE("model=IFS-legacy keeps the legacy code 255, and the ocean diagnostics then go to o2d") {
    run([] {
        auto mars = operRequest(262104, "sfc");
        mars.set("model", "IFS-legacy");
        // 162 is cy50r2 in the legacy numbering
        const auto misc = miscWithGeneratingProcessIdentifier(162);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 255);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "o2d");
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 162);
        EXPECT_EQUAL(handle->getString("modelVersion"), "cy50r2");
    });
}

CASE("model=aifs-single is unchanged (1)") {
    run([] {
        auto mars = operRequest(167, "sfc");
        mars.set("class", "ai");
        mars.set("model", "aifs-single");
        const auto misc = miscWithGeneratingProcessIdentifier(1);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 1);
        EXPECT_EQUAL(handle->getString("modelName"), "aifs-single");
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 1);
    });
}

//----------------------------------------------------------------------------------------------------------------------
// generatingProcessIdentifier and the misc dictionary
//----------------------------------------------------------------------------------------------------------------------

CASE("generatingProcessIdentifier in the numbering of the component scheme: 2 is cy50r2") {
    run([] {
        const auto mars = operRequest(167, "sfc");
        const auto misc = miscWithGeneratingProcessIdentifier(2);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 2);
        EXPECT_EQUAL(handle->getString("modelVersion"), "cy50r2");
    });
}

CASE("generatingProcessIdentifier in the legacy numbering is written as given, not translated") {
    run([] {
        const auto mars = operRequest(167, "sfc");
        // 162 is cy50r2 in the legacy numbering only: it is not a cycle of the component scheme
        const auto misc = miscWithGeneratingProcessIdentifier(162);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 162);
        EXPECT_EQUAL(handle->getString("modelVersion"), "unknown");
    });
}

CASE("a backgroundProcess in the misc dictionary (e.g. from an input header) is ignored") {
    run([] {
        const auto mars = operRequest(262104, "sfc");
        auto misc       = miscWithGeneratingProcessIdentifier(2);
        misc.set("backgroundProcess", 255L);

        const auto handle = encode(mars, misc);

        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 254);
        EXPECT_EQUAL(handle->getString("modelName"), "IFS");
        EXPECT_EQUAL(handle->getString("mars.levtype"), "sfc");
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
