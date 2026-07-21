#pragma once

#include <cstddef>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::shape {

///
/// @brief Supported canonical ProductTimeSpec shape cases.
///
/// Shape classification describes the canonical statistical-window topology of
/// the final ProductTimeSpec representation.
///
/// The active cases are grouped as follows:
/// - instant-product representations distinguished by source `timespan`
///   encoding;
/// - IFS single-loop and multi-loop statistical representations;
/// - AIFS single-loop statistical representations whose increment semantics are
///   always missing;
/// - from-start variants distinguished by zero-length versus positive-step
///   semantics;
/// - fake-loop compatibility cases that preserve legacy source encodings while
///   producing canonical windows.
///
/// `Count` is a sentinel used exclusively to size the registry.
///
enum class ProductTimeSpecShapeKind : std::size_t {
    InstantTimespanMissing,
    InstantTimespanNone,

    IFSStandardSingleLoop,
    IFSFakeDoubleLoopSingleLoop,
    IFSFromStartSingleLoopAtZero,
    IFSFromStartSingleLoopPositive,
    IFSSynopticSingleLoop,

    AIFSStandardSingleLoop,
    AIFSFakeDoubleLoopSingleLoop,
    AIFSFromStartSingleLoopAtZero,
    AIFSFromStartSingleLoopPositive,

    IFSStandardMultiLoop,
    IFSFakeSingleLoopDoubleLoop,

    Count  // Used for sizing the registry, not a valid classification.
};

///
/// @brief One canonical ProductTimeSpec statistical window.
///
/// Each canonical window stores the statistical processing applied over that
/// window, the GRIB `typeOfTimeIncrement` associated with it, its range, and the
/// increment associated with samples contributing to it.
///
struct ProductTimeSpecWindow {
    /// @brief Statistical processing performed over this canonical window.
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{tables::TypeOfStatisticalProcessing::Missing};

    /// @brief GRIB `typeOfTimeIncrement` describing the increment semantics.
    metkit::mars2grib::backend::tables::TypeOfTimeIntervals typeOfTimeIncrement{
        metkit::mars2grib::backend::tables::TypeOfTimeIntervals::Missing};

    /// @brief Length of the canonical statistical window.
    deductions::TimeDuration timeRange{};

    /// @brief Increment associated with samples contributing to the window.
    deductions::TimeDuration timeIncrement{};
};

///
/// @brief Ordered canonical ProductTimeSpec window sequence.
///
/// Windows are stored in outermost-to-innermost order.
///
struct ProductTimeSpecShape {
    /// @brief Canonical windows in outermost-to-innermost order.
    std::vector<ProductTimeSpecWindow> values{};
};

/// @brief Return the stable diagnostic name of one shape case.
/// @param[in] value Shape classification value.
/// @return Stable human-readable shape-case name.
inline std::string productTimeSpecShapeTypeName(ProductTimeSpecShapeKind value) {

    switch (value) {
        case ProductTimeSpecShapeKind::InstantTimespanMissing:
            return "InstantTimespanMissing";
        case ProductTimeSpecShapeKind::InstantTimespanNone:
            return "InstantTimespanNone";

        case ProductTimeSpecShapeKind::IFSStandardSingleLoop:
            return "IFSStandardSingleLoop";
        case ProductTimeSpecShapeKind::IFSFakeDoubleLoopSingleLoop:
            return "IFSFakeDoubleLoopSingleLoop";
        case ProductTimeSpecShapeKind::IFSFromStartSingleLoopAtZero:
            return "IFSFromStartSingleLoopAtZero";
        case ProductTimeSpecShapeKind::IFSFromStartSingleLoopPositive:
            return "IFSFromStartSingleLoopPositive";
        case ProductTimeSpecShapeKind::IFSSynopticSingleLoop:
            return "IFSSynopticSingleLoop";

        case ProductTimeSpecShapeKind::AIFSStandardSingleLoop:
            return "AIFSStandardSingleLoop";
        case ProductTimeSpecShapeKind::AIFSFakeDoubleLoopSingleLoop:
            return "AIFSFakeDoubleLoopSingleLoop";
        case ProductTimeSpecShapeKind::AIFSFromStartSingleLoopAtZero:
            return "AIFSFromStartSingleLoopAtZero";
        case ProductTimeSpecShapeKind::AIFSFromStartSingleLoopPositive:
            return "AIFSFromStartSingleLoopPositive";

        case ProductTimeSpecShapeKind::IFSStandardMultiLoop:
            return "IFSStandardMultiLoop";
        case ProductTimeSpecShapeKind::IFSFakeSingleLoopDoubleLoop:
            return "IFSFakeSingleLoopDoubleLoop";
    }

    return "InvalidShapeKind";
}

/// @brief Serialize one resolved shape artifact as diagnostic JSON.
/// @param[in] value Resolved canonical window sequence.
/// @return One JSON object describing the final shape state.
inline std::string productTimeSpecShapeJson(const ProductTimeSpecShape& value) {
    std::ostringstream out;
    out << '{' << detail::jsonQuote_modelInput("windows") << ':' << '[';
    for (std::size_t i = 0; i < value.values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << '{' << detail::jsonQuote_modelInput("typeOfStatisticalProcessing") << ':'
            << detail::jsonQuote_modelInput(
                   tables::enum2name_TypeOfStatisticalProcessing_or_throw(value.values[i].typeOfStatisticalProcessing))
            << ',' << detail::jsonQuote_modelInput("typeOfTimeIncrement") << ':'
            << detail::jsonQuote_modelInput(
                   tables::enum2name_TypeOfTimeIntervals_or_throw(value.values[i].typeOfTimeIncrement))
            << ',' << detail::jsonQuote_modelInput("timeRange") << ':'
            << detail::durationJson_modelInput(value.values[i].timeRange) << ','
            << detail::jsonQuote_modelInput("timeIncrement") << ':'
            << detail::durationJson_modelInput(value.values[i].timeIncrement) << '}';
    }
    out << ']' << '}';
    return out.str();
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::shape
