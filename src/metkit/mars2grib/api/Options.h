/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file Options.h
 * @brief Configuration options for the Mars2Grib encoding API.
 *
 * This header defines the public configuration structure used to
 * control the behavior of the **Mars2Grib encoder**.
 *
 * The `Options` structure is part of the **user-facing API** and is
 * intentionally simple, explicit, and stable. Each option enables or
 * disables a well-defined aspect of the encoding process.
 *
 * Options:
 * - do NOT change the semantic meaning of the input data
 * - do NOT introduce implicit defaults in metadata
 * - only affect validation, override behavior, or encoding strategies
 *
 * Options can be:
 * - constructed programmatically
 * - passed directly to the `Mars2Grib` constructor
 * - populated from an `eckit::LocalConfiguration`
 *
 * @ingroup mars2grib_api
 */
#pragma once

namespace metkit::mars2grib {

/**
 * @brief Encoding options for the Mars2Grib API.
 *
 * This structure controls optional behaviors of the GRIB encoding
 * process. All options are **opt-in** and have conservative defaults
 * to preserve backward compatibility and predictable behavior.
 *
 * The default-constructed `Options` object corresponds to the
 * standard mars2grib encoding behavior.
 */
struct Options {

    /**
     * @brief Enable or disable input validation checks.
     *
     * When enabled, the encoder performs consistency and validity
     * checks at selected critical points during the encoding phase.
     *
     * Disabling this option may improve performance but can result
     * in failures that are harder to diagnose in the presence of
     * malformed or inconsistent input.
     *
     * @default true
     */
    bool applyChecks = true;

    /**
     * @brief Enable metadata override semantics.
     *
     * When enabled, values provided through parameter dictionary are
     * allowed to override values resolved from the MARS dictionary.
     *
     * When disabled, conflicting overrides result in an error.
     *
     * @default false
     */
    bool enableOverride = false;

    /**
     * @brief Enable bits-per-value compression.
     *
     * When enabled, the encoder is allowed to select a bits-per-value
     * packing strategy to reduce message size.
     *
     * This option affects only the **encoding strategy** and does not
     * alter the numerical values of the field.
     *
     * @default false
     */
    bool enableBitsPerValueCompression = false;


    /**
     * @brief Enable semantic normalization of the MARS dictionary.
     *
     * When active, the MARS request is sanitized against the library
     * language definition to ensure key-value consistency and
     * case-insensitivity before resolution.
     *
     * @default false
     */
    bool sanitizeMars = false;

    /**
     * @brief Enable semantic normalization of the auxiliary metadata.
     *
     * When active, the auxiliary (Misc) dictionary is sanitized against
     * the library language definition. This is recommended when
     * parameters are provided as raw strings.
     *
     * @default false
     */
    bool sanitizeMisc = false;

    /**
     * @brief Automatically normalize MARS 'grid' syntax.
     *
     * If enabled, the encoder detects and converts legacy MARS grid
     * specifications (e.g., 'L640x320') into standard GRIB-compliant
     * increment strings ('deltaLon/deltaLat').
     *
     * This is a **procedural normalization** that ensures resolution
     * compatibility for Gaussian or reduced grids.
     *
     * @default true
     */
    bool fixMarsGrid = true;

};

}  // namespace metkit::mars2grib
