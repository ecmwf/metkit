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
 * @file ProductTimeSpec.h
 * @brief Canonical temporal intermediate representation used by mars2grib.
 *
 * This header defines the value types, resolver construction artifacts,
 * immutable canonical time-range container, diagnostic exception, and reusable
 * utilities used to resolve the temporal meaning of one MARS product.
 *
 * A resolved ProductTimeSpec is the authoritative temporal representation for
 * downstream point-in-time and statistical backends. Backends must consume the
 * resolved object and must not reinterpret raw MARS temporal keys independently.
 *
 * The resolver follows a classifier-first pipeline:
 *
 * 1. extract and normalize input values;
 * 2. classify the anchor, shape, and increment semantics;
 * 3. construct resolver-only semantic artifacts;
 * 4. materialize canonical ProductTimeWindow objects;
 * 5. validate the final immutable ProductTimeSpec.
 *
 * @note ProductTimeSpecClassification, ProductTimeSpecShape, and
 *       ProductTimeSpecIncrement are resolver orchestration types. They are not
 *       backend input types.
 */
#pragma once

#include <array>
#include <cstddef>
#include <exception>
#include <optional>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"

namespace metkit::mars2grib::product_time_spec {

/** @brief Short alias for the mars2grib GRIB table namespace. */
namespace tables = metkit::mars2grib::backend::tables;

/**
 * @brief GRIB type-of-time-increment value.
 *
 * The ProductTimeSpec terminology follows GRIB table 4.11 and therefore uses
 * the name TypeOfTimeIncrement. The underlying mars2grib table type currently
 * retains the historical name TypeOfTimeIntervals.
 */
using TypeOfTimeIncrement = tables::TypeOfTimeIntervals;

/**
 * @brief Number of canonical time windows stored without dynamic allocation.
 *
 * Three windows cover the currently supported domain: up to two outer
 * `stattype` windows and one innermost `timespan` window. The containers remain
 * structurally capable of representing larger sequences through overflow
 * storage.
 */
inline constexpr std::size_t inlineProductTimeWindows = 3;

/**
 * @brief Default time used when a MARS date source has no explicit time.
 *
 * The same midnight value is used when resolving a label date without `time`,
 * constructing initial conditions from `hdate`, and constructing a reference
 * anchor from `year` and `month`.
 */
inline const eckit::Time defaultMarsTime{0, 0, 0};

/**
 * @brief Unit-qualified duration used by the canonical temporal model.
 *
 * ProductTimeSpec distinguishes elapsed durations from calendar durations.
 * Seconds and hours are elapsed durations; days and months are calendar-based
 * durations when produced from `stattype`.
 *
 * Values produced by the resolver use a non-negative length. Zero durations
 * are canonicalized as zero seconds.
 */
struct ProductTimeDuration {
    /** @brief GRIB-compatible unit of the duration. */
    tables::TimeUnit unit{tables::TimeUnit::Second};

    /** @brief Number of units in the duration. */
    long length{0};
};

/**
 * @brief Compare two durations by their canonical representation.
 *
 * This is structural equality of the `(unit, length)` pair; it is not a
 * placement-dependent semantic comparison between calendar and elapsed
 * durations.
 */
bool operator==(const ProductTimeDuration&, const ProductTimeDuration&) noexcept;

/** @brief Return the negation of ProductTimeDuration structural equality. */
bool operator!=(const ProductTimeDuration&, const ProductTimeDuration&) noexcept;

/**
 * @brief Canonical GRIB statistical time-range tuple.
 *
 * Each window owns the complete per-range information required by GRIB
 * statistical product definitions: processing type, time-increment type,
 * range length, and sampling increment.
 *
 * Canonical windows are ordered from outermost to innermost. Instant products
 * contain one normalization-only placeholder with missing processing and
 * increment types and zero-second durations.
 */
struct ProductTimeWindow {
    /** @brief Statistical operation applied over this window. */
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};

    /** @brief GRIB interpretation of the time increment for this window. */
    TypeOfTimeIncrement typeOfTimeIncrement{TypeOfTimeIncrement::Missing};

    /** @brief Length of the statistical window. */
    ProductTimeDuration timeRange{};

