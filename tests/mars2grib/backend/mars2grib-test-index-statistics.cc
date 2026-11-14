// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <array>
#include <cstddef>
#include <optional>
#include <string>

// Dictionary traits must precede the templated backend headers.
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsMatcher.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeRegistry.h"

namespace {

using metkit::mars2grib::backend::concepts_::StatisticsType;
using metkit::mars2grib::backend::deductions::SimulationRegime;
using metkit::mars2grib::backend::deductions::SimulationType;
using metkit::mars2grib::backend::deductions::TimeDuration;
using metkit::mars2grib::backend::deductions::Timespan;
using metkit::mars2grib::backend::deductions::TimespanKind;
using metkit::mars2grib::backend::models::product_time_spec::ProductTimeSpecInput;
using metkit::mars2grib::backend::models::product_time_spec::detail::isIndexStatistics;
using metkit::mars2grib::backend::models::product_time_spec::shape::classify_Shape_or_throw;
using metkit::mars2grib::backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
using metkit::mars2grib::backend::tables::TimeUnit;
using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;

struct IndexCase {
    long param;
    StatisticsType inner;
};

constexpr std::array<IndexCase, 11> indexCases{{
    {132044, StatisticsType::Maximum},
    {132045, StatisticsType::Average},
    {132049, StatisticsType::Maximum},
    {132059, StatisticsType::Maximum},
    {132144, StatisticsType::Accumulation},
    {132165, StatisticsType::Average},
    {132167, StatisticsType::Average},
    {132201, StatisticsType::Maximum},
    {132202, StatisticsType::Minimum},
    {132216, StatisticsType::Maximum},
    {132228, StatisticsType::Accumulation},
}};

ProductTimeSpecInput ordinaryInput() {
    ProductTimeSpecInput input;
    input.marsClass                            = "od";
    input.marsStream                           = "enfo";
    input.marsType                             = "fc";
    input.marsParamId                          = 235165;
    input.regime                               = SimulationRegime::IFS;
    input.simulationType                       = SimulationType::Forecast;
    input.step                                 = TimeDuration{96, TimeUnit::Hour};
    input.timespan                             = Timespan{TimespanKind::Duration, TimeDuration{24, TimeUnit::Hour}};
    input.innerMostTypeOfStatisticalProcessing = TypeOfStatisticalProcessing::Average;
    return input;
}

void markAsIndex(ProductTimeSpecInput& input) {
    input.marsParamId       = 132045;
    input.marsType          = "efi";
    input.isIndexStatistics = isIndexStatistics(input.marsParamId);
}

// First establish that the ordinary representation really matches. Then verify
// that changing its product identity cannot retain that ordinary shape.
void expectOrdinaryThenRejectIndex(ProductTimeSpecInput input, ProductTimeSpecShapeKind expected) {
    EXPECT(classify_Shape_or_throw(input) == expected);
    markAsIndex(input);
    EXPECT_THROWS(classify_Shape_or_throw(input));
}

}  // namespace

CASE("Index parameters have explicit membership and match their inner operation") {
    using metkit::mars2grib::backend::concepts_::statisticsMatcher;
    const metkit::mars2grib::Options options{};
    for (const auto& test : indexCases) {
        eckit::LocalConfiguration mars;
        mars.set("param", test.param);
        EXPECT(isIndexStatistics(test.param));
        EXPECT_EQUAL(statisticsMatcher(mars, options), static_cast<std::size_t>(test.inner));
    }
    // Neighbours in the same numeric range must not acquire index semantics.
    for (long param : {-1L, 228L, 132000L, 132043L, 132046L, 132215L, 132217L, 132229L, 132999L}) {
        EXPECT(!isIndexStatistics(param));
    }
}

CASE("Model input identifies an index independently of MARS type") {
    using metkit::mars2grib::backend::models::product_time_spec::make_ProductTimeSpecInput_or_throw;
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "fc");
    mars.set("param", 132144L);
    mars.set("date", 20260930L);
    mars.set("time", 0L);
    mars.set("step", 96L);
    mars.set("timespan", 24L);
    const eckit::LocalConfiguration misc;
    const metkit::mars2grib::Options options{};
    const auto input =
        make_ProductTimeSpecInput_or_throw(TypeOfStatisticalProcessing::Accumulation, mars, misc, options);
    EXPECT(input.isIndexStatistics);
    EXPECT(input.to_json().find("\"isIndexStatistics\":true") != std::string::npos);
    EXPECT_THROWS(classify_Shape_or_throw(input));
}

