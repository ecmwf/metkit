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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

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
template <class Cntx_t>
inline std::string enum2name_ShapeOfTheReferenceSystem_or_throw(ShapeOfTheReferenceSystem value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6367470:
            {
                std::string result = "earth-spherical-radius-6367470";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified:
            {
                std::string result = "earth-spherical-radius-specified";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthOblateIAU1965:
            {
                std::string result = "earth-oblate-iau-1965";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified:
            {
                std::string result = "earth-oblate-axes-km-specified";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthOblateIAGGRS80:
            {
                std::string result = "earth-oblate-iag-grs80";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthWGS84:
            {
                std::string result = "earth-wgs84";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6371229:
            {
                std::string result = "earth-spherical-radius-6371229";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified:
            {
                std::string result = "earth-oblate-axes-m-specified";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum:
            {
                std::string result = "earth-spherical-radius-6371200-wgs84-datum";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830:
            {
                std::string result = "earth-osgb1936-airy1830";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic:
            {
                std::string result = "earth-wgs84-corrected-geomagnetic";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::SunSphericalStonyhurst:
            {
                std::string result = "sun-spherical-stonyhurst";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ShapeOfTheReferenceSystem::Missing:
            {
                std::string result = "missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
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
template <class Cntx_t>
inline ShapeOfTheReferenceSystem name2enum_ShapeOfTheReferenceSystem_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "earth-spherical-radius-6367470") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6367470;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-spherical-radius-specified") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-oblate-iau-1965") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateIAU1965;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-oblate-axes-km-specified") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-oblate-iag-grs80") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateIAGGRS80;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-wgs84") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthWGS84;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-spherical-radius-6371229") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-oblate-axes-m-specified") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-spherical-radius-6371200-wgs84-datum") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-osgb1936-airy1830") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "earth-wgs84-corrected-geomagnetic") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "sun-spherical-stonyhurst") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::SunSphericalStonyhurst;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    else if (name == "missing") {
        {
            ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
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
template <class Cntx_t>
inline ShapeOfTheReferenceSystem long2enum_ShapeOfTheReferenceSystem_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6367470;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadiusSpecified;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateIAU1965;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateAxesKmSpecified;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateIAGGRS80;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthWGS84;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 6:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6371229;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 7:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOblateAxesMetersSpecified;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 8:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthSphericalRadius6371200WGS84Datum;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 9:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthOSGB1936Airy1830;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 10:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::EarthWGS84CorrectedGeomagnetic;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 11:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::SunSphericalStonyhurst;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                ShapeOfTheReferenceSystem result = ShapeOfTheReferenceSystem::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid ShapeOfTheReferenceSystem numeric value: actual='" +
                                              std::to_string(value) + "', expected={0..11,255}",
                                          Here());
    }
}

}  // namespace metkit::mars2grib::backend::tables