    /** @brief Increment between fields contributing to this window. */
    ProductTimeDuration timeIncrement{};
};

/**
 * @brief One normalized block parsed from the MARS `stattype` value.
 *
 * A block contains both the period-derived outer time range and the
 * operation-derived statistical processing type. Blocks are retained in MARS
 * textual order, which is outermost to innermost.
 */
struct ParsedStatTypeBlock {
    /** @brief Period represented by this `stattype` block. */
    ProductTimeDuration timeRange{};

    /** @brief Statistical operation represented by this `stattype` block. */
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};
};

/**
 * @brief Small-buffer sequence of parsed `stattype` blocks.
 *
 * The currently supported source-language domain fits in the inline storage.
 * Dynamic storage permits future or independently validated inputs to contain
 * more blocks without changing the type.
 *
 * The sequence is mutable only while the resolver is constructing its internal
 * artifacts. Consumers receive it through const access.
 */
class ParsedStatTypeBlocks {
public:
    using value_type = ParsedStatTypeBlock;
    using const_iterator = const value_type*;

    /** @brief Return the number of parsed blocks. */
    std::size_t size() const noexcept;

    /** @brief Return whether the sequence contains no blocks. */
    bool empty() const noexcept;

    /**
     * @brief Return a block without bounds checking.
     * @param i Zero-based block index.
     * @pre `i < size()`.
     */
    const value_type& operator[](std::size_t i) const noexcept;

    /**
     * @brief Return a block with bounds checking.
     * @param i Zero-based block index.
     * @throws An exception if `i >= size()`.
     */
    const value_type& at(std::size_t i) const;

    /** @brief Return an iterator to the first parsed block. */
    const_iterator begin() const noexcept;

    /** @brief Return the past-the-end iterator. */
    const_iterator end() const noexcept;

    /**
     * @brief Append one parsed block during resolver construction.
     * @param value Block to append after the current innermost block.
     *
     * This mutator is part of the resolver construction API. Parsed blocks are
     * exposed immutably once incorporated into a resolved artifact.
     */
    void append(const value_type& value);

private:
    /** @brief Return the base address of the active contiguous representation. */
    const value_type* data() const noexcept;

    std::array<value_type, inlineProductTimeWindows> inline_{};
    std::vector<value_type> overflow_{};
    std::size_t size_{0};
};

/** @brief Normalized semantic category of the MARS `timespan` key. */
enum class TimespanKind {
    /** The key is absent. */
    Missing,

    /** The key contains a supported positive duration. */
    Duration,

    /** The key is the literal `none`. */
    None,

    /** The key is `fs` or another supported from-start alias. */
    FromStart
};

/**
 * @brief Source regime used to construct the three canonical anchor datetimes.
 *
 * The kind is determined by the presence of direct `hdate` and `year`/`month`
 * sources. The ordinary `date`/`time` source participates in inheritance but
 * does not create another anchor kind.
 */
enum class TimeAnchorKind : std::size_t {
    /** No direct hindcast or year/month reference anchor is present. */
    LabelOnly,

    /** A direct hindcast date is present, without a year/month anchor. */
    Hindcast,

    /** A direct year/month reference anchor is present, without `hdate`. */
    ForecastAnchor,

    /** Both a direct hindcast date and a year/month reference anchor exist. */
    HindcastForecastAnchor,

    /** Number of valid values; reserved for table sizing and validation. */
    Count
};

/** @brief Structural classification of the product's temporal support. */
enum class ProductTimeSpecShapeKind : std::size_t {
    /** Point-in-time product with no real statistical window. */
    Instant,

    /** One statistical window represented by a duration-valued `timespan`. */
    StandardSingleLoop,

    /** Outer `stattype` windows followed by an innermost `timespan` window. */
    MultiLoop,

    /** One `stattype` block promoted to the only canonical statistical window. */
    FakeDoubleLoopSingleLoop,

    /** One statistical window extending from the reference time to the step. */
    FromStartSingleLoop,

    /** Number of valid values; reserved for table sizing and validation. */
    Count
};

/** @brief Classification of the innermost sampling-increment semantics. */
enum class TimeIncrementKind : std::size_t {
    /** Instant product; no real time increment exists. */
    NoIncrement,

