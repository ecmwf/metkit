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
 * @file ProductTimeSpec.cpp
 * @brief Implementation of the canonical ProductTimeSpec temporal model.
 *
 * This translation unit is intentionally documented independently of
 * ProductTimeSpec.h. Every function repeats its complete local contract so that
 * an implementation reader does not need to switch to the declaration file to
 * understand purpose, parameters, return values, preconditions, exceptions, or
 * invariants.
 *
 * The implementation provides:
 *
 * - checked elapsed-time and calendar-time arithmetic;
 * - small-buffer containers for parsed `stattype` blocks and canonical windows;
 * - immutable ProductTimeSpec accessors and diagnostic JSON serialization;
 * - stage-aware ProductTimeSpec exceptions;
 * - stable textual names for temporal and GRIB enumerations;
 * - real-statistical-window counting and representation-policy utilities.
 *
 * Duration arithmetic distinguishes two semantic families:
 *
 * - seconds and hours are elapsed durations;
 * - days and months are calendar durations and therefore require aligned
 *   DateTime values.
 *
 * Canonical windows are stored in outermost-to-innermost order. Instant products
 * contain one zero-length placeholder window in the canonical IR, but have zero
 * real statistical windows.
 */

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"

#include <cassert>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace metkit::mars2grib::product_time_spec {
namespace {

/** Number of elapsed seconds in one hour. */
constexpr long secondsPerHour = 3600L;
/** Number of elapsed seconds in a civil 24-hour day. */
constexpr long secondsPerDay = 24L * secondsPerHour;

/**
 * @brief Multiply two non-negative duration factors with overflow checking.
 *
 * This helper is used when converting a unit-qualified duration to elapsed
 * seconds. The ProductTimeSpec arithmetic paths pass non-negative values, so the
 * overflow check is deliberately specialized for positive multiplication.
 *
 * @param lhs Left multiplication factor.
 * @param rhs Right multiplication factor.
 * @param context Human-readable operation name included in an error message.
 * @return `lhs * rhs`, or zero when either factor is zero.
 *
 * @throws std::overflow_error If both factors are positive and their product
 *         cannot be represented by `long`.
 *
 * @pre `lhs >= 0` and `rhs >= 0` for the intended ProductTimeSpec use cases.
 */
long checkedMultiply(long lhs, long rhs, const char* context) {
    if (lhs == 0 || rhs == 0) {
        return 0;
    }
    if (lhs > 0 && rhs > 0 && lhs > std::numeric_limits<long>::max() / rhs) {
        throw std::overflow_error(std::string(context) + ": duration overflow");
    }
    return lhs * rhs;
}

/**
 * @brief Add a non-negative elapsed number of seconds to a DateTime.
 *
 * The operation is elapsed-time arithmetic. It does not impose calendar
 * alignment requirements and may cross day, month, and year boundaries.
 *
 * @param dt Starting DateTime.
 * @param seconds Non-negative elapsed duration in seconds.
 * @return `dt` shifted forward by `seconds`.
 *
 * @throws std::invalid_argument If `seconds` is negative.
 */
eckit::DateTime addSeconds(const eckit::DateTime& dt, long seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("addSeconds requires a non-negative duration");
    }
    return dt + static_cast<eckit::Second>(seconds);
}

/**
 * @brief Subtract a non-negative elapsed number of seconds from a DateTime.
 *
 * The subtraction is implemented explicitly on the date and second-of-day
 * components because the required eckit operation is not expressed here as a
 * direct negative-second addition. Whole days are removed first; any remaining
 * seconds are then subtracted with an explicit borrow from the preceding day.
 *
 * @param dt Starting DateTime.
 * @param seconds Non-negative elapsed duration in seconds.
 * @return `dt` shifted backward by `seconds`.
 *
 * @throws std::invalid_argument If `seconds` is negative.
 */
eckit::DateTime subtractSeconds(const eckit::DateTime& dt, long seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("subtractSeconds requires a non-negative duration");
    }

    eckit::Date date = dt.date();
    eckit::Second time = dt.time();

    // Split the elapsed duration into a date component and a remainder that
    // can be subtracted from the second-of-day component.
    const long wholeDays = seconds / secondsPerDay;
    const long remainingSeconds = seconds % secondsPerDay;
    date -= wholeDays;

    // Borrow one civil day when the current second-of-day is smaller than
    // the remaining elapsed-second subtraction.
    if (time < static_cast<eckit::Second>(remainingSeconds)) {
        date -= 1;
        time += static_cast<eckit::Second>(secondsPerDay);
    }

    time -= static_cast<eckit::Second>(remainingSeconds);
    return eckit::DateTime(date, eckit::Time(time));
}

/**
 * @brief Shift a month-aligned DateTime by a signed number of calendar months.
 *
 * Calendar-month arithmetic is defined only for the first day of a month at
 * midnight. The implementation converts `(year, month)` to a linear month index,
 * applies the signed displacement, normalizes negative remainders, and rebuilds
 * the first day of the resulting month at `defaultMarsTime`.
 *
 * @param dt Month-aligned starting DateTime.
 * @param months Signed calendar-month displacement. Positive values move
 *        forward and negative values move backward.
 * @return The first day of the shifted month at midnight.
 *
 * @throws std::invalid_argument If `dt` is not the first day of a month at
 *         `00:00:00`.
 */
eckit::DateTime shiftCalendarMonths(const eckit::DateTime& dt, long months) {
    if (!isOnFirstOfMonthMidnight(dt)) {
        throw std::invalid_argument(
            "calendar-month arithmetic requires day=1 at 00:00:00");
    }

    const long year = dt.date().year();
    const long monthIndex = dt.date().month() - 1L;
    // Linearizing the calendar month permits the same arithmetic for positive
    // and negative shifts; the remainder is normalized below for C++ negative
    // modulo behavior.
    const long total = year * 12L + monthIndex + months;

    long newYear = total / 12L;
    long newMonthIndex = total % 12L;
    if (newMonthIndex < 0) {
        newMonthIndex += 12L;
        --newYear;
    }

    return eckit::DateTime(eckit::Date(newYear, newMonthIndex + 1L, 1L),
                           defaultMarsTime);
}

