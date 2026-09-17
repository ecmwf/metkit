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
template <class Cntx_t>
inline std::string enum2name_ProductionStatusOfProcessedData_or_throw(ProductionStatusOfProcessedData value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case ProductionStatusOfProcessedData::OperationalProducts:
            {
                std::string result = "OperationalProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::OperationalTestProducts:
            {
                std::string result = "OperationalTestProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::ResearchProducts:
            {
                std::string result = "ResearchProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::ReanalysisProducts:
            {
                std::string result = "ReanalysisProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::TiggeOperational:
            {
                std::string result = "TiggeOperational";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::TiggeTest:
            {
                std::string result = "TiggeTest";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::S2SOperationalProducts:
            {
                std::string result = "S2SOperationalProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::S2STestProducts:
            {
                std::string result = "S2STestProducts";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::UerraOperational:
            {
                std::string result = "UerraOperational";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::UerraTest:
            {
                std::string result = "UerraTest";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::CopernicusRegionalReanalysis:
            {
                std::string result = "CopernicusRegionalReanalysis";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest:
            {
                std::string result = "CopernicusRegionalReanalysisTest";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::DestinationEarth:
            {
                std::string result = "DestinationEarth";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::DestinationEarthTest:
            {
                std::string result = "DestinationEarthTest";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case ProductionStatusOfProcessedData::Missing:
            {
                std::string result = "Missing";
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException("Invalid ProductionStatusOfProcessedData enum value", Here());
    }

    mars2gribUnreachable();
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
template <class Cntx_t>
inline ProductionStatusOfProcessedData name2enum_ProductionStatusOfProcessedData_or_throw(const std::string& name, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    if (name == "OperationalProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::OperationalProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "OperationalTestProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::OperationalTestProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "ResearchProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::ResearchProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "ReanalysisProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::ReanalysisProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "TiggeOperational") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::TiggeOperational;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "TiggeTest") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::TiggeTest;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "S2SOperationalProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::S2SOperationalProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "S2STestProducts") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::S2STestProducts;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "UerraOperational") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::UerraOperational;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "UerraTest") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::UerraTest;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "CopernicusRegionalReanalysis") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::CopernicusRegionalReanalysis;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "CopernicusRegionalReanalysisTest") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "DestinationEarth") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::DestinationEarth;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "DestinationEarthTest") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::DestinationEarthTest;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (name == "Missing") {
        {
            ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::Missing;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    throw Mars2GribTableException("Invalid ProductionStatusOfProcessedData name: '" + name + "'", Here());

    mars2gribUnreachable();
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
template <class Cntx_t>
inline ProductionStatusOfProcessedData long2enum_ProductionStatusOfProcessedData_or_throw(long value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());

    using metkit::mars2grib::utils::exceptions::Mars2GribTableException;

    switch (value) {
        case 0:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::OperationalProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 1:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::OperationalTestProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 2:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::ResearchProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 3:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::ReanalysisProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 4:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::TiggeOperational;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 5:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::TiggeTest;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 6:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::S2SOperationalProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 7:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::S2STestProducts;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 8:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::UerraOperational;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 9:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::UerraTest;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 10:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::CopernicusRegionalReanalysis;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 11:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::CopernicusRegionalReanalysisTest;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 12:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::DestinationEarth;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 13:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::DestinationEarthTest;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        case 255:
            {
                ProductionStatusOfProcessedData result = ProductionStatusOfProcessedData::Missing;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        default:
            throw Mars2GribTableException(
                "Invalid ProductionStatusOfProcessedData numeric value: " + std::to_string(value), Here());
    }

    mars2gribUnreachable();
}


}  // namespace metkit::mars2grib::backend::tables