    /** A positive source increment is present and semantically used. */
    ExplicitIncrement,

    /** A positive increment is supplied by explicit option-driven policy. */
    DefaultedIncrement,

    /** A single-window `class=ml` product with a semantically missing increment. */
    AifsPureMissingIncrement,

    /** Number of valid values; reserved for table sizing and validation. */
    Count
};

/**
 * @brief Final product kind retained by the canonical IR.
 *
 * The final kind is currently semantically identical to the frontend shape
 * classification.
 */
using ProductTimeSpecKind = ProductTimeSpecShapeKind;

/**
 * @brief Snapshot of policy options used to resolve a ProductTimeSpec.
 *
 * The options are retained in the final object so diagnostics can explain the
 * policy under which the canonical representation was produced.
 */
struct ProductTimeSpecOptions {
    /**
     * Permit an eligible missing statistical increment to be supplied from
     * `defaultTimeIncrementInSeconds`.
     */
    bool allowDefaultTimeIncrementInSeconds{false};

    /** Permit the explicitly statistical zero-length from-start case. */
    bool allowZeroLengthFsWindow{false};

    /** Permit positive integer-hour `timespan` values outside the whitelist. */
    bool allowNonEnumeratedPositiveIntegerTimespanHours{false};

    /**
     * Permit an explicit source increment to be ignored where the semantic
     * model requires the missing-increment sentinel: instant products and
     * AIFS-pure single-window products.
     */
    bool allowRedundantTimeIncrement{false};

    /**
     * Accept a missing `timespan` as a compatibility representation of an
     * instant product. The normative instant representation is
     * `timespan=none` with no `stattype` blocks.
     */
    bool allowMissingTimespanForInstantProduct{false};

    /**
     * Non-missing GRIB type-of-time-increment used for real explicit or
     * defaulted increments when the value is not source-derived.
     */
    // TypeOfTimeIncrement defaultTypeOfTimeIncrement{TypeOfTimeIncrement::Missing};
};

/**
 * @brief Valid three-axis classification produced by the resolver.
 *
 * This is transient orchestration state. Invalid combinations throw before this
 * structure is produced, and downstream temporal backends must not consume it.
 */
struct ProductTimeSpecClassification {
    /** @brief Resolved anchor-source regime. */
    TimeAnchorKind anchorType{TimeAnchorKind::LabelOnly};

    /** @brief Resolved temporal-support shape. */
    ProductTimeSpecShapeKind shapeType{ProductTimeSpecShapeKind::Instant};

    /** @brief Resolved innermost increment regime. */
    TimeIncrementKind incrementType{TimeIncrementKind::NoIncrement};
};

/**
 * @brief Resolved anchor datetimes used by all temporal arithmetic.
 *
 * Valid anchors satisfy:
 *
 * `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
 */
struct ProductTimeSpecAnchor {
    /** @brief Datetime represented by the product label. */
    eckit::DateTime labelDateTime{};

    /** @brief Datetime of the simulation initial conditions. */
    eckit::DateTime initialConditionsDateTime{};

    /** @brief Datetime from which forecast-step arithmetic is performed. */
    eckit::DateTime referenceDateTime{};

    /** @brief Source regime used to construct the datetimes. */
    TimeAnchorKind anchorType{TimeAnchorKind::LabelOnly};
};

/**
 * @brief Resolver-only description of the absolute support and window sources.
 *
 * Shape construction determines where the product support starts and ends and
 * identifies the source ranges from which canonical ProductTimeWindow objects
 * will later be materialized. It does not itself build the final windows.
 */
struct ProductTimeSpecShape {
    /** @brief Start of the outermost temporal support interval. */
    eckit::DateTime windowStartDateTime{};

    /** @brief End of the outermost temporal support interval. */
    eckit::DateTime windowEndDateTime{};

    /**
     * @brief Innermost range derived from `timespan` or resolved step.
     *
     * Present for standard single-loop, multi-loop, and from-start products;
     * absent for instant and fake-double-loop products.
     */
    std::optional<ProductTimeDuration> innerTimeRange{};