/**
 * @brief Append one already-serialized JSON object member.
 *
 * The helper owns comma placement between members. It writes the member name as
 * a JSON string and writes `value` through its stream insertion operator.
 * Callers must therefore pass a value that is already valid JSON when string
 * quoting or object/array syntax is required.
 *
 * @tparam Value Stream-insertable serialized value type.
 * @param out Destination stream containing the JSON object body.
 * @param first Mutable flag that is true before the first member is written.
 *        The function updates it to false.
 * @param key Unescaped member name; this function applies JSON quoting.
 * @param value JSON representation written after the member-name colon.
 */
template <typename Value>
void appendJsonMember(std::ostringstream& out,
                      bool& first,
                      const char* key,
                      const Value& value) {
    if (!first) {
        out << ',';
    }
    first = false;
    out << jsonQuote(key) << ':' << value;
}

/**
 * @brief Serialize an optional integer as a JSON scalar.
 *
 * @param value Optional integer value.
 * @return The decimal integer representation when present, otherwise the JSON
 *         literal `null`.
 *
 * @note The returned string is complete JSON syntax and must not be quoted by
 *       the caller.
 */
std::string optionalLongJson(const std::optional<long>& value) {
    return value ? std::to_string(*value) : "null";
}

/**
 * @brief Serialize a ProductTimeDuration as a compact JSON object.
 *
 * The object contains the stable textual unit name and the integral length:
 * `{"unit":"...","length":N}`.
 *
 * @param value Duration to serialize.
 * @return A complete compact JSON object.
 */
std::string durationJson(const ProductTimeDuration& value) {
    std::ostringstream out;
    out << '{' << jsonQuote("unit") << ':' << jsonQuote(name(value.unit)) << ','
        << jsonQuote("length") << ':' << value.length << '}';
    return out.str();
}

}  // namespace

/**
 * @brief Compare two ProductTimeDuration values structurally.
 *
 * Equality requires both the GRIB unit and the stored length to match. This is
 * not a semantic conversion: for example, one hour and 3600 seconds compare
 * unequal unless both have first been canonicalized to the same representation.
 *
 * @param lhs Left duration.
 * @param rhs Right duration.
 * @return True when both `(unit, length)` pairs are identical.
 */
bool operator==(const ProductTimeDuration& lhs,
                const ProductTimeDuration& rhs) noexcept {
    return lhs.unit == rhs.unit && lhs.length == rhs.length;
}

/**
 * @brief Compare two ProductTimeDuration values for structural inequality.
 *
 * @param lhs Left duration.
 * @param rhs Right duration.
 * @return The logical negation of `operator==`.
 */
bool operator!=(const ProductTimeDuration& lhs,
                const ProductTimeDuration& rhs) noexcept {
    return !(lhs == rhs);
}

/**
 * @brief Return the number of parsed `stattype` blocks.
 *
 * The count is independent of whether storage currently resides in the inline
 * array or in the dynamically allocated overflow vector.
 *
 * @return Number of valid elements in outermost-to-innermost order.
 */
std::size_t ParsedStatTypeBlocks::size() const noexcept {
    return size_;
}

/**
 * @brief Test whether no parsed `stattype` blocks are stored.
 *
 * @return True exactly when `size() == 0`.
 */
bool ParsedStatTypeBlocks::empty() const noexcept {
    return size_ == 0;
}

/**
 * @brief Access a parsed `stattype` block without runtime bounds checking.
 *
 * In assertion-enabled builds, an invalid index triggers the local assertion.
 * In assertion-disabled builds, violating the precondition results in undefined
 * behavior, as for other unchecked sequence accessors.
 *
 * @param i Zero-based index in outermost-to-innermost order.
 * @return Const reference to the selected block.
 *
 * @pre `i < size()`.
 */
const ParsedStatTypeBlock& ParsedStatTypeBlocks::operator[](std::size_t i) const noexcept {
    assert(i < size_);
    return data()[i];
}

/**
 * @brief Access a parsed `stattype` block with bounds checking.
 *
 * @param i Zero-based index in outermost-to-innermost order.
 * @return Const reference to the selected block.
 *
 * @throws std::out_of_range If `i >= size()`.
 */
const ParsedStatTypeBlock& ParsedStatTypeBlocks::at(std::size_t i) const {
    if (i >= size_) {
        throw std::out_of_range("ParsedStatTypeBlocks index out of range");
    }
    return data()[i];
}

/**
 * @brief Return an iterator to the outermost parsed `stattype` block.
 *
 * The iterator is a pointer into the active contiguous representation. It
 * remains valid until the container is mutated or destroyed.
 *
 * @return Iterator to the first element; equal to `end()` when empty.
 */
ParsedStatTypeBlocks::const_iterator ParsedStatTypeBlocks::begin() const noexcept {
    return data();
}

/**
 * @brief Return the past-the-end iterator for parsed `stattype` blocks.
 *
 * @return `begin() + size()` for the active contiguous representation.
 */
ParsedStatTypeBlocks::const_iterator ParsedStatTypeBlocks::end() const noexcept {
    return data() + size_;
}

/**
 * @brief Append one parsed `stattype` block during resolver construction.
 *
 * Elements are appended in outermost-to-innermost order. Up to
 * `inlineProductTimeWindows` elements are stored directly in `inline_`. On the
 * first overflow, all existing inline elements are copied into `overflow_`
 * before the new value is appended. This migration preserves the contiguous
 * pointer-iterator contract of `begin()`, `end()`, and `data()`.
 *
 * @param value Block to append after the current innermost block.
 *
 * @throws std::bad_alloc If vector allocation fails during migration or growth.
 * @throws Any exception propagated by copying `value_type` into the vector.
 *
 * @post `size()` is increased by one and all elements remain contiguous in the
 *       active representation.
 */
void ParsedStatTypeBlocks::append(const value_type& value) {
    // Stay allocation-free while the currently supported source domain fits
    // in the inline capacity.
    if (overflow_.empty() && size_ < inline_.size()) {
        inline_[size_++] = value;
        return;
    }

    // The first overflowing append migrates the complete inline prefix. Keeping
    // all elements in one vector is required because the public iterator type is
    // a raw pointer and therefore cannot span split storage.
    if (overflow_.empty()) {
        overflow_.reserve(inline_.size() * 2);
        overflow_.insert(overflow_.end(), inline_.begin(), inline_.begin() + size_);
    }

    overflow_.push_back(value);
    ++size_;
}

/**
 * @brief Return the base pointer of the active contiguous block storage.
 *
 * Before overflow, the active representation is `inline_`. Once migration has
 * occurred, `overflow_` contains every element, including those originally held
 * inline, and becomes the active representation permanently.
 *
 * @return Pointer to the first storage slot of the active representation.
 */
