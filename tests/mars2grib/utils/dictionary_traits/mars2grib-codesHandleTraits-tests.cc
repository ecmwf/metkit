#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


CASE("CodesHandle: has vs typed has") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");

    using metkit::mars2grib::utils::dict_traits::has;

    // tablesVersionLatest è long scalar
    EXPECT(has(*h, "tablesVersionLatest"));
    EXPECT(has<long>(*h, "tablesVersionLatest"));
    EXPECT(!has<double>(*h, "tablesVersionLatest"));
}

CASE("CodesHandle: get_or_throw type mismatch throws") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    EXPECT_THROWS(get_or_throw<double>(*h, "tablesVersionLatest"));
    EXPECT_THROWS(get_or_throw<std::string>(*h, "tablesVersionLatest"));
}

CASE("CodesHandle: get_opt type mismatch returns nullopt") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");
    using metkit::mars2grib::utils::dict_traits::get_opt;

    auto v = get_opt<double>(*h, "tablesVersionLatest");
    EXPECT(!v.has_value());
}

CASE("CodesHandle: bool semantics") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    // set_or_throw<long>(*h, "myBool", 2L);
    bool xxx = get_or_throw<bool>(*h, "localUsePresent");
    EXPECT(!xxx);

    set_or_throw<long>(*h, "subCentre", 1L);
    // (*h).set( "subCentre", 0L );
    std::cout << "After set subCentre" << get_or_throw<bool>(*h, "subCentre") << std::endl;
    // set_or_throw<bool>(*h, "setLocalDefinition", true );
    // EXPECT(!get_or_throw<bool>(*h, "myBool"));
}

CASE("CodesHandle: scalar vs vector distinction") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");
    using metkit::mars2grib::utils::dict_traits::has;

    EXPECT(has<long>(*h, "tablesVersionLatest"));
    // EXPECT(!has<std::vector<long>>(*h, "tablesVersionLatest"));
}

CASE("CodesHandle: missing") {

    auto h = metkit::codes::codesHandleFromSample("GRIB2");
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::isMissing;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;

    set_or_throw<long>(*h, "productDefinitionTemplateNumber", 0L);
    setMissing_or_throw(*h, "scaleFactorOfFirstFixedSurface");

    EXPECT(isMissing(*h, "scaleFactorOfFirstFixedSurface"));
    // EXPECT(!get_opt<long>(*h, "myMissing").has_value());
    // EXPECT_THROWS(get_or_throw<long>(*h, "myMissing"));
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
