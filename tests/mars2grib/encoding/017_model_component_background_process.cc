/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// backgroundProcess per component of the coupled system (ecCodes ECC-2354), deduced from the MARS keys:
/// the IFS ocean diagnostics (t20d, sc300m, pt300m) on levtype=sfc carry 254 and stay on `sfc`, the native
/// ocean fields on levtype=o2d carry 250 (nemo) and stay on `o2d`, wave fields carry 248 (ecwam), gfas and
/// ldas follow the stream, class mc is IFS-compo (252). The legacy cycle code of
/// generatingProcessIdentifier is translated to the numbering of the component scheme.

#include <exception>
#include <string>
#include <vector>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

// An od oper forecast field at step 6, as mars2mars hands it to the encoder
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

// The legacy cycle code, as the GRIB1 input or the `ncycle` override gives it
eckit::LocalConfiguration legacyCycle(long generatingProcessIdentifier = 162) {
    eckit::LocalConfiguration misc;
    misc.set("generatingProcessIdentifier", generatingProcessIdentifier);
    return misc;
}

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

void checkComponent(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                    long backgroundProcess, const std::string& modelName, const std::string& levtype,
                    long generatingProcessIdentifier, const std::string& modelVersion) {
    auto encoder      = metkit::mars2grib::Mars2Grib();
    const auto handle = encoder.encode(std::vector<double>(200, 1.0), mars, misc);

    EXPECT_EQUAL(handle->getLong("backgroundProcess"), backgroundProcess);
    EXPECT_EQUAL(handle->getString("modelName"), modelName);
    EXPECT_EQUAL(handle->getString("mars.levtype"), levtype);
    EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), generatingProcessIdentifier);
    EXPECT_EQUAL(handle->getString("modelVersion"), modelVersion);
}

}  // namespace

CASE("IFS ocean diagnostics on levtype=sfc are IFS (254) and stay on sfc") {
    run([] {
        for (const long param : {262104L, 262118L, 262144L}) {  // t20d, sc300m, pt300m
            checkComponent(operRequest(param, "sfc"), legacyCycle(), 254, "IFS", "sfc", 2, "cy50r2");
        }
    });
}

CASE("native ocean fields on levtype=o2d are nemo (250) and stay on o2d") {
    run([] {
        for (const long param : {262104L, 262118L, 262144L}) {
            checkComponent(operRequest(param, "o2d"), legacyCycle(), 250, "nemo", "o2d", 2, "cy50r2");
        }
    });
}

CASE("wave fields are ecwam (248)") {
    run([] { checkComponent(operRequest(140229, "sfc"), legacyCycle(), 248, "ecwam", "sfc", 2, "cy50r2"); });
}

CASE("atmospheric fields are IFS (254)") {
    run([] { checkComponent(operRequest(167, "sfc"), legacyCycle(), 254, "IFS", "sfc", 2, "cy50r2"); });
}

CASE("the legacy cycle codes are translated: cy49r2, cy50r1, cy50r2") {
    run([] {
        checkComponent(operRequest(167, "sfc"), legacyCycle(159), 254, "IFS", "sfc", 0, "cy49r2");
        checkComponent(operRequest(167, "sfc"), legacyCycle(161), 254, "IFS", "sfc", 1, "unknown");
        checkComponent(operRequest(167, "sfc"), legacyCycle(162), 254, "IFS", "sfc", 2, "cy50r2");
    });
}

CASE("a cycle code already in the new numbering, or unknown, is kept") {
    run([] {
        checkComponent(operRequest(167, "sfc"), legacyCycle(2), 254, "IFS", "sfc", 2, "cy50r2");
        checkComponent(operRequest(167, "sfc"), legacyCycle(110), 254, "IFS", "sfc", 110, "unknown");
    });
}

CASE("model=IFS-NEMO resolves to the component of the field") {
    run([] {
        auto sfc = operRequest(262104, "sfc");
        sfc.set("model", "IFS-NEMO");
        checkComponent(sfc, legacyCycle(), 254, "IFS", "sfc", 2, "cy50r2");

        auto o2d = operRequest(262104, "o2d");
        o2d.set("model", "IFS-NEMO");
        checkComponent(o2d, legacyCycle(), 250, "nemo", "o2d", 2, "cy50r2");
    });
}

CASE("model=IFS-legacy keeps 255 and the legacy cycle code, and the ocean diagnostics go to o2d") {
    run([] {
        auto mars = operRequest(262104, "sfc");
        mars.set("model", "IFS-legacy");
        checkComponent(mars, legacyCycle(), 255, "IFS", "o2d", 162, "cy50r2");
    });
}

CASE("gfas and ldas follow the stream, atmospheric composition the class") {
    run([] {
        auto gfas = operRequest(167, "sfc");
        gfas.set("class", "mc");
        gfas.set("stream", "gfas");
        auto encoder = metkit::mars2grib::Mars2Grib();
        auto handle  = encoder.encode(std::vector<double>(200, 1.0), gfas, legacyCycle());
        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 144);
        EXPECT_EQUAL(handle->getString("modelName"), "gfas");

        auto ldas = operRequest(167, "sfc");
        ldas.set("stream", "ldas");
        handle = encoder.encode(std::vector<double>(200, 1.0), ldas, legacyCycle());
        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 246);
        EXPECT_EQUAL(handle->getString("modelName"), "ldas");
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 2);

        auto compo = operRequest(167, "sfc");
        compo.set("class", "mc");
        checkComponent(compo, legacyCycle(), 252, "IFS-compo", "sfc", 2, "cy50r2");
    });
}

CASE("a backgroundProcess in the misc dictionary (an input header) is ignored") {
    run([] {
        auto misc = legacyCycle(162);
        misc.set("backgroundProcess", 255L);
        checkComponent(operRequest(262104, "sfc"), misc, 254, "IFS", "sfc", 2, "cy50r2");
    });
}

CASE("an AIFS model is unchanged") {
    run([] {
        auto mars = operRequest(167, "sfc");
        mars.set("class", "ai");
        mars.set("model", "aifs-single");
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 1.0), mars, legacyCycle(1));
        EXPECT_EQUAL(handle->getLong("backgroundProcess"), 1);
        EXPECT_EQUAL(handle->getString("modelName"), "aifs-single");
        EXPECT_EQUAL(handle->getLong("generatingProcessIdentifier"), 1);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