const ParsedStatTypeBlock* ParsedStatTypeBlocks::data() const noexcept {
    return overflow_.empty() ? inline_.data() : overflow_.data();
}

/**
 * @brief Return the number of canonical ProductTimeWindow records.
 *
 * The canonical IR is non-empty after successful final construction. During
 * resolver assembly, however, this mutable construction container may
 * temporarily be empty.
 *
 * @return Number of stored canonical records, including the instant-product
 *         placeholder when present.
 */
std::size_t ProductTimeWindows::size() const noexcept {
    return size_;
}

/**
 * @brief Return the canonical GRIB `numberOfTimeRanges` cardinality.
 *
 * This is a semantic synonym for `size()`. It counts canonical records, not real
 * statistical windows: an instant product contains one canonical placeholder
 * record but zero real statistical windows.
 *
 * @return `size()`.
 */
std::size_t ProductTimeWindows::numberOfTimeRanges() const noexcept {
    return size_;
}

/**
 * @brief Test whether no canonical time windows have been appended.
 *
 * @return True exactly when `size() == 0`.
 */
bool ProductTimeWindows::empty() const noexcept {
    return size_ == 0;
}

/**
 * @brief Access a canonical time window without runtime bounds checking.
 *
 * Windows are indexed in outermost-to-innermost order. In assertion-enabled
 * builds an invalid index triggers the local assertion; otherwise violating the
 * precondition is undefined behavior.
 *
 * @param i Zero-based canonical-window index.
 * @return Const reference to the selected window.
 *
 * @pre `i < size()`.
 */
const ProductTimeWindow& ProductTimeWindows::operator[](std::size_t i) const noexcept {
    assert(i < size_);
    return data()[i];
}

/**
 * @brief Access a canonical time window with bounds checking.
 *
 * @param i Zero-based canonical-window index in outermost-to-innermost order.
 * @return Const reference to the selected window.
 *
 * @throws std::out_of_range If `i >= size()`.
 */
const ProductTimeWindow& ProductTimeWindows::at(std::size_t i) const {
    if (i >= size_) {
        throw std::out_of_range("ProductTimeWindows index out of range");
    }
    return data()[i];
}

/**
 * @brief Return an iterator to the outermost canonical time window.
 *
 * The iterator is a pointer into the active contiguous representation and is
 * invalidated by subsequent mutation of this construction container.
 *
 * @return Iterator to the first window; equal to `end()` when empty.
 */
ProductTimeWindows::const_iterator ProductTimeWindows::begin() const noexcept {
    return data();
}

/**
 * @brief Return the past-the-end canonical-window iterator.
 *
 * @return `begin() + size()` for the active contiguous representation.
 */
ProductTimeWindows::const_iterator ProductTimeWindows::end() const noexcept {
    return data() + size_;
}

/**
 * @brief Append one canonical window during resolver construction.
 *
 * Windows must be appended in canonical outermost-to-innermost order. Up to
 * `inlineProductTimeWindows` records use inline storage. On first overflow, all
 * existing records are migrated to `overflow_`, after which the vector remains
 * the active contiguous representation.
 *
 * @param value Canonical window to append after the current innermost window.
 *
 * @throws std::bad_alloc If vector allocation fails during migration or growth.
 * @throws Any exception propagated by copying `value_type` into the vector.
 *
 * @post `size()` is increased by one and pointer iteration remains contiguous.
 */
void ProductTimeWindows::append(const value_type& value) {
    // Stay allocation-free while the currently supported source domain fits
    // in the inline capacity.
    if (overflow_.empty() && size_ < inline_.size()) {
        inline_[size_++] = value;
        return;
    }

    // The first overflowing append migrates the complete inline prefix. Keeping
    // all elements in one vector is required because the public iterator type is
    // a raw pointer and therefore cannot span split storage.
    if (overflow_.empty()) {
        overflow_.reserve(inline_.size() * 2);
        overflow_.insert(overflow_.end(), inline_.begin(), inline_.begin() + size_);
    }

    overflow_.push_back(value);
    ++size_;
}

/**
 * @brief Return the base pointer of the active contiguous window storage.
 *
 * @return `inline_.data()` before overflow migration, otherwise
 *         `overflow_.data()`. After migration, the vector contains all windows,
 *         including the former inline prefix.
 */
const ProductTimeWindow* ProductTimeWindows::data() const noexcept {
    return overflow_.empty() ? inline_.data() : overflow_.data();
}

/**
 * @brief Construct the immutable canonical temporal representation.
 *
 * This constructor stores already-resolved components. It performs no local
 * consistency validation; the resolver is responsible for running final checks
 * before or around construction. Consequently, direct callers must provide a
 * coherent object satisfying all ProductTimeSpec invariants.
 *
 * @param anchor Resolved label, initial-conditions, and reference datetimes,
 *        together with the anchor classification.
 * @param windowStartDateTime Absolute beginning of the outermost support.
 * @param windowEndDateTime Absolute end of the outermost support.
 * @param windows Canonical records in outermost-to-innermost order. A valid
 *        final object contains at least one record; an instant product contains
 *        one zero-length placeholder record.
 * @param options Snapshot of the policy values used during resolution.
 * @param kind Final temporal shape classification.
 * @param incrementKind Final innermost increment classification.
 *
 * @post All arguments are owned by the new object. Value-like arguments are
 *       moved into their corresponding members.
 */
ProductTimeSpec::ProductTimeSpec(ProductTimeSpecAnchor anchor,
                                 eckit::DateTime windowStartDateTime,
                                 eckit::DateTime windowEndDateTime,
                                 ProductTimeWindows windows,
                                 ProductTimeSpecOptions options,
                                 ProductTimeSpecKind kind,
                                 TimeIncrementKind incrementKind) :
    anchor_(std::move(anchor)),
    windowStartDateTime_(std::move(windowStartDateTime)),
    windowEndDateTime_(std::move(windowEndDateTime)),
    windows_(std::move(windows)),
    options_(std::move(options)),
    kind_(kind),
    incrementKind_(incrementKind) {}

/**
 * @brief Return the product label DateTime.
 *
 * The label is the externally identifying temporal coordinate. Depending on
 * the anchor regime, initial conditions and reference time may be inherited
 * from it or constructed from more specific sources.
 *
 * @return Const reference to `anchor().labelDateTime`.
 */
const eckit::DateTime& ProductTimeSpec::labelDateTime() const noexcept {
    return anchor_.labelDateTime;
}

