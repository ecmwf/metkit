/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cstdlib>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


namespace metkit::mars2grib::test {


static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    return value != nullptr && std::stol(value) != 0L;
}();


constexpr long number_of_real_coefficients(long T) {
    return (T + 1) * (T + 2);
}


constexpr long default_sub_set_truncation(long T) {
    return T >= 213 ? 20L : std::min(10L, T);
}


CASE("encoding") {
    struct test_t {
        std::string label;
        long J;   // truncation (J = K = M)
        long JS;  // subSetTruncation (JS = KS = MS)
    } tests[]{
        {"T1279/42", 1279, 42},  //
        {"T1279/20", 1279, 20},  //
        {"T1279", 1279, -1},     // (default subset)
        {"T20/20", 20, 20},      //
        {"T20/10", 20, 10},      //
        {"T20", 20, -1},         // (default subset)
        {"T1/1", 1, 1},          //
        {"T1", 1, -1},           // (default subset)
    };

    for (const auto& [label, J, JS] : tests) {
        SECTION(label) {
            // MARS
            eckit::LocalConfiguration mars;
            mars.set("class", "od");
            mars.set("stream", "oper");
            mars.set("type", "fc");
            mars.set("expver", "test");
            mars.set("packing", "complex");
            mars.set("param", 130);
            mars.set("levtype", "pl");
            mars.set("levelist", 1000);
            mars.set("date", 2026'09'09);
            mars.set("time", 00'00);
            mars.set("step", 0);

            const auto grid = std::string("T") + std::to_string(J);
            if (useGridSpec) {
                mars.set("grid", grid);
            }
            else {
                mars.set("truncation", J);
            }

            const auto S = JS >= 0 ? JS : default_sub_set_truncation(J);
            eckit::LocalConfiguration misc;
            misc.set("subSetTruncation", JS);


            // GRIB
            const std::vector<double> vals(number_of_real_coefficients(J), 273.15);
            const auto handle = JS >= 0 ? Mars2Grib().encode(vals, mars, misc) : Mars2Grib().encode(vals, mars);
            ASSERT(handle);

            if (useGridSpec) {
                EXPECT(handle->getString("gridSpec") == R"({"grid":")" + grid + R"("})");
            }

            EXPECT(handle->getLong("gridDefinitionTemplateNumber") == 50L);
            EXPECT(handle->getLong("dataRepresentationTemplateNumber") == 51L);

            EXPECT(handle->getLong("spectralType") == 1L);
            EXPECT(handle->getLong("spectralMode") == 1L);
            EXPECT(handle->getLong("bitsPerValue") == 16L);            // Default
            EXPECT(handle->getLong("unpackedSubsetPrecision") == 1L);  // IEEE 32-bit

            EXPECT(handle->getLong("J") == J);
            EXPECT(handle->getLong("K") == J);
            EXPECT(handle->getLong("M") == J);
            EXPECT(handle->getLong("JS") == S);
            EXPECT(handle->getLong("KS") == S);
            EXPECT(handle->getLong("MS") == S);

            EXPECT(handle->getLong("numberOfDataPoints") == number_of_real_coefficients(J));
            EXPECT(handle->getLong("TS") == number_of_real_coefficients(S));

            if (JS == J) {
                // no coefficients are packed, these are unused
                EXPECT(handle->getLong("referenceValue") == 0L);
                EXPECT(handle->getLong("binaryScaleFactor") == 0L);
                EXPECT(handle->getLong("decimalScaleFactor") == 0L);
                EXPECT(handle->getLong("laplacianScalingFactor") == 0L);
            }
        }
    }
}


}  // namespace metkit::mars2grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