    /** @brief Parsed outer ranges in outermost-to-innermost order. */
    ParsedStatTypeBlocks stattypeBlocks{};

    /**
     * @brief Construction-time evidence that a zero-length from-start range is
     * intentional and option-authorized.
     *
     * This flag is not retained in the final ProductTimeSpec; the final state is
     * derived from the canonical object and its option snapshot.
     */
    bool zeroLengthFromStartWindowByDesign{false};

    /** @brief Resolved support shape. */
    ProductTimeSpecShapeKind shapeType{ProductTimeSpecShapeKind::Instant};
};

/**
 * @brief Resolver-only materialization of innermost increment semantics.
 *
 * Explicit and defaulted increments contain a positive duration and a
 * non-missing type. NoIncrement and AifsPureMissingIncrement use the canonical
 * missing sentinel: zero seconds and a missing type.
 */
struct ProductTimeSpecIncrement {
    /** @brief Resolved increment duration for the innermost canonical window. */
    ProductTimeDuration timeIncrement{};

    /** @brief Resolved GRIB interpretation of the increment. */
    TypeOfTimeIncrement typeOfTimeIncrement{TypeOfTimeIncrement::Missing};

    /** @brief Semantic origin and use of the increment. */
    TimeIncrementKind incrementType{TimeIncrementKind::NoIncrement};
};

/**
 * @brief Small-buffer immutable sequence of canonical time windows.
 *
 * Windows are ordered from outermost to innermost. The sequence always contains
 * at least one element in a valid ProductTimeSpec. For an instant product, that
 * element is the zero-length normalization placeholder.
 *
 * Mutation is restricted to resolver construction through append(). Once held
 * by ProductTimeSpec, only const access is exposed.
 */
class ProductTimeWindows {
public:
    using value_type = ProductTimeWindow;
    using const_iterator = const value_type*;

    /** @brief Return the number of canonical windows. */
    std::size_t size() const noexcept;

    /**
     * @brief Return the number of canonical GRIB time-range records.
     *
     * This is an API-level synonym for size(). It mirrors the ecCodes
     * `numberOfTimeRanges` concept at statistical lowering call sites.
     */
    std::size_t numberOfTimeRanges() const noexcept;

    /** @brief Return whether no windows have been appended. */
    bool empty() const noexcept;

    /**
     * @brief Return a canonical window without bounds checking.
     * @param i Zero-based index in outermost-to-innermost order.
     * @pre `i < size()`.
     */
    const value_type& operator[](std::size_t i) const noexcept;

    /**
     * @brief Return a canonical window with bounds checking.
     * @param i Zero-based index in outermost-to-innermost order.
     * @throws An exception if `i >= size()`.
     */
    const value_type& at(std::size_t i) const;

    /** @brief Return an iterator to the outermost canonical window. */
    const_iterator begin() const noexcept;

    /** @brief Return the past-the-end iterator. */
    const_iterator end() const noexcept;

    /**
     * @brief Append one canonical window during resolver construction.
     * @param value Window to append after the current innermost window.
     */
    void append(const value_type& value);

private:
    /** @brief Return the base address of the active contiguous representation. */
    const value_type* data() const noexcept;

    std::array<value_type, inlineProductTimeWindows> inline_{};
    std::vector<value_type> overflow_{};
    std::size_t size_{0};
};

/**
 * @brief Immutable canonical temporal representation of one MARS product.
 *
 * ProductTimeSpec contains the resolved anchor, absolute outer support interval,
 * canonical time windows, policy snapshot, shape kind, and increment kind.
 * It is the only temporal representation consumed by point-in-time and
 * statistical lowering code.
 *
 * Valid instances satisfy the final ProductTimeSpec invariants, including:
 *
 * - ordered anchor datetimes;
 * - a non-empty canonical window sequence;
 * - outermost-to-innermost window ordering;
 * - support consistency with the outermost range;
 * - shape-specific range cardinality;
 * - valid per-window statistical processing and increment semantics.
 */