/**
 * @brief Return the simulation initial-conditions DateTime.
 *
 * Valid resolved anchors satisfy
 * `labelDateTime() <= initialConditionsDateTime() <= referenceDateTime()`.
 *
 * @return Const reference to `anchor().initialConditionsDateTime`.
 */
const eckit::DateTime& ProductTimeSpec::initialConditionsDateTime() const noexcept {
    return anchor_.initialConditionsDateTime;
}

/**
 * @brief Return the reference DateTime used for forecast-step arithmetic.
 *
 * Point-in-time and statistical backends derive relative temporal quantities
 * from this resolved anchor instead of reinterpreting raw MARS keys.
 *
 * @return Const reference to `anchor().referenceDateTime`.
 */
const eckit::DateTime& ProductTimeSpec::referenceDateTime() const noexcept {
    return anchor_.referenceDateTime;
}

/**
 * @brief Return the source regime used to construct the anchor datetimes.
 *
 * @return `LabelOnly`, `Hindcast`, `ForecastAnchor`, or
 *         `HindcastForecastAnchor` for a valid resolved object.
 */
TimeAnchorKind ProductTimeSpec::anchorType() const noexcept {
    return anchor_.anchorType;
}

/**
 * @brief Return the complete resolved anchor artifact.
 *
 * @return Const reference containing all three anchor datetimes and their
 *         `TimeAnchorKind`.
 */
const ProductTimeSpecAnchor& ProductTimeSpec::anchor() const noexcept {
    return anchor_;
}

/**
 * @brief Return the absolute beginning of the product's outermost support.
 *
 * For instant products this equals `windowEndDateTime()`. For statistical
 * products it is consistent with subtracting the outermost canonical range from
 * the support end under the corresponding duration semantics.
 *
 * @return Const reference to the resolved support start.
 */
const eckit::DateTime& ProductTimeSpec::windowStartDateTime() const noexcept {
    return windowStartDateTime_;
}

/**
 * @brief Return the absolute end of the product's outermost support.
 *
 * Statistical and point-in-time lowering both use this resolved value. Backend
 * names such as `forecastTime` are deliberately not stored in the canonical IR.
 *
 * @return Const reference to the resolved support end.
 */
const eckit::DateTime& ProductTimeSpec::windowEndDateTime() const noexcept {
    return windowEndDateTime_;
}

/**
 * @brief Return the number of canonical GRIB time-range records.
 *
 * This is not the number of real statistical windows for instant products:
 * instant products return one because the canonical IR stores one normalization
 * placeholder. Use `realStatisticalWindowCount(*this)` when semantic real-window
 * cardinality is required.
 *
 * @return `timeRanges().numberOfTimeRanges()`.
 */
std::size_t ProductTimeSpec::numberOfTimeRanges() const noexcept {
    return windows_.numberOfTimeRanges();
}

/**
 * @brief Return the number of canonical ProductTimeWindow records.
 *
 * @return `timeRanges().size()`. This is equivalent to
 *         `numberOfTimeRanges()`.
 */
std::size_t ProductTimeSpec::size() const noexcept {
    return windows_.size();
}

/**
 * @brief Access a canonical time window without runtime bounds checking.
 *
 * @param i Zero-based index in outermost-to-innermost order.
 * @return Const reference to the selected canonical window.
 *
 * @pre `i < size()`.
 * @note The assertion and undefined-behavior contract is inherited from
 *       ProductTimeWindows::operator[].
 */
const ProductTimeWindow& ProductTimeSpec::operator[](std::size_t i) const noexcept {
    return windows_[i];
}

/**
 * @brief Access a canonical time window with bounds checking.
 *
 * @param i Zero-based index in outermost-to-innermost order.
 * @return Const reference to the selected canonical window.
 *
 * @throws std::out_of_range If `i >= size()`.
 */
const ProductTimeWindow& ProductTimeSpec::at(std::size_t i) const {
    return windows_.at(i);
}

/**
 * @brief Return the complete immutable canonical-window sequence.
 *
 * @return Const reference to windows ordered outermost to innermost. The
 *         sequence includes the instant-product placeholder when `kind()` is
 *         `Instant`.
 */
const ProductTimeWindows& ProductTimeSpec::timeRanges() const noexcept {
    return windows_;
}

/**
 * @brief Return an iterator to the outermost canonical time window.
 *
 * @return `timeRanges().begin()`.
 */
ProductTimeWindows::const_iterator ProductTimeSpec::begin() const noexcept {
    return windows_.begin();
}

/**
 * @brief Return the past-the-end canonical-window iterator.
 *
 * @return `timeRanges().end()`.
 */
ProductTimeWindows::const_iterator ProductTimeSpec::end() const noexcept {
    return windows_.end();
}

/**
 * @brief Return the policy snapshot used to resolve this specification.
 *
 * The snapshot is retained for diagnostics and reproducibility. It records
 * compatibility allowances and default values even when a particular option
 * was not exercised by this product.
 *
 * @return Const reference to the stored ProductTimeSpecOptions.
 */
const ProductTimeSpecOptions& ProductTimeSpec::options() const noexcept {
    return options_;
}

/**
 * @brief Return the final temporal shape of the product.
 *
 * @return The canonical shape classification: instant, standard single-loop,
 *         multi-loop, fake-double-loop single-loop, or from-start single-loop.
 */
ProductTimeSpecKind ProductTimeSpec::kind() const noexcept {
    return kind_;
}

/**
 * @brief Return the semantic origin and use of the innermost increment.
 *
 * @return `NoIncrement`, `ExplicitIncrement`, `DefaultedIncrement`, or
 *         `AifsPureMissingIncrement` for a valid resolved specification.
 */
TimeIncrementKind ProductTimeSpec::incrementKind() const noexcept {
    return incrementKind_;
}

/**
 * @brief Serialize the complete canonical ProductTimeSpec as compact JSON.
 *
 * The output contains the resolved anchor, support interval, shape and increment
 * classifications, canonical range array, and full option snapshot. Enumeration
 * values are emitted through the stable `name(...)` helpers. DateTimes use the
 * ISO representation produced by `eckit::DateTime::iso(true)`.
 *
 * The method is intended for diagnostics, exception context, logs, and tests.
 * It does not serialize raw resolver input or construction artifacts that are
 * not retained by the final canonical object.
 *
 * @return A complete compact JSON object with no trailing newline.
 *
 * @throws std::bad_alloc If string or stream allocation fails.
 * @throws Any exception propagated by DateTime ISO conversion or helper
 *         serialization.
 */
