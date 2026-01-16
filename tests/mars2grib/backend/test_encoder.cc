#include <iostream>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/backend/LocalConfigurationFrozenEncoder.h"
#include "metkit/mars2grib/utils/encoder_cfg.h"

int main() {

    std::cout << "Testing LocalConfigurationFrozenEncoder instantiation..." << std::endl;
    metkit::mars2grib::utils::EncoderCfg cfg = {
        {{{0, {{"nil", "default"}}},
          {0, {{"origin", "default"}, {"tables", "default"}, {"forecastTime", "default"}, {"dataType", "default"}}},
          {1, {{"mars", "default"}, {"longrange", "default"}}},
          {0, {{"shapeOfTheEarth", "default"}, {"representation", "latlon"}}},
          {0,
           {{"generatingProcess", "default"},
            {"forecastTime", "default"},
            {"pointInTime", "default"},
            {"level", "default"},
            {"param", "default"}}},
          {0, {{"packing", "simple"}}}}}};

    // Print configuration
    std::cout << "================================================================================================="
              << std::endl;
    int secId = 0;
    for (const auto& sec : cfg.sec_) {
        std::cout << "Section " << secId << "." << sec.templateNumber_ << ":" << std::endl;
        for (const auto& concept : sec.concepts_) {
            std::cout << "Section " << secId << "." << sec.templateNumber_ << ": Concept " << concept.first << " / "
                      << concept.second << std::endl;
        }
        ++secId;
    }
    std::cout << "================================================================================================="
              << std::endl
              << std::endl
              << std::endl
              << std::endl;


    metkit::mars2grib::backend::LocalConfigurationFrozenEncoder encoder(cfg);

    encoder.debug_print();


    const std::string yaml(R"json({
step: 12,
lat: 45.5,
flag: true,
name: test
})json");


    const eckit::YAMLConfiguration root(yaml);

    eckit::LocalConfiguration mars_dict(root);
    eckit::LocalConfiguration geo_dict(root);
    eckit::LocalConfiguration par_dict(root);
    eckit::LocalConfiguration opt_dict(root);

    auto handlePtr = metkit::codes::codesHandleFromSample("GRIB2");
    auto& out_dict = *handlePtr;


    encoder.encode(mars_dict, geo_dict, par_dict, opt_dict, out_dict);

    // Exit point
    return 0;
};