class ProductTimeSpec {
public:
    /**
     * @brief Construct a canonical candidate from fully resolved components.
     *
     * @param anchor Resolved anchor datetimes and anchor kind.
     * @param windowStartDateTime Start of the outermost product support.
     * @param windowEndDateTime End of the outermost product support.
     * @param windows Canonical windows in outermost-to-innermost order.
     * @param options Snapshot of the policy used during resolution.
     * @param kind Resolved temporal-support shape.
     * @param incrementKind Resolved innermost increment semantics.
     *
     * @note This constructor is intended for the resolver. Construction does not
     *       replace the resolver's final whole-object consistency check.
     */
    ProductTimeSpec(ProductTimeSpecAnchor anchor,
                    eckit::DateTime windowStartDateTime,
                    eckit::DateTime windowEndDateTime,
                    ProductTimeWindows windows,
                    ProductTimeSpecOptions options,
                    ProductTimeSpecKind kind,
                    TimeIncrementKind incrementKind);

    /** @brief Return the datetime represented by the product label. */
    const eckit::DateTime& labelDateTime() const noexcept;

    /** @brief Return the simulation initial-conditions datetime. */
    const eckit::DateTime& initialConditionsDateTime() const noexcept;

    /** @brief Return the datetime from which step arithmetic is performed. */
    const eckit::DateTime& referenceDateTime() const noexcept;

    /** @brief Return the anchor-source regime. */
    TimeAnchorKind anchorType() const noexcept;

    /** @brief Return the complete resolved anchor object. */
    const ProductTimeSpecAnchor& anchor() const noexcept;

    /** @brief Return the start of the outermost temporal support interval. */
    const eckit::DateTime& windowStartDateTime() const noexcept;

    /** @brief Return the end of the outermost temporal support interval. */
    const eckit::DateTime& windowEndDateTime() const noexcept;

    /**
     * @brief Return the number of canonical time-range records.
     *
     * Instant products return one because their canonical IR contains one
     * zero-length placeholder. Point-in-time lowering must not encode that
     * placeholder as a statistical range.
     */
    std::size_t numberOfTimeRanges() const noexcept;

    /** @brief Return the same cardinality as numberOfTimeRanges(). */
    std::size_t size() const noexcept;

    /**
     * @brief Return a canonical window without bounds checking.
     * @param i Zero-based index in outermost-to-innermost order.
     * @pre `i < size()`.
     */
    const ProductTimeWindow& operator[](std::size_t i) const noexcept;

    /**
     * @brief Return a canonical window with bounds checking.
     * @param i Zero-based index in outermost-to-innermost order.
     * @throws An exception if `i >= size()`.
     */
    const ProductTimeWindow& at(std::size_t i) const;

    /** @brief Return the complete immutable canonical window sequence. */
    const ProductTimeWindows& timeRanges() const noexcept;

    /** @brief Return an iterator to the outermost canonical window. */
    ProductTimeWindows::const_iterator begin() const noexcept;

    /** @brief Return the past-the-end iterator. */
    ProductTimeWindows::const_iterator end() const noexcept;

    /** @brief Return the policy snapshot used to resolve this object. */
    const ProductTimeSpecOptions& options() const noexcept;

    /** @brief Return the resolved temporal-support shape. */
    ProductTimeSpecKind kind() const noexcept;

    /** @brief Return the resolved innermost increment semantics. */
    TimeIncrementKind incrementKind() const noexcept;

    /**
     * @brief Serialize the complete canonical object as diagnostic JSON.
     *
     * The output includes the anchor, support interval, shape and increment
     * kinds, canonical windows, and option snapshot. Unused inline storage slots
     * are never serialized.
     */
    std::string to_json() const;

private:
    ProductTimeSpecAnchor anchor_;
    eckit::DateTime windowStartDateTime_;
    eckit::DateTime windowEndDateTime_;
    ProductTimeWindows windows_;
    ProductTimeSpecOptions options_;
    ProductTimeSpecKind kind_;
    TimeIncrementKind incrementKind_;
};

/** @brief Resolver stage at which a ProductTimeSpec failure occurred. */
enum class ProductTimeSpecStage {
    /** Reading, parsing, or normalizing source dictionaries and options. */
    InputExtraction,

    /** Classifying the anchor-source regime. */
    TimeAnchorClassification,

    /** Classifying the temporal-support shape. */
    ShapeClassification,