std::string ProductTimeSpec::to_json() const {
    std::ostringstream out;
    out << '{';

    out << jsonQuote("anchor") << ":{";
    out << jsonQuote("labelDateTime") << ':' << jsonQuote(labelDateTime().iso(true)) << ',';
    out << jsonQuote("initialConditionsDateTime") << ':'
        << jsonQuote(initialConditionsDateTime().iso(true)) << ',';
    out << jsonQuote("referenceDateTime") << ':' << jsonQuote(referenceDateTime().iso(true)) << ',';
    out << jsonQuote("anchorType") << ':' << jsonQuote(name(anchorType())) << "},";

    out << jsonQuote("windowStartDateTime") << ':'
        << jsonQuote(windowStartDateTime_.iso(true)) << ',';
    out << jsonQuote("windowEndDateTime") << ':'
        << jsonQuote(windowEndDateTime_.iso(true)) << ',';
    out << jsonQuote("kind") << ':' << jsonQuote(name(kind_)) << ',';
    out << jsonQuote("incrementKind") << ':' << jsonQuote(name(incrementKind_)) << ',';
    out << jsonQuote("numberOfTimeRanges") << ':' << numberOfTimeRanges() << ',';

    // Canonical records are emitted in their semantic order: outermost first,
    // innermost last.
    out << jsonQuote("timeRanges") << ":[";
    for (std::size_t i = 0; i < size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        const auto& window = (*this)[i];
        out << '{'
            << jsonQuote("typeOfStatisticalProcessing") << ':'
            << jsonQuote(name(window.typeOfStatisticalProcessing)) << ','
            << jsonQuote("typeOfTimeIncrement") << ':'
            << jsonQuote(name(window.typeOfTimeIncrement)) << ','
            << jsonQuote("timeRange") << ':' << durationJson(window.timeRange) << ','
            << jsonQuote("timeIncrement") << ':' << durationJson(window.timeIncrement)
            << '}';
    }
    out << "],";

    // Serialize the full policy snapshot, not only options exercised by this
    // particular product, so diagnostics remain reproducible.
    out << jsonQuote("options") << ":{";
    out << jsonQuote("allowDefaultTimeIncrementInSeconds") << ':'
        << (options_.allowDefaultTimeIncrementInSeconds ? "true" : "false") << ',';
    out << jsonQuote("allowZeroLengthFsWindow") << ':'
        << (options_.allowZeroLengthFsWindow ? "true" : "false") << ',';
    out << jsonQuote("allowNonEnumeratedPositiveIntegerTimespanHours") << ':'
        << (options_.allowNonEnumeratedPositiveIntegerTimespanHours ? "true" : "false") << ',';
    out << jsonQuote("allowRedundantTimeIncrement") << ':'
        << (options_.allowRedundantTimeIncrement ? "true" : "false") << ',';
    out << jsonQuote("allowMissingTimespanForInstantProduct") << ':'
        << (options_.allowMissingTimespanForInstantProduct ? "true" : "false") << ',';
    out << "}}";

    return out.str();
}

/**
 * @brief Construct a stage-aware ProductTimeSpec diagnostic exception.
 *
 * The human-readable base message is prefixed with the resolver stage. Separate
 * JSON payloads retain the best available state at failure time. Payloads that
 * do not yet exist at the failing stage are represented by empty strings.
 *
 * Because the class also inherits `std::nested_exception`, construction captures
 * the currently handled exception when invoked from a catch block, preserving
 * the lower-level cause for later diagnostic traversal.
 *
 * @param stage Resolver stage at which the failure was detected.
 * @param reason Human-readable failure reason without the standard prefix.
 * @param inputJson Normalized resolver-input snapshot, when available.
 * @param classificationJson Three-axis classification snapshot, when available.
 * @param artifactJson Construction-artifact snapshot, when available.
 * @param finalSpecJson Canonical ProductTimeSpec snapshot, when available.
 * @param loc Source code location associated with the failure.
 */
Mars2GribProductTimeSpecException::Mars2GribProductTimeSpecException(
    ProductTimeSpecStage stage,
    std::string reason,
    std::string inputJson,
    std::string classificationJson,
    std::string artifactJson,
    std::string finalSpecJson,
    const eckit::CodeLocation& loc) :
    eckit::Exception("ProductTimeSpec[" + name(stage) + "]: " + std::move(reason), loc),
    stage_(stage),
    inputJson_(std::move(inputJson)),
    classificationJson_(std::move(classificationJson)),
    artifactJson_(std::move(artifactJson)),
    finalSpecJson_(std::move(finalSpecJson)) {}

/**
 * @brief Return the resolver stage associated with this failure.
 *
 * @return Stage supplied to the exception constructor.
 */
ProductTimeSpecStage Mars2GribProductTimeSpecException::stage() const noexcept {
    return stage_;
}

/**
 * @brief Return the normalized resolver-input diagnostic snapshot.
 *
 * @return Const reference to the stored JSON string. The string is empty when
 *         no complete input snapshot was available at the failure point.
 */
const std::string& Mars2GribProductTimeSpecException::inputJson() const noexcept {
    return inputJson_;
}

/**
 * @brief Return the classification diagnostic snapshot.
 *
 * @return Const reference to the stored JSON string. The string is empty when
 *         classification had not completed or was not applicable.
 */
const std::string& Mars2GribProductTimeSpecException::classificationJson() const noexcept {
    return classificationJson_;
}

/**
 * @brief Return the construction-artifact diagnostic snapshot.
 *
 * @return Const reference to the stored JSON string. The string is empty when
 *         no artifact snapshot existed at the failure stage.
 */
const std::string& Mars2GribProductTimeSpecException::artifactJson() const noexcept {
    return artifactJson_;
}

/**
 * @brief Return the final canonical-spec diagnostic snapshot.
 *
 * @return Const reference to the stored JSON string. The string is empty when
 *         canonical construction had not completed.
 */
const std::string& Mars2GribProductTimeSpecException::finalSpecJson() const noexcept {
    return finalSpecJson_;
}

/**
 * @brief Encode an arbitrary byte string as a JSON string literal.
 *
 * The function surrounds the result with double quotes, escapes JSON syntax and
 * standard control characters, and emits other bytes below `0x20` as four-digit
 * `\\u00xx` escapes. Bytes at or above `0x20` that are not quote or backslash
 * are copied unchanged; the function does not validate or transcode UTF-8.
 *
 * @param value Unquoted input string.
 * @return Complete JSON string literal including surrounding quotes.
 */
