/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file SpecializedEncoder.h
 * @brief Fully specialized, concept-driven GRIB encoder.
 *
 * This header defines `SpecializedEncoder`, a **fully configured GRIB encoder**
 * specialized on:
 * - the concrete dictionary types
 * - a fixed encoder configuration
 * - a frozen concept/section dispatch table
 *
 * The encoder executes the complete GRIB encoding pipeline by iterating over:
 * - encoding stages
 * - GRIB sections
 * - concept setter callbacks
 *
 * All dispatch decisions (concept applicability, ordering, section mapping)
 * are resolved **before runtime** when the encoder is constructed.
 *
 * At runtime, encoding is reduced to a sequence of direct function calls.
 *
 * @ingroup mars2grib_backend_encoder
 */
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
#include "metkit/mars2grib/backend/concepts/conceptRegistry.h"
#include "metkit/mars2grib/backend/encoderConfiguration.h"
#include "metkit/mars2grib/backend/sections/initializers/sectionRegistry.h"

namespace metkit::mars2grib::backend {

using metkit::mars2grib::backend::concepts_::Fn;
using metkit::mars2grib::backend::concepts_::NUM_SECTIONS;
using metkit::mars2grib::backend::concepts_::NUM_STAGES;
using metkit::mars2grib::backend::config::EncoderCfg;
using metkit::mars2grib::backend::config::encoderConfiguration_to_json;
using metkit::mars2grib::backend::config::makeEncoderCallbacks;
using metkit::mars2grib::backend::config::makeEncoderConfiguration;


/**
 * @brief Fully specialized, dictionary-typed GRIB encoder.
 *
 * `SpecializedEncoder` represents the final execution form of the mars2grib
 * backend encoder.
 *
 * Characteristics:
 * - fully templated on all dictionary types
 * - immutable configuration
 * - no dynamic dispatch during encoding
 * - exception-safe with structured diagnostic reporting
 *
 * The encoder owns a **Concept Setters Table** that defines, for each:
 * - encoding stage
 * - GRIB section
 * the ordered list of concept setter callbacks to execute.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the output GRIB dictionary
 */
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
class SpecializedEncoder {
public:

    /// Type of a single concept setter callback
    using Setter_t = Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>;

    /**
     * @brief Concept setters dispatch table.
     *
     * Indexed as:
     *   [stage][section] -> vector of setter callbacks
     *
     * The table is fully constructed during encoder initialization
     * and never modified afterwards.
     */
    using ConceptSettersTable = std::array<std::array<std::vector<Setter_t>, NUM_SECTIONS>,  // sections
                                           NUM_STAGES + 1                                    // stages
                                           >;

public:

    /**
     * @brief Construct a specialized encoder from a resolved configuration.
     *
     * The constructor builds the complete concept dispatch table using
     * the provided encoder configuration.
     *
     * @param cfg Fully resolved encoder configuration
     */
    explicit SpecializedEncoder(const config::EncoderCfg& cfg) :
        cfg_{cfg}, settersTable_{makeEncoderCallbacks<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>(cfg)} {}

    explicit SpecializedEncoder(const eckit::LocalConfiguration& cfg) :
        SpecializedEncoder<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>(makeEncoderConfiguration(cfg)) {}

    SpecializedEncoder(const SpecializedEncoder&)            = delete;
    SpecializedEncoder& operator=(const SpecializedEncoder&) = delete;

    SpecializedEncoder(SpecializedEncoder&&)            = delete;
    SpecializedEncoder& operator=(SpecializedEncoder&&) = delete;

    ~SpecializedEncoder() = default;

    /**
     * @brief Encode a complete GRIB message (all stages).
     *
     * This method executes the full encoding pipeline:
     * - creates an initial GRIB2 sample
     * - iterates over all encoding stages
     * - applies section initializers and concept setters
     * - forces materialization between stages
     *
     * The returned dictionary represents a fully populated GRIB message.
     *
     * @param mars MARS dictionary
     * @param geo  Geometry dictionary
     * @param par  Parameter dictionary
     * @param opt  Options dictionary
     *
     * @return Owning pointer to the encoded GRIB dictionary
     *
     * @throws Mars2GribEncoderException
     *         On any failure during encoding, enriched with:
     *         - input dictionaries (JSON)
     *         - encoder configuration (JSON)
     */
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

    /// Specialized encoder configuration
    const config::EncoderCfg cfg_;

    /// Fully resolved concept dispatch table
    const ConceptSettersTable settersTable_;
};


}  // namespace metkit::mars2grib::backend