    /** Classifying the sampling-increment semantics. */
    TimeIncrementClassification,

    /** Checking compatibility between independently valid classifications. */
    ClassificationConsistencyCheck,

    /** Constructing the three resolved anchor datetimes. */
    TimeAnchorConstruction,

    /** Constructing the absolute support and structural window sources. */
    ShapeConstruction,

    /** Materializing the innermost increment policy. */
    TimeIncrementConstruction,

    /** Materializing the canonical ProductTimeWindow sequence. */
    CanonicalWindowConstruction,

    /** Assembling the immutable ProductTimeSpec candidate. */
    ProductTimeSpecConstruction,

    /** Validating invariants on the complete canonical candidate. */
    FinalConsistencyCheck
};

/**
 * @brief Internal exception carrying stage-specific ProductTimeSpec diagnostics.
 *
 * This exception is used after a normalized input snapshot exists. It stores
 * the resolver stage, human-readable reason, normalized input JSON, and any
 * classification, construction-artifact, or final-object JSON available at the
 * point of failure.
 *
 * The public deduction boundary is expected to wrap this exception while
 * preserving the nested exception chain.
 */
class Mars2GribProductTimeSpecException : public eckit::Exception,
                                         public std::nested_exception {
public:
    /**
     * @brief Construct a stage-tagged ProductTimeSpec exception.
     *
     * @param stage Resolver stage that failed.
     * @param reason Human-readable failure description.
     * @param inputJson Normalized ProductTimeSpec input snapshot.
     * @param classificationJson Resolved classifications, when available.
     * @param artifactJson Construction artifacts already produced, when available.
     * @param finalSpecJson Final canonical candidate, when available.
     * @param loc Source location at which the exception is created.
     */
    Mars2GribProductTimeSpecException(ProductTimeSpecStage stage,
                                      std::string reason,
                                      std::string inputJson,
                                      std::string classificationJson = {},
                                      std::string artifactJson = {},
                                      std::string finalSpecJson = {},
                                      const eckit::CodeLocation& loc = eckit::CodeLocation());

    /** @brief Return the resolver stage that failed. */
    ProductTimeSpecStage stage() const noexcept;

    /** @brief Return the normalized input JSON snapshot. */
    const std::string& inputJson() const noexcept;

    /** @brief Return classification JSON, or an empty string if unavailable. */
    const std::string& classificationJson() const noexcept;

    /** @brief Return construction-artifact JSON, or an empty string if unavailable. */
    const std::string& artifactJson() const noexcept;

    /** @brief Return final-candidate JSON, or an empty string if unavailable. */
    const std::string& finalSpecJson() const noexcept;

private:
    ProductTimeSpecStage stage_;
    std::string inputJson_;
    std::string classificationJson_;
    std::string artifactJson_;
    std::string finalSpecJson_;
};

/**
 * @brief Quote and escape a string as one JSON string value.
 * @param value Unescaped input text.
 * @return JSON text including the surrounding quotation marks.
 */
std::string jsonQuote(const std::string& value);

/** @brief Return the stable diagnostic name of a TimespanKind value. */
std::string name(TimespanKind);

/** @brief Return the stable diagnostic name of a TimeAnchorKind value. */
std::string name(TimeAnchorKind);

/** @brief Return the stable diagnostic name of a ProductTimeSpecShapeKind value. */
std::string name(ProductTimeSpecShapeKind);

/** @brief Return the stable diagnostic name of a TimeIncrementKind value. */
std::string name(TimeIncrementKind);

/** @brief Return the stable diagnostic name of a ProductTimeSpecStage value. */
std::string name(ProductTimeSpecStage);

/** @brief Return the stable diagnostic name of a GRIB time unit. */
std::string name(tables::TimeUnit);

/** @brief Return the stable diagnostic name of a statistical processing type. */
std::string name(tables::TypeOfStatisticalProcessing);

/** @brief Return the stable diagnostic name of a type-of-time-increment value. */
std::string name(TypeOfTimeIncrement);

