#pragma once

#include <iostream>
#include <string>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/concept_core.h"
#include "metkit/mars2grib/backend/concepts/representation/representation_enum.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchGridDefinitionTemplateNumber.h"

// Exceptions
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// DEFAULT APPLICABILITY (user will override manually)
// ======================================================
template <std::size_t Stage, std::size_t Section, RepresentationType Variant>
constexpr bool representationApplicable() {

    bool condition1 = Stage == StageAllocate && Section == SecGridDefinitionSection;
    bool condition2 = Stage == StagePreset && Section == SecGridDefinitionSection;

    return condition1 || condition2;
}


// ======================================================
// MAIN OPERATION
// ======================================================
template <std::size_t Stage, std::size_t Section, RepresentationType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void RepresentationOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                      OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (representationApplicable<Stage, Section, Variant>()) {

        try {

            // =============================================================
            // Logging
            LOG_DEBUG_LIB(LibMetkit) << "[Concept Representation] Op called: "
                                     << "Stage=" << Stage << ", Section=" << Section
                                     << ", Variant=" << std::string(representationTypeName<Variant>()) << std::endl;


            // Allocate data representation
            if constexpr (Stage == StageAllocate) {

                if constexpr (Variant == RepresentationType::Latlon) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {0});

                    // Allocate the grib header
                    set_or_throw<std::string>(out, "gridType", "regular_ll");
                }
                else if constexpr (Variant == RepresentationType::RegularGaussian) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {40});

                    // Allocate the grib header
                    set_or_throw<std::string>(out, "gridType", "regular_gg");
                }
                else if constexpr (Variant == RepresentationType::ReducedGaussian) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {40});

                    // Retrive information from geo dict
                    std::vector<long> PlArray = get_or_throw<std::vector<long>>(geo, "pl");

                    long numberOfParallelsBetweenAPoleAndTheEquator =
                        get_or_throw<long>(geo, "numberOfParallelsBetweenAPoleAndTheEquator");

                    // Allocate the grib header
                    set_or_throw<std::string>(out, "gridType", "reduced_gg");
                    set_or_throw<long>(out, "interpretationOfNumberOfPoints", 1L);
                    set_or_throw<long>(out, "numberOfParallelsBetweenAPoleAndTheEquator",
                                       numberOfParallelsBetweenAPoleAndTheEquator);
                    set_or_throw<std::vector<long>>(out, "pl", PlArray);
                }
                else if constexpr (Variant == RepresentationType::SphericalHarmonics) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {50});

                    // Allocate the header
                    set_or_throw<std::string>(out, "gridType", "sh");
                }
                else if constexpr (Variant == RepresentationType::Healpix) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {150});

                    // Allocate the header
                    set_or_throw<std::string>(out, "gridType", "healpix");
                }
                else if constexpr (Variant == RepresentationType::Orca) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {101});

                    // throw exceptions
                    throw Mars2GribConceptException(std::string(representationName),
                                                    std::string(representationTypeName<Variant>()),
                                                    std::to_string(Stage), std::to_string(Section),
                                                    "Support for Orca representation not implemented", Here());
                }
                else if constexpr (Variant == RepresentationType::Fesom) {

                    // Checks
                    checks::matchGridDefinitionTemplateNumber_or_throw(opt, out, {101});

                    // throw exceptions
                    throw Mars2GribConceptException(std::string(representationName),
                                                    std::string(representationTypeName<Variant>()),
                                                    std::to_string(Stage), std::to_string(Section),
                                                    "Support for Fesom representation not implemented", Here());
                }
                else {
                    // throw exceptions
                    throw Mars2GribConceptException(
                        std::string(representationName), std::string(representationTypeName<Variant>()),
                        std::to_string(Stage), std::to_string(Section), "Unknown `representation` concept...", Here());
                }
            }


            // Preset data representation
            if constexpr (Stage == StagePreset) {
                if constexpr (Variant == RepresentationType::Latlon) {
                    // Retrive information from geo dict
                    long Ni = get_or_throw<long>(geo, "numberOfPointsAlongAParallel");
                    long Nj = get_or_throw<long>(geo, "numberOfPointsAlongAMeridian");
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    long iDirectionIncrementInDegrees = get_or_throw<long>(geo, "iDirectionIncrementInDegrees");
                    long jDirectionIncrementInDegrees = get_or_throw<long>(geo, "jDirectionIncrementInDegrees");

                    // Configure the grib header
                    set_or_throw<long>(out, "Ni", Ni);
                    set_or_throw<long>(out, "Nj", Nj);
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    set_or_throw<long>(out, "iDirectionIncrementInDegrees", iDirectionIncrementInDegrees);
                    set_or_throw<long>(out, "jDirectionIncrementInDegrees", jDirectionIncrementInDegrees);
                }
                else if constexpr (Variant == RepresentationType::RegularGaussian) {
                    // Retrive information from geo dict
                    const auto truncateDegrees = get_opt<long>(geo, "truncateDegrees").value_or(0);
                    // long numberOfPointsAlongAMeridian = get_or_throw<long>( geo, "numberOfPointsAlongAMeridian" ); //
                    // TODO (knobel) long numberOfPointsAlongAParallel = get_or_throw<long>( geo,
                    // "numberOfPointsAlongAParallel" );  // TODO (knobel)
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    long numberOfParallelsBetweenAPoleAndTheEquator =
                        get_or_throw<long>(geo, "numberOfParallelsBetweenAPoleAndTheEquator");
                    long iDirectionIncrementInDegrees = get_or_throw<long>(geo, "iDirectionIncrementInDegrees");

                    // Configure grib header
                    set_or_throw(out, "truncateDegrees", truncateDegrees);
                    // set_or_throw<long>( out, "numberOfPointsAlongAMeridian", numberOfPointsAlongAMeridian );  // TODO
                    // (knobel) set_or_throw<long>( out, "numberOfPointsAlongAParallel", numberOfPointsAlongAParallel );
                    // // TODO (knobel)
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    set_or_throw<long>(out, "iDirectionIncrementInDegrees", iDirectionIncrementInDegrees);
                }
                else if constexpr (Variant == RepresentationType::ReducedGaussian) {
                    // Retrive information from geo dict
                    const auto truncateDegrees = get_opt<long>(geo, "truncateDegrees").value_or(0);
                    // long numberOfPointsAlongAMeridian = get_or_throw<long>( geo, "numberOfPointsAlongAMeridian" ); //
                    // TODO (knobel)
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    // long numberOfParallelsBetweenAPoleAndTheEquator = get_or_throw<long>( geo,
                    // "numberOfParallelsBetweenAPoleAndTheEquator" );

                    // Configure grib header
                    set_or_throw<long>(out, "truncateDegrees", truncateDegrees);
                    // set_or_throw<long>( out, "numberOfPointsAlongAMeridian", numberOfPointsAlongAMeridian );  // TODO
                    // (knobel)
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    // set_or_throw<long>( out, "numberOfParallelsBetweenAPoleAndTheEquator",
                    // numberOfParallelsBetweenAPoleAndTheEquator );
                    setMissing_or_throw(out, "iDirectionIncrement");
                }
                else if constexpr (Variant == RepresentationType::SphericalHarmonics) {
                    // Retrive information from geo dict
                    long pentagonalResolutionParameterJ = get_or_throw<long>(geo, "pentagonalResolutionParameterJ");
                    long pentagonalResolutionParameterK = get_or_throw<long>(geo, "pentagonalResolutionParameterK");
                    long pentagonalResolutionParameterM = get_or_throw<long>(geo, "pentagonalResolutionParameterM");

                    // Configure grib header
                    set_or_throw<long>(out, "pentagonalResolutionParameterJ", pentagonalResolutionParameterJ);
                    set_or_throw<long>(out, "pentagonalResolutionParameterK", pentagonalResolutionParameterK);
                    set_or_throw<long>(out, "pentagonalResolutionParameterM", pentagonalResolutionParameterM);
                }
                else if constexpr (Variant == RepresentationType::Healpix) {
                    // Retrive information from geo dict
                    long nside              = get_or_throw<long>(geo, "nside");
                    long orderingConvention = get_or_throw<long>(geo, "orderingConvention");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");

                    // Configure grib header
                    set_or_throw<long>(out, "nside", nside);
                    set_or_throw<long>(out, "orderingConvention", orderingConvention);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                }
                else if constexpr (Variant == RepresentationType::Orca) {
                    // throw exceptions
                    throw Mars2GribConceptException(std::string(representationName),
                                                    std::string(representationTypeName<Variant>()),
                                                    std::to_string(Stage), std::to_string(Section),
                                                    "Support for Orca representation not implemented", Here());
                }
                else if constexpr (Variant == RepresentationType::Fesom) {
                    // throw exceptions
                    throw Mars2GribConceptException(std::string(representationName),
                                                    std::string(representationTypeName<Variant>()),
                                                    std::to_string(Stage), std::to_string(Section),
                                                    "Support for Fesom representation not implemented", Here());
                }
                else {
                    // throw exceptions
                    throw Mars2GribConceptException(
                        std::string(representationName), std::string(representationTypeName<Variant>()),
                        std::to_string(Stage), std::to_string(Section), "Unknown `representation` concept...", Here());
                };
            };
        }
        catch (...) {
            // Rethrow nested exceptions
            std::throw_with_nested(Mars2GribConceptException(
                std::string(representationName), std::string(representationTypeName<Variant>()), std::to_string(Stage),
                std::to_string(Section), "Unable to set `ensemble` concept...", Here()));
        }

        // Successful operation
        return;

    }  // if constexpr ( longrangeApplicable(Stage, Section, Variant) )

    // Paranoid check. Should never arrive here
    throw Mars2GribConceptException(std::string(representationName), std::string(representationTypeName<Variant>()),
                                    std::to_string(Stage), std::to_string(Section),
                                    "Concept called when not applicable...", Here());

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::cnpts