std::string jsonQuote(const std::string& value) {
    std::ostringstream out;
    out << '"';
    // Iterate as unsigned bytes so control-character comparisons are not
    // affected by implementation-defined signed-char behavior.
    for (const unsigned char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c) << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
    return out.str();
}

/**
 * @brief Return the stable diagnostic name of a TimespanKind value.
 *
 * @param value Normalized `timespan` category.
 * @return One of `Missing`, `Duration`, `None`, or `FromStart`. An invalid
 *         underlying enum value produces `InvalidTimespanKind` rather than
 *         throwing.
 */
std::string name(TimespanKind value) {
    switch (value) {
        case TimespanKind::Missing: return "Missing";
        case TimespanKind::Duration: return "Duration";
        case TimespanKind::None: return "None";
        case TimespanKind::FromStart: return "FromStart";
    }
    return "InvalidTimespanKind";
}

/**
 * @brief Return the stable diagnostic name of a TimeAnchorKind value.
 *
 * @param value Anchor-source classification.
 * @return The enumerator name, `Count`, or `InvalidTimeAnchorKind` for an
 *         out-of-domain underlying value.
 */
std::string name(TimeAnchorKind value) {
    switch (value) {
        case TimeAnchorKind::LabelOnly: return "LabelOnly";
        case TimeAnchorKind::Hindcast: return "Hindcast";
        case TimeAnchorKind::ForecastAnchor: return "ForecastAnchor";
        case TimeAnchorKind::HindcastForecastAnchor: return "HindcastForecastAnchor";
        case TimeAnchorKind::Count: return "Count";
    }
    return "InvalidTimeAnchorKind";
}

/**
 * @brief Return the stable diagnostic name of a temporal shape.
 *
 * @param value Shape classification to name.
 * @return The enumerator name, `Count`, or
 *         `InvalidProductTimeSpecShapeKind` for an invalid underlying value.
 */
std::string name(ProductTimeSpecShapeKind value) {
    switch (value) {
        case ProductTimeSpecShapeKind::Instant: return "Instant";
        case ProductTimeSpecShapeKind::StandardSingleLoop: return "StandardSingleLoop";
        case ProductTimeSpecShapeKind::MultiLoop: return "MultiLoop";
        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop: return "FakeDoubleLoopSingleLoop";
        case ProductTimeSpecShapeKind::FromStartSingleLoop: return "FromStartSingleLoop";
        case ProductTimeSpecShapeKind::Count: return "Count";
    }
    return "InvalidProductTimeSpecShapeKind";
}

/**
 * @brief Return the stable diagnostic name of an increment classification.
 *
 * @param value Increment classification to name.
 * @return The enumerator name, `Count`, or `InvalidTimeIncrementKind` for an
 *         invalid underlying value.
 */
std::string name(TimeIncrementKind value) {
    switch (value) {
        case TimeIncrementKind::NoIncrement: return "NoIncrement";
        case TimeIncrementKind::ExplicitIncrement: return "ExplicitIncrement";
        case TimeIncrementKind::DefaultedIncrement: return "DefaultedIncrement";
        case TimeIncrementKind::AifsPureMissingIncrement: return "AifsPureMissingIncrement";
        case TimeIncrementKind::Count: return "Count";
    }
    return "InvalidTimeIncrementKind";
}

/**
 * @brief Return the stable diagnostic name of a resolver stage.
 *
 * @param value Resolver stage to name.
 * @return The exact stage name, or `InvalidProductTimeSpecStage` for an invalid
 *         underlying enum value.
 */
std::string name(ProductTimeSpecStage value) {
    switch (value) {
        case ProductTimeSpecStage::InputExtraction: return "InputExtraction";
        case ProductTimeSpecStage::TimeAnchorClassification: return "TimeAnchorClassification";
        case ProductTimeSpecStage::ShapeClassification: return "ShapeClassification";
        case ProductTimeSpecStage::TimeIncrementClassification: return "TimeIncrementClassification";
        case ProductTimeSpecStage::ClassificationConsistencyCheck: return "ClassificationConsistencyCheck";
        case ProductTimeSpecStage::TimeAnchorConstruction: return "TimeAnchorConstruction";
        case ProductTimeSpecStage::ShapeConstruction: return "ShapeConstruction";
        case ProductTimeSpecStage::TimeIncrementConstruction: return "TimeIncrementConstruction";
        case ProductTimeSpecStage::CanonicalWindowConstruction: return "CanonicalWindowConstruction";
        case ProductTimeSpecStage::ProductTimeSpecConstruction: return "ProductTimeSpecConstruction";
        case ProductTimeSpecStage::FinalConsistencyCheck: return "FinalConsistencyCheck";
    }
    return "InvalidProductTimeSpecStage";
}

/**
 * @brief Return the stable diagnostic name of a GRIB time unit.
 *
 * This function names both units currently accepted by ProductTimeSpec
 * arithmetic and additional values exposed by the underlying GRIB table type.
 * Naming a value does not imply that `addDuration` or `subtractDuration`
 * supports arithmetic for that unit.
 *
 * @param value GRIB time-unit value.
 * @return Enumerator name, or `InvalidTimeUnit` for an invalid underlying value.
 */
std::string name(tables::TimeUnit value) {
    switch (value) {
        case tables::TimeUnit::Minute: return "Minute";
        case tables::TimeUnit::Hour: return "Hour";
        case tables::TimeUnit::Day: return "Day";
        case tables::TimeUnit::Month: return "Month";
        case tables::TimeUnit::Year: return "Year";
        case tables::TimeUnit::Decade: return "Decade";
        case tables::TimeUnit::Normal: return "Normal";
        case tables::TimeUnit::Century: return "Century";
        case tables::TimeUnit::Hours3: return "Hours3";
        case tables::TimeUnit::Hours6: return "Hours6";
        case tables::TimeUnit::Hours12: return "Hours12";
        case tables::TimeUnit::Second: return "Second";
        case tables::TimeUnit::Missing: return "Missing";
    }
    return "InvalidTimeUnit";
}

/**
 * @brief Return the stable diagnostic name of a GRIB statistical operation.
 *
 * @param value GRIB type-of-statistical-processing value.
 * @return Enumerator name, including `Missing`, or
 *         `InvalidTypeOfStatisticalProcessing` for an invalid underlying value.
 */