CASE("Index statistics select exactly the fake-single-loop double-loop shape") {
    auto input = ordinaryInput();
    EXPECT(classify_Shape_or_throw(input) == ProductTimeSpecShapeKind::IFSStandardSingleLoop);
    markAsIndex(input);
    for (const std::string& type : {"efi", "efic", "sot"}) {
        input.marsType = type;
        EXPECT(classify_Shape_or_throw(input) == ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop);
    }
    for (const std::string& type : {"fc", "an", "cpf"}) {
        input.marsType = type;
        EXPECT_THROWS(classify_Shape_or_throw(input));
    }
    input.marsType = "efi";
    input.timespan.duration.reset();
    EXPECT_THROWS(classify_Shape_or_throw(input));
}

CASE("Index statistics cannot use instant, synoptic, AIFS or explicit multiloop shapes") {
    auto input                                 = ordinaryInput();
    input.timespan                             = Timespan{TimespanKind::None, std::nullopt};
    input.innerMostTypeOfStatisticalProcessing = TypeOfStatisticalProcessing::Missing;
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::Instant);

    input                = ordinaryInput();
    input.isSynoptic     = true;
    input.simulationType = SimulationType::Analysis;
    input.marsType       = "an";
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::IFSSynopticSingleLoop);

    input           = ordinaryInput();
    input.regime    = SimulationRegime::AIFS;
    input.marsClass = "ml";
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::AIFSStandardSingleLoop);

    input          = ordinaryInput();
    input.stattype = {{TimeDuration{48, TimeUnit::Hour}, TypeOfStatisticalProcessing::Average}};
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::IFSStandardMultiLoop);
}

CASE("Index statistics cannot use from-start shapes at zero or positive steps") {
    for (const auto regime : {SimulationRegime::IFS, SimulationRegime::AIFS}) {
        auto input               = ordinaryInput();
        input.regime             = regime;
        input.marsClass          = regime == SimulationRegime::IFS ? "od" : "ml";
        input.timespan           = Timespan{TimespanKind::FromStart, std::nullopt};
        const auto positiveShape = regime == SimulationRegime::IFS
                                       ? ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive
                                       : ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive;
        expectOrdinaryThenRejectIndex(input, positiveShape);
        input.step                                                = TimeDuration{0, TimeUnit::Hour};
        input.innerMostTypeOfStatisticalProcessing                = TypeOfStatisticalProcessing::Accumulation;
        input.allowZeroLengthFsWindow                             = true;
        input.isAllowedInnerTypeOfStatisticalProcessingAtStepZero = true;
        const auto zeroShape = regime == SimulationRegime::IFS
                                   ? ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero
                                   : ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero;
        expectOrdinaryThenRejectIndex(input, zeroShape);
    }
}

CASE("Index statistics cannot use fake-double-loop single-loop or seasonal shapes") {
    for (const auto regime : {SimulationRegime::IFS, SimulationRegime::AIFS}) {
        auto input                                           = ordinaryInput();
        input.regime                                         = regime;
        input.requiresFakeDoubleLoopSingleLoopRepresentation = true;
        input.timespan                                       = Timespan{TimespanKind::None, std::nullopt};
        input.stattype      = {{TimeDuration{24, TimeUnit::Hour}, TypeOfStatisticalProcessing::Average}};
        const auto expected = regime == SimulationRegime::IFS ? ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop
                                                              : ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop;
        expectOrdinaryThenRejectIndex(input, expected);
    }

    auto input       = ordinaryInput();
    input.marsStream = "sfmd";
    input.step.reset();
    input.marsFcmonth                                    = 1;
    input.requiresFakeDoubleLoopSingleLoopRepresentation = true;
    input.timespan                                       = Timespan{TimespanKind::None, std::nullopt};
    input.stattype = {{TimeDuration{1, TimeUnit::Month}, TypeOfStatisticalProcessing::Average}};
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::SeasonalSingleLoop);
    input.timespan = Timespan{TimespanKind::Duration, TimeDuration{24, TimeUnit::Hour}};
    expectOrdinaryThenRejectIndex(input, ProductTimeSpecShapeKind::SeasonalMultiloop);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
