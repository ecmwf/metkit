/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "pdt.h"

#include <functional>
#include <unordered_map>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

using metkit::mars2grib::utils::dict_traits::get_or_throw;

namespace metkit::mars2grib::frontend {

enum class TimeExtent : std::uint64_t {
    None = 0,
    TimeRange,
    PointInTime,
};

TimeExtent parseTimeExtent(const std::string& s) {
    if (s == "None") {
        return TimeExtent::None;
    }
    if (s == "timeRange") {
        return TimeExtent::TimeRange;
    }
    if (s == "pointInTime") {
        return TimeExtent::PointInTime;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid TimeExtent", Here()};
}

enum class TimeFormat : std::uint64_t {
    None = 0,
    LocalTime,
    WithReferencePeriod,
};

TimeFormat parseTimeFormat(const std::string& s) {
    if (s == "None") {
        return TimeFormat::None;
    }
    if (s == "localTime") {
        return TimeFormat::LocalTime;
    }
    if (s == "withReferencePeriod") {
        return TimeFormat::WithReferencePeriod;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid TimeFormat", Here()};
}

enum class SpatialExtent : std::uint64_t {
    None = 0,
    RandomPatterns,
    GeneralisedTile,
    ClusterStatCircular,
    FocalStatistics,
    ClusterStatRectangular,
};

SpatialExtent parseSpatialExtent(const std::string& s) {
    if (s == "None") {
        return SpatialExtent::None;
    }
    if (s == "randomPatterns") {
        return SpatialExtent::RandomPatterns;
    }
    if (s == "clusterStatCircular") {
        return SpatialExtent::ClusterStatCircular;
    }
    if (s == "generalisedTile") {
        return SpatialExtent::GeneralisedTile;
    }
    if (s == "focalStatistics") {
        return SpatialExtent::FocalStatistics;
    }
    if (s == "clusterStatRectangular") {
        return SpatialExtent::ClusterStatRectangular;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid SpatialExtent", Here()};
}

enum class ProcessType : std::uint64_t {
    None = 0,
    Percentile,
    Quantile,
    Probability,
    Reforecast,
    Categorial,
    DerivedForecast,
};

ProcessType parseProcessType(const std::string& s) {
    if (s == "None") {
        return ProcessType::None;
    }
    if (s == "percentile") {
        return ProcessType::Percentile;
    }
    if (s == "quantile") {
        return ProcessType::Quantile;
    }
    if (s == "probability") {
        return ProcessType::Probability;
    }
    if (s == "reforecast") {
        return ProcessType::Reforecast;
    }
    if (s == "categorial") {
        return ProcessType::Categorial;
    }
    if (s == "derivedForecast") {
        return ProcessType::DerivedForecast;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid ProcessType", Here()};
}

enum class ProcessSubType : std::uint64_t {
    None = 0,
    LargeEnsemble,
    Ensemble,
};

ProcessSubType parseProcessSubType(const std::string& s) {
    if (s == "None") {
        return ProcessSubType::None;
    }
    if (s == "largeEnsemble") {
        return ProcessSubType::LargeEnsemble;
    }
    if (s == "ensemble") {
        return ProcessSubType::Ensemble;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid ProcessSubType", Here()};
}

enum class ProductCategory : std::uint64_t {
    None = 0,
    Aerosol,
    CcittIA5,
    CrossSect,
    Radar,
    SpatialStatisticalProcessing,
    Wave,
    PostProcess,
    Chemical,
    Partitioned,
    Hovmoeller,
    Optical,
    SpatioTemporalTile,
    Satellite,
};

ProductCategory parseProductCategory(const std::string& s) {
    if (s == "None") {
        return ProductCategory::None;
    }
    if (s == "aerosol") {
        return ProductCategory::Aerosol;
    }
    if (s == "ccittIA5") {
        return ProductCategory::CcittIA5;
    }
    if (s == "crossSect") {
        return ProductCategory::CrossSect;
    }
    if (s == "radar") {
        return ProductCategory::Radar;
    }
    if (s == "spatialStatisticalProcessing") {
        return ProductCategory::SpatialStatisticalProcessing;
    }
    if (s == "wave") {
        return ProductCategory::Wave;
    }
    if (s == "postProcess") {
        return ProductCategory::PostProcess;
    }
    if (s == "chemical") {
        return ProductCategory::Chemical;
    }
    if (s == "partitioned") {
        return ProductCategory::Partitioned;
    }
    if (s == "hovmoeller") {
        return ProductCategory::Hovmoeller;
    }
    if (s == "optical") {
        return ProductCategory::Optical;
    }
    if (s == "spatioTemporalTile") {
        return ProductCategory::SpatioTemporalTile;
    }
    if (s == "satellite") {
        return ProductCategory::Satellite;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid ProductCategory", Here()};
}

enum class ProductSubCategory : std::uint64_t {
    None = 0,
    SpectraFormula,
    QualityValue,
    PeriodRange,
    SpectraList,
    SourceSink,
    StatisticalOverLatLong,
    RadioNuclide,
    OpticalSourceSink,
    Distribution,
    Optical,
};

ProductSubCategory parseProductSubCategory(const std::string& s) {
    if (s == "None") {
        return ProductSubCategory::None;
    }
    if (s == "spectraFormula") {
        return ProductSubCategory::SpectraFormula;
    }
    if (s == "qualityValue") {
        return ProductSubCategory::QualityValue;
    }
    if (s == "periodRange") {
        return ProductSubCategory::PeriodRange;
    }
    if (s == "spectraList") {
        return ProductSubCategory::SpectraList;
    }
    if (s == "sourceSink") {
        return ProductSubCategory::SourceSink;
    }
    if (s == "statisticalOverLatLong") {
        return ProductSubCategory::StatisticalOverLatLong;
    }
    if (s == "radioNuclide") {
        return ProductSubCategory::RadioNuclide;
    }
    if (s == "opticalSourceSink") {
        return ProductSubCategory::OpticalSourceSink;
    }
    if (s == "distribution") {
        return ProductSubCategory::Distribution;
    }
    if (s == "optical") {
        return ProductSubCategory::Optical;
    }
    throw eckit::Exception{"\"" + s + "\" is not a valid ProductSubCategory", Here()};
}

struct PDTCat {
    TimeExtent timeExtent;
    TimeFormat timeFormat;
    SpatialExtent spatialExtent;
    ProcessType processType;
    ProcessSubType processSubType;
    ProductCategory productCategory;
    ProductSubCategory productSubCategory;

    bool operator==(const PDTCat& other) const noexcept {
        return std::tie(timeExtent, timeFormat, spatialExtent, processType, processSubType, productCategory,
                        productSubCategory) == std::tie(other.timeExtent, other.timeFormat, other.spatialExtent,
                                                        other.processType, other.processSubType, other.productCategory,
                                                        other.productSubCategory);
    }
};

}  // namespace metkit::mars2grib::frontend

inline void hash_combine(std::size_t& seed, std::size_t value) {
    // Standard hash-combine recipe
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

template <>
struct std::hash<metkit::mars2grib::frontend::PDTCat> {
    std::size_t operator()(const metkit::mars2grib::frontend::PDTCat& x) const noexcept {
        std::size_t h = 0;
        hash_combine(h, std::hash<metkit::mars2grib::frontend::TimeExtent>{}(x.timeExtent));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::TimeFormat>{}(x.timeFormat));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::SpatialExtent>{}(x.spatialExtent));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::ProcessType>{}(x.processType));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::ProcessSubType>{}(x.processSubType));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::ProductCategory>{}(x.productCategory));
        hash_combine(h, std::hash<metkit::mars2grib::frontend::ProductSubCategory>{}(x.productSubCategory));
        return h;
    }
};

namespace metkit::mars2grib::frontend {

PDTCat parsePDTCat(const eckit::LocalConfiguration& pdt) {
    return {parseTimeExtent(get_or_throw<std::string>(pdt, "timeExtent")),
            parseTimeFormat(get_or_throw<std::string>(pdt, "timeFormat")),
            parseSpatialExtent(get_or_throw<std::string>(pdt, "spatialExtent")),
            parseProcessType(get_or_throw<std::string>(pdt, "processType")),
            parseProcessSubType(get_or_throw<std::string>(pdt, "processSubType")),
            parseProductCategory(get_or_throw<std::string>(pdt, "productCategory")),
            parseProductSubCategory(get_or_throw<std::string>(pdt, "productSubCategory"))};
}

using DecisionMap = std::unordered_map<PDTCat, std::int64_t>;

static const DecisionMap map{
    {{PDTCat{TimeExtent::None, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Satellite, ProductSubCategory::None},
      31},
     {PDTCat{TimeExtent::None, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Satellite, ProductSubCategory::QualityValue},
      35},
     {PDTCat{TimeExtent::None, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Radar, ProductSubCategory::None},
      20},
     {PDTCat{TimeExtent::None, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::CcittIA5, ProductSubCategory::None},
      254},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::None, ProductSubCategory::None},
      8},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Aerosol, ProductSubCategory::None},
      46},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Aerosol, ProductSubCategory::SourceSink},
      82},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::CrossSect, ProductSubCategory::None},
      1001},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::PostProcess, ProductSubCategory::None},
      72},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::None},
      42},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::Distribution},
      67},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::SourceSink},
      78},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::RadioNuclide},
      126},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Hovmoeller, ProductSubCategory::None},
      1101},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Optical, ProductSubCategory::None},
      110},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::SpatioTemporalTile, ProductSubCategory::None},
      62},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::LargeEnsemble, ProductCategory::None, ProductSubCategory::None},
      118},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::None, ProductSubCategory::None},
      11},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Aerosol, ProductSubCategory::None},
      85},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Aerosol, ProductSubCategory::SourceSink},
      84},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::PostProcess, ProductSubCategory::None},
      73},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Chemical, ProductSubCategory::None},
      43},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Chemical, ProductSubCategory::Distribution},
      68},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Chemical, ProductSubCategory::SourceSink},
      79},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Chemical, ProductSubCategory::RadioNuclide},
      127},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Optical, ProductSubCategory::None},
      111},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::SpatioTemporalTile, ProductSubCategory::None},
      63},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::Ensemble,
             ProductCategory::Satellite, ProductSubCategory::None},
      34},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Percentile,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      10},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Quantile, ProcessSubType::None,
             ProductCategory::None, ProductSubCategory::None},
      87},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Probability,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      9},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Probability,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      120},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Reforecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      61},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::Categorial,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      91},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::None, ProcessType::DerivedForecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      12},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::ClusterStatCircular, ProcessType::DerivedForecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      14},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::GeneralisedTile, ProcessType::None,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      114},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::GeneralisedTile, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      116},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::FocalStatistics, ProcessType::Probability,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      122},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::None, SpatialExtent::ClusterStatRectangular,
             ProcessType::DerivedForecast, ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      13},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::None, ProductSubCategory::None},
      95},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::PostProcess, ProductSubCategory::None},
      97},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      96},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::PostProcess, ProductSubCategory::None},
      98},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::WithReferencePeriod, SpatialExtent::None, ProcessType::None,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      105},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::WithReferencePeriod, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      106},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::WithReferencePeriod, SpatialExtent::None, ProcessType::Probability,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      112},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::WithReferencePeriod, SpatialExtent::None, ProcessType::DerivedForecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      107},
     {PDTCat{TimeExtent::TimeRange, TimeFormat::WithReferencePeriod, SpatialExtent::FocalStatistics,
             ProcessType::Probability, ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      123},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::None, ProductSubCategory::None},
      0},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Aerosol, ProductSubCategory::Optical},
      48},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Aerosol, ProductSubCategory::OpticalSourceSink},
      80},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::CrossSect, ProductSubCategory::None},
      1000},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::CrossSect, ProductSubCategory::StatisticalOverLatLong},
      1002},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::SpatialStatisticalProcessing, ProductSubCategory::None},
      15},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Wave, ProductSubCategory::SpectraFormula},
      101},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Wave, ProductSubCategory::SpectraList},
      99},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Wave, ProductSubCategory::PeriodRange},
      103},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::PostProcess, ProductSubCategory::None},
      70},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::None},
      40},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::Distribution},
      57},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::SourceSink},
      76},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Chemical, ProductSubCategory::RadioNuclide},
      124},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Partitioned, ProductSubCategory::None},
      53},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Hovmoeller, ProductSubCategory::None},
      1100},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Optical, ProductSubCategory::None},
      108},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::SpatioTemporalTile, ProductSubCategory::None},
      55},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None, ProcessSubType::None,
             ProductCategory::Satellite, ProductSubCategory::None},
      32},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::LargeEnsemble, ProductCategory::None, ProductSubCategory::None},
      117},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      1},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Aerosol, ProductSubCategory::None},
      45},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Aerosol, ProductSubCategory::Optical},
      49},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Aerosol, ProductSubCategory::OpticalSourceSink},
      81},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Wave, ProductSubCategory::SpectraFormula},
      102},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Wave, ProductSubCategory::SpectraList},
      100},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Wave, ProductSubCategory::PeriodRange},
      104},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::PostProcess, ProductSubCategory::None},
      71},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Chemical, ProductSubCategory::None},
      41},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Chemical, ProductSubCategory::Distribution},
      58},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Chemical, ProductSubCategory::SourceSink},
      77},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Chemical, ProductSubCategory::RadioNuclide},
      125},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Partitioned, ProductSubCategory::None},
      54},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Optical, ProductSubCategory::None},
      109},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::SpatioTemporalTile, ProductSubCategory::None},
      59},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::Satellite, ProductSubCategory::None},
      33},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Percentile,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      6},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Quantile,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      86},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Probability,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      5},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Probability,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      119},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Reforecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      60},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::Categorial,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      51},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::None, ProcessType::DerivedForecast,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      2},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::RandomPatterns, ProcessType::None,
             ProcessSubType::LargeEnsemble, ProductCategory::None, ProductSubCategory::None},
      143},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::ClusterStatCircular,
             ProcessType::DerivedForecast, ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      4},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::GeneralisedTile, ProcessType::None,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      113},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::GeneralisedTile, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      115},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::FocalStatistics, ProcessType::Probability,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      121},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::None, SpatialExtent::ClusterStatRectangular,
             ProcessType::DerivedForecast, ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      3},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::None, ProductCategory::None, ProductSubCategory::None},
      88},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::None, ProductCategory::PostProcess, ProductSubCategory::None},
      93},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::None, ProductSubCategory::None},
      92},
     {PDTCat{TimeExtent::PointInTime, TimeFormat::LocalTime, SpatialExtent::None, ProcessType::None,
             ProcessSubType::Ensemble, ProductCategory::PostProcess, ProductSubCategory::None},
      94}}};

std::int64_t templateNumberFromPDT(const PDTCat& pdt) {
    if (auto search = map.find(pdt); search != map.end()) {
        return search->second;
    }
    throw eckit::Exception{"PDT categories can not be mapped to a pdt number!", Here()};
}

std::int64_t templateNumberFromPDT(const eckit::LocalConfiguration& pdt) {
    return templateNumberFromPDT(parsePDTCat(pdt));
}

}  // namespace metkit::mars2grib::frontend