/**
 * @brief Canonicalize a non-negative elapsed duration expressed in seconds.
 *
 * Positive whole-hour values are represented in hours. Zero and values not
 * exactly representable as whole hours are represented in seconds. Calendar
 * day and month units are never introduced by this function.
 *
 * @param seconds Elapsed duration in seconds.
 * @return Canonical elapsed duration.
 * @pre `seconds >= 0`.
 */
ProductTimeDuration canonicalElapsedDuration(long seconds);

/**
 * @brief Test whether a datetime is exactly at 00:00:00.
 * @param value Datetime to inspect.
 */
bool isAtMidnight(const eckit::DateTime& value);

/**
 * @brief Test whether a datetime is on the first day of a month at 00:00:00.
 * @param value Datetime to inspect.
 */
bool isOnFirstOfMonthMidnight(const eckit::DateTime& value);

/**
 * @brief Add a unit-aware ProductTimeDuration to a datetime.
 *
 * Seconds and hours use elapsed-time arithmetic. Days and months use calendar
 * arithmetic and preserve their calendar semantics.
 *
 * @param value Starting datetime.
 * @param duration Duration to add.
 * @return Resulting datetime.
 */
eckit::DateTime addDuration(const eckit::DateTime& value,
                            const ProductTimeDuration& duration);

/**
 * @brief Subtract a unit-aware ProductTimeDuration from a datetime.
 *
 * Seconds and hours use elapsed-time arithmetic. Days and months use calendar
 * arithmetic and preserve their calendar semantics.
 *
 * @param value Ending datetime.
 * @param duration Duration to subtract.
 * @return Resulting datetime.
 */
eckit::DateTime subtractDuration(const eckit::DateTime& value,
                                 const ProductTimeDuration& duration);

/**
 * @brief Check whether an increment fits within a range at a realized start.
 *
 * The comparison is semantic and placement-aware. It compares
 * `realizedStart + increment` with `realizedStart + range`, using elapsed or
 * calendar arithmetic according to each duration's unit.
 *
 * @param realizedStart Absolute start at which both durations are realized.
 * @param increment Sampling increment to test.
 * @param range Statistical window length.
 * @return `true` when the increment endpoint is not later than the range endpoint.
 */
bool durationFitsAt(const eckit::DateTime& realizedStart,
                    const ProductTimeDuration& increment,
                    const ProductTimeDuration& range);

/**
 * @brief Count real statistical windows before canonical IR construction.
 *
 * Semantics by shape:
 *
 * - Instant: 0;
 * - StandardSingleLoop: 1;
 * - FakeDoubleLoopSingleLoop: 1;
 * - FromStartSingleLoop: 1, including an authorized zero-length window;
 * - MultiLoop: `stattypeBlockCount + 1`.
 *
 * @param shapeType Resolved shape classification.
 * @param stattypeBlockCount Number of parsed outer `stattype` blocks.
 * @return Number of real statistical windows represented by the shape.
 */
std::size_t realStatisticalWindowCount(ProductTimeSpecShapeKind shapeType,
                                       std::size_t stattypeBlockCount);

/**
 * @brief Count real statistical windows in a final canonical object.
 *
 * The instant normalization placeholder is excluded. Every canonical window of
 * a non-instant product is real, including an authorized zero-length from-start
 * window.
 *
 * @param spec Final canonical ProductTimeSpec.
 * @return Zero for an instant product; otherwise `spec.numberOfTimeRanges()`.
 */
std::size_t realStatisticalWindowCount(const ProductTimeSpec& spec);

/**
 * @brief Test whether a class/stream pair requires FakeDoubleLoop representation.
 *
 * This function implements only the representation-policy lookup. Shape
 * classification remains responsible for validating the accompanying
 * `timespan` and `stattype` structure.
 *
 * The current policy includes:
 *
 * - `e6`: `sttd`, `stte`;
 * - `od`, `rd`, `c3`: `sfmd`, `shmd`;
 * - `gh`, `eh`: `msmm`, `rfsd`.
 *
 * @param marsClass Normalized MARS class.
 * @param marsStream Normalized MARS stream.
 * @return `true` when the pair must use FakeDoubleLoop single-loop representation.
 */
bool requiresFakeDoubleLoopRepresentation(const std::string& marsClass,
                                          const std::string& marsStream);

}  // namespace metkit::mars2grib::product_time_spec