std::string name(tables::TypeOfStatisticalProcessing value) {
    switch (value) {
        case tables::TypeOfStatisticalProcessing::Average: return "Average";
        case tables::TypeOfStatisticalProcessing::Accumulation: return "Accumulation";
        case tables::TypeOfStatisticalProcessing::Maximum: return "Maximum";
        case tables::TypeOfStatisticalProcessing::Minimum: return "Minimum";
        case tables::TypeOfStatisticalProcessing::DifferenceEndMinusStart: return "DifferenceEndMinusStart";
        case tables::TypeOfStatisticalProcessing::RootMeanSquare: return "RootMeanSquare";
        case tables::TypeOfStatisticalProcessing::StandardDeviation: return "StandardDeviation";
        case tables::TypeOfStatisticalProcessing::Covariance: return "Covariance";
        case tables::TypeOfStatisticalProcessing::DifferenceStartMinusEnd: return "DifferenceStartMinusEnd";
        case tables::TypeOfStatisticalProcessing::Ratio: return "Ratio";
        case tables::TypeOfStatisticalProcessing::StandardizedAnomaly: return "StandardizedAnomaly";
        case tables::TypeOfStatisticalProcessing::Summation: return "Summation";
        case tables::TypeOfStatisticalProcessing::ReturnPeriod: return "ReturnPeriod";
        case tables::TypeOfStatisticalProcessing::Median: return "Median";
        case tables::TypeOfStatisticalProcessing::Severity: return "Severity";
        case tables::TypeOfStatisticalProcessing::Mode: return "Mode";
        case tables::TypeOfStatisticalProcessing::IndexProcessing: return "IndexProcessing";
        case tables::TypeOfStatisticalProcessing::Missing: return "Missing";
    }
    return "InvalidTypeOfStatisticalProcessing";
}

/**
 * @brief Return the stable diagnostic name of a GRIB type-of-time-increment.
 *
 * @param value GRIB table 4.11 value represented by TypeOfTimeIncrement.
 * @return Enumerator name, including `Reserved` and `Missing`, or
 *         `InvalidTypeOfTimeIncrement` for an invalid underlying value.
 */
std::string name(TypeOfTimeIncrement value) {
    switch (value) {
        case TypeOfTimeIncrement::Reserved: return "Reserved";
        case TypeOfTimeIncrement::SameForecastTimeStartIncremented:
            return "SameForecastTimeStartIncremented";
        case TypeOfTimeIncrement::SameStartTimeForecastIncremented:
            return "SameStartTimeForecastIncremented";
        case TypeOfTimeIncrement::StartIncrementedForecastDecrementedConstantValid:
            return "StartIncrementedForecastDecrementedConstantValid";
        case TypeOfTimeIncrement::StartDecrementedForecastIncrementedConstantValid:
            return "StartDecrementedForecastIncrementedConstantValid";
        case TypeOfTimeIncrement::FloatingSubinterval: return "FloatingSubinterval";
        case TypeOfTimeIncrement::Missing: return "Missing";
    }
    return "InvalidTypeOfTimeIncrement";
}

/**
 * @brief Canonicalize a non-negative elapsed duration expressed in seconds.
 *
 * Positive whole-hour values are represented as hours to retain compact,
 * human-readable GRIB-compatible durations. Zero and non-hour-aligned values are
 * represented as seconds. This function handles elapsed durations only; it does
 * not infer calendar days or months from a second count.
 *
 * @param seconds Non-negative elapsed duration in seconds.
 * @return `{Hour, seconds / 3600}` for positive whole hours; otherwise
 *         `{Second, seconds}`.
 *
 * @throws std::invalid_argument If `seconds` is negative.
 */
ProductTimeDuration canonicalElapsedDuration(long seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("elapsed duration must be non-negative");
    }
    // Zero remains zero seconds. Positive whole-hour values use the more
    // compact hour representation; no conversion to calendar days is attempted.
    if (seconds > 0 && seconds % secondsPerHour == 0) {
        return ProductTimeDuration{tables::TimeUnit::Hour, seconds / secondsPerHour};
    }
    return ProductTimeDuration{tables::TimeUnit::Second, seconds};
}

/**
 * @brief Test whether a DateTime has a zero time-of-day component.
 *
 * @param value DateTime to inspect.
 * @return True when hour, minute, and second are all zero.
 */
bool isAtMidnight(const eckit::DateTime& value) {
    return value.time().hours() == 0 && value.time().minutes() == 0 &&
           value.time().seconds() == 0;
}

/**
 * @brief Test whether a DateTime is aligned for calendar-month arithmetic.
 *
 * @param value DateTime to inspect.
 * @return True when `value` is day one of its month at `00:00:00`.
 */
bool isOnFirstOfMonthMidnight(const eckit::DateTime& value) {
    return isAtMidnight(value) && value.date().day() == 1;
}

/**
 * @brief Add a non-negative ProductTimeDuration using unit-specific semantics.
 *
 * Supported units and semantics are:
 *
 * - `Second`: elapsed-second arithmetic;
 * - `Hour`: checked conversion to elapsed seconds;
 * - `Day`: calendar-day arithmetic, requiring a midnight start;
 * - `Month`: calendar-month arithmetic, requiring day one at midnight.
 *
 * Day and month results are rebuilt at `defaultMarsTime`. Units recognized by
 * the underlying GRIB table but not listed above are rejected.
 *
 * @param value Starting DateTime.
 * @param duration Non-negative unit-qualified duration.
 * @return DateTime shifted forward by `duration`.
 *
 * @throws std::invalid_argument If the length is negative, the unit is
 *         unsupported, or the starting DateTime violates calendar alignment.
 * @throws std::overflow_error If converting hours to seconds overflows `long`.
 */
eckit::DateTime addDuration(const eckit::DateTime& value,
                            const ProductTimeDuration& duration) {
    if (duration.length < 0) {
        throw std::invalid_argument("duration length must be non-negative");
    }

    switch (duration.unit) {
        case tables::TimeUnit::Second:
            return addSeconds(value, duration.length);
        case tables::TimeUnit::Hour:
            return addSeconds(value, checkedMultiply(duration.length, secondsPerHour,
                                                     "hour duration"));
        case tables::TimeUnit::Day: {
            if (!isAtMidnight(value)) {
                throw std::invalid_argument(
                    "calendar-day arithmetic requires a midnight DateTime");
            }
            eckit::Date date = value.date();
            date += duration.length;
            return eckit::DateTime(date, defaultMarsTime);
        }
        case tables::TimeUnit::Month:
            return shiftCalendarMonths(value, duration.length);
        default:
            throw std::invalid_argument("unsupported ProductTimeDuration unit: " +
                                        name(duration.unit));
    }
}

