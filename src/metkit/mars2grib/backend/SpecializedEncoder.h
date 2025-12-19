#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

// Header only helpers for frozen encoder
#include "metkit/mars2grib/backend/concepts/concept_registry.h"
#include "metkit/mars2grib/backend/encoderConfiguration.h"
#include "metkit/mars2grib/backend/sections/initializers/section_registry.h"

namespace metkit::mars2grib::backend {

using metkit::mars2grib::backend::cnpts::Fn;
using metkit::mars2grib::backend::cnpts::NUM_SECTIONS;
using metkit::mars2grib::backend::cnpts::NUM_STAGES;
using metkit::mars2grib::backend::config::EncoderCfg;
using metkit::mars2grib::backend::config::encoderConfiguration_to_json;
using metkit::mars2grib::backend::config::makeEncoderCallbacks;
using metkit::mars2grib::backend::config::makeEncoderConfiguration;


// This is an generic encoder fully templated on all the dictionaries
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
class SpecializedEncoder {
public:

    // Definition of a callback
    using Setter_t = Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;

    // Definition of callbacks container
    using ConceptSettersTable = std::array<std::array<std::vector<Setter_t>, NUM_SECTIONS>,  // sections
                                           NUM_STAGES + 1                                    // stages
                                           >;

public:


    explicit SpecializedEncoder(const config::EncoderCfg& cfg) :
        cfg_{cfg}, settersTable_{makeEncoderCallbacks<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>(cfg)} {}

    explicit SpecializedEncoder(const eckit::LocalConfiguration& cfg) :
        SpecializedEncoder<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>(makeEncoderConfiguration(cfg)) {}

    SpecializedEncoder(const SpecializedEncoder&)            = delete;
    SpecializedEncoder& operator=(const SpecializedEncoder&) = delete;

    SpecializedEncoder(SpecializedEncoder&&)            = delete;
    SpecializedEncoder& operator=(SpecializedEncoder&&) = delete;

    ~SpecializedEncoder() = default;

    // =================================================================
    // Encode ALL STAGES
    // =================================================================
    std::unique_ptr<OutDict_t> encode(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par,
                                      const OptDict_t& opt) const {

        using metkit::mars2grib::backend::config::encoderConfiguration_to_json;
        using metkit::mars2grib::utils::dict_traits::clone_or_throw;
        using metkit::mars2grib::utils::dict_traits::dict_to_json;
        using metkit::mars2grib::utils::dict_traits::make_from_sample_or_throw;
        using metkit::mars2grib::utils::exceptions::Mars2GribEncoderException;
        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;


        try {


            LOG_DEBUG_LIB(LibMetkit) << std::endl
                                     << std::endl
                                     << std::endl
                                     << std::endl
                                     << std::endl
                                     << std::endl
                                     << std::endl
                                     << std::endl;
            LOG_DEBUG_LIB(LibMetkit)
                << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                << std::endl;

            // Create an initial sample
            std::unique_ptr<OutDict_t> samplePtr = make_from_sample_or_throw<OutDict_t>("GRIB2");
            for (const auto& stage : settersTable_) {
                LOG_DEBUG_LIB(LibMetkit) << std::endl << std::endl << std::endl;
                LOG_DEBUG_LIB(LibMetkit) << "STAGE :: ****************************************************************"
                                         << std::endl;
                for (const auto& section : stage) {
                    LOG_DEBUG_LIB(LibMetkit) << std::endl << std::endl << std::endl;
                    LOG_DEBUG_LIB(LibMetkit)
                        << "  SECTION :: ****************************************************************" << std::endl;
                    for (const auto& conceptSetter : section) {
                        LOG_DEBUG_LIB(LibMetkit) << std::endl << std::endl << std::endl;
                        LOG_DEBUG_LIB(LibMetkit)
                            << "      CONCEPT :: ****************************************************************"
                            << std::endl;
                        if (conceptSetter != nullptr) {
                            conceptSetter(mars, geo, par, opt, *samplePtr);
                        }
                        // long isValid = samplePtr->getLong( "isMessageValid" );
                        // if ( isValid == 0 ) {
                        //     throw Mars2GribGenericException(
                        //       "isMessageValid after conceptSetter",
                        //       Here()
                        //     );
                        // }
                        // samplePtr = clone_or_throw<OutDict_t>( *samplePtr );
                    }
                    // long isValid = samplePtr->getLong( "isMessageValid" );
                    // if ( isValid == 0 ) {
                    //     throw Mars2GribGenericException(
                    //       "isMessageValid after conceptSetter",
                    //       Here()
                    //     );
                    // }
                    // samplePtr = clone_or_throw<OutDict_t>( *samplePtr );
                }
                // long isValid = samplePtr->getLong( "isMessageValid" );
                // if ( isValid == 0 ) {
                //     throw Mars2GribGenericException(
                //       "isMessageValid after conceptSetter",
                //       Here()
                //     );
                // }

                // The clone is required to force materialization and commit
                // in-memory modifications that may still be deferred due to
                // internal ecCodes optimizations.
                LOG_DEBUG_LIB(LibMetkit) << std::endl << std::endl << std::endl;
                LOG_DEBUG_LIB(LibMetkit) << "End of stage, cloning the sample to force materialization." << std::endl;
                samplePtr = clone_or_throw<OutDict_t>(*samplePtr);
            }
            // long isValid = samplePtr->getLong( "isMessageValid" );
            // if ( isValid == 0 ) {
            //     throw Mars2GribGenericException(
            //       "isMessageValid after conceptSetter",
            //       Here()
            //     );
            // }

            return samplePtr;
        }
        catch (...) {

            std::string json = encoderConfiguration_to_json(cfg_);

            LOG_DEBUG_LIB(LibMetkit) << "=========================================================================="
                                     << std::endl;
            LOG_DEBUG_LIB(LibMetkit) << "FAILED CONFIGURATION" << std::endl;
            LOG_DEBUG_LIB(LibMetkit) << json << std::endl;
            LOG_DEBUG_LIB(LibMetkit) << "=========================================================================="
                                     << std::endl;

            std::throw_with_nested(Mars2GribEncoderException("Error during SpecializedEncoder::encode",
                                                             dict_to_json<MarsDict_t>(mars),      // marsDict_json_
                                                             dict_to_json<GeoDict_t>(geo),        // geoDict_json_
                                                             dict_to_json<ParDict_t>(par),        // parDict_json_
                                                             dict_to_json<OptDict_t>(opt),        // optDict_json_
                                                             encoderConfiguration_to_json(cfg_),  // encoderCfg_json_
                                                             Here()));
        }
    }

private:

    const config::EncoderCfg cfg_;
    const ConceptSettersTable settersTable_;
};


}  // namespace metkit::mars2grib::backend
