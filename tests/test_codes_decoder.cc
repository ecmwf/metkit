/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date   Nov 2022
/// @author Philipp Geier

#include <unistd.h>
#include <cstring>
#include <limits>

#include "eccodes.h"

#include "metkit/codes/CodesSplitter.h"
#include "metkit/codes/CodesDecoder.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/message/Message.h"
#include "eckit/message/Reader.h"
#include "eckit/testing/Test.h"


using namespace eckit::testing;

namespace metkit {
namespace codes {
namespace test {

//----------------------------------------------------------------------------------------------------------------------

class MetadataSetter : public eckit::LocalConfiguration {
public:
    using eckit::LocalConfiguration::getDouble;
    using eckit::LocalConfiguration::getLong;
    using eckit::LocalConfiguration::getString;
    using eckit::LocalConfiguration::has;

    template <typename T>
    void setValue(const std::string& key, const T& value) {
        set(key, value);
    }

    template <typename T>
    T get(const std::string& key) {
        T value;
        eckit::LocalConfiguration::get(key, value);
        return value;
    }

    std::vector<std::string> keys() { return eckit::LocalConfiguration::keys(); }
};


static unsigned char unstr_latlon[] = {0x47, 0x52, 0x49, 0x42, 0xff, 0xff, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x9f, 0x00, 0x00, 0x00, 0x15, 0x01, 0x00, 0x62, 0x00, 0xff, 0x19, 0x00, 0x00, 0x00, 0x01,
                                       0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x02, 0x00, 0x01, 0x00,
                                       0x01, 0x00, 0x02, 0x04, 0x01, 0x30, 0x30, 0x30, 0x31, 0x00, 0x00, 0x00, 0x23, 0x03, 0x00,
                                       0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x65, 0x06, 0x00, 0x00, 0x0a, 0x01, 0x66, 0xa3,
                                       0x41, 0xd2, 0x1d, 0xcf, 0x11, 0xb2, 0x88, 0x0c, 0x0f, 0x16, 0x45, 0xf3, 0xd1, 0xdc, 0x00,
                                       0x00, 0x00, 0x22, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
                                       0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff,
                                       0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x15, 0x05, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0xff,
                                       0x00, 0x00, 0x00, 0x05, 0x07, 0x37, 0x37, 0x37, 0x37};


#define MD_EXPECT_STRING(md, name, eq)                                                                                     \
    EXPECT(md.has(name));                                                                                                  \
    std::cout << "expect string for " << name << " to equal " << eq << " (got " << md.getString(name) << ")" << std::endl; \
    EXPECT(md.getString(name) == eq);

// we accept two possible encodings, to enable testing with different versions of ecCodes (ECC-1704)
#define MD_EXPECT_STRINGS(md, name, eq1, eq2)                                                                                     \
    EXPECT(md.has(name));                                                                                                  \
    std::cout << "expect string for " << name << " to equal " << eq1 << " or " << eq2 << " (got " << md.getString(name) << ")" << std::endl; \
    EXPECT(md.getString(name) == eq1 || md.getString(name) == eq2);

#define MD_EXPECT_LONG(md, name, eq)                                                                                   \
    EXPECT(md.has(name));                                                                                              \
    std::cout << "expect long for " << name << " to equal " << eq << " (got " << md.getLong(name) << ")" << std::endl; \
    EXPECT(md.getLong(name) == eq);

#define MD_EXPECT_GE_LONG(md, name, eq)                                                                                   \
    EXPECT(md.has(name));                                                                                              \
    std::cout << "expect long for " << name << " to be greater than or equal to " << eq << " (got " << md.getLong(name) << ")" << std::endl; \
    EXPECT(md.getLong(name) >= eq);

#define MD_EXPECT_DOUBLE(md, name, eq)                                                                                                                     \
    EXPECT(md.has(name));                                                                                                                                  \
    std::cout << "expect double for " << name << " to equal " << std::to_string(eq) << " (got " << std::to_string(md.getDouble(name)) << ")" << std::endl; \
    EXPECT(std::to_string(md.getDouble(name)) == std::to_string(eq));


//----------------------------------------------------------------------------------------------------------------------

CASE("test codessplitter unstr_latlot.tmpl Native") {
    eckit::MemoryHandle data(static_cast<void*>(unstr_latlon), sizeof(unstr_latlon));

    std::cout << "Data location " << ((size_t)&unstr_latlon) << std::endl;

    eckit::message::Reader reader(data);
    eckit::message::Message msg;

    msg = reader.next();
    EXPECT(msg);

    MetadataSetter md;
    eckit::message::TypedSetter<MetadataSetter> gatherer{md};
    eckit::message::GetMetadataOptions mdOpts{};
    mdOpts.valueRepresentation = eckit::message::ValueRepresentation::Native;
    mdOpts.nameSpace           = "";
    msg.getMetadata(gatherer, mdOpts);

    {
        MD_EXPECT_STRING(md, "globalDomain", "g");
        MD_EXPECT_LONG(md, "GRIBEditionNumber", 2);
        MD_EXPECT_GE_LONG(md, "tablesVersionLatestOfficial", 30);
        MD_EXPECT_GE_LONG(md, "tablesVersionLatest", 30);
        MD_EXPECT_LONG(md, "grib2divider", 1000000);
        MD_EXPECT_LONG(md, "angleSubdivisions", 1000000);
        MD_EXPECT_LONG(md, "missingValue", 9999);
        MD_EXPECT_LONG(md, "ieeeFloats", 1);
        MD_EXPECT_LONG(md, "isHindcast", 0);
        MD_EXPECT_LONG(md, "section0Length", 16);
        MD_EXPECT_STRING(md, "identifier", "GRIB");
        MD_EXPECT_LONG(md, "discipline", 2);
        MD_EXPECT_LONG(md, "editionNumber", 2);
        MD_EXPECT_LONG(md, "totalLength", 159);
        MD_EXPECT_LONG(md, "section1Length", 21);
        MD_EXPECT_STRING(md, "centre", "ecmf");
        MD_EXPECT_STRING(md, "centreDescription", "European Centre for Medium-Range Weather Forecasts");
        MD_EXPECT_LONG(md, "subCentre", 255);
        MD_EXPECT_LONG(md, "tablesVersion", 25);
        MD_EXPECT_STRING(md, "masterDir", "grib2/tables/[tablesVersion]");
        MD_EXPECT_LONG(md, "localTablesVersion", 0);
        MD_EXPECT_LONG(md, "significanceOfReferenceTime", 0);
        MD_EXPECT_LONG(md, "year", 1);
        MD_EXPECT_LONG(md, "month", 1);
        MD_EXPECT_LONG(md, "day", 1);
        MD_EXPECT_LONG(md, "hour", 0);
        MD_EXPECT_LONG(md, "minute", 0);
        MD_EXPECT_LONG(md, "second", 0);
        MD_EXPECT_LONG(md, "dataDate", 10101);
        MD_EXPECT_DOUBLE(md, "julianDay", 1.7214235e+06);
        MD_EXPECT_LONG(md, "dataTime", 0);
        MD_EXPECT_LONG(md, "productionStatusOfProcessedData", 0);
        MD_EXPECT_STRING(md, "typeOfProcessedData", "an");
        MD_EXPECT_LONG(md, "selectStepTemplateInterval", 1);
        MD_EXPECT_LONG(md, "selectStepTemplateInstant", 1);
        MD_EXPECT_STRING(md, "stepType", "instant");
        MD_EXPECT_LONG(md, "is_chemical", 0);
        MD_EXPECT_LONG(md, "is_chemical_distfn", 0);
        MD_EXPECT_LONG(md, "is_chemical_srcsink", 0);
        MD_EXPECT_LONG(md, "is_aerosol", 0);
        MD_EXPECT_LONG(md, "is_aerosol_optical", 0);
        MD_EXPECT_LONG(md, "setCalendarId", 0);
        MD_EXPECT_LONG(md, "deleteCalendarId", 0);
        MD_EXPECT_LONG(md, "grib2LocalSectionPresent", 0);
        MD_EXPECT_LONG(md, "deleteLocalDefinition", 0);
        MD_EXPECT_LONG(md, "section2Length", 17);
        MD_EXPECT_LONG(md, "addEmptySection2", 0);
        MD_EXPECT_LONG(md, "grib2LocalSectionNumber", 1);
        MD_EXPECT_STRING(md, "marsClass", "od");
        MD_EXPECT_STRING(md, "marsType", "an");
        MD_EXPECT_STRING(md, "marsStream", "oper");
        MD_EXPECT_STRING(md, "experimentVersionNumber", "0001");
        MD_EXPECT_STRING(md, "class", "od");
        MD_EXPECT_STRING(md, "type", "an");
        MD_EXPECT_STRING(md, "stream", "oper");
        MD_EXPECT_LONG(md, "productDefinitionTemplateNumberInternal", -1);
        MD_EXPECT_LONG(md, "localDefinitionNumber", 1);
        MD_EXPECT_LONG(md, "eps", 0);
        MD_EXPECT_LONG(md, "addExtraLocalSection", 0);
        MD_EXPECT_LONG(md, "deleteExtraLocalSection", 0);
        MD_EXPECT_LONG(md, "extraLocalSectionPresent", 0);
        MD_EXPECT_LONG(md, "gridDescriptionSectionPresent", 1);
        MD_EXPECT_LONG(md, "section3Length", 35);
        MD_EXPECT_LONG(md, "sourceOfGridDefinition", 0);
        MD_EXPECT_LONG(md, "numberOfDataPoints", 496);
        MD_EXPECT_LONG(md, "numberOfOctectsForNumberOfPoints", 0);
        MD_EXPECT_LONG(md, "interpretationOfNumberOfPoints", 0);
        MD_EXPECT_LONG(md, "PLPresent", 0);
        MD_EXPECT_LONG(md, "gridDefinitionTemplateNumber", 101);
        MD_EXPECT_STRING(md, "gridDefinitionDescription", "General unstructured grid");
        MD_EXPECT_LONG(md, "shapeOfTheEarth", 6);
        MD_EXPECT_LONG(md, "numberOfGridUsed", 10);
        MD_EXPECT_LONG(md, "numberOfGridInReference", 1);
        MD_EXPECT_STRING(md, "unstructuredGridType", "unknown");
        MD_EXPECT_STRING(md, "unstructuredGridSubtype", "T");
        MD_EXPECT_STRING(md, "unstructuredGridUUID", "unknown");
        MD_EXPECT_STRING(md, "gridName", "unknown");
        MD_EXPECT_STRING(md, "gridType", "unstructured_grid");
        MD_EXPECT_LONG(md, "section4Length", 34);
        MD_EXPECT_LONG(md, "NV", 0);
        MD_EXPECT_LONG(md, "neitherPresent", 0);
        MD_EXPECT_STRING(md, "datasetForLocal", "unknown");
        MD_EXPECT_LONG(md, "productDefinitionTemplateNumber", 0);
        MD_EXPECT_LONG(md, "genVertHeightCoords", 0);
        MD_EXPECT_LONG(md, "parameterCategory", 0);
        MD_EXPECT_LONG(md, "parameterNumber", 0);
        MD_EXPECT_STRING(md, "parameterUnits", "Proportion");
        MD_EXPECT_STRING(md, "parameterName", "Land cover (0 = sea, 1 = land)");
        MD_EXPECT_LONG(md, "typeOfGeneratingProcess", 2);
        MD_EXPECT_LONG(md, "backgroundProcess", 0);
        MD_EXPECT_LONG(md, "generatingProcessIdentifier", 1);
        MD_EXPECT_LONG(md, "hoursAfterDataCutoff", 0);
        MD_EXPECT_LONG(md, "minutesAfterDataCutoff", 0);
        // MD_EXPECT_LONG(md, "indicatorOfUnitOfTimeRange", 1);
        MD_EXPECT_LONG(md, "stepUnits", 1);
        MD_EXPECT_LONG(md, "forecastTime", 0);
        MD_EXPECT_LONG(md, "startStep", 0);
        MD_EXPECT_LONG(md, "endStep", 0);
        MD_EXPECT_STRING(md, "stepRange", "0");
        MD_EXPECT_LONG(md, "validityDate", 10101);
        MD_EXPECT_STRINGS(md, "validityTime", "0", "0000");
        MD_EXPECT_STRING(md, "typeOfFirstFixedSurface", "168");
        MD_EXPECT_STRING(md, "unitsOfFirstFixedSurface", "Numeric");
        MD_EXPECT_STRING(md, "nameOfFirstFixedSurface", "Ocean model level");
        MD_EXPECT_LONG(md, "scaleFactorOfFirstFixedSurface", 0);
        MD_EXPECT_LONG(md, "scaledValueOfFirstFixedSurface", 2147483647);
        MD_EXPECT_LONG(md, "typeOfSecondFixedSurface", 255);
        MD_EXPECT_STRING(md, "unitsOfSecondFixedSurface", "unknown");
        MD_EXPECT_STRING(md, "nameOfSecondFixedSurface", "Missing");
        MD_EXPECT_LONG(md, "scaleFactorOfSecondFixedSurface", 0);
        MD_EXPECT_LONG(md, "scaledValueOfSecondFixedSurface", 2147483647);
        MD_EXPECT_STRING(md, "pressureUnits", "hPa");
        MD_EXPECT_STRING(md, "typeOfLevel", "oceanModel");
        MD_EXPECT_LONG(md, "level", 0);
        MD_EXPECT_LONG(md, "bottomLevel", 0);
        MD_EXPECT_LONG(md, "topLevel", 0);
        MD_EXPECT_STRING(md, "tempPressureUnits", "hPa");
        MD_EXPECT_STRING(md, "levtype", "o3d");
        MD_EXPECT_LONG(md, "PVPresent", 0);
        MD_EXPECT_STRING(md, "deletePV", "1");
        MD_EXPECT_LONG(md, "lengthOfHeaders", 107);
        MD_EXPECT_LONG(md, "section5Length", 21);
        MD_EXPECT_LONG(md, "numberOfValues", 496);
        MD_EXPECT_LONG(md, "dataRepresentationTemplateNumber", 0);
        MD_EXPECT_STRING(md, "packingType", "grid_simple");
        MD_EXPECT_LONG(md, "referenceValue", 0);
        MD_EXPECT_DOUBLE(md, "referenceValueError", 1.17549e-38);
        MD_EXPECT_LONG(md, "binaryScaleFactor", -15);
        MD_EXPECT_LONG(md, "decimalScaleFactor", 0);
        MD_EXPECT_LONG(md, "optimizeScaleFactor", 0);
        MD_EXPECT_LONG(md, "bitsPerValue", 0);
        MD_EXPECT_LONG(md, "typeOfOriginalFieldValues", 0);
        MD_EXPECT_LONG(md, "section6Length", 6);
        MD_EXPECT_LONG(md, "bitMapIndicator", 255);
        MD_EXPECT_LONG(md, "bitmapPresent", 0);
        MD_EXPECT_LONG(md, "section7Length", 5);
        MD_EXPECT_DOUBLE(md, "packingError", 1.17549e-38);
        MD_EXPECT_DOUBLE(md, "unpackedError", 1.17549e-38);
        MD_EXPECT_LONG(md, "maximum", 0);
        MD_EXPECT_LONG(md, "minimum", 0);
        MD_EXPECT_LONG(md, "average", 0);
        MD_EXPECT_LONG(md, "numberOfMissing", 0);
        MD_EXPECT_LONG(md, "standardDeviation", 0);
        MD_EXPECT_LONG(md, "skewness", 0);
        MD_EXPECT_LONG(md, "kurtosis", 0);
        MD_EXPECT_LONG(md, "isConstant", 1);
        MD_EXPECT_LONG(md, "changeDecimalPrecision", 0);
        MD_EXPECT_LONG(md, "decimalPrecision", 0);
        MD_EXPECT_LONG(md, "setBitsPerValue", 0);
        MD_EXPECT_LONG(md, "getNumberOfValues", 496);
        MD_EXPECT_LONG(md, "scaleValuesBy", 1);
        MD_EXPECT_LONG(md, "offsetValuesBy", 0);
        MD_EXPECT_STRING(md, "productType", "unknown");
        MD_EXPECT_LONG(md, "section8Length", 4);
        MD_EXPECT_STRING(md, "7777", "7777");
        MD_EXPECT_STRING(md, "uuidOfHGrid", "66a341d21dcf11b2880c0f1645f3d1dc");
    }
}


//----------------------------------------------------------------------------------------------------------------------

CASE("test codessplitter unstr_latlot.tmpl String") {
    eckit::MemoryHandle data(static_cast<void*>(unstr_latlon), sizeof(unstr_latlon));

    std::cout << "Data location " << ((size_t)&unstr_latlon) << std::endl;

    eckit::message::Reader reader(data);
    eckit::message::Message msg;

    msg = reader.next();
    EXPECT(msg);

    MetadataSetter md;
    eckit::message::TypedSetter<MetadataSetter> gatherer{md};
    eckit::message::GetMetadataOptions mdOpts{};
    mdOpts.valueRepresentation = eckit::message::ValueRepresentation::String;
    mdOpts.nameSpace           = "";
    msg.getMetadata(gatherer, mdOpts);

    {
        MD_EXPECT_STRING(md, "globalDomain", "g");
        MD_EXPECT_STRING(md, "GRIBEditionNumber", "2");
        // This is not easy to test, as the latest official version can increment...
        // MD_EXPECT_STRING(md, "tablesVersionLatestOfficial", "30");
        // MD_EXPECT_STRING(md, "tablesVersionLatest", "30");
        MD_EXPECT_STRING(md, "grib2divider", "1e+06");
        MD_EXPECT_STRING(md, "angleSubdivisions", "1e+06");
        MD_EXPECT_STRING(md, "missingValue", "9999");
        MD_EXPECT_STRING(md, "ieeeFloats", "1");
        MD_EXPECT_STRING(md, "isHindcast", "0");
        MD_EXPECT_STRING(md, "section0Length", "16");
        MD_EXPECT_STRING(md, "identifier", "GRIB");
        MD_EXPECT_STRING(md, "discipline", "2");
        MD_EXPECT_STRING(md, "editionNumber", "2");
        MD_EXPECT_STRING(md, "totalLength", "159");
        MD_EXPECT_STRING(md, "section1Length", "21");
        MD_EXPECT_STRING(md, "centre", "ecmf");
        MD_EXPECT_STRING(md, "centreDescription", "European Centre for Medium-Range Weather Forecasts");
        MD_EXPECT_STRING(md, "subCentre", "255");
        MD_EXPECT_STRING(md, "tablesVersion", "25");
        MD_EXPECT_STRING(md, "masterDir", "grib2/tables/[tablesVersion]");
        MD_EXPECT_STRING(md, "localTablesVersion", "0");
        MD_EXPECT_STRING(md, "significanceOfReferenceTime", "0");
        MD_EXPECT_STRING(md, "year", "1");
        MD_EXPECT_STRING(md, "month", "1");
        MD_EXPECT_STRING(md, "day", "1");
        MD_EXPECT_STRING(md, "hour", "0");
        MD_EXPECT_STRING(md, "minute", "0");
        MD_EXPECT_STRING(md, "second", "0");
        MD_EXPECT_STRING(md, "dataDate", "10101");
        MD_EXPECT_STRING(md, "julianDay", "1.72142e+06");
        MD_EXPECT_STRING(md, "dataTime", "0000");
        MD_EXPECT_STRING(md, "productionStatusOfProcessedData", "0");
        MD_EXPECT_STRING(md, "typeOfProcessedData", "an");
        MD_EXPECT_STRING(md, "selectStepTemplateInterval", "1");
        MD_EXPECT_STRING(md, "selectStepTemplateInstant", "1");
        MD_EXPECT_STRING(md, "stepType", "instant");
        MD_EXPECT_STRING(md, "is_chemical", "0");
        MD_EXPECT_STRING(md, "is_chemical_distfn", "0");
        MD_EXPECT_STRING(md, "is_chemical_srcsink", "0");
        MD_EXPECT_STRING(md, "is_aerosol", "0");
        MD_EXPECT_STRING(md, "is_aerosol_optical", "0");
        MD_EXPECT_STRING(md, "setCalendarId", "0");
        MD_EXPECT_STRING(md, "deleteCalendarId", "0");
        MD_EXPECT_STRING(md, "grib2LocalSectionPresent", "0");
        MD_EXPECT_STRING(md, "deleteLocalDefinition", "0");
        MD_EXPECT_STRING(md, "section2Length", "17");
        MD_EXPECT_STRING(md, "addEmptySection2", "0");
        MD_EXPECT_STRING(md, "grib2LocalSectionNumber", "1");
        MD_EXPECT_STRING(md, "marsClass", "od");
        MD_EXPECT_STRING(md, "marsType", "an");
        MD_EXPECT_STRING(md, "marsStream", "oper");
        MD_EXPECT_STRING(md, "experimentVersionNumber", "0001");
        MD_EXPECT_STRING(md, "class", "od");
        MD_EXPECT_STRING(md, "type", "an");
        MD_EXPECT_STRING(md, "stream", "oper");
        MD_EXPECT_STRING(md, "productDefinitionTemplateNumberInternal", "-1");
        MD_EXPECT_STRING(md, "localDefinitionNumber", "1");
        MD_EXPECT_STRING(md, "eps", "0");
        MD_EXPECT_STRING(md, "addExtraLocalSection", "0");
        MD_EXPECT_STRING(md, "deleteExtraLocalSection", "0");
        MD_EXPECT_STRING(md, "extraLocalSectionPresent", "0");
        MD_EXPECT_STRING(md, "gridDescriptionSectionPresent", "1");
        MD_EXPECT_STRING(md, "section3Length", "35");
        MD_EXPECT_STRING(md, "sourceOfGridDefinition", "0");
        MD_EXPECT_STRING(md, "numberOfDataPoints", "496");
        MD_EXPECT_STRING(md, "numberOfOctectsForNumberOfPoints", "0");
        MD_EXPECT_STRING(md, "interpretationOfNumberOfPoints", "0");
        MD_EXPECT_STRING(md, "PLPresent", "0");
        MD_EXPECT_STRING(md, "gridDefinitionTemplateNumber", "101");
        MD_EXPECT_STRING(md, "gridDefinitionDescription", "General unstructured grid");
        MD_EXPECT_STRING(md, "shapeOfTheEarth", "6");
        MD_EXPECT_STRING(md, "numberOfGridUsed", "10");
        MD_EXPECT_STRING(md, "numberOfGridInReference", "1");
        MD_EXPECT_STRING(md, "uuidOfHGrid", "66a341d21dcf11b2880c0f1645f3d1dc");
        MD_EXPECT_STRING(md, "unstructuredGridType", "unknown");
        MD_EXPECT_STRING(md, "unstructuredGridSubtype", "T");
        MD_EXPECT_STRING(md, "unstructuredGridUUID", "unknown");
        MD_EXPECT_STRING(md, "gridName", "unknown");
        MD_EXPECT_STRING(md, "gridType", "unstructured_grid");
        MD_EXPECT_STRING(md, "section4Length", "34");
        MD_EXPECT_STRING(md, "NV", "0");
        MD_EXPECT_STRING(md, "neitherPresent", "0");
        MD_EXPECT_STRING(md, "datasetForLocal", "unknown");
        MD_EXPECT_STRING(md, "productDefinitionTemplateNumber", "0");
        MD_EXPECT_STRING(md, "genVertHeightCoords", "0");
        MD_EXPECT_STRING(md, "parameterCategory", "0");
        MD_EXPECT_STRING(md, "parameterNumber", "0");
        MD_EXPECT_STRING(md, "parameterUnits", "Proportion");
        MD_EXPECT_STRING(md, "parameterName", "Land cover (0 = sea, 1 = land)");
        MD_EXPECT_STRING(md, "typeOfGeneratingProcess", "2");
        MD_EXPECT_STRING(md, "backgroundProcess", "0");
        MD_EXPECT_STRING(md, "generatingProcessIdentifier", "1");
        MD_EXPECT_STRING(md, "hoursAfterDataCutoff", "0");
        MD_EXPECT_STRING(md, "minutesAfterDataCutoff", "0");
        // MD_EXPECT_STRING(md, "indicatorOfUnitOfTimeRange", "h");
        MD_EXPECT_STRING(md, "stepUnits", "h");
        MD_EXPECT_STRING(md, "forecastTime", "0");
        MD_EXPECT_STRING(md, "startStep", "0");
        MD_EXPECT_STRING(md, "endStep", "0");
        MD_EXPECT_STRING(md, "stepRange", "0");
        MD_EXPECT_STRING(md, "validityDate", "10101");
        MD_EXPECT_STRINGS(md, "validityTime", "0", "0000");
        MD_EXPECT_STRING(md, "typeOfFirstFixedSurface", "168");
        MD_EXPECT_STRING(md, "unitsOfFirstFixedSurface", "Numeric");
        MD_EXPECT_STRING(md, "nameOfFirstFixedSurface", "Ocean model level");
        MD_EXPECT_STRING(md, "scaleFactorOfFirstFixedSurface", "0");
        MD_EXPECT_STRING(md, "scaledValueOfFirstFixedSurface", "MISSING");
        MD_EXPECT_STRING(md, "typeOfSecondFixedSurface", "255");
        MD_EXPECT_STRING(md, "unitsOfSecondFixedSurface", "unknown");
        MD_EXPECT_STRING(md, "nameOfSecondFixedSurface", "Missing");
        MD_EXPECT_STRING(md, "scaleFactorOfSecondFixedSurface", "0");
        MD_EXPECT_STRING(md, "scaledValueOfSecondFixedSurface", "MISSING");
        MD_EXPECT_STRING(md, "pressureUnits", "hPa");
        MD_EXPECT_STRING(md, "typeOfLevel", "oceanModel");
        MD_EXPECT_STRING(md, "level", "0");
        MD_EXPECT_STRING(md, "bottomLevel", "0");
        MD_EXPECT_STRING(md, "topLevel", "0");
        MD_EXPECT_STRING(md, "tempPressureUnits", "hPa");
        MD_EXPECT_STRING(md, "levtype", "o3d");
        MD_EXPECT_STRING(md, "PVPresent", "0");
        MD_EXPECT_STRING(md, "deletePV", "1");
        MD_EXPECT_STRING(md, "lengthOfHeaders", "107");
        MD_EXPECT_STRING(md, "section5Length", "21");
        MD_EXPECT_STRING(md, "numberOfValues", "496");
        MD_EXPECT_STRING(md, "dataRepresentationTemplateNumber", "0");
        MD_EXPECT_STRING(md, "packingType", "grid_simple");
        MD_EXPECT_STRING(md, "referenceValue", "0");
        MD_EXPECT_STRING(md, "referenceValueError", "1.17549e-38");
        MD_EXPECT_STRING(md, "binaryScaleFactor", "-15");
        MD_EXPECT_STRING(md, "decimalScaleFactor", "0");
        MD_EXPECT_STRING(md, "optimizeScaleFactor", "0");
        MD_EXPECT_STRING(md, "bitsPerValue", "0");
        MD_EXPECT_STRING(md, "typeOfOriginalFieldValues", "0");
        MD_EXPECT_STRING(md, "section6Length", "6");
        MD_EXPECT_STRING(md, "bitMapIndicator", "255");
        MD_EXPECT_STRING(md, "bitmapPresent", "0");
        MD_EXPECT_STRING(md, "section7Length", "5");
        MD_EXPECT_STRING(md, "packingError", "1.17549e-38");
        MD_EXPECT_STRING(md, "unpackedError", "1.17549e-38");
        MD_EXPECT_STRING(md, "maximum", "0");
        MD_EXPECT_STRING(md, "minimum", "0");
        MD_EXPECT_STRING(md, "average", "0");
        MD_EXPECT_STRING(md, "numberOfMissing", "0");
        MD_EXPECT_STRING(md, "standardDeviation", "0");
        MD_EXPECT_STRING(md, "skewness", "0");
        MD_EXPECT_STRING(md, "kurtosis", "0");
        MD_EXPECT_STRING(md, "isConstant", "1");
        MD_EXPECT_STRING(md, "changeDecimalPrecision", "0");
        MD_EXPECT_STRING(md, "decimalPrecision", "0");
        MD_EXPECT_STRING(md, "setBitsPerValue", "0");
        MD_EXPECT_STRING(md, "getNumberOfValues", "496");
        MD_EXPECT_STRING(md, "scaleValuesBy", "1");
        MD_EXPECT_STRING(md, "offsetValuesBy", "0");
        MD_EXPECT_STRING(md, "productType", "unknown");
        MD_EXPECT_STRING(md, "section8Length", "4");
        MD_EXPECT_STRING(md, "7777", "7777");
    }
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace test
}  // namespace codes
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