/**
 * @brief Subtract a non-negative ProductTimeDuration using unit-specific semantics.
 *
 * Supported units and semantics are:
 *
 * - `Second`: elapsed-second subtraction;
 * - `Hour`: checked conversion to elapsed seconds;
 * - `Day`: calendar-day subtraction, requiring a midnight start;
 * - `Month`: calendar-month subtraction, requiring day one at midnight.
 *
 * Day and month results are rebuilt at `defaultMarsTime`. Units recognized by
 * the underlying GRIB table but not listed above are rejected.
 *
 * @param value Starting DateTime.
 * @param duration Non-negative unit-qualified duration.
 * @return DateTime shifted backward by `duration`.
 *
 * @throws std::invalid_argument If the length is negative, the unit is
 *         unsupported, or the starting DateTime violates calendar alignment.
 * @throws std::overflow_error If converting hours to seconds overflows `long`.
 */
eckit::DateTime subtractDuration(const eckit::DateTime& value,
                                 const ProductTimeDuration& duration) {
    if (duration.length < 0) {
        throw std::invalid_argument("duration length must be non-negative");
    }

    switch (duration.unit) {
        case tables::TimeUnit::Second:
            return subtractSeconds(value, duration.length);
        case tables::TimeUnit::Hour:
            return subtractSeconds(value, checkedMultiply(duration.length, secondsPerHour,
                                                          "hour duration"));
        case tables::TimeUnit::Day: {
            if (!isAtMidnight(value)) {
                throw std::invalid_argument(
                    "calendar-day arithmetic requires a midnight DateTime");
            }
            eckit::Date date = value.date();
            date -= duration.length;
            return eckit::DateTime(date, defaultMarsTime);
        }
        case tables::TimeUnit::Month:
            return shiftCalendarMonths(value, -duration.length);
        default:
            throw std::invalid_argument("unsupported ProductTimeDuration unit: " +
                                        name(duration.unit));
    }
}

/**
 * @brief Test whether an increment is no longer than a range at a realized start.
 *
 * The comparison is placement-aware. Both durations are independently added to
 * the same DateTime and the resulting endpoints are compared. This is necessary
 * for calendar units because their elapsed lengths depend on the actual month or
 * date at which they are realized.
 *
 * @param realizedStart DateTime at which both durations are realized.
 * @param increment Candidate sampling increment.
 * @param range Statistical-window length.
 * @return True when `realizedStart + increment <= realizedStart + range`.
 *
 * @throws Any exception propagated by `addDuration`, including negative-length,
 *         unsupported-unit, alignment, and overflow failures.
 */
bool durationFitsAt(const eckit::DateTime& realizedStart,
                    const ProductTimeDuration& increment,
                    const ProductTimeDuration& range) {
    return addDuration(realizedStart, increment) <= addDuration(realizedStart, range);
}

/**
 * @brief Compute real statistical-window cardinality before canonicalization.
 *
 * The result is derived from the resolved shape and the number of parsed outer
 * `stattype` blocks:
 *
 * - instant: zero;
 * - each single-loop shape: one;
 * - multi-loop: `stattypeBlockCount + 1`, where the additional window is the
 *   innermost `timespan` window.
 *
 * @param shapeType Resolved temporal shape.
 * @param stattypeBlockCount Number of parsed outer `stattype` blocks. It is used
 *        only for `MultiLoop`.
 * @return Number of real statistical windows represented by the construction
 *         artifacts.
 *
 * @throws std::invalid_argument If `shapeType` is `Count` or has an invalid
 *         underlying value.
 */
std::size_t realStatisticalWindowCount(ProductTimeSpecShapeKind shapeType,
                                       std::size_t stattypeBlockCount) {
    switch (shapeType) {
        case ProductTimeSpecShapeKind::Instant: return 0;
        case ProductTimeSpecShapeKind::StandardSingleLoop: return 1;
        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop: return 1;
        case ProductTimeSpecShapeKind::FromStartSingleLoop: return 1;
        case ProductTimeSpecShapeKind::MultiLoop: return stattypeBlockCount + 1;
        case ProductTimeSpecShapeKind::Count: break;
    }
    throw std::invalid_argument("invalid ProductTimeSpecShapeKind");
}

/**
 * @brief Compute real statistical-window cardinality from the canonical IR.
 *
 * For every statistical shape, each canonical ProductTimeWindow is real and the
 * result equals `numberOfTimeRanges()`. For an instant product, the single
 * zero-length canonical placeholder is excluded and the result is zero.
 *
 * @param spec Final canonical ProductTimeSpec.
 * @return Zero for `Instant`; otherwise `spec.numberOfTimeRanges()`.
 */
std::size_t realStatisticalWindowCount(const ProductTimeSpec& spec) {
    return spec.kind() == ProductTimeSpecKind::Instant ? 0 : spec.numberOfTimeRanges();
}

/**
 * @brief Test whether a MARS class/stream pair requires fake-double-loop syntax.
 *
 * This is a source-representation policy predicate. A true result means that
 * eligible single-loop statistics for the pair must use the
 * `timespan=none` plus one-`stattype` fake-double-loop representation. It does
 * not itself classify a product or validate the remaining temporal keys.
 *
 * The current required pairs are:
 *
 * - class `e6`: streams `sttd`, `stte`;
 * - classes `od`, `rd`, `c3`: streams `sfmd`, `shmd`;
 * - classes `gh`, `eh`: streams `msmm`, `rfsd`.
 *
 * @param marsClass Normalized MARS `class` value.
 * @param marsStream Normalized MARS `stream` value.
 * @return True exactly for one of the listed class/stream combinations.
 */
bool requiresFakeDoubleLoopRepresentation(const std::string& marsClass,
                                          const std::string& marsStream) {
    if (marsClass == "e6") {
        return marsStream == "sttd" || marsStream == "stte";
    }
    if (marsClass == "od" || marsClass == "rd" || marsClass == "c3") {
        return marsStream == "sfmd" || marsStream == "shmd";
    }
    if (marsClass == "gh" || marsClass == "eh") {
        return marsStream == "msmm" || marsStream == "rfsd";
    }
    return false;
}

}  // namespace metkit::mars2grib::product_time_spec