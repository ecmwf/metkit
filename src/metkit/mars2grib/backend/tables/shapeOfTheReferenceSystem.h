/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief Shape of the reference system.
///
/// This enumeration represents GRIB code values defining the geometric
/// model of the Earth (or Sun) and the associated reference system.
///
/// The numeric values map **directly** to ecCodes GRIB table 3.2
/// and must not be changed manually.
///
/// @note
/// The value `255` corresponds to the GRIB *missing* value.
///
/// @important
/// This enum is a **GRIB-table representation only**.
/// No policy, defaulting, or deduction logic belongs here.
///
/// @todo [owner: mival,dgov][scope: tables][reason: correctness][prio: medium]
/// - Generate this enum and all conversion helpers automatically from
/// ecCodes definitions at build time.
///
enum class ShapeOfTheReferenceSystem : long {
    EarthSphericalRadius6367470           = 0,
    EarthSphericalRadiusSpecified         = 1,
    EarthOblateIAU1965                    = 2,
    EarthOblateAxesKmSpecified            = 3,
    EarthOblateIAGGRS80                   = 4,
    EarthWGS84                            = 5,
    EarthSphericalRadius6371229           = 6,
    EarthOblateAxesMetersSpecified        = 7,
    EarthSphericalRadius6371200WGS84Datum = 8,
    EarthOSGB1936Airy1830                 = 9,
    EarthWGS84CorrectedGeomagnetic        = 10,
    SunSphericalStonyhurst                = 11,
    Missing                               = 255
};

///
/// @brief Convert `ShapeOfTheReferenceSystem` to its canonical name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical string name
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is invalid.
///
inline std::string enum2name_ShapeOfTheReferenceSystem_or_throw(ShapeOfTheReferenceSystem value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6367470:
            return "earth-spherical-radius-6367470";
        case ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified:
            return "earth-spherical-radius-specified";
        case ShapeOfTheReferenceSystem::EarthOblateIAU1965:
            return "earth-oblate-iau-1965";
        case ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified:
            return "earth-oblate-axes-km-specified";
        case ShapeOfTheReferenceSystem::EarthOblateIAGGRS80:
            return "earth-oblate-iag-grs80";
        case ShapeOfTheReferenceSystem::EarthWGS84:
            return "earth-wgs84";
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6371229:
            return "earth-spherical-radius-6371229";
        case ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified:
            return "earth-oblate-axes-m-specified";
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum:
            return "earth-spherical-radius-6371200-wgs84-datum";
        case ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830:
            return "earth-osgb1936-airy1830";
        case ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic:
            return "earth-wgs84-corrected-geomagnetic";
        case ShapeOfTheReferenceSystem::SunSphericalStonyhurst:
            return "sun-spherical-stonyhurst";
        case ShapeOfTheReferenceSystem::Missing:
            return "missing";
        default:
            throw Mars2GribTableException("Invalid ShapeOfTheReferenceSystem enum value", Here());
    }
}


///
/// @brief Convert a canonical name to `ShapeOfTheReferenceSystem`.
///
/// @param[in] name Canonical string name
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the name is not recognized.
///
inline ShapeOfTheReferenceSystem name2enum_ShapeOfTheReferenceSystem_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "earth-spherical-radius-6367470")
        return ShapeOfTheReferenceSystem::EarthSphericalRadius6367470;
    else if (name == "earth-spherical-radius-specified")
        return ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified;
    else if (name == "earth-oblate-iau-1965")
        return ShapeOfTheReferenceSystem::EarthOblateIAU1965;
    else if (name == "earth-oblate-axes-km-specified")
        return ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified;
    else if (name == "earth-oblate-iag-grs80")
        return ShapeOfTheReferenceSystem::EarthOblateIAGGRS80;
    else if (name == "earth-wgs84")
        return ShapeOfTheReferenceSystem::EarthWGS84;
    else if (name == "earth-spherical-radius-6371229")
        return ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;
    else if (name == "earth-oblate-axes-m-specified")
        return ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified;
    else if (name == "earth-spherical-radius-6371200-wgs84-datum")
        return ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum;
    else if (name == "earth-osgb1936-airy1830")
        return ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830;
    else if (name == "earth-wgs84-corrected-geomagnetic")
        return ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic;
    else if (name == "sun-spherical-stonyhurst")
        return ShapeOfTheReferenceSystem::SunSphericalStonyhurst;
    else if (name == "missing")
        return ShapeOfTheReferenceSystem::Missing;
    else
        throw Mars2GribTableException("Invalid ShapeOfTheReferenceSystem name: '" + name + "'", Here());
}


///
/// @brief Convert a numeric GRIB code to `ShapeOfTheReferenceSystem`.
///
/// @param[in] value Numeric GRIB code
///
/// @return Corresponding enumeration value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the value is not defined in the GRIB table.
///
inline ShapeOfTheReferenceSystem long2enum_ShapeOfTheReferenceSystem_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return ShapeOfTheReferenceSystem::EarthSphericalRadius6367470;
        case 1:
            return ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified;
        case 2:
            return ShapeOfTheReferenceSystem::EarthOblateIAU1965;
        case 3:
            return ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified;
        case 4:
            return ShapeOfTheReferenceSystem::EarthOblateIAGGRS80;
        case 5:
            return ShapeOfTheReferenceSystem::EarthWGS84;
        case 6:
            return ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;
        case 7:
            return ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified;
        case 8:
            return ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum;
        case 9:
            return ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830;
        case 10:
            return ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic;
        case 11:
            return ShapeOfTheReferenceSystem::SunSphericalStonyhurst;
        case 255:
            return ShapeOfTheReferenceSystem::Missing;
        default:
            throw Mars2GribTableException("Invalid ShapeOfTheReferenceSystem numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..11,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables
