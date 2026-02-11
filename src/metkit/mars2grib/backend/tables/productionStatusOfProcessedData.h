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
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::tables {

///
/// @brief GRIB Production Status of Processed Data.
///
/// This enumeration represents GRIB Code Table values describing the
/// *production status* of the processed data, as encoded in the
/// Product Definition Section.
///
/// The values distinguish between operational, test, research,
/// reanalysis, and programme-specific data streams (e.g. TIGGE, S2S,
/// Copernicus, Destination Earth).
///
/// The numeric values map **directly** to GRIB code table entries and
/// must not be modified manually.
///
/// @section Source of truth
/// The authoritative definition of these values is maintained in
/// the GRIB code tables distributed with ecCodes.
///
/// @todo [owner: mival][scope: tables][reason: correctness][prio: high]
/// - Generate this enumeration and all associated mappings
/// automatically from ecCodes GRIB code tables at build time.
/// - This avoids silent divergence between the encoder and the
/// ecCodes version used at runtime.
///
enum class ProductionStatusOfProcessedData : long {
    OperationalProducts              = 0,
    OperationalTestProducts          = 1,
    ResearchProducts                 = 2,
    ReanalysisProducts               = 3,
    TiggeOperational                 = 4,
    TiggeTest                        = 5,
    S2SOperationalProducts           = 6,
    S2STestProducts                  = 7,
    UerraOperational                 = 8,
    UerraTest                        = 9,
    CopernicusRegionalReanalysis     = 10,
    CopernicusRegionalReanalysisTest = 11,
    DestinationEarth                 = 12,
    DestinationEarthTest             = 13,
    Missing                          = 255
};

///
/// @brief Convert `ProductionStatusOfProcessedData` to its symbolic name.
///
/// @param[in] value Enumeration value
///
/// @return Canonical symbolic name corresponding to the enum value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the enumeration value is not recognised
///
inline std::string enum2name_ProductionStatusOfProcessedData_or_throw(ProductionStatusOfProcessedData value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case ProductionStatusOfProcessedData::OperationalProducts:
            return "OperationalProducts";
        case ProductionStatusOfProcessedData::OperationalTestProducts:
            return "OperationalTestProducts";
        case ProductionStatusOfProcessedData::ResearchProducts:
            return "ResearchProducts";
        case ProductionStatusOfProcessedData::ReanalysisProducts:
            return "ReanalysisProducts";
        case ProductionStatusOfProcessedData::TiggeOperational:
            return "TiggeOperational";
        case ProductionStatusOfProcessedData::TiggeTest:
            return "TiggeTest";
        case ProductionStatusOfProcessedData::S2SOperationalProducts:
            return "S2SOperationalProducts";
        case ProductionStatusOfProcessedData::S2STestProducts:
            return "S2STestProducts";
        case ProductionStatusOfProcessedData::UerraOperational:
            return "UerraOperational";
        case ProductionStatusOfProcessedData::UerraTest:
            return "UerraTest";
        case ProductionStatusOfProcessedData::CopernicusRegionalReanalysis:
            return "CopernicusRegionalReanalysis";
        case ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest:
            return "CopernicusRegionalReanalysisTest";
        case ProductionStatusOfProcessedData::DestinationEarth:
            return "DestinationEarth";
        case ProductionStatusOfProcessedData::DestinationEarthTest:
            return "DestinationEarthTest";
        case ProductionStatusOfProcessedData::Missing:
            return "Missing";
        default:
            throw Mars2GribTableException("Invalid ProductionStatusOfProcessedData enum value", Here());
    }

    __builtin_unreachable();
}

///
/// @brief Convert a symbolic name to `ProductionStatusOfProcessedData`.
///
/// @param[in] name Canonical symbolic name
///
/// @return Corresponding `ProductionStatusOfProcessedData` enum value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the name is not recognised
///
inline ProductionStatusOfProcessedData name2enum_ProductionStatusOfProcessedData_or_throw(const std::string& name) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "OperationalProducts")
        return ProductionStatusOfProcessedData::OperationalProducts;
    if (name == "OperationalTestProducts")
        return ProductionStatusOfProcessedData::OperationalTestProducts;
    if (name == "ResearchProducts")
        return ProductionStatusOfProcessedData::ResearchProducts;
    if (name == "ReanalysisProducts")
        return ProductionStatusOfProcessedData::ReanalysisProducts;
    if (name == "TiggeOperational")
        return ProductionStatusOfProcessedData::TiggeOperational;
    if (name == "TiggeTest")
        return ProductionStatusOfProcessedData::TiggeTest;
    if (name == "S2SOperationalProducts")
        return ProductionStatusOfProcessedData::S2SOperationalProducts;
    if (name == "S2STestProducts")
        return ProductionStatusOfProcessedData::S2STestProducts;
    if (name == "UerraOperational")
        return ProductionStatusOfProcessedData::UerraOperational;
    if (name == "UerraTest")
        return ProductionStatusOfProcessedData::UerraTest;
    if (name == "CopernicusRegionalReanalysis")
        return ProductionStatusOfProcessedData::CopernicusRegionalReanalysis;
    if (name == "CopernicusRegionalReanalysisTest")
        return ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest;
    if (name == "DestinationEarth")
        return ProductionStatusOfProcessedData::DestinationEarth;
    if (name == "DestinationEarthTest")
        return ProductionStatusOfProcessedData::DestinationEarthTest;
    if (name == "Missing")
        return ProductionStatusOfProcessedData::Missing;

    throw Mars2GribTableException("Invalid ProductionStatusOfProcessedData name: '" + name + "'", Here());

    __builtin_unreachable();
}

///
/// @brief Convert a numeric GRIB code to `ProductionStatusOfProcessedData`.
///
/// @param[in] value Numeric GRIB code table value
///
/// @return Corresponding `ProductionStatusOfProcessedData` enum value
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribTableException
/// If the numeric value is not valid
///
inline ProductionStatusOfProcessedData long2enum_ProductionStatusOfProcessedData_or_throw(long value) {

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            return ProductionStatusOfProcessedData::OperationalProducts;
        case 1:
            return ProductionStatusOfProcessedData::OperationalTestProducts;
        case 2:
            return ProductionStatusOfProcessedData::ResearchProducts;
        case 3:
            return ProductionStatusOfProcessedData::ReanalysisProducts;
        case 4:
            return ProductionStatusOfProcessedData::TiggeOperational;
        case 5:
            return ProductionStatusOfProcessedData::TiggeTest;
        case 6:
            return ProductionStatusOfProcessedData::S2SOperationalProducts;
        case 7:
            return ProductionStatusOfProcessedData::S2STestProducts;
        case 8:
            return ProductionStatusOfProcessedData::UerraOperational;
        case 9:
            return ProductionStatusOfProcessedData::UerraTest;
        case 10:
            return ProductionStatusOfProcessedData::CopernicusRegionalReanalysis;
        case 11:
            return ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest;
        case 12:
            return ProductionStatusOfProcessedData::DestinationEarth;
        case 13:
            return ProductionStatusOfProcessedData::DestinationEarthTest;
        case 255:
            return ProductionStatusOfProcessedData::Missing;
        default:
            throw Mars2GribTableException(
                "Invalid ProductionStatusOfProcessedData numeric value: " + std::to_string(value), Here());
    }

    __builtin_unreachable();
}


}  // namespace metkit::mars2grib::backend::tables